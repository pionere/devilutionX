/**
 * @file player.cpp
 *
 * Implementation of player functionality, leveling, actions, creation, loading, etc.
 */
#include "all.h"
#include "plrctrls.h"

DEVILUTION_BEGIN_NAMESPACE

int myplr;
PlayerStruct plr[MAX_PLRS];
bool gbDeathflag;
int deathdelay;
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
const char ArmourChar[4] = { 'L', 'M', 'H', 0 };
/** Maps from weapon animation to letter used in graphic files. */
const char WepChar[10] = { 'N', 'U', 'S', 'D', 'B', 'A', 'M', 'H', 'T', 0 };
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

/* data */

/** Specifies the X-coordinate delta from the player start location in Tristram. */
const int plrxoff[MAX_PLRS] = { 0, 2, 0, 2 }; //, 1, 0, 1, 2, 1 };
/** Specifies the Y-coordinate delta from the player start location in Tristram. */
const int plryoff[MAX_PLRS] = { 0, 2, 2, 0 }; //, 1, 1, 0, 1, 2 };
/** Specifies the X-coordinate delta from a player, used for instanced when casting resurrect. */
const int plrxoff2[9] = { 0, 1, 0, 1, 2, 0, 1, 2, 2 };
/** Specifies the Y-coordinate delta from a player, used for instanced when casting resurrect. */
const int plryoff2[9] = { 0, 0, 1, 1, 0, 2, 2, 1, 2 };
/** Specifies the frame of each animation for which an action is triggered, for each player class. */
const BYTE PlrGFXAnimLens[NUM_CLASSES][11] = {
	{ 10, 16, 8, 2, 20, 20, 6, 20, 8, 9, 14 },
	{ 8, 18, 8, 4, 20, 16, 7, 20, 8, 10, 12 },
	{ 8, 16, 8, 6, 20, 12, 8, 20, 8, 12, 8 },
#ifdef HELLFIRE
	{ 8, 16, 8, 3, 20, 18, 6, 20, 8, 12, 13 },
	{ 8, 18, 8, 4, 20, 16, 7, 20, 8, 10, 12 },
	{ 10, 16, 8, 2, 20, 20, 6, 20, 8, 9, 14 },
#endif
};
/** Maps from player class to player velocity. */
const int PWVel[NUM_CLASSES][3] = {
	{ 2048, 1024, 512 },
	{ 2048, 1024, 512 },
	{ 2048, 1024, 512 },
#ifdef HELLFIRE
	{ 2048, 1024, 512 },
	{ 2048, 1024, 512 },
	{ 2048, 1024, 512 },
#endif
};
/** Total number of frames in walk animation. */
/*const int AnimLenFromClass[NUM_CLASSES] = {
	8,
	8,
	8,
#ifdef HELLFIRE
	8,
	8,
	8,
#endif
};*/
/** Maps from player_class to starting stat in strength. */
const int StrengthTbl[NUM_CLASSES] = {
	20,
	15,
	10,
#ifdef HELLFIRE
	20,
	15,
	35,
#endif
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
	20,
	25,
	20,
#ifdef HELLFIRE
	20,
	25,
	10,
#endif
};
/** Maps from player_class to starting stat in vitality. */
const int VitalityTbl[NUM_CLASSES] = {
	30,
	20,
	20,
#ifdef HELLFIRE
	25,
	20,
	35,
#endif
};
const int Abilities[NUM_CLASSES] = {
	SPL_REPAIR, SPL_DISARM, SPL_RECHARGE
#ifdef HELLFIRE
	, SPL_WHITTLE, SPL_IDENTIFY, SPL_BLODBOIL
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

static void SetPlayerGPtrs(BYTE *pData, BYTE *(&pAnim)[8])
{
	int i;

	for (i = 0; i < lengthof(pAnim); i++) {
		pAnim[i] = CelGetFrameStart(pData, i);
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
	char pszName[256];
	const char *szCel, *cs;
	PlayerStruct *p;
	BYTE *pData;
	BYTE *(*pAnim)[8];
	unsigned i, mask;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("LoadPlrGFX: illegal player %d", pnum);
	}

	p = &plr[pnum];
	GetPlrGFXCells(p->_pClass, &szCel, &cs);
	snprintf(prefix, sizeof(prefix), "%c%c%c", *szCel, ArmourChar[p->_pgfxnum >> 4], WepChar[p->_pgfxnum & 0xF]);

	for (i = 0, mask = gfxflag; i < NUM_PFIDXs; i++, mask >>= 1) {
		if (!(mask & 1))
			continue;

		switch (i) {
		case PFIDX_STAND:
			szCel = currLvl._dType != DTYPE_TOWN ? "AS" : "ST";
			pData = p->_pNData;
			pAnim = &p->_pNAnim;
			break;
		case PFIDX_WALK:
			szCel = currLvl._dType != DTYPE_TOWN ? "AW" : "WL";
			pData = p->_pWData;
			pAnim = &p->_pWAnim;
			break;
		case PFIDX_ATTACK:
			szCel = "AT";
			pData = p->_pAData;
			pAnim = &p->_pAAnim;
			break;
		case PFIDX_HIT:
			szCel = "HT";
			pData = p->_pHData;
			pAnim = &p->_pHAnim;
			break;
		case PFIDX_LIGHTNING:
			szCel = "LM";
			pData = p->_pLData;
			pAnim = &p->_pLAnim;
			break;
		case PFIDX_FIRE:
			szCel = "FM";
			pData = p->_pFData;
			pAnim = &p->_pFAnim;
			break;
		case PFIDX_MAGIC:
			szCel = "QM";
			pData = p->_pTData;
			pAnim = &p->_pTAnim;
			break;
		case PFIDX_DEATH:
			assert((p->_pgfxnum & 0xF) == ANIM_ID_UNARMED);
			szCel = "DT";
			pData = p->_pDData;
			pAnim = &p->_pDAnim;
			break;
		case PFIDX_BLOCK:
			szCel = "BL";
			pData = p->_pBData;
			pAnim = &p->_pBAnim;
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}

		snprintf(pszName, sizeof(pszName), "PlrGFX\\%s\\%s\\%s%s.CL2", cs, prefix, prefix, szCel);
		LoadFileWithMem(pszName, pData);
		SetPlayerGPtrs(pData, *pAnim);
		p->_pGFXLoad |= 1 << i;
	}
}

void InitPlayerGFX(int pnum)
{
	unsigned gfxflag;
	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("InitPlayerGFX: illegal player %d", pnum);
	}

	if (plr[pnum]._pHitPoints < (1 << 6)) {
		plr[pnum]._pgfxnum = ANIM_ID_UNARMED;
		gfxflag = PFILE_DEATH;
	} else {
		gfxflag = PFILE_NONDEATH;
		if (currLvl._dType == DTYPE_TOWN)
			gfxflag &= ~(PFILE_ATTACK | PFILE_HIT | PFILE_BLOCK);
		else if (!(plr[pnum]._pSkillFlags & SFLAG_BLOCK))
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
	char pszName[256];
	char Type[16];

	dwMaxSize = 0;

	for (c = 0; c < NUM_CLASSES; c++) {
		GetPlrGFXCells(c, &cc, &cst);
		for (a = &ArmourChar[0]; *a; a++) {
			for (w = &WepChar[0]; *w; w++) { // BUGFIX loads non-existing animations; DT is only for N, BT is only for U, D & H (fixed)
				if (szCel[0] == 'D' && szCel[1] == 'T' && *w != 'N') {
					continue; //Death has no weapon
				}
				if (szCel[0] == 'B' && szCel[1] == 'L' && (*w != 'U' && *w != 'D' && *w != 'H')) {
					continue; //No block without weapon
				}
				snprintf(Type, sizeof(Type), "%c%c%c", *cc, *a, *w);
				snprintf(pszName, sizeof(pszName), "PlrGFX\\%s\\%s\\%s%s.CL2", cst, Type, Type, szCel);
				if (SFileOpenFile(pszName, &hsFile)) {
					/// ASSERT: assert(hsFile != NULL);
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
	PlayerStruct *p;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("InitPlrGFXMem: illegal player %d", pnum);
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

	p = &plr[pnum];
	p->_pNData = DiabloAllocPtr(plr_sframe_size);
	p->_pWData = DiabloAllocPtr(plr_wframe_size);
	p->_pAData = DiabloAllocPtr(plr_aframe_size);
	p->_pHData = DiabloAllocPtr(plr_hframe_size);
	p->_pLData = DiabloAllocPtr(plr_lframe_size);
	p->_pFData = DiabloAllocPtr(plr_fframe_size);
	p->_pTData = DiabloAllocPtr(plr_qframe_size);
	p->_pDData = DiabloAllocPtr(plr_dframe_size);
	p->_pBData = DiabloAllocPtr(plr_bframe_size);

	p->_pGFXLoad = 0;
}

void FreePlayerGFX(int pnum)
{
	PlayerStruct *p;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("FreePlayerGFX: illegal player %d", pnum);
	}

	p = &plr[pnum];
	MemFreeDbg(p->_pNData);
	MemFreeDbg(p->_pWData);
	MemFreeDbg(p->_pAData);
	MemFreeDbg(p->_pHData);
	MemFreeDbg(p->_pLData);
	MemFreeDbg(p->_pFData);
	MemFreeDbg(p->_pTData);
	MemFreeDbg(p->_pDData);
	MemFreeDbg(p->_pBData);
	p->_pGFXLoad = 0;
}

/**
 * @brief Sets the new Player Animation with all relevant information for rendering

 * @param pnum Player Id
 * @param anims Pointer to Animation Data
 * @param dir the direction of the player
 * @param numFrames Number of Frames in Animation
 * @param Delay Delay after each Animation sequence
 * @param width Width of sprite
 * @param numSkippedFrames Number of Frames that will be skipped (for example with modifier "faster attack")
 * @param processAnimationPending true if first ProcessAnimation will be called in same gametick after NewPlrAnim
 * @param stopDistributingAfterFrame Distribute the NumSkippedFrames only before this frame
*/
void NewPlrAnim(int pnum, BYTE **anims, int dir, unsigned numFrames, int Delay, int width) //, int numSkippedFrames /*= 0*/, bool processAnimationPending /*= false*/, int stopDistributingAfterFrame /*= 0*/)
{
	PlayerStruct *p;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("NewPlrAnim: illegal player %d", pnum);
	}
	p = &plr[pnum];
	p->_pdir = dir;
	p->_pAnimData = anims[dir];
	p->_pAnimLen = numFrames;
	p->_pAnimFrame = 1;
	p->_pAnimCnt = 0;
	p->_pAnimDelay = Delay;
	p->_pAnimWidth = width;
	p->_pAnimXOffset = (width - 64) >> 1;
	//p->_pAnimNumSkippedFrames = numSkippedFrames; ANIM_GAMELOGIC
	//p->_pAnimGameTicksSinceSequenceStarted = processAnimationPending ? -1 : 0;
	//p->_pAnimStopDistributingAfterFrame = stopDistributingAfterFrame;
}

static void ClearPlrPVars(int pnum)
{
	/*PlayerStruct *p;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("ClearPlrPVars: illegal player %d", pnum);
	}
	p = &plr[pnum];
	p->_pVar1 = 0;
	p->_pVar2 = 0;
	p->_pVar3 = 0;
	p->_pVar4 = 0;
	p->_pVar5 = 0;
	p->_pVar6 = 0;
	p->_pVar7 = 0;
	p->_pVar8 = 0;*/
}

void SetPlrAnims(int pnum)
{
	PlayerStruct *p;
	int pc, gn;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("SetPlrAnims: illegal player %d", pnum);
	}
	p = &plr[pnum];
	p->_pNWidth = 96;
	p->_pWWidth = 96;
	p->_pAWidth = 128;
	p->_pHWidth = 96;
	p->_pSWidth = 96;
	p->_pDWidth = 128;
	p->_pBWidth = 96;

	pc = p->_pClass;

	if (currLvl._dType == DTYPE_TOWN) {
		p->_pNFrames = PlrGFXAnimLens[pc][7];
		p->_pWFrames = PlrGFXAnimLens[pc][8];
		p->_pDFrames = PlrGFXAnimLens[pc][4];
		p->_pSFrames = PlrGFXAnimLens[pc][5];
	} else {
		p->_pNFrames = PlrGFXAnimLens[pc][0];
		p->_pWFrames = PlrGFXAnimLens[pc][2];
		p->_pAFrames = PlrGFXAnimLens[pc][1];
		p->_pHFrames = PlrGFXAnimLens[pc][6];
		p->_pSFrames = PlrGFXAnimLens[pc][5];
		p->_pDFrames = PlrGFXAnimLens[pc][4];
		p->_pBFrames = PlrGFXAnimLens[pc][3];
		p->_pAFNum = PlrGFXAnimLens[pc][9];
	}
	p->_pSFNum = PlrGFXAnimLens[pc][10];

	gn = p->_pgfxnum & 0xF;
	if (pc == PC_WARRIOR) {
		if (gn == ANIM_ID_BOW) {
			if (currLvl._dType != DTYPE_TOWN) {
				p->_pNFrames = 8;
			}
			p->_pAWidth = 96;
			p->_pAFNum = 11;
		} else if (gn == ANIM_ID_AXE) {
			p->_pAFrames = 20;
			p->_pAFNum = 10;
		} else if (gn == ANIM_ID_STAFF) {
			// p->_pAFrames = 16;
			p->_pAFNum = 11;
		}
	} else if (pc == PC_ROGUE) {
		if (gn == ANIM_ID_AXE) {
			p->_pAFrames = 22;
			p->_pAFNum = 13;
		} else if (gn == ANIM_ID_BOW) {
			p->_pAFrames = 12;
			p->_pAFNum = 7;
		} else if (gn == ANIM_ID_STAFF) {
			p->_pAFrames = 16;
			p->_pAFNum = 11;
		}
	} else if (pc == PC_SORCERER) {
		p->_pSWidth = 128;
		if (gn == ANIM_ID_UNARMED) {
			p->_pAFrames = 20;
		} else if (gn == ANIM_ID_UNARMED_SHIELD) {
			p->_pAFNum = 9;
		} else if (gn == ANIM_ID_BOW) {
			p->_pAFrames = 20;
			p->_pAFNum = 16;
		} else if (gn == ANIM_ID_AXE) {
			p->_pAFrames = 24;
			p->_pAFNum = 16;
		}
#ifdef HELLFIRE
	} else if (pc == PC_MONK) {
		p->_pNWidth = 112;
		p->_pWWidth = 112;
		p->_pAWidth = 130;
		p->_pHWidth = 98;
		p->_pSWidth = 114;
		p->_pDWidth = 160;
		p->_pBWidth = 98;

		switch (gn) {
		case ANIM_ID_UNARMED:
		case ANIM_ID_UNARMED_SHIELD:
			p->_pAFrames = 12;
			p->_pAFNum = 7;
			break;
		case ANIM_ID_BOW:
			p->_pAFrames = 20;
			p->_pAFNum = 14;
			break;
		case ANIM_ID_AXE:
			p->_pAFrames = 23;
			p->_pAFNum = 14;
			break;
		case ANIM_ID_STAFF:
			p->_pAFrames = 13;
			p->_pAFNum = 8;
			break;
		}
	} else if (pc == PC_BARD) {
		if (gn == ANIM_ID_AXE) {
			p->_pAFrames = 22;
			p->_pAFNum = 13;
		} else if (gn == ANIM_ID_BOW) {
			p->_pAFrames = 12;
			p->_pAFNum = 11;
		} else if (gn == ANIM_ID_STAFF) {
			p->_pAFrames = 16;
			p->_pAFNum = 11;
		} else if (gn == ANIM_ID_SWORD_SHIELD || gn == ANIM_ID_SWORD) {
			p->_pAFrames = 10;
		}
	} else if (pc == PC_BARBARIAN) {
		if (gn == ANIM_ID_AXE) {
			p->_pAFrames = 20;
			p->_pAFNum = 8;
		} else if (gn == ANIM_ID_BOW) {
			if (currLvl._dType != DTYPE_TOWN) {
				p->_pNFrames = 8;
			}
			p->_pAWidth = 96;
			p->_pAFNum = 11;
		} else if (gn == ANIM_ID_STAFF) {
			//p->_pAFrames = 16;
			p->_pAFNum = 11;
		} else if (gn == ANIM_ID_MACE || gn == ANIM_ID_MACE_SHIELD) {
			p->_pAFNum = 8;
		}
#endif
	}
}

/**
 * @param c plr_classes value
 */
void CreatePlayer(int pnum, BYTE c)
{
	PlayerStruct *p;
	int val, hp, mana;
	int i;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("CreatePlayer: illegal player %d", pnum);
	}

	p = &plr[pnum];
	memset(p, 0, sizeof(*p));
	//SetRndSeed(SDL_GetTicks());

	p->_pClass = c;

	val = StrengthTbl[c];
	p->_pStrength = val;
	p->_pBaseStr = val;

	val = DexterityTbl[c];
	p->_pDexterity = val;
	p->_pBaseDex = val;

	val = VitalityTbl[c];
	p->_pVitality = val;
	p->_pBaseVit = val;

	hp = val << (6 + 1);
	p->_pHitPoints = p->_pMaxHP = p->_pHPBase = p->_pMaxHPBase = hp;

	val = MagicTbl[c];
	p->_pMagic = val;
	p->_pBaseMag = val;

	mana = val << (6 + 1);
	p->_pMana = p->_pMaxMana = p->_pManaBase = p->_pMaxManaBase = mana;

	p->_pLevel = 1;
	p->_pLvlUp = false; // indicator whether the stat button should be shown
	//p->_pNextExper = PlrExpLvlsTbl[1];
	p->_pLightRad = 10;

	//p->_pAblSkills = SPELL_MASK(Abilities[c]);
	//p->_pAblSkills |= SPELL_MASK(SPL_WALK) | SPELL_MASK(SPL_ATTACK) | SPELL_MASK(SPL_RATTACK) | SPELL_MASK(SPL_BLOCK);

	//p->_pAtkSkill = SPL_ATTACK;
	//p->_pAtkSkillType = RSPLTYPE_ABILITY;
	//p->_pMoveSkill = SPL_WALK;
	//p->_pMoveSkillType = RSPLTYPE_ABILITY;
	//p->_pAltAtkSkill = SPL_INVALID;
	//p->_pAltAtkSkillType = RSPLTYPE_INVALID;
	//p->_pAltMoveSkill = SPL_INVALID;
	//p->_pAltMoveSkillType = RSPLTYPE_INVALID;

	if (c == PC_SORCERER) {
		p->_pSkillLvl[SPL_FIREBOLT] = 2;
		p->_pSkillExp[SPL_FIREBOLT] = SkillExpLvlsTbl[1];
		p->_pMemSkills = SPELL_MASK(SPL_FIREBOLT);
	}
	for (i = 0; i < lengthof(p->_pAtkSkillHotKey); i++)
		p->_pAtkSkillHotKey[i] = SPL_INVALID;
	for (i = 0; i < lengthof(p->_pAtkSkillTypeHotKey); i++)
		p->_pAtkSkillTypeHotKey[i] = RSPLTYPE_INVALID;
	for (i = 0; i < lengthof(p->_pMoveSkillHotKey); i++)
		p->_pMoveSkillHotKey[i] = SPL_INVALID;
	for (i = 0; i < lengthof(p->_pMoveSkillTypeHotKey); i++)
		p->_pMoveSkillTypeHotKey[i] = RSPLTYPE_INVALID;
	for (i = 0; i < lengthof(p->_pAltAtkSkillHotKey); i++)
		p->_pAltAtkSkillHotKey[i] = SPL_INVALID;
	for (i = 0; i < lengthof(p->_pAltAtkSkillTypeHotKey); i++)
		p->_pAltAtkSkillTypeHotKey[i] = RSPLTYPE_INVALID;
	for (i = 0; i < lengthof(p->_pAltMoveSkillHotKey); i++)
		p->_pAltMoveSkillHotKey[i] = SPL_INVALID;
	for (i = 0; i < lengthof(p->_pAltMoveSkillTypeHotKey); i++)
		p->_pAltMoveSkillTypeHotKey[i] = RSPLTYPE_INVALID;

	InitDungMsgs(pnum);
	CreatePlrItems(pnum);
	//SetRndSeed(0);
}

void NextPlrLevel(int pnum)
{
	PlayerStruct *p;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("NextPlrLevel: illegal player %d", pnum);
	}
	p = &plr[pnum];
	p->_pLevel++;

	p->_pStatPts += 4;
	p->_pLvlUp = TRUE;

	p->_pNextExper = PlrExpLvlsTbl[p->_pLevel];

	PlrFillHp(pnum);
	PlrFillMana(pnum);

	CalcPlrInv(pnum, true);

#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	if (sgbControllerActive)
		FocusOnCharInfo();
#endif
}

static void AddPlrSkillExp(int pnum, int lvl, int exp)
{
	PlayerStruct *p;
	int i, n = 0, dLvl;
	BYTE shr, sn, sl;
	unsigned xp;
	BYTE skills[NUM_SPELLS];

	p = &plr[pnum];
	// collect the active skills
	for (i = 0; i < NUM_SPELLS; i++) {
		if (p->_pSkillActivity[i] != 0 && (4 * p->_pSkillLvl[i]) < lvl + 8) {
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
		p->_pSkillActivity[sn]--;

		sl = p->_pSkillLvl[sn];
		dLvl = 8 + lvl - (4 * sl);
		xp = (exp * dLvl) >> shr; // / (8 * n);

		xp += p->_pSkillExp[sn];
		if (xp > SkillExpLvlsTbl[MAXSPLLEVEL] - 1) {
			xp = SkillExpLvlsTbl[MAXSPLLEVEL] - 1;
		}

		p->_pSkillExp[sn] = xp;
		while (xp >= SkillExpLvlsTbl[sl]) {
			sl++;
		}
		if (sl == p->_pSkillLvl[sn])
			continue;
		assert(sl <= MAXSPLLEVEL);
		p->_pSkillLvl[sn] = sl;
		NetSendCmdBParam2(false, CMD_PLRSKILLLVL, sn, sl);
	}
}

void AddPlrExperience(int pnum, int lvl, int exp)
{
	PlayerStruct *p;
	int expCap, dLvl;

	if (pnum != myplr) {
		return;
	}

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("AddPlrExperience: illegal player %d", pnum);
	}

	p = &plr[pnum];
	if (p->_pHitPoints < (1 << 6)) {
		return;
	}

	// Add xp to the used skills
	AddPlrSkillExp(pnum, lvl, exp);

	// Adjust xp based on difference in level between player and monster
	dLvl = 8 + lvl - p->_pLevel;
	if (dLvl <= 0) {
		return;
	}
	exp = exp * dLvl / 8;

	// Prevent power leveling
	if (gbMaxPlayers != 1) {
		assert(p->_pLevel >= 0 && p->_pLevel <= MAXCHARLEVEL);
		// cap to 1/32 of current levels xp
		expCap = p->_pNextExper / 32;
		if (exp > expCap) {
			exp = expCap;
		}
		// cap to 128 * current level
		expCap = 128 * p->_pLevel;
		if (exp > expCap) {
			exp = expCap;
		}
	}

	p->_pExperience += exp;
	if (p->_pExperience > PlrExpLvlsTbl[MAXCHARLEVEL] - 1) {
		p->_pExperience = PlrExpLvlsTbl[MAXCHARLEVEL] - 1;
	}

	// Increase player level if applicable
	while (p->_pExperience >= p->_pNextExper) {
		assert(p->_pLevel < MAXCHARLEVEL);
		NextPlrLevel(pnum);
		NetSendCmdBParam1(false, CMD_PLRLEVEL, p->_pLevel);
	}
}

void AddPlrMonstExper(int lvl, int exp, char pmask)
{
	int totplrs, i, e;

	if (!(pmask & (1 << myplr)))
		return;

	totplrs = 0;
	for (i = 0; i < MAX_PLRS; i++) {
		if (pmask & (1 << i)) {
			totplrs++;
		}
	}

	e = exp / totplrs;
	AddPlrExperience(myplr, lvl, e);
}

void InitPlayer(int pnum, bool FirstTime, bool active)
{
	PlayerStruct *p;
	//int i;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("InitPlayer: illegal player %d", pnum);
	}
	p = &plr[pnum];

	if (FirstTime) {
		p->_pManaShield = 0;

		p->_pAblSkills = SPELL_MASK(Abilities[p->_pClass]);
		p->_pAblSkills |= SPELL_MASK(SPL_WALK) | SPELL_MASK(SPL_BLOCK)
			| SPELL_MASK(SPL_ATTACK) | SPELL_MASK(SPL_RATTACK);

		p->_pAtkSkill = SPL_ATTACK;
		p->_pAtkSkillType = RSPLTYPE_ABILITY;
		p->_pMoveSkill = SPL_WALK;
		p->_pMoveSkillType = RSPLTYPE_ABILITY;
		p->_pAltAtkSkill = SPL_INVALID;
		p->_pAltAtkSkillType = RSPLTYPE_INVALID;
		p->_pAltMoveSkill = SPL_INVALID;
		p->_pAltMoveSkillType = RSPLTYPE_INVALID;
		if (!(p->_pSkillFlags & SFLAG_MELEE))
			p->_pAtkSkill = SPL_RATTACK;

		p->_pNextExper = PlrExpLvlsTbl[p->_pLevel];
	}

#ifdef _DEBUG
	if (active && (p->plrlevel == currLvl._dLevelIdx || leveldebug)) {
#else
	if (active && p->plrlevel == currLvl._dLevelIdx) {
#endif
		if (currLvl._dLevelIdx != DLV_TOWN)
			p->_pSkillFlags	|= SFLAG_DUNGEON;
		else
			p->_pSkillFlags	&= ~SFLAG_DUNGEON;

		SetPlrAnims(pnum);

		//p->_pxoff = 0;
		//p->_pyoff = 0;
		//p->_pxvel = 0;
		//p->_pyvel = 0;

		ClearPlrPVars(pnum);

		/*if (p->_pHitPoints >= (1 << 6)) {
			p->_pmode = PM_STAND;
			NewPlrAnim(pnum, p->_pNAnim, DIR_S, p->_pNFrames, 3, p->_pNWidth);
			p->_pAnimFrame = RandRange(1, p->_pNFrames - 1);
			p->_pAnimCnt = random_(2, 3);
		} else {
			p->_pmode = PM_DEATH;
			NewPlrAnim(pnum, p->_pDAnim, DIR_S, p->_pDFrames, 1, p->_pDWidth);
			p->_pAnimFrame = p->_pAnimLen - 1;
			p->_pVar8 = 2 * p->_pAnimLen;
		}*/

		if (pnum == myplr) {
			p->_px = ViewX;
			p->_py = ViewY;
		} else {
			SyncInitPlrPos(pnum);
			/*for (i = 0; i <= 8 && !PosOkPlayer(pnum, plrxoff2[i] + p->_px, plryoff2[i] + p->_py); i++)
				;
			p->_px += plrxoff2[i];
			p->_py += plryoff2[i];*/
		}

		PlrStartStand(pnum, DIR_S);
		// TODO: randomize AnimFrame/AnimCnt for live players?
		// p->_pAnimFrame = RandRange(1, p->_pNFrames - 1);
		// p->_pAnimCnt = random_(2, 3);

		//p->_pfutx = p->_px;
		//p->_pfuty = p->_py;

		p->walkpath[0] = WALK_NONE;
		p->destAction = ACTION_NONE;

		if (pnum == myplr) {
			p->_plid = AddLight(p->_px, p->_py, p->_pLightRad);
		} else {
			p->_plid = -1;
		}
		p->_pvid = AddVision(p->_px, p->_py, std::max(PLR_MIN_VISRAD, (int)p->_pLightRad), pnum == myplr);
	}

#ifdef _DEBUG
	if (debug_mode_key_inverted_v && FirstTime) {
		p->_pMemSkills = SPL_INVALID;
	} else if (debug_mode_god_mode && FirstTime) {
		p->_pMemSkills |= SPELL_MASK(SPL_TELEPORT);
		if (p->_pSkillLvl[SPL_TELEPORT] == 0) {
			p->_pSkillLvl[SPL_TELEPORT] = 1;
		}
	}
#endif

	// TODO: BUGFIX: should only be set if p->plrlevel == currLvl._dLevelIdx?
	if (p->_pmode != PM_DEATH)
		p->_pInvincible = FALSE;

	if (pnum == myplr) {
		// TODO: BUGFIX: sure?
		//    - what if we just joined with a dead player?
		//    - what if the player was killed while entering a portal?
		deathdelay = 0;
		gbDeathflag = false;
		ScrollInfo._sxoff = 0;
		ScrollInfo._syoff = 0;
		ScrollInfo._sdir = SDIR_NONE;
	}
}

static bool PlrDirOK(int pnum, int dir)
{
	int px, py;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("PlrDirOK: illegal player %d", pnum);
	}

	px = plr[pnum]._px + offset_x[dir];
	py = plr[pnum]._py + offset_y[dir];

	if (px < 0 || !dPiece[px][py] || !PosOkPlayer(pnum, px, py)) {
		return false;
	}

	if (dir == DIR_E) {
		return !nSolidTable[dPiece[px][py + 1]] && !(dFlags[px][py + 1] & BFLAG_PLAYERLR);
	}

	if (dir == DIR_W) {
		return !nSolidTable[dPiece[px + 1][py]] && !(dFlags[px + 1][py] & BFLAG_PLAYERLR);
	}

	return true;
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
	PlayerStruct *p;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("FixPlayerLocation: illegal player %d", pnum);
	}
	p = &plr[pnum];
	p->_pfutx = p->_poldx = p->_px;
	p->_pfuty = p->_poldy = p->_py;
	p->_pxoff = 0;
	p->_pyoff = 0;
	if (pnum == myplr) {
		ScrollInfo._sxoff = 0;
		ScrollInfo._syoff = 0;
		ScrollInfo._sdir = SDIR_NONE;
		ViewX = p->_px;
		ViewY = p->_py;
	}
}

void PlrStartStand(int pnum, int dir)
{
	PlayerStruct *p;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("PlrStartStand: illegal player %d", pnum);
	}
	p = &plr[pnum];
	if (p->_pHitPoints >= (1 << 6)) {
		if (!(p->_pGFXLoad & PFILE_STAND)) {
			LoadPlrGFX(pnum, PFILE_STAND);
		}

		NewPlrAnim(pnum, p->_pNAnim, dir, p->_pNFrames, 3, p->_pNWidth);
		p->_pmode = PM_STAND;
		RemovePlrFromMap(pnum);
		dPlayer[p->_px][p->_py] = pnum + 1;
		FixPlayerLocation(pnum);
	} else {
		SyncPlrKill(pnum, -1);
	}
}

static void StartWalkStand(int pnum)
{
	PlayerStruct *p;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("StartWalkStand: illegal player %d", pnum);
	}
	p = &plr[pnum];
	p->_pmode = PM_STAND;
	p->_pfutx = p->_px;
	p->_pfuty = p->_py;
	p->_pxoff = 0;
	p->_pyoff = 0;

	if (pnum == myplr) {
		ScrollInfo._sxoff = 0;
		ScrollInfo._syoff = 0;
		ScrollInfo._sdir = SDIR_NONE;
		ViewX = p->_px;
		ViewY = p->_py;
	}
}

static void PlrChangeLightOff(int pnum)
{
	PlayerStruct *p;
	int x, y;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrChangeLightOff: illegal player %d", pnum);
	}
	p = &plr[pnum];
	x = p->_pxoff + 2 * p->_pyoff;
	y = 2 * p->_pyoff - p->_pxoff;

	x = x / 8;
	y = y / 8;

	CondChangeLightOff(p->_plid, x, y);
}

static void PlrChangeOffset(int pnum)
{
	PlayerStruct *p;
	int px, py;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("PlrChangeOffset: illegal player %d", pnum);
	}

	p = &plr[pnum];
	px = p->_pVar6 >> 8;
	py = p->_pVar7 >> 8;

	p->_pVar6 += p->_pxvel;
	p->_pVar7 += p->_pyvel;

	p->_pxoff = p->_pVar6 >> 8;
	p->_pyoff = p->_pVar7 >> 8;

	px -= p->_pxoff;
	py -= p->_pyoff;

	if (pnum == myplr && ScrollInfo._sdir != SDIR_NONE) {
		ScrollInfo._sxoff += px;
		ScrollInfo._syoff += py;
		// TODO: follow with the cursor if a monster is selected? (does not work well with upscale)
		//if (gbActionBtnDown && (px | py) != 0 && pcursmonst != -1)
		//	SetCursorPos(MouseX + px, MouseY + py);
	}

	if (p->_plid != -1)
		PlrChangeLightOff(pnum);
}

/**
 * @brief Starting a move action towards NW, N, or NE
 */
static void StartWalk(int pnum, int xvel, int yvel, int xadd, int yadd, int EndDir, int sdir)
{
	PlayerStruct *p;
	int px, py;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("StartWalk: illegal player %d", pnum);
	}

	p = &plr[pnum];

	SetPlayerOld(p);

	px = xadd + p->_px;
	py = yadd + p->_py;

	if (!PlrDirOK(pnum, EndDir)) {
		return;
	}

	p->_pfutx = px;
	p->_pfuty = py;

	if (pnum == myplr) {
		ScrollInfo._sdx = p->_px - ViewX;
		ScrollInfo._sdy = p->_py - ViewY;
	}

	dPlayer[px][py] = -(pnum + 1);
	p->_pmode = PM_WALK;
	p->_pxvel = xvel;
	p->_pyvel = yvel;
	p->_pxoff = 0;
	p->_pyoff = 0;
	p->_pVar1 = xadd;   // dx after the movement
	p->_pVar2 = yadd;   // dy after the movement
	p->_pVar3 = EndDir; // Player's direction when ending movement.
	p->_pVar6 = 0;      // Same as _pxoff but contains the value in a higher range
	p->_pVar7 = 0;      // Same as _pyoff but contains the value in a higher range
	p->_pVar8 = 0;      // speed helper

	if (!(p->_pGFXLoad & PFILE_WALK)) {
		LoadPlrGFX(pnum, PFILE_WALK);
	}

	NewPlrAnim(pnum, p->_pWAnim, EndDir, p->_pWFrames, 0, p->_pWWidth);
	//NewPlrAnim(pnum, p->_pWAnim, EndDir, p->_pWFrames, 0, p->_pWWidth, 0, true); ANIM_GAMELOGIC

	if (pnum != myplr) {
		return;
	}

	if (gbZoomflag) {
		if (abs(ScrollInfo._sdx) >= 3 || abs(ScrollInfo._sdy) >= 3) {
			ScrollInfo._sdir = SDIR_NONE;
		} else {
			ScrollInfo._sdir = sdir;
		}
	} else if (abs(ScrollInfo._sdx) >= 2 || abs(ScrollInfo._sdy) >= 2) {
		ScrollInfo._sdir = SDIR_NONE;
	} else {
		ScrollInfo._sdir = sdir;
	}
}

/**
 * @brief Starting a move action towards SW, S, or SE
 */
#if defined(__clang__) || defined(__GNUC__)
__attribute__((no_sanitize("shift-base")))
#endif
static void StartWalk2(int pnum, int xvel, int yvel, int xoff, int yoff, int xadd, int yadd, int EndDir, int sdir)
{
	PlayerStruct *p;
	int px, py;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("StartWalk2: illegal player %d", pnum);
	}

	p = &plr[pnum];

	SetPlayerOld(p);

	px = xadd + p->_px;
	py = yadd + p->_py;

	if (!PlrDirOK(pnum, EndDir)) {
		return;
	}

	p->_pfutx = px;
	p->_pfuty = py;

	if (pnum == myplr) {
		ScrollInfo._sdx = p->_px - ViewX;
		ScrollInfo._sdy = p->_py - ViewY;
	}

	dPlayer[p->_px][p->_py] = -(pnum + 1);
	p->_pVar1 = p->_px;  // the starting x-coordinate of the player
	p->_pVar2 = p->_py;  // the starting y-coordinate of the player
	p->_px = px; // Move player to the next tile to maintain correct render order
	p->_py = py;
	dPlayer[p->_px][p->_py] = pnum + 1;
	p->_pxoff = xoff; // Offset player sprite to align with their previous tile position
	p->_pyoff = yoff;

	if (p->_plid != -1) {
		ChangeLightXY(p->_plid, p->_px, p->_py);
		PlrChangeLightOff(pnum);
	}

	p->_pmode = PM_WALK2;
	p->_pxvel = xvel;
	p->_pyvel = yvel;
	p->_pVar6 = xoff * 256; // Same as _pxoff but contains the value in a higher range
	p->_pVar7 = yoff * 256; // Same as _pyoff but contains the value in a higher range
	p->_pVar3 = EndDir;     // Player's direction when ending movement.
	p->_pVar8 = 0;          // speed helper

	if (!(p->_pGFXLoad & PFILE_WALK)) {
		LoadPlrGFX(pnum, PFILE_WALK);
	}
	NewPlrAnim(pnum, p->_pWAnim, EndDir, p->_pWFrames, 0, p->_pWWidth);

	if (pnum != myplr) {
		return;
	}

	if (gbZoomflag) {
		if (abs(ScrollInfo._sdx) >= 3 || abs(ScrollInfo._sdy) >= 3) {
			ScrollInfo._sdir = SDIR_NONE;
		} else {
			ScrollInfo._sdir = sdir;
		}
	} else if (abs(ScrollInfo._sdx) >= 2 || abs(ScrollInfo._sdy) >= 2) {
		ScrollInfo._sdir = SDIR_NONE;
	} else {
		ScrollInfo._sdir = sdir;
	}
}

/**
 * @brief Starting a move action towards W or E
 */
#if defined(__clang__) || defined(__GNUC__)
__attribute__((no_sanitize("shift-base")))
#endif
static void StartWalk3(int pnum, int xvel, int yvel, int xoff, int yoff, int xadd, int yadd, int mapx, int mapy, int EndDir, int sdir)
{
	PlayerStruct *p;
	int px, py, x, y;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("StartWalk3: illegal player %d", pnum);
	}

	p = &plr[pnum];

	SetPlayerOld(p);

	px = xadd + p->_px;
	py = yadd + p->_py;
	x = mapx + p->_px;
	y = mapy + p->_py;

	if (!PlrDirOK(pnum, EndDir)) {
		return;
	}

	p->_pfutx = px;
	p->_pfuty = py;

	if (pnum == myplr) {
		ScrollInfo._sdx = p->_px - ViewX;
		ScrollInfo._sdy = p->_py - ViewY;
	}

	dPlayer[p->_px][p->_py] = -(pnum + 1);
	dPlayer[px][py] = -(pnum + 1);
	p->_pVar4 = x; // Used for storing X-position of a tile which should have its BFLAG_PLAYERLR flag removed after walking. When starting to walk the game places the player in the dPlayer array -1 in the Y coordinate, and uses BFLAG_PLAYERLR to check if it should be using -1 to the Y coordinate when rendering the player
	p->_pVar5 = y; // Used for storing Y-position of a tile which should have its BFLAG_PLAYERLR flag removed after walking. When starting to walk the game places the player in the dPlayer array -1 in the Y coordinate, and uses BFLAG_PLAYERLR to check if it should be using -1 to the Y coordinate when rendering the player
	dFlags[x][y] |= BFLAG_PLAYERLR;
	p->_pxoff = xoff; // Offset player sprite to align with their previous tile position
	p->_pyoff = yoff;

	if (p->_plid != -1) {
		//ChangeLightXY(p->_plid, p->_pVar4, p->_pVar5);
		PlrChangeLightOff(pnum);
	}

	p->_pmode = PM_WALK3;
	p->_pxvel = xvel;
	p->_pyvel = yvel;
	p->_pVar1 = px;         // the Player's x-coordinate after the movement
	p->_pVar2 = py;         // the Player's y-coordinate after the movement
	p->_pVar6 = xoff * 256; // Same as _pxoff but contains the value in a higher range
	p->_pVar7 = yoff * 256; // Same as _pyoff but contains the value in a higher range
	p->_pVar3 = EndDir;     // Player's direction when ending movement.
	p->_pVar8 = 0;          // speed helper

	if (!(p->_pGFXLoad & PFILE_WALK)) {
		LoadPlrGFX(pnum, PFILE_WALK);
	}
	NewPlrAnim(pnum, p->_pWAnim, EndDir, p->_pWFrames, 0, p->_pWWidth);

	if (pnum != myplr) {
		return;
	}

	if (gbZoomflag) {
		if (abs(ScrollInfo._sdx) >= 3 || abs(ScrollInfo._sdy) >= 3) {
			ScrollInfo._sdir = SDIR_NONE;
		} else {
			ScrollInfo._sdir = sdir;
		}
	} else if (abs(ScrollInfo._sdx) >= 2 || abs(ScrollInfo._sdy) >= 2) {
		ScrollInfo._sdir = SDIR_NONE;
	} else {
		ScrollInfo._sdir = sdir;
	}
}

static bool StartAttack(int pnum)
{
	PlayerStruct *p;
	int i, dx, dy, sn, sl, dir;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("StartAttack: illegal player %d", pnum);
	}

	p = &plr[pnum];

	i = p->destParam1;
	if (p->destAction == ACTION_ATTACK) {
		dx = i;
		dy = p->destParam2;
		sn = p->destParam3;
		sl = p->destParam4;
	} else if (p->destAction == ACTION_ATTACKMON) {
		dx = monster[i]._mfutx;
		dy = monster[i]._mfuty;
		if (abs(p->_px - dx) > 1 || abs(p->_py - dy) > 1)
			return false;
		if (monster[i].mtalkmsg != TEXT_NONE && monster[i].mtalkmsg != TEXT_VILE14) {
			TalktoMonster(i, pnum);
			return true;
		}
		sn = p->destParam2;
		sl = p->destParam3;
	} else if (p->destAction == ACTION_ATTACKPLR) {
		dx = plr[i]._pfutx;
		dy = plr[i]._pfuty;
		if (abs(p->_px - dx) > 1 || abs(p->_py - dy) > 1)
			return false;
		sn = p->destParam2;
		sl = p->destParam3;
	} else {
		assert(p->destAction == ACTION_OPERATE || p->destAction == ACTION_DISARM);
		dx = p->destParam2;
		dy = p->destParam3;
		if (abs(p->_px - dx) > 1 || abs(p->_py - dy) > 1)
			return false;
		if (abs(dObject[dx][dy]) != i + 1) // this should always be false, but never trust the internet
			return false;
		if (object[i]._oBreak != 1) {
			if (p->destAction == ACTION_DISARM)
				DisarmObject(pnum, i);
			OperateObject(pnum, i, false);
			return true;
		}
		if (p->destAction == ACTION_DISARM && pnum == myplr)
			NewCursor(CURSOR_HAND);
		sn = SPL_ATTACK;
		sl = 0;
	}

	dir = GetDirection(p->_px, p->_py, dx, dy);
	p->_pmode = PM_ATTACK;
	p->_pVar5 = sn; // attack 'spell'
	p->_pVar6 = sl; // attack 'spell'-level
	p->_pVar7 = 0;  // 'flags' of sfx and hit
	p->_pVar8 = 0;  // speed helper

	if (!(p->_pGFXLoad & PFILE_ATTACK)) {
		LoadPlrGFX(pnum, PFILE_ATTACK);
	}
	NewPlrAnim(pnum, p->_pAAnim, dir, p->_pAFrames, 0, p->_pAWidth);
	/* ANIM_GAMELOGIC
	// Every Attack start with Frame 2. Because ProcessPlayerAnimation is called after 
	//  StartAttack and its increases the AnimationFrame.
	int skippedAnimationFrames = 1;
	if (p->_pIFlags & ISPL_FASTATTACK) {
		skippedAnimationFrames += 1;
	}
	if (p->_pIFlags & ISPL_FASTERATTACK) {
		skippedAnimationFrames += 2;
	}
	if (p->_pIFlags & ISPL_FASTESTATTACK) {
		skippedAnimationFrames += 2;
	}
	NewPlrAnim(pnum, p->_pAAnim, dir, p->_pAFrames, 0, p->_pAWidth, skippedAnimationFrames, true, p->_pAFNum);*/

	FixPlayerLocation(pnum);
	return true;
}

static void StartRangeAttack(int pnum)
{
	PlayerStruct *p;
	int i, dx, dy, sn, sl, dir;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("StartRangeAttack: illegal player %d", pnum);
	}

	p = &plr[pnum];

	i = p->destParam1;
	if (p->destAction == ACTION_RATTACK) {
		dx = i;
		dy = p->destParam2;
		sn = p->destParam3;
		sl = p->destParam4;
	} else if (p->destAction == ACTION_RATTACKMON) {
		if (monster[i].mtalkmsg != TEXT_NONE && monster[i].mtalkmsg != TEXT_VILE14) {
			TalktoMonster(i, pnum);
			return;
		}
		dx = monster[i]._mfutx;
		dy = monster[i]._mfuty;
		sn = p->destParam2;
		sl = p->destParam3;
	} else {
		assert(p->destAction == ACTION_RATTACKPLR);
		dx = plr[i]._pfutx;
		dy = plr[i]._pfuty;
		sn = p->destParam2;
		sl = p->destParam3;
	}

	p->_pVar1 = dx;
	p->_pVar2 = dy;
	p->_pVar5 = sn; // attack 'spell'
	p->_pVar6 = sl; // attack 'spell'-level
	p->_pVar7 = 0;  // 'flag' of launch
	p->_pVar8 = 0;  // speed helper
	p->_pmode = PM_RATTACK;

	dir = GetDirection(p->_px, p->_py, dx, dy);

	if (!(p->_pGFXLoad & PFILE_ATTACK)) {
		LoadPlrGFX(pnum, PFILE_ATTACK);
	}
	NewPlrAnim(pnum, p->_pAAnim, dir, p->_pAFrames, 0, p->_pAWidth);
	/* ANIM_GAMELOGIC
	// Every Attack start with Frame 2. Because ProcessPlayerAnimation is called after
	//  StartRangeAttack and its increases the AnimationFrame.
	int skippedAnimationFrames = 1;
	if (p->_pIFlags & ISPL_FASTATTACK) {
		skippedAnimationFrames += 1;
	}
	NewPlrAnim(pnum, p->_pAAnim, dir, p->_pAFrames, 0, p->_pAWidth, skippedAnimationFrames, true, p->_pAFNum);*/

	FixPlayerLocation(pnum);
}

static void StartBlock(int pnum, int dir)
{
	PlayerStruct *p;
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("StartBlock: illegal player %d", pnum);
	}

	p = &plr[pnum];
	p->_pmode = PM_BLOCK;
	p->_pVar1 = 0; // extended blocking
	if (!(p->_pGFXLoad & PFILE_BLOCK)) {
		LoadPlrGFX(pnum, PFILE_BLOCK);
	}
	NewPlrAnim(pnum, p->_pBAnim, dir, p->_pBFrames, 2, p->_pBWidth);
	/* ANIM_GAMELOGIC
	// Block can start with Frame 1 if Player 2 hits Player 1. In this case Player 1 will
	//  not call again ProcessPlayerAnimation.
	int skippedAnimationFrames = 0;
	if (p->_pIFlags & ISPL_FASTBLOCK) {
		skippedAnimationFrames = (p->_pBFrames - 1); // ISPL_FASTBLOCK means there is only one AnimationFrame.
	}
	NewPlrAnim(pnum, p->_pBAnim, dir, p->_pBFrames, 2, p->_pBWidth, skippedAnimationFrames);*/

	FixPlayerLocation(pnum);
}

static void StartSpell(int pnum)
{
	PlayerStruct *p;
	int i, dx, dy;
	player_graphic gfx;
	unsigned char **anim;
	SpellData *sd;

	if ((unsigned)pnum >= MAX_PLRS)
		app_fatal("StartSpell: illegal player %d", pnum);

	p = &plr[pnum];

	i = p->destParam2;
	if (p->destAction == ACTION_SPELL) {
		dx = i;
		dy = p->destParam3;
	} else if (p->destAction == ACTION_SPELLMON) {
		dx = monster[i]._mfutx;
		dy = monster[i]._mfuty;
	} else {
		assert(p->destAction == ACTION_SPELLPLR);
		dx = plr[i]._pfutx;
		dy = plr[i]._pfuty;
	}

	p->_pVar1 = dx;                   // x-tile of the target
	p->_pVar2 = dy;                   // y-tile of the target
	p->_pVar3 = p->destParam1a;       // the spell to be cast -- used in SyncPlrAnim
	p->_pVar4 = p->destParam1c;       // the level of the spell to be used
	p->_pVar5 = (char)p->destParam1b; // source of the spell
	p->_pVar7 = FALSE;                // 'flag' of cast
	p->_pVar8 = 0;                    // speed helper
	p->_pmode = PM_SPELL;

	sd = &spelldata[p->_pVar3];
	if (sd->sTargeted)
		p->_pdir = GetDirection(p->_px, p->_py, dx, dy);
	switch (sd->sType) {
	case STYPE_FIRE:
		gfx = PFILE_FIRE;
		anim = p->_pFAnim;
		break;
	case STYPE_LIGHTNING:
		gfx = PFILE_LIGHTNING;
		anim = p->_pLAnim;
		break;
	case STYPE_MAGIC:
		gfx = PFILE_MAGIC;
		anim = p->_pTAnim;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	if (!(p->_pGFXLoad & gfx)) {
		LoadPlrGFX(pnum, gfx);
	}
	NewPlrAnim(pnum, anim, p->_pdir, p->_pSFrames, 0, p->_pSWidth);
	//NewPlrAnim(pnum, anim, p->_pdir, p->_pSFrames, 0, p->_pSWidth, 1, true); ANIM_GAMELOGIC

	PlaySfxLoc(sd->sSFX, p->_px, p->_py);

	FixPlayerLocation(pnum);
}

/*
 * @brief Find a place for the given player starting from its current location.
 *
 * TODO: In the original code it was possible to auto-townwarp after resurrection.
 *       The new solution prevents this, but in some cases it could be useful
 *       (in some cases it is annoying).
 *
 * @return TRUE if the player had to be displaced.
 */
bool PlacePlayer(int pnum)
{
	int i, nx, ny, x, y;
	bool done;

	for (i = 0; i < lengthof(plrxoff2); i++) {
		nx = plr[pnum]._px + plrxoff2[i];
		ny = plr[pnum]._py + plryoff2[i];

		if (PosOkPlayer(pnum, nx, ny) && PosOkPortal(nx, ny)) {
			break;
		}
	}

	if (i == 0)
		return false;

	if (i == lengthof(plrxoff2)) {
		done = false;

		for (i = 1; i < 50 && !done; i++) {
			for (y = -i; y <= i && !done; y++) {
				ny = plr[pnum]._py + y;

				for (x = -i; x <= i && !done; x++) {
					nx = plr[pnum]._px + x;

					if (PosOkPlayer(pnum, nx, ny) && PosOkPortal(nx, ny)) {
						done = true;
					}
				}
			}
		}
	}

	plr[pnum]._px = nx;
	plr[pnum]._py = ny;
	return true;
}

void RemovePlrFromMap(int pnum)
{
	int pp, pn;
	int dx, dy, y, x;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("RemovePlrFromMap: illegal player %d", pnum);
	}

	dx = plr[pnum]._poldx;
	dy = plr[pnum]._poldy;
	// BUGFIX: is this necessary?
	if (!IN_DUNGEON_AREA(dx, dy))
		return;

	if (dx < MAXDUNX - 1)
		dFlags[dx + 1][dy] &= ~BFLAG_PLAYERLR;
	if (dy < MAXDUNY - 1)
		dFlags[dx][dy + 1] &= ~BFLAG_PLAYERLR;

	// BUGFIX: is this necessary?
	if (dx < 1)
		dx = 1;
	else if (dx >= MAXDUNX - 1)
		dx = MAXDUNX - 2;
	if (dy < 1)
		dy = 1;
	else if (dy >= MAXDUNY - 1)
		dy = MAXDUNY - 2;

	pp = pnum + 1;
	pn = -(pnum + 1);
	for (y = dy - 1; y <= dy + 1; y++) {
		for (x = dx - 1; x <= dx + 1; x++) {
			if (dPlayer[x][y] == pp || dPlayer[x][y] == pn) {
				dPlayer[x][y] = 0;
			}
		}
	}
}

/*void RemovePlrFromMap(int pnum)
{
	int x, y;
	int pp, pn;

	pp = pnum + 1;
	pn = -(pnum + 1);

	for (y = 0; y < MAXDUNY; y++)
		for (x = 0; x < MAXDUNX; x++)
			if (dPlayer[x][y] == pp || dPlayer[x][y] == pn) {
				dPlayer[x][y] = 0;

				if (x < MAXDUNX - 1)
					dFlags[x + 1][y] &= ~BFLAG_PLAYERLR;
				if (y < MAXDUNY - 1)
					dFlags[x][y + 1] &= ~BFLAG_PLAYERLR;
			}
}*/

void StartPlrHit(int pnum, int dam, bool forcehit)
{
	PlayerStruct *p;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("StartPlrHit: illegal player %d", pnum);
	}

	p = &plr[pnum];
	if (p->_pHitPoints < (1 << 6)) {
		SyncPlrKill(pnum, -1); // BUGFIX: is this really necessary?
		return;
	}

	PlaySfxLoc(sgSFXSets[SFXS_PLR_69][p->_pClass], p->_px, p->_py, 2);

	if (!forcehit) {
		if (p->_pManaShield != 0 || (dam << 2) < p->_pMaxHP)
			return;
	}

	if (!(p->_pGFXLoad & PFILE_HIT)) {
		LoadPlrGFX(pnum, PFILE_HIT);
	}
	NewPlrAnim(pnum, p->_pHAnim, p->_pdir, p->_pHFrames, 0, p->_pHWidth);
	/* ANIM_GAMELOGIC
	// GotHit can start with Frame 1. GotHit can for example be called in ProcessMonsters()
	//  and this is after ProcessPlayers().
	int skippedAnimationFrames = 0;
	const int ZenFlags = ISPL_FASTRECOVER | ISPL_FASTERRECOVER | ISPL_FASTESTRECOVER;
	// if multiple hitrecovery modes are present the skipping of frames can go so far,
	// that they skip frames that would skip. so the additional skipping thats skipped.
	// That means we can't add the different modes together.
	if ((p->_pIFlags & ZenFlags) == ZenFlags) {
		skippedAnimationFrames = 4;
	} else if (p->_pIFlags & ISPL_FASTESTRECOVER) {
		skippedAnimationFrames = 3;
	} else if (p->_pIFlags & ISPL_FASTERRECOVER) {
		skippedAnimationFrames = 2;
	} else if (p->_pIFlags & ISPL_FASTRECOVER) {
		skippedAnimationFrames = 1;
	} else {
		skippedAnimationFrames = 0;
	}
	NewPlrAnim(pnum, p->_pHAnim, p->_pdir, p->_pHFrames, 0, p->_pHWidth, skippedAnimationFrames);*/

	p->_pmode = PM_GOTHIT;
	RemovePlrFromMap(pnum);
	dPlayer[p->_px][p->_py] = pnum + 1;
	FixPlayerLocation(pnum);
}

static void PlrDeadItem(ItemStruct *is, PlayerStruct *p)
{
	int dir, x, y, i, xx, yy;

	if (is->_itype == ITYPE_NONE || numitems >= MAXITEMS)
		return;

	dir = p->_pdir;
	x = p->_px;
	y = p->_py;

	static_assert(lengthof(offset_x) == lengthof(offset_y), "Mismatching offset tables.");
	static_assert(lengthof(offset_x) == 8, "Offset table is expected to be an omnidirectional table.");
	for (i = 0; i < lengthof(offset_x); i++) {
		xx = x + offset_x[dir];
		yy = y + offset_y[dir];
		if (CanPut(xx, yy)) {
			break;
		}
		dir = (dir + 1) & 7;
	}

	if (i == lengthof(offset_x)) {
		if (!FindItemLocation(x, y, &x, &y, DSIZEX / 2))
			return;
		xx = x;
		yy = y;
	}

	// RespawnDeadItem
	assert(numitems < MAXITEMS);

	i = itemavail[0];
	dItem[xx][yy] = i + 1;
	itemavail[0] = itemavail[MAXITEMS - numitems - 1];
	itemactive[numitems] = i;
	copy_pod(items[i], *is);
	items[i]._ix = xx;
	items[i]._iy = yy;
	RespawnItem(i, true);
	numitems++;
	NetSendCmdPItem(false, CMD_RESPAWNITEM, is, xx, yy);

	is->_itype = ITYPE_NONE;
}

#if defined(__clang__) || defined(__GNUC__)
__attribute__((no_sanitize("shift-base")))
#endif
void StartPlrKill(int pnum, int earflag)
{
	bool diablolevel;
	int i;
	PlayerStruct *p;
	ItemStruct ear;
	ItemStruct *pi;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("StartPlrKill: illegal player %d", pnum);
	}

	p = &plr[pnum];
	if (p->_pmode == PM_DEATH) {
		return;
	}

	if (myplr == pnum) {
		NetSendCmdParam1(true, CMD_PLRDEAD, earflag);
	}

	diablolevel = gbMaxPlayers != 1 && p->plrlevel == 16;

	PlaySfxLoc(sgSFXSets[SFXS_PLR_71][p->_pClass], p->_px, p->_py);

	if (p->_pgfxnum != ANIM_ID_UNARMED) {
		p->_pgfxnum = ANIM_ID_UNARMED;
		p->_pGFXLoad = 0;
		SetPlrAnims(pnum);
	}

	if (!(p->_pGFXLoad & PFILE_DEATH)) {
		LoadPlrGFX(pnum, PFILE_DEATH);
	}

	NewPlrAnim(pnum, p->_pDAnim, p->_pdir, p->_pDFrames, 1, p->_pDWidth);

	p->_pmode = PM_DEATH;
	p->_pInvincible = TRUE;
	p->_pVar8 = 1;

	if (pnum != myplr && !earflag && !diablolevel) {
		for (i = 0; i < NUM_INVLOC; i++) {
			p->InvBody[i]._itype = ITYPE_NONE;
		}
		CalcPlrInv(pnum, false);
	}

	if (p->plrlevel == currLvl._dLevelIdx) {
		RemovePlrFromMap(pnum);
		dFlags[p->_px][p->_py] |= BFLAG_DEAD_PLAYER;
		FixPlayerLocation(pnum);

		if (pnum == myplr) {
			deathdelay = 30;

			if (pcurs >= CURSOR_FIRSTITEM) {
				PlrDeadItem(&p->HoldItem, p);
				NewCursor(CURSOR_HAND);
			}

			if (earflag != -1) {
				if (earflag != 0) {
					// pvp
					CreateBaseItem(&ear, IDI_EAR);
					snprintf(ear._iName, sizeof(ear._iName), "Ear of %s", p->_pName);
					const int earSets[NUM_CLASSES] = {
							ICURS_EAR_WARRIOR, ICURS_EAR_ROGUE, ICURS_EAR_SORCERER
#ifdef HELLFIRE
							, ICURS_EAR_SORCERER, ICURS_EAR_ROGUE, ICURS_EAR_WARRIOR
#endif
					};
					ear._iCurs = earSets[p->_pClass];

					ear._iCreateInfo = p->_pName[0] << 8 | p->_pName[1];
					ear._iSeed = p->_pName[2] << 24 | p->_pName[3] << 16 | p->_pName[4] << 8 | p->_pName[5];
					ear._ivalue = p->_pLevel;

					if (FindGetItem(IDI_EAR, ear._iCreateInfo, ear._iSeed) == -1) {
						PlrDeadItem(&ear, p);
					}
				} else {
					// pvm
					plr->_pExperience -= (plr->_pExperience - PlrExpLvlsTbl[plr->_pLevel - 1]) >> 2;

					if (!diablolevel) {
						pi = &p->InvBody[0];
						for (i = NUM_INVLOC; i != 0; i--, pi++) {
							PlrDeadItem(pi, p);
						}

						CalcPlrInv(pnum, false);
					}
				}
			}
		}
	}
	PlrSetHp(pnum, 0);
}

void SyncPlrKill(int pnum, int earflag)
{
	if (currLvl._dType == DTYPE_TOWN) {
		PlrSetHp(pnum, 64);
		return;
	}

	StartPlrKill(pnum, earflag);
}

void RemovePlrMissiles(int pnum)
{
	int i, mi;

	static_assert(MAX_MINIONS == MAX_PLRS, "RemovePlrMissiles requires that owner of a monster has the same id as the monster itself.");
	if (currLvl._dType != DTYPE_TOWN && pnum == myplr && !(MINION_NR_INACTIVE(pnum))) {
		MonStartKill(pnum, pnum);
		AddDead(pnum);
		DeleteMonsterList();
	}

	for (i = 0; i < nummissiles; i++) {
		mi = missileactive[i];
		if (missile[mi]._miSource == pnum && missile[mi]._miType == MIS_STONE) {
			monster[missile[mi]._miVar2]._mmode = missile[mi]._miVar1;
			DeleteMissile(mi, i);
			i--;
		}
	}
}

static void InitLevelChange(int pnum)
{
	PlayerStruct *p;

	ClrPlrPath(pnum);
	p = &plr[pnum];
	if (p->plrlevel == currLvl._dLevelIdx) {
		AddUnLight(p->_plid);
		AddUnVision(p->_pvid);
		RemovePlrMissiles(pnum);
		RemovePlrFromMap(pnum);
	}
	SetPlayerOld(p);
	if (pnum == myplr) {
		if (gbQtextflag) {
			gbQtextflag = false;
			stream_stop();
		}
		p->pLvlLoad = 10;
		dPlayer[p->_px][p->_py] = pnum + 1;
	} else {
		p->_pLvlVisited[p->plrlevel] = TRUE;
	}

	p->destAction = ACTION_NONE;
	p->_pLvlChanging = TRUE;
}

#if defined(__clang__) || defined(__GNUC__)
__attribute__((no_sanitize("shift-base")))
#endif
void StartNewLvl(int pnum, int fom, int lvl)
{
	InitLevelChange(pnum);

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("StartNewLvl: illegal player %d", pnum);
	}

	switch (fom) {
	case WM_DIABNEXTLVL:
	case WM_DIABPREVLVL:
	case WM_DIABRTNLVL:
	case WM_DIABTOWNWARP:
		plr[pnum].plrlevel = lvl;
		break;
	case WM_DIABSETLVL:
		plr[pnum].plrlevel = lvl;
		break;
	case WM_DIABTWARPUP:
		if (pnum == myplr)
			plr[pnum].pTownWarps |= 1 << (currLvl._dType - 2);
		plr[pnum].plrlevel = lvl;
		break;
	default:
		app_fatal("StartNewLvl %d", fom);
		break;
	}

	if (pnum == myplr) {
		plr[pnum]._pmode = PM_NEWLVL;
		plr[pnum]._pInvincible = TRUE;
		PostMessage(fom, 0, 0);
		if (gbMaxPlayers != 1) {
			NetSendCmdParam2(true, CMD_NEWLVL, fom, lvl);
		}
	}
}

void RestartTownLvl(int pnum)
{
	InitLevelChange(pnum);
	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("RestartTownLvl: illegal player %d", pnum);
	}

	plr[pnum].plrlevel = 0;
	plr[pnum]._pInvincible = FALSE;

	PlrSetHp(pnum, 64);
	PlrSetMana(pnum, 0);

	CalcPlrInv(pnum, false);

	if (pnum == myplr) {
		gbDeathflag = false;
		gamemenu_off();
		plr[pnum]._pmode = PM_NEWLVL;
		plr[pnum]._pInvincible = TRUE;
		PostMessage(WM_DIABRETOWN, 0, 0);
	}
}

void StartWarpLvl(int pnum, int pidx)
{
	InitLevelChange(pnum);

	if (plr[pnum].plrlevel != 0) {
		plr[pnum].plrlevel = 0;
	} else {
		plr[pnum].plrlevel = portal[pidx].level;
	}

	if (pnum == myplr) {
		SetCurrentPortal(pidx);
		plr[pnum]._pmode = PM_NEWLVL;
		plr[pnum]._pInvincible = TRUE;
		PostMessage(WM_DIABWARPLVL, 0, 0);
	}
}

static bool PlrDoStand(int pnum)
{
	return false;
}

static inline void PlrStepAnim(int pnum)
{
	PlayerStruct *p;

	p = &plr[pnum];
	p->_pAnimCnt++;
	if (p->_pAnimCnt > p->_pAnimDelay) {
		p->_pAnimCnt = 0;
		p->_pAnimFrame++;
	}
}

static bool PlrDoWalk(int pnum)
{
	PlayerStruct *p;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("PlrDoWalk: illegal player %d", pnum);
	}

	p = &plr[pnum];
	p->_pVar8++;
	if (p->_pIFlags2 & ISPH_FASTESTWALK) {
		PlrStepAnim(pnum);
	} else if (p->_pIFlags2 & ISPH_FASTERWALK) {
		if ((p->_pVar8 & 1) == 1)
			PlrStepAnim(pnum);
	} else if (p->_pIFlags2 & ISPH_FASTWALK) {
		if ((p->_pVar8 & 3) == 2)
			PlrStepAnim(pnum);
	}

	if ((p->_pAnimFrame & 3) == 3) {
		PlaySfxLoc(PS_WALK1, p->_px, p->_py);
	}

	if (p->_pAnimFrame < p->_pWFrames) {
		PlrChangeOffset(pnum);
		return false;
	}

	switch (p->_pmode) {
	case PM_WALK: // Movement towards NW, N, and NE
		dPlayer[p->_px][p->_py] = 0;
		p->_px += p->_pVar1;
		p->_py += p->_pVar2;
		dPlayer[p->_px][p->_py] = pnum + 1;
		break;
	case PM_WALK2: // Movement towards SW, S, and SE
		dPlayer[p->_pVar1][p->_pVar2] = 0;
		break;
	case PM_WALK3: // Movement towards W and E
		dPlayer[p->_px][p->_py] = 0;
		dFlags[p->_pVar4][p->_pVar5] &= ~BFLAG_PLAYERLR;
		p->_px = p->_pVar1;
		p->_py = p->_pVar2;
		dPlayer[p->_px][p->_py] = pnum + 1;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	if (pnum == myplr && ScrollInfo._sdir != SDIR_NONE) {
		ViewX = p->_px - ScrollInfo._sdx;
		ViewY = p->_py - ScrollInfo._sdy;
	}

	if (p->walkpath[0] != WALK_NONE) {
		StartWalkStand(pnum);
	} else {
		PlrStartStand(pnum, p->_pVar3);
	}

	ClearPlrPVars(pnum);

	ChangeLightXYOff(p->_plid, p->_px, p->_py);
	ChangeVisionXY(p->_pvid, p->_px, p->_py);
	return true;
}

static bool WeaponDur(int pnum, int durrnd)
{
	PlayerStruct *p;
	ItemStruct *pi;
	if (pnum != myplr) {
		return false;
	}

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("WeaponDur: illegal player %d", pnum);
	}

	if (random_(3, durrnd) != 0) {
		return false;
	}

	p = &plr[pnum];
	pi = &p->InvBody[INVLOC_HAND_LEFT];
	if (pi->_itype != ITYPE_NONE && pi->_iClass == ICLASS_WEAPON) {
		if (pi->_iDurability == DUR_INDESTRUCTIBLE) {
			return false;
		}

		pi->_iDurability--;
		if (pi->_iDurability == 0) {
			NetSendCmdDelItem(true, INVLOC_HAND_LEFT);
			pi->_itype = ITYPE_NONE;
			CalcPlrInv(pnum, true);
			return true;
		}
	}

	pi = &p->InvBody[INVLOC_HAND_RIGHT];
	if (pi->_itype != ITYPE_NONE && pi->_iClass == ICLASS_WEAPON) {
		if (pi->_iDurability == DUR_INDESTRUCTIBLE) {
			return false;
		}

		pi->_iDurability--;
		if (pi->_iDurability == 0) {
			NetSendCmdDelItem(true, INVLOC_HAND_RIGHT);
			pi->_itype = ITYPE_NONE;
			CalcPlrInv(pnum, true);
			return true;
		}
	}

	if (p->InvBody[INVLOC_HAND_LEFT]._itype == ITYPE_NONE && pi->_itype == ITYPE_SHIELD) {
		if (pi->_iDurability == DUR_INDESTRUCTIBLE) {
			return false;
		}

		pi->_iDurability--;
		if (pi->_iDurability == 0) {
			NetSendCmdDelItem(true, INVLOC_HAND_RIGHT);
			pi->_itype = ITYPE_NONE;
			CalcPlrInv(pnum, true);
			return true;
		}
	}

	pi = &p->InvBody[INVLOC_HAND_LEFT];
	if (p->InvBody[INVLOC_HAND_RIGHT]._itype == ITYPE_NONE && pi->_itype == ITYPE_SHIELD) {
		if (pi->_iDurability == DUR_INDESTRUCTIBLE) {
			return false;
		}

		pi->_iDurability--;
		if (pi->_iDurability == 0) {
			NetSendCmdDelItem(true, INVLOC_HAND_LEFT);
			pi->_itype = ITYPE_NONE;
			CalcPlrInv(pnum, true);
			return true;
		}
	}

	return false;
}

static bool PlrHitMonst(int pnum, int sn, int sl, int mnum)
{
	PlayerStruct *p;
	MonsterStruct *mon;
	bool ret;
	int hper, tmac, dam, skdam, damsl, dambl, dampc;

	if ((unsigned)mnum >= MAXMONSTERS) {
		app_fatal("PlrHitMonst: illegal monster %d", mnum);
	}

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("PlrHitMonst: illegal player %d", pnum);
	}

	p = &plr[pnum];
	mon = &monster[mnum];

	tmac = mon->mArmorClass;
	if (p->_pIEnAc > 0) {
		int _pIEnAc = p->_pIEnAc - 1;
		if (_pIEnAc > 0)
			tmac >>= _pIEnAc;
		else
			tmac -= tmac >> 2;
	}
	hper = p->_pIHitChance - tmac;
	if (sn == SPL_SWIPE) {
		hper -= 30 - sl * 2;
	}
	if (random_(4, 100) >= hper && mon->_mmode != MM_STONE)
#ifdef _DEBUG
		if (!debug_mode_god_mode)
#endif
			return false;

	if (CheckMonsterHit(mnum, &ret))
		return ret;

	dam = 0;
	damsl = p->_pISlMaxDam;
	if (damsl != 0)
		dam += CalcMonsterDam(mon->mMagicRes, MISR_SLASH, p->_pISlMinDam, damsl);
	dambl = p->_pIBlMaxDam;
	if (dambl != 0)
		dam += CalcMonsterDam(mon->mMagicRes, MISR_BLUNT, p->_pIBlMinDam, dambl);
	dampc = p->_pIPcMaxDam;
	if (dampc != 0)
		dam += CalcMonsterDam(mon->mMagicRes, MISR_PUNCTURE, p->_pIPcMinDam, dampc);

	if (random_(6, 200) < p->_pICritChance) {
		dam <<= 1;
	}

	if (sn == SPL_SWIPE) {
		dam = (dam * (48 + sl)) >> 6;
	}
	if (p->_pILifeSteal != 0) {
		skdam = (dam * p->_pILifeSteal) >> 7;
		PlrIncHp(pnum, skdam);
	}
	if (p->_pIManaSteal != 0) {
		skdam = (dam * p->_pIManaSteal) >> 7;
		PlrIncMana(pnum, skdam);
	}

	int fdam = p->_pIFMaxDam;
	if (fdam != 0)
		fdam = CalcMonsterDam(mon->mMagicRes, MISR_FIRE, p->_pIFMinDam, fdam);
	int ldam = p->_pILMaxDam;
	if (ldam != 0)
		ldam = CalcMonsterDam(mon->mMagicRes, MISR_LIGHTNING, p->_pILMinDam, ldam);
	int mdam = p->_pIMMaxDam;
	if (mdam != 0)
		mdam = CalcMonsterDam(mon->mMagicRes, MISR_MAGIC, p->_pIMMinDam, mdam);
	int adam = 0;
	if (adam != 0)
		adam = CalcMonsterDam(mon->mMagicRes, MISR_ACID, p->_pIAMinDam, adam);

	if ((fdam | ldam | mdam | adam) != 0) {
		dam += fdam + ldam + mdam + adam;
		AddElementalExplosion(mon->_mx, mon->_my, fdam, ldam, mdam, adam);
	}

	if (pnum == myplr) {
		mon->_mhitpoints -= dam;
	}

	if (mon->_mhitpoints < (1 << 6)) {
		MonStartKill(mnum, pnum);
	} else {
		if (p->_pIFlags & ISPL_NOHEALMON) {
			mon->_mFlags |= MFLAG_NOHEAL;
		}
		if ((p->_pIFlags & ISPL_KNOCKBACK) && mon->_mmode != MM_STONE) {
			MonGetKnockback(mnum);
		}
		MonStartHit(mnum, pnum, dam);
	}
	return true;
}

static bool PlrHitPlr(int offp, int sn, int sl, int defp)
{
	PlayerStruct *ops, *dps;
	int hper, blkper, dam, damsl, dambl, dampc;

	if ((unsigned)defp >= MAX_PLRS) {
		app_fatal("PlrHitPlr: illegal target player %d", defp);
	}

	dps = &plr[defp];
	if (dps->_pInvincible) {
		return false;
	}

	if ((unsigned)offp >= MAX_PLRS) {
		app_fatal("PlrHitPlr: illegal attacking player %d", offp);
	}
	ops = &plr[offp];
	hper = ops->_pIHitChance - dps->_pIAC;
	if (sn == SPL_SWIPE) {
		hper -= 30 - sl * 2;
	}

	if (hper < 5)
		hper = 5;
	if (hper > 95)
		hper = 95;
	if (random_(4, 100) >= hper)
		return false;

	blkper = dps->_pIBlockChance;
	if (blkper != 0
	 && (dps->_pmode == PM_STAND || dps->_pmode == PM_BLOCK)) {
		// assert(dps->_pSkillFlags & SFLAG_BLOCK);
		blkper = blkper - (ops->_pLevel << 1);
		if (blkper > random_(5, 100)) {
			PlrStartBlock(defp, GetDirection(dps->_px, dps->_py, ops->_px, ops->_py));
			return true;
		}
	}

	dam = 0;
	damsl = ops->_pISlMaxDam;
	if (damsl != 0)
		dam += CalcPlrDam(dps, MISR_SLASH, ops->_pISlMinDam, damsl);
	dambl = ops->_pIBlMaxDam;
	if (dambl != 0)
		dam += CalcPlrDam(dps, MISR_BLUNT, ops->_pIBlMinDam, dambl);
	dampc = ops->_pIPcMaxDam;
	if (dampc != 0)
		dam += CalcPlrDam(dps, MISR_PUNCTURE, ops->_pIPcMinDam, dampc);

	if (random_(6, 200) < ops->_pICritChance) {
		dam <<= 1;
	}

	if (sn == SPL_SWIPE) {
		dam = (dam * (48 + sl)) >> 6;
	}
	if (ops->_pILifeSteal != 0) {
		PlrIncHp(offp, (dam * ops->_pILifeSteal) >> 7);
	}

	int fdam = ops->_pIFMaxDam;
	if (fdam != 0) {
		fdam = CalcPlrDam(dps, MISR_FIRE, ops->_pIFMinDam, fdam);
	}
	int ldam = ops->_pILMaxDam;
	if (ldam != 0) {
		ldam = CalcPlrDam(dps, MISR_LIGHTNING, ops->_pILMinDam, ldam);
	}
	int mdam = ops->_pIMMaxDam;
	if (mdam != 0) {
		mdam = CalcPlrDam(dps, MISR_LIGHTNING, ops->_pIMMinDam, mdam);
	}
	int adam = ops->_pIAMaxDam;
	if (adam != 0) {
		adam = CalcPlrDam(dps, MISR_ACID, ops->_pIAMinDam, adam);
	}
	if ((fdam | ldam | mdam | adam) != 0) {
		dam += fdam + ldam + mdam + adam;
		AddElementalExplosion(dps->_px, dps->_py, fdam, ldam, mdam, adam);
	}

	if (offp == myplr)
		NetSendCmdDwParam2(true, CMD_PLRDAMAGE, defp, dam);
	StartPlrHit(defp, dam, false);
	return true;
}

static bool PlrTryHit(int pnum, int sn, int sl, int dx, int dy)
{
	int mpo;

	mpo = dMonster[dx][dy];
	if (mpo != 0) {
		mpo = mpo >= 0 ? mpo - 1 : -(mpo + 1);
		return !CanTalkToMonst(mpo) && PlrHitMonst(pnum, sn, sl, mpo);
	}
	mpo = dPlayer[dx][dy];
	if (mpo != 0 && plr[myplr]._pTeam != plr[mpo]._pTeam) {
		mpo = mpo >= 0 ? mpo - 1 : -(mpo + 1);
		return PlrHitPlr(pnum, sn, sl, mpo);
	}
	mpo = dObject[dx][dy];
	if (mpo != 0) {
		mpo = mpo >= 0 ? mpo - 1 : -(mpo + 1);
		if (object[mpo]._oBreak == 1) {
			OperateObject(pnum, mpo, false);
			return true;
		}
	}
	return false;
}

static bool PlrDoAttack(int pnum)
{
	PlayerStruct *p;
	int dir, hitcnt;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("PlrDoAttack: illegal player %d", pnum);
	}

	p = &plr[pnum];
	p->_pVar8++;
	if (p->_pIFlags & ISPL_FASTESTATTACK) {
		PlrStepAnim(pnum);
	} else if (p->_pIFlags & ISPL_FASTERATTACK) {
		if ((p->_pVar8 & 1) == 1)
			PlrStepAnim(pnum);
	} else if (p->_pIFlags & ISPL_FASTATTACK) {
		if ((p->_pVar8 & 3) == 2)
			PlrStepAnim(pnum);
	} else if (p->_pIFlags & ISPL_QUICKATTACK) {
		if ((p->_pVar8 & 7) == 4)
			PlrStepAnim(pnum);
	}
	if (p->_pAnimFrame < p->_pAFNum - 1)
		return false;
	if (p->_pVar7 == 0) {
		p->_pVar7++;
		PlaySfxLoc(PS_SWING, p->_px, p->_py, 2);
	}
	if (p->_pAnimFrame == p->_pAFNum - 1) {
		return false;
	}
	dir = p->_pdir;
	if (p->_pVar7 == 1) {
		p->_pVar7++;

		if (HasMana(pnum, p->_pVar5, SPLFROM_MANA)) {
			UseMana(pnum, p->_pVar5, SPLFROM_MANA);
			hitcnt = PlrTryHit(pnum, p->_pVar5, p->_pVar6,
				p->_px + offset_x[dir], p->_py + offset_y[dir]);
			if (p->_pVar5 == SPL_SWIPE) {
				hitcnt += PlrTryHit(pnum, SPL_SWIPE, p->_pVar6,
					p->_px + offset_x[(dir + 1) % 8], p->_py + offset_y[(dir + 1) % 8]);

				hitcnt += PlrTryHit(pnum, SPL_SWIPE, p->_pVar6,
					p->_px + offset_x[(dir + 7) % 8], p->_py + offset_y[(dir + 7) % 8]);
			}

			if (hitcnt != 0 && WeaponDur(pnum, 16 + hitcnt * 16)) {
				PlrStartStand(pnum, dir);
				ClearPlrPVars(pnum);
				return true;
			}
		}
	}

	if (p->_pAnimFrame < p->_pAFrames)
		return false;

	PlrStartStand(pnum, dir);
	ClearPlrPVars(pnum);
	return true;
}

static bool PlrDoRangeAttack(int pnum)
{
	PlayerStruct *p;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("PlrDoRangeAttack: illegal player %d", pnum);
	}

	p = &plr[pnum];
	p->_pVar8++;
	if (p->_pIFlags & ISPL_FASTESTATTACK) {
		PlrStepAnim(pnum);
	} else if (p->_pIFlags & ISPL_FASTERATTACK) {
		if ((p->_pVar8 & 1) == 1)
			PlrStepAnim(pnum);
	} else if (p->_pIFlags & ISPL_FASTATTACK) {
		if ((p->_pVar8 & 3) == 2)
			PlrStepAnim(pnum);
	} else if (p->_pIFlags & ISPL_QUICKATTACK) {
		if ((p->_pVar8 & 7) == 4)
			PlrStepAnim(pnum);
	}
	if (p->_pAnimFrame < p->_pAFNum)
		return false;

	if (p->_pVar7 == 0) {
		p->_pVar7++;
		if (HasMana(pnum, p->_pVar5, SPLFROM_MANA)
		 && AddMissile(p->_px, p->_py, p->_pVar1, p->_pVar2, p->_pdir,
			 spelldata[p->_pVar5].sMissile, 0, pnum, 0, 0, p->_pVar6)) {
			UseMana(pnum, p->_pVar5, SPLFROM_MANA);

			if (WeaponDur(pnum, 40)) {
				PlrStartStand(pnum, p->_pdir);
				ClearPlrPVars(pnum);
				return true;
			}
		}
	}

	if (p->_pAnimFrame < p->_pAFrames)
		return false;

	PlrStartStand(pnum, p->_pdir);
	ClearPlrPVars(pnum);
	return true;
}

static void ShieldDur(int pnum)
{
	ItemStruct *pi;

	if (pnum != myplr) {
		return;
	}

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("ShieldDur: illegal player %d", pnum);
	}

	pi = &plr[pnum].InvBody[INVLOC_HAND_LEFT];
	if (pi->_itype == ITYPE_SHIELD) {
		if (pi->_iDurability == DUR_INDESTRUCTIBLE) {
			return;
		}

		pi->_iDurability--;
		if (pi->_iDurability == 0) {
			NetSendCmdDelItem(true, INVLOC_HAND_LEFT);
			pi->_itype = ITYPE_NONE;
			CalcPlrInv(pnum, true);
		}
	}

	pi = &plr[pnum].InvBody[INVLOC_HAND_RIGHT];
	if (pi->_itype == ITYPE_SHIELD) {
		if (pi->_iDurability != DUR_INDESTRUCTIBLE) {
			pi->_iDurability--;
			if (pi->_iDurability == 0) {
				NetSendCmdDelItem(true, INVLOC_HAND_RIGHT);
				pi->_itype = ITYPE_NONE;
				CalcPlrInv(pnum, true);
			}
		}
	}
}

void PlrStartBlock(int pnum, int dir)
{
	PlayerStruct *p;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrStartBlock: illegal player %d", pnum);
	}

	p = &plr[pnum];
	if (p->_pHitPoints < (1 << 6)) {
		SyncPlrKill(pnum, -1); // BUGFIX: is this really necessary?
		return;
	}

	if (p->_pmode != PM_BLOCK)
		StartBlock(pnum, dir);

	PlaySfxLoc(IS_ISWORD, p->_px, p->_py);
	if (random_(3, 10) == 0) {
		ShieldDur(pnum);
	}
}

static bool PlrDoBlock(int pnum)
{
	PlayerStruct *p;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("PlrDoBlock: illegal player %d", pnum);
	}
	p = &plr[pnum];
	if (p->_pIFlags & ISPL_FASTBLOCK && p->_pAnimCnt == 0 && p->_pAnimFrame != 1) {
		PlrStepAnim(pnum);
	}

	if (p->_pAnimFrame >= p->_pBFrames) {
		if (p->_pVar1 == 0) {
			if (p->destAction == ACTION_BLOCK) {
				// extend the blocking animation TODO: does not work with too fast animations (WARRIORs)
				p->destAction = ACTION_NONE;
				p->_pAnimData = p->_pBAnim[p->destParam1];
				// _pVar1 = _pBFrames * (BASE_DELAY + 1) / 2 (+ 1)
				p->_pVar1 = p->_pBFrames + (p->_pBFrames >> 1) + 1;
				p->_pAnimDelay = p->_pVar1;
				p->_pAnimCnt = 0;
			} else {
				PlrStartStand(pnum, p->_pdir);
				//ClearPlrPVars(pnum);
				return true;
			}
		}
		p->_pVar1--;
	}

	return false;
}

static void ArmorDur(int pnum)
{
	ItemStruct *pi, *pio;
	inv_body_loc loc;

	if (pnum != myplr) {
		return;
	}

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("ArmorDur: illegal player %d", pnum);
	}

	loc = INVLOC_CHEST;
	pi = &plr[pnum].InvBody[INVLOC_CHEST];
	pio = &plr[pnum].InvBody[INVLOC_HEAD];
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

	if (pi->_iDurability == DUR_INDESTRUCTIBLE) {
		return;
	}

	pi->_iDurability--;
	if (pi->_iDurability != 0) {
		return;
	}

	NetSendCmdDelItem(true, loc);
	pi->_itype = ITYPE_NONE;
	CalcPlrInv(pnum, true);
}

static bool PlrDoSpell(int pnum)
{
	PlayerStruct *p;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("PlrDoSpell: illegal player %d", pnum);
	}
	p = &plr[pnum];
	p->_pVar8++;
	if (p->_pIFlags2 & ISPH_FASTESTCAST) {
		PlrStepAnim(pnum);
	} else if (p->_pIFlags2 & ISPH_FASTERCAST) {
		if ((p->_pVar8 & 1) == 1)
			PlrStepAnim(pnum);
	} else if (p->_pIFlags2 & ISPH_FASTCAST) {
		if ((p->_pVar8 & 3) == 2)
			PlrStepAnim(pnum);
	}

	if (p->_pAnimFrame < p->_pSFNum)
		return false;

	if (!p->_pVar7) {
		p->_pVar7 = TRUE;

		if (HasMana(pnum, p->_pVar3, p->_pVar5)
		 && AddMissile(p->_px, p->_py, p->_pVar1, p->_pVar2, p->_pdir,
				spelldata[p->_pVar3].sMissile, 0, pnum, 0, 0, p->_pVar4) != -1) {
			UseMana(pnum, p->_pVar3, p->_pVar5);
		}
	}

	if (p->_pAnimFrame < p->_pSFrames)
		return false;

	PlrStartStand(pnum, p->_pdir);
	ClearPlrPVars(pnum);
	return true;
}

static bool PlrDoGotHit(int pnum)
{
	PlayerStruct *p;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("PlrDoGotHit: illegal player %d", pnum);
	}
	p = &plr[pnum];
	p->_pVar8++;
	if (p->_pIFlags & ISPL_FASTESTRECOVER) {
		PlrStepAnim(pnum);
	} else if (p->_pIFlags & ISPL_FASTERRECOVER) {
		if ((p->_pVar8 & 1) == 1)
			PlrStepAnim(pnum);
	} else if (p->_pIFlags & ISPL_FASTRECOVER) {
		if ((p->_pVar8 & 3) == 2)
			PlrStepAnim(pnum);
	}

	if (p->_pAnimFrame < p->_pHFrames)
		return false;
	PlrStartStand(pnum, p->_pdir);
	ClearPlrPVars(pnum);
	if (random_(3, 4) != 0) {
		ArmorDur(pnum);
	}

	return true;
}

static bool PlrDoDeath(int pnum)
{
	PlayerStruct *p;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("PlrDoDeath: illegal player %d", pnum);
	}

	p = &plr[pnum];
	if ((unsigned)p->_pVar8 >= 2 * p->_pDFrames) {
		if (deathdelay > 1 && pnum == myplr) {
			deathdelay--;
			if (deathdelay == 1) {
				gbDeathflag = true;
				if (gbMaxPlayers == 1) {
					gamemenu_on();
				}
			}
		}

		p->_pAnimDelay = 10000;
		p->_pAnimFrame = p->_pAnimLen;
		dFlags[p->_px][p->_py] |= BFLAG_DEAD_PLAYER;
	} else {
		p->_pVar8++;
	}

	return false;
}

static bool PlrDoNewLvl(int pnum)
{
	return false;
}

static void CheckNewPath(int pnum)
{
	PlayerStruct *p;
	int i, x, y;
	int xvel3, xvel, yvel;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("CheckNewPath: illegal player %d", pnum);
	}
	p = &plr[pnum];
	if (p->_pHitPoints < (1 << 6)) {
		SyncPlrKill(pnum, -1); // BUGFIX: is this really necessary?
		return;
	}

	if (p->destAction == ACTION_ATTACKMON) {
		MakePlrPath(pnum, monster[p->destParam1]._mfutx, monster[p->destParam1]._mfuty, false);
	} else if (p->destAction == ACTION_ATTACKPLR) {
		MakePlrPath(pnum, plr[p->destParam1]._pfutx, plr[p->destParam1]._pfuty, false);
	}

	if (p->walkpath[0] != WALK_NONE) {
		if (p->_pmode == PM_STAND) {
			if (pnum == myplr) {
				if (p->destAction == ACTION_ATTACKMON || p->destAction == ACTION_ATTACKPLR) {
					if (StartAttack(pnum)) {
						ClrPlrPath(pnum);
						p->destAction = ACTION_NONE;
						return;
					}
				}
			}

			if (currLvl._dLevelIdx != DLV_TOWN) {
				xvel3 = PWVel[p->_pClass][0];
				xvel = PWVel[p->_pClass][1];
				yvel = PWVel[p->_pClass][2];
			} else {
				xvel3 = 2048;
				xvel = 1024;
				yvel = 512;
			}

			switch (p->walkpath[0]) {
			case WALK_N:
				StartWalk(pnum, 0, -xvel, -1, -1, DIR_N, SDIR_N);
				break;
			case WALK_NE:
				StartWalk(pnum, xvel, -yvel, 0, -1, DIR_NE, SDIR_NE);
				break;
			case WALK_E:
				StartWalk3(pnum, xvel3, 0, -32, -16, 1, -1, 1, 0, DIR_E, SDIR_E);
				break;
			case WALK_SE:
				StartWalk2(pnum, xvel, yvel, -32, -16, 1, 0, DIR_SE, SDIR_SE);
				break;
			case WALK_S:
				StartWalk2(pnum, 0, xvel, 0, -32, 1, 1, DIR_S, SDIR_S);
				break;
			case WALK_SW:
				StartWalk2(pnum, -xvel, yvel, 32, -16, 0, 1, DIR_SW, SDIR_SW);
				break;
			case WALK_W:
				StartWalk3(pnum, -xvel3, 0, 32, -16, -1, 1, 0, 1, DIR_W, SDIR_W);
				break;
			case WALK_NW:
				StartWalk(pnum, -xvel, -yvel, -1, 0, DIR_NW, SDIR_NW);
				break;
			default:
				ASSUME_UNREACHABLE
				break;
			}

			for (i = 0; i < MAX_PATH_LENGTH - 1; i++) {
				p->walkpath[i] = p->walkpath[i + 1];
			}

			p->walkpath[MAX_PATH_LENGTH - 1] = WALK_NONE;

			if (p->_pmode == PM_STAND) {
				PlrStartStand(pnum, p->_pdir);
				p->destAction = ACTION_NONE;
			}
		}

		return;
	}
	if (p->destAction == ACTION_NONE) {
		return;
	}

	if (p->_pmode == PM_STAND) {
		switch (p->destAction) {
		case ACTION_ATTACK:
		case ACTION_ATTACKMON:
		case ACTION_ATTACKPLR:
		case ACTION_OPERATE:
		case ACTION_DISARM:
			StartAttack(pnum);
			break;
		case ACTION_RATTACK:
		case ACTION_RATTACKMON:
		case ACTION_RATTACKPLR:
			StartRangeAttack(pnum);
			break;
		/*case ACTION_SPELLWALL:
			StartSpell(pnum, p->destParam3, p->destParam1, p->destParam2);
			p->_pVar3 = p->destParam3;
			p->_pVar4 = p->destParam4;
			break;*/
		case ACTION_SPELL:
		case ACTION_SPELLMON:
		case ACTION_SPELLPLR:
			StartSpell(pnum);
			break;
		case ACTION_PICKUPITEM:
			if (pnum == myplr) {
				i = p->destParam1;
				x = abs(p->_px - items[i]._ix);
				y = abs(p->_py - items[i]._iy);
				if (x <= 1 && y <= 1 && pcurs == CURSOR_HAND && !items[i]._iRequest) {
					NetSendCmdGItem(true, CMD_REQUESTGITEM, myplr, myplr, i);
					items[i]._iRequest = TRUE;
				}
			}
			break;
		case ACTION_PICKUPAITEM:
			if (pnum == myplr) {
				i = p->destParam1;
				x = abs(p->_px - items[i]._ix);
				y = abs(p->_py - items[i]._iy);
				if (x <= 1 && y <= 1 && pcurs == CURSOR_HAND) {
					NetSendCmdGItem(true, CMD_REQUESTAGITEM, myplr, myplr, i);
				}
			}
			break;
		case ACTION_TALK:
			if (pnum == myplr) {
				TalkToTowner(pnum, p->destParam1);
			}
			break;
		case ACTION_OPERATETK:
			i = p->destParam1;
			OperateObject(pnum, i, true);
			break;
		case ACTION_BLOCK:
			StartBlock(pnum, p->destParam1);
			break;
		case ACTION_WALK:
			break;
		default:
			ASSUME_UNREACHABLE
		}

		FixPlayerLocation(pnum);
		p->destAction = ACTION_NONE;

		return;
	}

	if (p->_pmode == PM_ATTACK && p->_pAnimFrame > p->_pAFNum) {
		if (p->destAction == ACTION_ATTACK
		 || p->destAction == ACTION_ATTACKMON
		 || p->destAction == ACTION_ATTACKPLR
		 || p->destAction == ACTION_OPERATE
		 || p->destAction == ACTION_DISARM) {
			StartAttack(pnum);
			p->destAction = ACTION_NONE;
		}
	} else if (p->_pmode == PM_RATTACK && p->_pAnimFrame > p->_pAFNum) {
		if (p->destAction == ACTION_RATTACK
		 || p->destAction == ACTION_RATTACKMON
		 || p->destAction == ACTION_RATTACKPLR) {
			StartRangeAttack(pnum);
			p->destAction = ACTION_NONE;
		}
	} else if (p->_pmode == PM_SPELL && p->_pAnimFrame > p->_pSFNum) {
		if (p->destAction == ACTION_SPELL
		 || p->destAction == ACTION_SPELLMON
		 || p->destAction == ACTION_SPELLPLR) {
			StartSpell(pnum);
			p->destAction = ACTION_NONE;
		}
	}
}

static bool PlrDeathModeOK(int pnum)
{
	PlayerStruct *p;

	if (pnum != myplr) {
		return true;
	}

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("PlrDeathModeOK: illegal player %d", pnum);
	}

	p = &plr[pnum];
	return p->_pmode == PM_DEATH || p->_pmode == PM_QUIT || p->_pmode == PM_NEWLVL;
}

static void ValidatePlayer()
{
	PlayerStruct *p;
	ItemStruct *pi;
	uint64_t msk;
	int gt, i;

	if ((unsigned)myplr >= MAX_PLRS) {
		app_fatal("ValidatePlayer: illegal player %d", myplr);
	}
	p = &plr[myplr];
	if (p->_pLevel > MAXCHARLEVEL)
		p->_pLevel = MAXCHARLEVEL;
	if (p->_pExperience > p->_pNextExper)
		p->_pExperience = p->_pNextExper;

	gt = 0;
	pi = p->InvList;
	for (i = p->_pNumInv; i != 0; i--, pi++) {
		if (pi->_itype == ITYPE_GOLD) {
			if (pi->_ivalue > GOLD_MAX_LIMIT)
				pi->_ivalue = GOLD_MAX_LIMIT;
			gt += pi->_ivalue;
		}
	}
	p->_pGold = gt;

	msk = 0;
	for (i = 1; i < NUM_SPELLS; i++) {
		if (spelldata[i].sBookLvl != SPELL_NA) {
			msk |= SPELL_MASK(i);
			if (p->_pSkillLvl[i] > MAXSPLLEVEL)
				p->_pSkillLvl[i] = MAXSPLLEVEL;
		}
	}

	p->_pMemSkills &= msk;
}

static void CheckCheatStats(int pnum)
{
	if (plr[pnum]._pStrength > 750) {
		plr[pnum]._pStrength = 750;
	}

	if (plr[pnum]._pDexterity > 750) {
		plr[pnum]._pDexterity = 750;
	}

	if (plr[pnum]._pMagic > 750) {
		plr[pnum]._pMagic = 750;
	}

	if (plr[pnum]._pVitality > 750) {
		plr[pnum]._pVitality = 750;
	}

	if (plr[pnum]._pHitPoints > 128000) {
		plr[pnum]._pHitPoints = 128000;
	}

	if (plr[pnum]._pMana > 128000) {
		plr[pnum]._pMana = 128000;
	}
}

void ProcessPlayers()
{
	int pnum;
	bool raflag;

	if ((unsigned)myplr >= MAX_PLRS) {
		app_fatal("ProcessPlayers: illegal player %d", myplr);
	}

	if (plr[myplr].pLvlLoad > 0) {
		plr[myplr].pLvlLoad--;
	}

	if (sfxdelay > 0) {
		sfxdelay--;
		if (sfxdelay == 0) {
#ifdef HELLFIRE
			switch (sfxdnum) {
			case USFX_DEFILER1:
				InitQTextMsg(TEXT_DEFILER1);
				break;
			case USFX_DEFILER2:
				InitQTextMsg(TEXT_DEFILER2);
				break;
			case USFX_DEFILER3:
				InitQTextMsg(TEXT_DEFILER3);
				break;
			case USFX_DEFILER4:
				InitQTextMsg(TEXT_DEFILER4);
				break;
			default:
				PlaySFX(sfxdnum);
			}
#else
				PlaySFX(sfxdnum);
#endif
		}
	}

	ValidatePlayer();

	for (pnum = 0; pnum < MAX_PLRS; pnum++) {
		if (plr[pnum].plractive && currLvl._dLevelIdx == plr[pnum].plrlevel && (pnum == myplr || !plr[pnum]._pLvlChanging)) {
			CheckCheatStats(pnum);

			if (!PlrDeathModeOK(pnum) && plr[pnum]._pHitPoints < (1 << 6)) {
				SyncPlrKill(pnum, -1);
			}

			if (pnum == myplr) {
				if ((plr[pnum]._pIFlags & ISPL_DRAINLIFE) && currLvl._dLevelIdx != DLV_TOWN && !plr[pnum]._pInvincible) {
					PlrDecHp(pnum, 4, 0);
				}
				if (plr[pnum]._pIFlags & ISPL_NOMANA && plr[pnum]._pMana > 0) {
					PlrSetMana(pnum, 0);
				}
			}

			do {
				switch (plr[pnum]._pmode) {
				case PM_STAND:
					raflag = PlrDoStand(pnum);
					break;
				case PM_WALK:
				case PM_WALK2:
				case PM_WALK3:
					raflag = PlrDoWalk(pnum);
					break;
				case PM_ATTACK:
					raflag = PlrDoAttack(pnum);
					break;
				case PM_RATTACK:
					raflag = PlrDoRangeAttack(pnum);
					break;
				case PM_BLOCK:
					raflag = PlrDoBlock(pnum);
					break;
				case PM_SPELL:
					raflag = PlrDoSpell(pnum);
					break;
				case PM_GOTHIT:
					raflag = PlrDoGotHit(pnum);
					break;
				case PM_DEATH:
					raflag = PlrDoDeath(pnum);
					break;
				case PM_NEWLVL:
					raflag = PlrDoNewLvl(pnum);
					break;
				case PM_QUIT:
					raflag = false; // should not happen (at the moment)
					break;
				default:
					ASSUME_UNREACHABLE
					raflag = false;
				}
				CheckNewPath(pnum);
			} while (raflag);

			plr[pnum]._pAnimCnt++;
			//plr[pnum]._pAnimGameTicksSinceSequenceStarted++; ANIM_GAMELOGIC
			if (plr[pnum]._pAnimCnt > plr[pnum]._pAnimDelay) {
				plr[pnum]._pAnimCnt = 0;
				plr[pnum]._pAnimFrame++;
				if (plr[pnum]._pAnimFrame > plr[pnum]._pAnimLen) {
					plr[pnum]._pAnimFrame = 1;
					//plr[pnum]._pAnimGameTicksSinceSequenceStarted = 0; ANIM_GAMELOGIC
				}
			}
		}
	}
}

/**
 * @brief Calculates the Frame to use for the Animation rendering ANIM_GAMELOGIC
 * @param pPlayer Player
 * @return The Frame to use for rendering
 */
/*int GetFrameToUseForPlayerRendering(int pnum)
{
	PlayerStruct *p;
	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("GetFrameToUseForPlayerRendering: illegal player %d", pnum);
	}

	p = &plr[pnum];
	// Normal logic is used,
	// - if no frame-skipping is required and so we have exactly one Animationframe per GameTick (_pAnimUsedNumFrames = 0)
	// or
	// - if we load from a savegame where the new variables are not stored (we don't want to break savegame compatiblity because of smoother rendering of one animation)
	if (p->_pAnimNumSkippedFrames <= 0)
		return p->_pAnimFrame;
	// After an attack hits (_pAFNum or _pSFNum) it can be canceled or another attack can be queued and this means the animation is canceled.
	// In normal attacks frame skipping always happens before the attack actual hit.
	// This has the advantage that the sword or bow always points to the enemy when the hit happens (_pAFNum or _pSFNum).
	// Our distribution logic must also regard this behaviour, so we are not allowed to distribute the skipped animations after the actual hit (_pAnimStopDistributingAfterFrame).
	int relevantAnimationLength;
	if (p->_pAnimStopDistributingAfterFrame != 0) {
		if (p->_pAnimFrame >= p->_pAnimStopDistributingAfterFrame)
			return p->_pAnimFrame;
		relevantAnimationLength = p->_pAnimStopDistributingAfterFrame - 1;
	} else {
		relevantAnimationLength = p->_pAnimLen;
	}
	float progressToNextGameTick = gfProgressToNextGameTick;
	float totalGameTicksForCurrentAnimationSequence = progressToNextGameTick + (float)p->_pAnimGameTicksSinceSequenceStarted; // we don't use the processed game ticks alone but also the fragtion of the next game tick (if a rendering happens between game ticks). This helps to smooth the animations.
	int animationMaxGameTickets = relevantAnimationLength;
	if (p->_pAnimDelay > 1)
		animationMaxGameTickets = (relevantAnimationLength * p->_pAnimDelay);
	float gameTickModifier = (float)animationMaxGameTickets / (float)(relevantAnimationLength - p->_pAnimNumSkippedFrames); // if we skipped Frames we need to expand the GameTicks to make one GameTick for this Animation "faster"
	int absolutAnimationFrame = 1 + (int)(totalGameTicksForCurrentAnimationSequence * gameTickModifier); // 1 added for rounding reasons. float to int cast always truncate.
	if (absolutAnimationFrame > relevantAnimationLength) // this can happen if we are at the last frame and the next game tick is due (nthread_GetProgressToNextGameTick returns 1.0f)
		return relevantAnimationLength;
	if (absolutAnimationFrame <= 0) {
		SDL_Log("GetFrameToUseForPlayerRendering: Calculated an invalid Animation Frame");
		return 1;
	}
	return absolutAnimationFrame;
}*/

void ClrPlrPath(int pnum)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("ClrPlrPath: illegal player %d", pnum);
	}

	memset(plr[pnum].walkpath, WALK_NONE, sizeof(plr[pnum].walkpath));
}

bool PosOkPlayer(int pnum, int x, int y)
{
	int mpo;

	if (IN_DUNGEON_AREA(x, y) && dPiece[x][y] != 0 && !nSolidTable[dPiece[x][y]]) {
		mpo = dPlayer[x][y];
		if (mpo != 0) {
			mpo = mpo >= 0 ? mpo - 1 : -(mpo + 1);
			if (mpo != pnum && plr[mpo]._pHitPoints >= (1 << 6)) {
				return false;
			}
		}
		mpo = dMonster[x][y];
		if (mpo != 0) {
			if (mpo < 0) {
				return false;
			}
			if (monster[mpo - 1]._mhitpoints >= (1 << 6)) {
				return false;
			}
			if (currLvl._dLevelIdx == DLV_TOWN) {
				return false;
			}
		}
		mpo = dObject[x][y];
		if (mpo != 0) {
			mpo = mpo >= 0 ? mpo - 1 : -(mpo + 1);
			if (object[mpo]._oSolidFlag) {
				return false;
			}
		}

		return true;
	}

	return false;
}

void MakePlrPath(int pnum, int xx, int yy, bool endspace)
{
	int path;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("MakePlrPath: illegal player %d", pnum);
	}

	if (plr[pnum]._pfutx == xx && plr[pnum]._pfuty == yy) {
		return;
	}

	path = FindPath(PosOkPlayer, pnum, plr[pnum]._pfutx, plr[pnum]._pfuty, xx, yy, plr[pnum].walkpath);
	if (path == 0) {
		return;
	}

	if (!endspace) {
		path--;

		switch (plr[pnum].walkpath[path]) {
		case WALK_NE:
			yy++;
			break;
		case WALK_NW:
			xx++;
			break;
		case WALK_SE:
			xx--;
			break;
		case WALK_SW:
			yy--;
			break;
		case WALK_N:
			xx++;
			yy++;
			break;
		case WALK_E:
			xx--;
			yy++;
			break;
		case WALK_S:
			xx--;
			yy--;
			break;
		case WALK_W:
			xx++;
			yy--;
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
	}

	plr[pnum].walkpath[path] = WALK_NONE;
}

void SyncPlrAnim(int pnum)
{
	PlayerStruct *p;
	int sType;
	unsigned char** anim;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("SyncPlrAnim: illegal player %d", pnum);
	}
	p = &plr[pnum];
	switch (p->_pmode) {
	case PM_STAND:
		anim = p->_pNAnim;
		break;
	case PM_WALK:
	case PM_WALK2:
	case PM_WALK3:
		anim = p->_pWAnim;
		break;
	case PM_ATTACK:
	case PM_RATTACK:
		anim = p->_pAAnim;
		break;
	case PM_BLOCK:
		anim = p->_pBAnim;
		break;
	case PM_GOTHIT:
		anim = p->_pHAnim;
		break;
	case PM_DEATH:
		anim = p->_pDAnim;
		break;
	case PM_SPELL:
		if (pnum == myplr)
			sType = spelldata[p->_pVar3].sType;
		else
			sType = STYPE_FIRE;
		switch (sType) {
		case STYPE_FIRE:      anim = p->_pFAnim; break;
		case STYPE_LIGHTNING: anim = p->_pLAnim; break;
		case STYPE_MAGIC:     anim = p->_pTAnim; break;
		default: ASSUME_UNREACHABLE;
		}
		break;
	case PM_NEWLVL:
	case PM_QUIT:
		anim = p->_pNAnim;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	p->_pAnimData = anim[p->_pdir];
}

void SyncInitPlrPos(int pnum)
{
	assert(plr[pnum].plrlevel == currLvl._dLevelIdx);

	if (PlacePlayer(pnum)) {
		RemovePlrFromMap(pnum);
		FixPlayerLocation(pnum);
	}
	dPlayer[plr[pnum]._px][plr[pnum]._py] = pnum + 1;
}

void SyncInitPlr(int pnum)
{
	/*if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("SyncInitPlr: illegal player %d", pnum);
	}*/

	//SetPlrAnims(pnum);
	InitPlayer(pnum, false, true);
	//SyncInitPlrPos(pnum);
}

/*void CheckStats(int pnum)
{
	PlayerStruct *p;
	const int *stats;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("CheckStats: illegal player %d", pnum);
	}
	p = &plr[pnum];
	stats = MaxStats[p->_pClass];
	if (p->_pBaseStr > stats[ATTRIB_STR]) {
		p->_pBaseStr = stats[ATTRIB_STR];
	} else if (p->_pBaseStr < 0) {
		p->_pBaseStr = 0;
	}
	if (p->_pBaseMag > stats[ATTRIB_MAG]) {
		p->_pBaseMag = stats[ATTRIB_MAG];
	} else if (p->_pBaseMag < 0) {
		p->_pBaseMag = 0;
	}
	if (p->_pBaseDex > stats[ATTRIB_DEX]) {
		p->_pBaseDex = stats[ATTRIB_DEX];
	} else if (p->_pBaseDex < 0) {
		p->_pBaseDex = 0;
	}
	if (p->_pBaseVit > stats[ATTRIB_VIT]) {
		p->_pBaseVit = stats[ATTRIB_VIT];
	} else if (p->_pBaseVit < 0) {
		p->_pBaseVit = 0;
	}
}*/

void PlrSetHp(int pnum, int val)
{
	PlayerStruct *p;
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrSetHp: illegal player %d", pnum);
	}
	p = &plr[pnum];
	p->_pHitPoints = val;
	p->_pHPBase = val + p->_pMaxHPBase - p->_pMaxHP;

	if (pnum == myplr)
		gbRedrawFlags |= REDRAW_HP_FLASK;
}

void PlrSetMana(int pnum, int val)
{
	PlayerStruct *p;
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrSetMana: illegal player %d", pnum);
	}
	p = &plr[pnum];
	if (p->_pIFlags & ISPL_NOMANA)
		val = 0;
	p->_pMana = val;
	p->_pManaBase = val - (p->_pMaxMana - p->_pMaxManaBase);

	if (pnum == myplr)
		gbRedrawFlags |= REDRAW_MANA_FLASK;
}

void PlrFillHp(int pnum)
{
	PlayerStruct *p;
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrFillHp: illegal player %d", pnum);
	}
	p = &plr[pnum];
	p->_pHitPoints = p->_pMaxHP;
	p->_pHPBase = p->_pMaxHPBase;
	if (pnum == myplr)
		gbRedrawFlags |= REDRAW_HP_FLASK;
}

void PlrFillMana(int pnum)
{
	PlayerStruct *p;
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrSetMana: illegal player %d", pnum);
	}
	p = &plr[pnum];
	if (p->_pIFlags & ISPL_NOMANA)
		return;
	p->_pMana = p->_pMaxMana;
	p->_pManaBase = p->_pMaxManaBase;
	if (pnum == myplr)
		gbRedrawFlags |= REDRAW_MANA_FLASK;
}

void PlrIncHp(int pnum, int hp)
{
	PlayerStruct *p;

	assert(hp >= 0);
	p = &plr[pnum];
	p->_pHitPoints += hp;
	if (p->_pHitPoints > p->_pMaxHP)
		p->_pHitPoints = p->_pMaxHP;
	p->_pHPBase += hp;
	if (p->_pHPBase > p->_pMaxHPBase)
		p->_pHPBase = p->_pMaxHPBase;
	if (pnum == myplr)
		gbRedrawFlags |= REDRAW_HP_FLASK;
}

void PlrIncMana(int pnum, int mana)
{
	PlayerStruct *p;

	assert(mana >= 0);
	p = &plr[pnum];
	if (p->_pIFlags & ISPL_NOMANA)
		return;

	p->_pMana += mana;
	if (p->_pMana > p->_pMaxMana) {
		p->_pMana = p->_pMaxMana;
	}
	p->_pManaBase += mana;
	if (p->_pManaBase > p->_pMaxManaBase) {
		p->_pManaBase = p->_pMaxManaBase;
	}
	if (pnum == myplr)
		gbRedrawFlags |= REDRAW_MANA_FLASK;
}

bool PlrDecHp(int pnum, int hp, int earflag)
{
	PlayerStruct *p;

	assert(hp >= 0);
	p = &plr[pnum];
	if (p->_pManaShield != 0) {
#ifdef HELLFIRE
		int div = 19 - (std::min((int)p->_pManaShield, 8) << 1);
#else
		int div = 3;
#endif
		hp -= hp / div;
		if (p->_pMana >= hp) {
			PlrDecMana(pnum, hp);
			return false;
		}
		hp -= p->_pMana;
		PlrSetMana(pnum, 0);
		if (pnum == myplr)
			NetSendCmd(true, CMD_REMSHIELD);
	}
	p->_pHPBase -= hp;
	p->_pHitPoints -= hp;
	if (p->_pHitPoints < (1 << 6)) {
		SyncPlrKill(pnum, earflag);
		return true;
	}
	if (pnum == myplr)
		gbRedrawFlags |= REDRAW_HP_FLASK;
	return false;
}

void PlrDecMana(int pnum, int mana)
{
	PlayerStruct *p;

	assert(mana >= 0);
	p = &plr[pnum];
	if (p->_pIFlags & ISPL_NOMANA)
		return;

	p->_pMana -= mana;
	p->_pManaBase -= mana;
	if (pnum == myplr)
		gbRedrawFlags |= REDRAW_MANA_FLASK;
}

void IncreasePlrStr(int pnum)
{
	PlayerStruct *p;
	int v;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("ModifyPlrStr: illegal player %d", pnum);
	}
	p = &plr[pnum];

	switch (p->_pClass) {
	case PC_WARRIOR:	v = (((p->_pBaseStr - StrengthTbl[PC_WARRIOR]) % 5) == 2) ? 3 : 2; break;
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
	p->_pStrength += v;
	p->_pBaseStr += v;

	CalcPlrInv(pnum, true);
}

void IncreasePlrMag(int pnum)
{
	PlayerStruct *p;
	int v, ms;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("ModifyPlrMag: illegal player %d", pnum);
	}
	p = &plr[pnum];

	switch (p->_pClass) {
	case PC_WARRIOR:	v = 1; break;
	case PC_ROGUE:		v = 2; break;
	case PC_SORCERER:	v = 3; break;
#ifdef HELLFIRE
	case PC_MONK:		v = (((p->_pBaseMag - MagicTbl[PC_MONK]) % 3) == 1) ? 2 : 1; break;
	case PC_BARD:		v = (((p->_pBaseMag - MagicTbl[PC_BARD]) % 3) == 1) ? 2 : 1; break;
	case PC_BARBARIAN:	v = 1; break;
#endif
	default:
		ASSUME_UNREACHABLE
		break;
	}

	p->_pMagic += v;
	p->_pBaseMag += v;

	ms = v << (6 + 1);

	p->_pMaxManaBase += ms;
	p->_pMaxMana += ms;
	if (!(p->_pIFlags & ISPL_NOMANA)) {
		p->_pManaBase += ms;
		p->_pMana += ms;
	}

	CalcPlrInv(pnum, true);
}

void IncreasePlrDex(int pnum)
{
	PlayerStruct *p;
	int v;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("ModifyPlrDex: illegal player %d", pnum);
	}
	p = &plr[pnum];

	switch (p->_pClass) {
	case PC_WARRIOR:	v = (((p->_pBaseDex - DexterityTbl[PC_WARRIOR]) % 3) == 1) ? 2 : 1; break;
	case PC_ROGUE:		v = 3; break;
	case PC_SORCERER:	v = (((p->_pBaseDex - DexterityTbl[PC_SORCERER]) % 3) == 1) ? 2 : 1; break;
#ifdef HELLFIRE
	case PC_MONK:		v = 2; break;
	case PC_BARD:		v = 3; break;
	case PC_BARBARIAN:	v = 1; break;
#endif
	default:
		ASSUME_UNREACHABLE
		break;
	}

	p->_pDexterity += v;
	p->_pBaseDex += v;

	CalcPlrInv(pnum, true);
}

void IncreasePlrVit(int pnum)
{
	PlayerStruct *p;
	int v, ms;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("ModifyPlrVit: illegal player %d", pnum);
	}
	p = &plr[pnum];

	switch (p->_pClass) {
	case PC_WARRIOR:	v = 2; break;
	case PC_ROGUE:		v = 1; break;
	case PC_SORCERER:	v = (((p->_pBaseVit - VitalityTbl[PC_SORCERER]) % 3) == 1) ? 2 : 1; break;
#ifdef HELLFIRE
	case PC_MONK:		v = (((p->_pBaseVit - VitalityTbl[PC_MONK]) % 3) == 1) ? 2 : 1; break;
	case PC_BARD:		v = (((p->_pBaseVit - VitalityTbl[PC_BARD]) % 3) == 1) ? 2 : 1; break;
	case PC_BARBARIAN:	v = 2; break;
#endif
	default:
		ASSUME_UNREACHABLE
		break;
	}

	p->_pVitality += v;
	p->_pBaseVit += v;

	ms = v << (6 + 1);

	p->_pHPBase += ms;
	p->_pMaxHPBase += ms;
	p->_pHitPoints += ms;
	p->_pMaxHP += ms;

	CalcPlrInv(pnum, true);
}

void RestorePlrHpVit(int pnum)
{
	PlayerStruct *p;
	int hp;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("RestorePlrHpVit: illegal player %d", pnum);
	}
	p = &plr[pnum];

	// base hp
	hp = p->_pBaseVit << (6 + 1);

	// check the delta
	hp -= p->_pMaxHPBase;
	assert(hp >= 0);

	// restore the lost hp
	p->_pMaxHPBase += hp;
	p->_pMaxHP += hp;

	// fill hp
	PlrFillHp(pnum);

	// CalcPlrInv(pnum, true);
}

void InitDungMsgs(int pnum)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("InitDungMsgs: illegal player %d", pnum);
	}

	plr[pnum].pDungMsgs = 0;
#ifdef HELLFIRE
	plr[pnum].pDungMsgs2 = 0;
#endif
}

void PlayDungMsgs()
{
	if ((unsigned)myplr >= MAX_PLRS) {
		app_fatal("PlayDungMsgs: illegal player %d", myplr);
	}

	sfxdelay = 0;

	if (gbMaxPlayers != 1)
		return;

	if (currLvl._dLevelIdx == DLV_CATHEDRAL1 && !plr[myplr]._pLvlVisited[DLV_CATHEDRAL1] && !(plr[myplr].pDungMsgs & DMSG_CATHEDRAL)) {
		sfxdelay = 40;
		sfxdnum = sgSFXSets[SFXS_PLR_97][plr[myplr]._pClass];
		plr[myplr].pDungMsgs |= DMSG_CATHEDRAL;
	} else if (currLvl._dLevelIdx == DLV_CATACOMBS1 && !plr[myplr]._pLvlVisited[DLV_CATACOMBS1] && !(plr[myplr].pDungMsgs & DMSG_CATACOMBS)) {
		sfxdelay = 40;
		sfxdnum = sgSFXSets[SFXS_PLR_96][plr[myplr]._pClass];
		plr[myplr].pDungMsgs |= DMSG_CATACOMBS;
	} else if (currLvl._dLevelIdx == DLV_CAVES1 && !plr[myplr]._pLvlVisited[DLV_CAVES1] && !(plr[myplr].pDungMsgs & DMSG_CAVES)) {
		sfxdelay = 40;
		sfxdnum = sgSFXSets[SFXS_PLR_98][plr[myplr]._pClass];
		plr[myplr].pDungMsgs |= DMSG_CAVES;
	} else if (currLvl._dLevelIdx == DLV_HELL1 && !plr[myplr]._pLvlVisited[DLV_HELL1] && !(plr[myplr].pDungMsgs & DMSG_HELL)) {
		sfxdelay = 40;
		sfxdnum = sgSFXSets[SFXS_PLR_99][plr[myplr]._pClass];
		plr[myplr].pDungMsgs |= DMSG_HELL;
	} else if (currLvl._dLevelIdx == DLV_HELL4 && !plr[myplr]._pLvlVisited[DLV_HELL4] && !(plr[myplr].pDungMsgs & DMSG_DIABLO)) { // BUGFIX: _pLvlVisited should check 16 or this message will never play
		sfxdelay = 40;
		sfxdnum = PS_DIABLVLINT;
		plr[myplr].pDungMsgs |= DMSG_DIABLO;
#ifdef HELLFIRE
	} else if (currLvl._dLevelIdx == DLV_NEST1 && !plr[myplr]._pLvlVisited[DLV_NEST1] && !(plr[myplr].pDungMsgs2 & DMSG2_DEFILER)) {
		sfxdelay = 10;
		sfxdnum = USFX_DEFILER1;
		plr[myplr].pDungMsgs2 |= DMSG2_DEFILER;
	} else if (currLvl._dLevelIdx == DLV_NEST3 && !plr[myplr]._pLvlVisited[DLV_NEST3] && !(plr[myplr].pDungMsgs2 & DMSG2_DEFILER1)) {
		sfxdelay = 10;
		sfxdnum = USFX_DEFILER3;
		plr[myplr].pDungMsgs2 |= DMSG2_DEFILER1;
	} else if (currLvl._dLevelIdx == DLV_CRYPT1 && !plr[myplr]._pLvlVisited[DLV_CRYPT1] && !(plr[myplr].pDungMsgs2 & DMSG2_DEFILER2)) {
		sfxdelay = 30;
		sfxdnum = sgSFXSets[SFXS_PLR_92][plr[myplr]._pClass];
		plr[myplr].pDungMsgs2 |= DMSG2_DEFILER2;
#endif
	} else if (currLvl._dLevelIdx == SL_SKELKING && !plr[myplr]._pLvlVisited[SL_SKELKING] && !(plr[myplr].pDungMsgs & DMSG_SKING)) {
		sfxdelay = 30;
		sfxdnum = USFX_SKING1;
		plr[myplr].pDungMsgs |= DMSG_SKING;
	}
}

DEVILUTION_END_NAMESPACE
