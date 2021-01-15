/**
 * @file player.cpp
 *
 * Implementation of player functionality, leveling, actions, creation, loading, etc.
 */
#include "all.h"
#include "plrctrls.h"

DEVILUTION_BEGIN_NAMESPACE

int plr_lframe_size;
int plr_wframe_size;
BYTE plr_gfx_flag = 0;
int plr_aframe_size;
int myplr;
PlayerStruct plr[MAX_PLRS];
int plr_fframe_size;
int plr_qframe_size;
BOOL deathflag;
int plr_hframe_size;
int plr_bframe_size;
BYTE plr_gfx_bflag = 0;
int plr_sframe_size;
int deathdelay;
int plr_dframe_size;

/** Maps from armor animation to letter used in graphic files. */
const char ArmourChar[4] = { 'L', 'M', 'H', 0 };
/** Maps from weapon animation to letter used in graphic files. */
const char WepChar[10] = { 'N', 'U', 'S', 'D', 'B', 'A', 'M', 'H', 'T', 0 };
/** Maps from player class to letter used in graphic files. */
const char CharChar[NUM_CLASSES] = {
	'W',
	'R',
	'S',
#ifdef HELLFIRE
	'M',
	'B',
	'C'
#endif
};
const char *const ClassStrTbl[NUM_CLASSES] = {
	"Warrior",
	"Rogue",
	"Sorceror",
#ifdef HELLFIRE
	"Monk",
	"Bard",
	"Barbarian"
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
const int AnimLenFromClass[NUM_CLASSES] = {
	8,
	8,
	8,
#ifdef HELLFIRE
	8,
	8,
	8,
#endif
};
/** Maps from player_class to starting stat in strength. */
const int StrengthTbl[NUM_CLASSES] = {
	30,
	20,
	15,
#ifdef HELLFIRE
	25,
	20,
	40,
#endif
};
/** Maps from player_class to starting stat in magic. */
const int MagicTbl[NUM_CLASSES] = {
	// clang-format off
	10,
	15,
	35,
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
	30,
	15,
#ifdef HELLFIRE
	25,
	25,
	20,
#endif
};
/** Maps from player_class to starting stat in vitality. */
const int VitalityTbl[NUM_CLASSES] = {
	25,
	20,
	20,
#ifdef HELLFIRE
	20,
	20,
	25,
#endif
};
/** Specifies the chance to block bonus of each player class.*/
const int ToBlkTbl[NUM_CLASSES] = {
	30,
	20,
	10,
#ifdef HELLFIRE
	25,
	25,
	30,
#endif
};
/** Maps from player_class to maximum stats. */
const int MaxStats[NUM_CLASSES][4] = {
	// clang-format off
	{ 250,  50,  60, 100 },
	{  55,  70, 250,  80 },
	{  45, 250,  85,  80 },
#ifdef HELLFIRE
	{ 150,  80, 150,  80 },
	{ 120, 120, 120, 100 },
	{ 255,   0,  55, 150 },
#endif
	// clang-format on
};
const int Abilities[NUM_CLASSES] = {
	SPL_REPAIR, SPL_DISARM, SPL_RECHARGE
#ifdef HELLFIRE
	, SPL_SEARCH, SPL_IDENTIFY, SPL_BLODBOIL
#endif
};

/** Specifies the experience point limit of each level. */
const int ExpLvlsTbl[MAXCHARLEVEL + 1] = {
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

static void SetPlayerGPtrs(BYTE *pData, BYTE *(&pAnim)[8])
{
	int i;

	for (i = 0; i < lengthof(pAnim); i++) {
		pAnim[i] = CelGetFrameStart(pData, i);
	}
}

static inline void GetPlrGFXCells(int pc, const char **szCel, const char **cs)
{
#ifdef HELLFIRE
	if (pc == PC_BARD && hfbard_mpq == NULL)
		pc = PC_ROGUE;
	else if (pc == PC_BARBARIAN && hfbarb_mpq == NULL)
		pc = PC_WARRIOR;
#endif
	*szCel = &CharChar[pc];
	*cs = ClassStrTbl[pc];
}

void LoadPlrGFX(int pnum, unsigned gfxflag)
{
	char prefix[16];
	char pszName[256];
	const char *szCel, *cs;
	PlayerStruct *p;
	BYTE *pData;
	BYTE *(*pAnim)[8];
	DWORD i, mask;

	if ((DWORD)pnum >= MAX_PLRS) {
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
			szCel = leveltype != DTYPE_TOWN ? "AS" : "ST";
			pData = p->_pNData;
			pAnim = &p->_pNAnim;
			break;
		case PFIDX_WALK:
			szCel = leveltype != DTYPE_TOWN ? "AW" : "WL";
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
	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("InitPlayerGFX: illegal player %d", pnum);
	}

	if (plr[pnum]._pHitPoints < (1 << 6)) {
		plr[pnum]._pgfxnum = ANIM_ID_UNARMED;
		gfxflag = PFILE_DEATH;
	} else {
		gfxflag = PFILE_NONDEATH;
		if (leveltype == DTYPE_TOWN)
			gfxflag &= ~(PFILE_ATTACK | PFILE_HIT | PFILE_BLOCK);
		else if (!plr[pnum]._pBlockFlag)
			gfxflag &= ~PFILE_BLOCK;
	}
	LoadPlrGFX(pnum, gfxflag);
}

static DWORD GetPlrGFXSize(const char *szCel)
{
	DWORD c;
	const char *a, *w, *cc, *cst;
	DWORD dwSize, dwMaxSize;
	HANDLE hsFile;
	char pszName[256];
	char Type[16];

	dwMaxSize = 0;

	for (c = 0; c < NUM_CLASSES; c++) {
#ifdef SPAWN
		if (c != PC_WARRIOR)
			continue;
#endif
		GetPlrGFXCells(c, &cc, &cst);
		for (a = &ArmourChar[0]; *a; a++) {
#ifdef SPAWN
			if (a != &ArmourChar[0])
				break;
#endif
			for (w = &WepChar[0]; *w; w++) { // BUGFIX loads non-existing animations; DT is only for N, BT is only for U, D & H (fixed)
				if (szCel[0] == 'D' && szCel[1] == 'T' && *w != 'N') {
					continue; //Death has no weapon
				}
				if (szCel[0] == 'B' && szCel[1] == 'L' && (*w != 'U' && *w != 'D' && *w != 'H')) {
					continue; //No block without weapon
				}
				snprintf(Type, sizeof(Type), "%c%c%c", *cc, *a, *w);
				snprintf(pszName, sizeof(pszName), "PlrGFX\\%s\\%s\\%s%s.CL2", cst, Type, Type, szCel);
				if (WOpenFile(pszName, &hsFile, TRUE)) {
					/// ASSERT: assert(hsFile != NULL);
					dwSize = WGetFileSize(hsFile, NULL, pszName);
					WCloseFile(hsFile);
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

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("InitPlrGFXMem: illegal player %d", pnum);
	}

	p = &plr[pnum];

	if (!(plr_gfx_flag & 0x1)) { //STAND
		plr_gfx_flag |= 0x1;
		// ST: TOWN, AS: DUNGEON
		plr_sframe_size = std::max(GetPlrGFXSize("ST"), GetPlrGFXSize("AS"));
	}
	p->_pNData = DiabloAllocPtr(plr_sframe_size);

	if (!(plr_gfx_flag & 0x2)) { //WALK
		plr_gfx_flag |= 0x2;
		// WL: TOWN, AW: DUNGEON
		plr_wframe_size = std::max(GetPlrGFXSize("WL"), GetPlrGFXSize("AW"));
	}
	p->_pWData = DiabloAllocPtr(plr_wframe_size);

	if (!(plr_gfx_flag & 0x4)) { //ATTACK
		plr_gfx_flag |= 0x4;
		plr_aframe_size = GetPlrGFXSize("AT");
	}
	p->_pAData = DiabloAllocPtr(plr_aframe_size);

	if (!(plr_gfx_flag & 0x8)) { //HIT
		plr_gfx_flag |= 0x8;
		plr_hframe_size = GetPlrGFXSize("HT");
	}
	p->_pHData = DiabloAllocPtr(plr_hframe_size);

	if (!(plr_gfx_flag & 0x10)) { //LIGHTNING
		plr_gfx_flag |= 0x10;
		plr_lframe_size = GetPlrGFXSize("LM");
	}
	p->_pLData = DiabloAllocPtr(plr_lframe_size);

	if (!(plr_gfx_flag & 0x20)) { //FIRE
		plr_gfx_flag |= 0x20;
		plr_fframe_size = GetPlrGFXSize("FM");
	}
	p->_pFData = DiabloAllocPtr(plr_fframe_size);

	if (!(plr_gfx_flag & 0x40)) { //MAGIC
		plr_gfx_flag |= 0x40;
		plr_qframe_size = GetPlrGFXSize("QM");
	}
	p->_pTData = DiabloAllocPtr(plr_qframe_size);

	if (!(plr_gfx_flag & 0x80)) { //DEATH
		plr_gfx_flag |= 0x80;
		plr_dframe_size = GetPlrGFXSize("DT");
	}
	p->_pDData = DiabloAllocPtr(plr_dframe_size);

	if (!(plr_gfx_bflag & 0x1)) { //BLOCK
		plr_gfx_bflag |= 0x1;
		plr_bframe_size = GetPlrGFXSize("BL");
	}
	p->_pBData = DiabloAllocPtr(plr_bframe_size);

	p->_pGFXLoad = 0;
}

void FreePlayerGFX(int pnum)
{
	PlayerStruct *p;

	if ((DWORD)pnum >= MAX_PLRS) {
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

void NewPlrAnim(int pnum, BYTE **anims, int dir, unsigned numFrames, int Delay, int width)
{
	PlayerStruct *p;

	if ((DWORD)pnum >= MAX_PLRS) {
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
	p->_pAnimWidth2 = (width - 64) >> 1;
}

static void ClearPlrPVars(int pnum)
{
	/*PlayerStruct *p;

	if ((DWORD)pnum >= MAX_PLRS) {
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

	if ((DWORD)pnum >= MAX_PLRS) {
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

	if (leveltype == DTYPE_TOWN) {
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
			if (leveltype != DTYPE_TOWN) {
				p->_pNFrames = 8;
			}
			p->_pAWidth = 96;
			p->_pAFNum = 11;
		} else if (gn == ANIM_ID_AXE) {
			p->_pAFrames = 20;
			p->_pAFNum = 10;
		} else if (gn == ANIM_ID_STAFF) {
			p->_pAFrames = 16;
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
			if (leveltype != DTYPE_TOWN) {
				p->_pNFrames = 8;
			}
			p->_pAWidth = 96;
			p->_pAFNum = 11;
		} else if (gn == ANIM_ID_STAFF) {
			p->_pAFrames = 16;
			p->_pAFNum = 11;
		} else if (gn == ANIM_ID_MACE || gn == ANIM_ID_MACE_SHIELD) {
			p->_pAFNum = 8;
		}
#endif
	}
}

static void ClearPlrRVars(PlayerStruct *p)
{
	// TODO: Missing debug assert p != NULL
	p->bReserved[0] = 0;
	p->bReserved[1] = 0;

	p->wReserved[0] = 0;
	p->wReserved[1] = 0;
	p->wReserved[2] = 0;
	p->wReserved[3] = 0;
	p->wReserved[4] = 0;
	p->wReserved[5] = 0;
	p->wReserved[6] = 0;

	p->dwReserved[0] = 0;
	p->dwReserved[1] = 0;
	p->dwReserved[2] = 0;
	p->dwReserved[3] = 0;
	p->dwReserved[4] = 0;
}

/**
 * @param c plr_classes value
 */
void CreatePlayer(int pnum, char c)
{
	PlayerStruct *p;
	int val, hp, mana;
	int i;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("CreatePlayer: illegal player %d", pnum);
	}

	p = &plr[pnum];
	memset(p, 0, sizeof(*p));
	SetRndSeed(SDL_GetTicks());

	p->_pClass = c;

	p->_pBaseToBlk = ToBlkTbl[c];

	val = StrengthTbl[c];
	p->_pStrength = val;
	p->_pBaseStr = val;

	val = MagicTbl[c];
	p->_pMagic = val;
	p->_pBaseMag = val;

	val = DexterityTbl[c];
	p->_pDexterity = val;
	p->_pBaseDex = val;

	val = VitalityTbl[c];
	p->_pVitality = val;
	p->_pBaseVit = val;

	hp = (val + 10) << 6;

	switch (c) {
	case PC_WARRIOR: hp <<= 1;		break;
	case PC_ROGUE: hp += hp >> 1;	break;
	case PC_SORCERER:				break;
#ifdef HELLFIRE
	case PC_MONK:
	case PC_BARD: hp += hp >> 1;	break;
	case PC_BARBARIAN: hp <<= 1;	break;
#endif
	default: ASSUME_UNREACHABLE
	}
	p->_pHitPoints = p->_pMaxHP = p->_pHPBase = p->_pMaxHPBase = hp;

	mana = p->_pMagic << 6;
	switch (c) {
	case PC_WARRIOR: 					break;
	case PC_ROGUE: mana += mana >> 1;	break;
	case PC_SORCERER: mana <<= 1;		break;
#ifdef HELLFIRE
	case PC_MONK: mana += mana >> 1;	break;
	case PC_BARD: mana += mana * 3 / 4;	break;
	case PC_BARBARIAN:					break;
#endif
	default: ASSUME_UNREACHABLE
	}

	p->_pMana = p->_pMaxMana = p->_pManaBase = p->_pMaxManaBase = mana;

	p->_pLevel = 1;
	p->_pLvlUp = FALSE; // indicator whether the stat button should be shown
	p->_pNextExper = ExpLvlsTbl[1];
#ifdef HELLFIRE
	if (c == PC_BARBARIAN) {
		p->_pMagResist = 1;
		p->_pFireResist = 1;
		p->_pLghtResist = 1;
	}
#endif
	p->_pLightRad = 10;

	p->_pAblSpells = SPELL_MASK(Abilities[c]);
	p->_pAblSpells |= SPELL_MASK(SPL_WALK);
	p->_pAblSpells |= SPELL_MASK(SPL_WATTACK);
	p->_pAblSpells |= SPELL_MASK(SPL_ATTACK);

	if (c == PC_SORCERER) {
		p->_pSplLvl[SPL_FIREBOLT] = 2;
		p->_pMemSpells = SPELL_MASK(SPL_FIREBOLT);
	}
	for (i = 0; i < lengthof(p->_pSplHotKey); i++)
		p->_pSplHotKey[i] = SPL_INVALID;
	for (i = 0; i < lengthof(p->_pSplTHotKey); i++)
		p->_pSplTHotKey[i] = RSPLTYPE_INVALID;

	// TODO: BUGFIX: is this necessary? does not seem to work with hellfire...
	switch (c) {
	case PC_WARRIOR:
		p->_pgfxnum = ANIM_ID_SWORD_SHIELD;
		break;
	case PC_ROGUE:
		p->_pgfxnum = ANIM_ID_BOW;
		break;
	case PC_SORCERER:
		p->_pgfxnum = ANIM_ID_STAFF;
		break;
#ifdef HELLFIRE
	case PC_MONK:
		p->_pgfxnum = ANIM_ID_STAFF;
		break;
	case PC_BARD:
		p->_pgfxnum = ANIM_ID_SWORD_SHIELD;
		break;
	case PC_BARBARIAN:
		p->_pgfxnum = ANIM_ID_SWORD_SHIELD;
		break;
#endif
	default:
		ASSUME_UNREACHABLE
	}

	InitDungMsgs(pnum);
	CreatePlrItems(pnum);
	SetRndSeed(0);
}

static int CalcStatDiff(int pnum)
{
	const int *stats;

	stats = MaxStats[plr[pnum]._pClass];
	return stats[ATTRIB_STR]
	    - plr[pnum]._pBaseStr
	    + stats[ATTRIB_MAG]
	    - plr[pnum]._pBaseMag
	    + stats[ATTRIB_DEX]
	    - plr[pnum]._pBaseDex
	    + stats[ATTRIB_VIT]
	    - plr[pnum]._pBaseVit;
}

void NextPlrLevel(int pnum)
{
	PlayerStruct *p;
	int hp, mana;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("NextPlrLevel: illegal player %d", pnum);
	}
	p = &plr[pnum];
	p->_pLevel++;

#ifdef HELLFIRE
	CalcPlrInv(pnum, TRUE);
#endif

	if (CalcStatDiff(pnum) < 5) {
		p->_pStatPts = CalcStatDiff(pnum);
	} else {
		p->_pStatPts += 5;
	}
	p->_pLvlUp = p->_pStatPts != 0;

	p->_pNextExper = ExpLvlsTbl[p->_pLevel];

	hp = p->_pClass == PC_SORCERER ? 64 : 128;

	p->_pMaxHP += hp;
	p->_pMaxHPBase += hp;

	if (p->_pClass == PC_WARRIOR)
		mana = 64;
#ifdef HELLFIRE
	else if (p->_pClass == PC_BARBARIAN)
		mana = 0;
#endif
	else
		mana = 128;

	p->_pMaxMana += mana;
	p->_pMaxManaBase += mana;

	PlrFillHp(pnum);
	PlrFillMana(pnum);

#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	if (sgbControllerActive)
		FocusOnCharInfo();
#endif
}

void AddPlrExperience(int pnum, int lvl, int exp)
{
	PlayerStruct *p;
	int powerLvlCap, expCap, newLvl, i;

	if (pnum != myplr) {
		return;
	}

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("AddPlrExperience: illegal player %d", pnum);
	}

	p = &plr[pnum];
	if (p->_pHitPoints < (1 << 6)) {
		return;
	}

	// Adjust xp based on difference in level between player and monster
	exp = exp * (10 + (lvl - p->_pLevel)) / 10;
	if (exp < 0) {
		exp = 0;
	}

	// Prevent power leveling
	if (gbMaxPlayers != 1) {
		powerLvlCap = p->_pLevel < 0 ? 0 : p->_pLevel;
		if (powerLvlCap > MAXCHARLEVEL) {
			powerLvlCap = MAXCHARLEVEL;
		}
		// cap to 1/20 of current levels xp
		if (exp > ExpLvlsTbl[powerLvlCap] / 20) {
			exp = ExpLvlsTbl[powerLvlCap] / 20;
		}
		// cap to 200 * current level
		expCap = 200 * powerLvlCap;
		if (exp > expCap) {
			exp = expCap;
		}
	}

	p->_pExperience += exp;
	if ((DWORD)p->_pExperience > MAXEXP) {
		p->_pExperience = MAXEXP;
	}

	if (p->_pExperience >= ExpLvlsTbl[MAXCHARLEVEL - 1]) {
		p->_pLevel = MAXCHARLEVEL;
		return;
	}

	// Increase player level if applicable
	newLvl = 0;
	while (p->_pExperience >= ExpLvlsTbl[newLvl]) {
		newLvl++;
	}
	if (newLvl != p->_pLevel) {
		for (i = newLvl - p->_pLevel; i > 0; i--) {
			NextPlrLevel(pnum);
		}
	}

	NetSendCmdParam1(FALSE, CMD_PLRLEVEL, p->_pLevel);
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

void InitPlayer(int pnum, BOOL FirstTime)
{
	PlayerStruct *p;
	DWORD i;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("InitPlayer: illegal player %d", pnum);
	}
	p = &plr[pnum];
	ClearPlrRVars(p);

	if (FirstTime) {
		p->_pRSplType = p->_pSplType = RSPLTYPE_INVALID;
		p->_pRSpell = p->_pSpell = SPL_INVALID;
		// TODO: BUGFIX: does not seem to be the best place to set this
		if ((p->_pgfxnum & 0xF) == ANIM_ID_BOW) {
			p->_pwtype = WT_RANGED;
		} else {
			p->_pwtype = WT_MELEE;
		}
		p->pManaShield = 0;

		p->_pBaseToBlk = ToBlkTbl[p->_pClass];

		p->_pAblSpells = SPELL_MASK(Abilities[p->_pClass]);
		p->_pAblSpells |= SPELL_MASK(SPL_WALK);
		p->_pAblSpells |= SPELL_MASK(SPL_WATTACK);
		p->_pAblSpells |= SPELL_MASK(SPL_ATTACK);
		CalcPlrAbilities(pnum);

		p->_pNextExper = ExpLvlsTbl[p->_pLevel];
	}

#ifdef _DEBUG
	if (p->plrlevel == currlevel || leveldebug) {
#else
	if (p->plrlevel == currlevel) {
#endif
		SetPlrAnims(pnum);

		p->_pxoff = 0;
		p->_pyoff = 0;
		p->_pxvel = 0;
		p->_pyvel = 0;

		ClearPlrPVars(pnum);

		if (p->_pHitPoints >= (1 << 6)) {
			p->_pmode = PM_STAND;
			NewPlrAnim(pnum, p->_pNAnim, DIR_S, p->_pNFrames, 3, p->_pNWidth);
			p->_pAnimFrame = RandRange(1, p->_pNFrames - 1);
			p->_pAnimCnt = random_(2, 3);
		} else {
			p->_pmode = PM_DEATH;
			NewPlrAnim(pnum, p->_pDAnim, DIR_S, p->_pDFrames, 1, p->_pDWidth);
			p->_pAnimFrame = p->_pAnimLen - 1;
			p->_pVar8 = 2 * p->_pAnimLen;
		}

		if (pnum == myplr) {
			if (!FirstTime || currlevel != 0) {
				p->_px = ViewX;
				p->_py = ViewY;
			}
			p->_ptargx = p->_px;
			p->_ptargy = p->_py;
		} else {
			p->_ptargx = p->_px;
			p->_ptargy = p->_py;
			for (i = 0; i < 8 && !PosOkPlayer(pnum, plrxoff2[i] + p->_px, plryoff2[i] + p->_py); i++)
				;
			p->_px += plrxoff2[i];
			p->_py += plryoff2[i];
		}

		p->_pfutx = p->_px;
		p->_pfuty = p->_py;
		p->walkpath[0] = WALK_NONE;
		p->destAction = ACTION_NONE;

		if (pnum == myplr) {
			p->_plid = AddLight(p->_px, p->_py, p->_pLightRad);
		} else {
			p->_plid = -1;
		}
		p->_pvid = AddVision(p->_px, p->_py, p->_pLightRad, pnum == myplr);
	}

#ifdef _DEBUG
	if (debug_mode_key_inverted_v && FirstTime) {
		p->_pMemSpells = SPL_INVALID;
	} else if (debug_mode_god_mode && FirstTime) {
		p->_pMemSpells |= SPELL_MASK(SPL_TELEPORT);
		if (p->_pSplLvl[SPL_TELEPORT] == 0) {
			p->_pSplLvl[SPL_TELEPORT] = 1;
		}
	}
#endif

	// TODO: BUGFIX: should only be set if p->plrlevel == currlevel?
	if (p->_pmode != PM_DEATH)
		p->_pInvincible = FALSE;

	if (pnum == myplr) {
		// TODO: BUGFIX: sure?
		//    - what if we just joined with a dead player?
		//    - what if the player was killed while entering a portal?
		deathdelay = 0;
		deathflag = FALSE;
		ScrollInfo._sxoff = 0;
		ScrollInfo._syoff = 0;
		ScrollInfo._sdir = SDIR_NONE;
	}
}

void InitMultiView()
{
	if ((DWORD)myplr >= MAX_PLRS) {
		app_fatal("InitPlayer: illegal player %d", myplr);
	}

	ViewX = plr[myplr]._px;
	ViewY = plr[myplr]._py;
}

static BOOL PlrDirOK(int pnum, int dir)
{
	int px, py;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("PlrDirOK: illegal player %d", pnum);
	}

	px = plr[pnum]._px + offset_x[dir];
	py = plr[pnum]._py + offset_y[dir];

	if (px < 0 || !dPiece[px][py] || !PosOkPlayer(pnum, px, py)) {
		return FALSE;
	}

	if (dir == DIR_E) {
		return !nSolidTable[dPiece[px][py + 1]] && !(dFlags[px][py + 1] & BFLAG_PLAYERLR);
	}

	if (dir == DIR_W) {
		return !nSolidTable[dPiece[px + 1][py]] && !(dFlags[px + 1][py] & BFLAG_PLAYERLR);
	}

	return TRUE;
}

void PlrClrTrans(int x, int y)
{
	int i, j;

	for (i = y - 1; i <= y + 1; i++) {
		for (j = x - 1; j <= x + 1; j++) {
			TransList[dTransVal[j][i]] = FALSE;
		}
	}
}

void PlrDoTrans(int x, int y)
{
	int i, j;

	if (leveltype != DTYPE_CATHEDRAL && leveltype != DTYPE_CATACOMBS) {
		TransList[1] = TRUE;
	} else {
		for (i = y - 1; i <= y + 1; i++) {
			for (j = x - 1; j <= x + 1; j++) {
				if (!nSolidTable[dPiece[j][i]] && dTransVal[j][i]) {
					TransList[dTransVal[j][i]] = TRUE;
				}
			}
		}
	}
}

void FixPlayerLocation(int pnum)
{
	PlayerStruct *p;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("FixPlayerLocation: illegal player %d", pnum);
	}
	p = &plr[pnum];
	p->_pfutx = p->_ptargx = p->_poldx = p->_px;
	p->_pfuty = p->_ptargy = p->_poldy = p->_py;
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

	if ((DWORD)pnum >= MAX_PLRS) {
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

	if ((DWORD)pnum >= MAX_PLRS) {
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
	int xmul, ymul;
	int lx, ly;
	int offx, offy;
	const LightListStruct *l;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("PlrChangeLightOff: illegal player %d", pnum);
	}

	p = &plr[pnum];
	// check if issue is upstream
	if (p->_plid == -1)
		return;

	l = &LightList[p->_plid];
	x = 2 * p->_pyoff + p->_pxoff;
	y = 2 * p->_pyoff - p->_pxoff;
	if (x < 0) {
		xmul = -1;
		x = -x;
	} else {
		xmul = 1;
	}
	if (y < 0) {
		ymul = -1;
		y = -y;
	} else {
		ymul = 1;
	}

	x = (x >> 3) * xmul;
	y = (y >> 3) * ymul;
	lx = x + (l->_lx << 3);
	ly = y + (l->_ly << 3);
	offx = l->_xoff + (l->_lx << 3);
	offy = l->_yoff + (l->_ly << 3);

	if (abs(lx - offx) < 3 && abs(ly - offy) < 3)
		return;

	ChangeLightOff(p->_plid, x, y);
}

static void PlrChangeOffset(int pnum)
{
	PlayerStruct *p;
	int px, py;

	if ((DWORD)pnum >= MAX_PLRS) {
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
	}

	PlrChangeLightOff(pnum);
}

static void StartWalk(int pnum, int xvel, int yvel, int xadd, int yadd, int EndDir, int sdir)
{
	PlayerStruct *p;
	int px, py;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("StartWalk: illegal player %d", pnum);
	}

	p = &plr[pnum];
	if (p->_pHitPoints < (1 << 6)) {
		SyncPlrKill(pnum, -1); // BUGFIX: is this really necessary?
		return;
	}

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
	p->_pVar1 = xadd;
	p->_pVar2 = yadd;
	p->_pVar3 = EndDir;
	p->_pVar6 = 0;
	p->_pVar7 = 0;
	p->_pVar8 = 0; // speed helper

	if (!(p->_pGFXLoad & PFILE_WALK)) {
		LoadPlrGFX(pnum, PFILE_WALK);
	}

	NewPlrAnim(pnum, p->_pWAnim, EndDir, p->_pWFrames, 0, p->_pWWidth);

	if (pnum != myplr) {
		return;
	}

	if (zoomflag) {
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

#if defined(__clang__) || defined(__GNUC__)
__attribute__((no_sanitize("shift-base")))
#endif
static void StartWalk2(int pnum, int xvel, int yvel, int xoff, int yoff, int xadd, int yadd, int EndDir, int sdir)
{
	PlayerStruct *p;
	int px, py;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("StartWalk2: illegal player %d", pnum);
	}

	p = &plr[pnum];
	if (p->_pHitPoints < (1 << 6)) {
		SyncPlrKill(pnum, -1); // BUGFIX: is this really necessary?
		return;
	}

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
	p->_pVar1 = p->_px;
	p->_pVar2 = p->_py;
	p->_px = px;
	p->_py = py;
	dPlayer[p->_px][p->_py] = pnum + 1;
	p->_pxoff = xoff;
	p->_pyoff = yoff;

	ChangeLightXY(p->_plid, p->_px, p->_py);
	PlrChangeLightOff(pnum);

	p->_pmode = PM_WALK2;
	p->_pxvel = xvel;
	p->_pyvel = yvel;
	p->_pVar6 = xoff * 256;
	p->_pVar7 = yoff * 256;
	p->_pVar3 = EndDir;
	p->_pVar8 = 0; // speed helper

	if (!(p->_pGFXLoad & PFILE_WALK)) {
		LoadPlrGFX(pnum, PFILE_WALK);
	}
	NewPlrAnim(pnum, p->_pWAnim, EndDir, p->_pWFrames, 0, p->_pWWidth);

	if (pnum != myplr) {
		return;
	}

	if (zoomflag) {
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

#if defined(__clang__) || defined(__GNUC__)
__attribute__((no_sanitize("shift-base")))
#endif
static void StartWalk3(int pnum, int xvel, int yvel, int xoff, int yoff, int xadd, int yadd, int mapx, int mapy, int EndDir, int sdir)
{
	PlayerStruct *p;
	int px, py, x, y;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("StartWalk3: illegal player %d", pnum);
	}

	p = &plr[pnum];
	if (p->_pHitPoints < (1 << 6)) {
		SyncPlrKill(pnum, -1); // BUGFIX: is this really necessary?
		return;
	}

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
	p->_pVar4 = x;
	p->_pVar5 = y;
	dFlags[x][y] |= BFLAG_PLAYERLR;
	p->_pxoff = xoff;
	p->_pyoff = yoff;

	if (leveltype != DTYPE_TOWN) {
		ChangeLightXY(p->_plid, x, y);
		PlrChangeLightOff(pnum);
	}

	p->_pmode = PM_WALK3;
	p->_pxvel = xvel;
	p->_pyvel = yvel;
	p->_pVar1 = px;
	p->_pVar2 = py;
	p->_pVar6 = xoff * 256;
	p->_pVar7 = yoff * 256;
	p->_pVar3 = EndDir;
	p->_pVar8 = 0; // speed helper

	if (!(p->_pGFXLoad & PFILE_WALK)) {
		LoadPlrGFX(pnum, PFILE_WALK);
	}
	NewPlrAnim(pnum, p->_pWAnim, EndDir, p->_pWFrames, 0, p->_pWWidth);

	if (pnum != myplr) {
		return;
	}

	if (zoomflag) {
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

static BOOL StartAttack(int pnum)
{
	PlayerStruct *p;
	int i, dx, dy, dir;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("StartAttack: illegal player %d", pnum);
	}

	p = &plr[pnum];
	if (p->_pHitPoints < (1 << 6)) {
		SyncPlrKill(pnum, -1); // BUGFIX: is this really necessary?
		return FALSE;
	}

	i = p->destParam1;
	if (p->destAction == ACTION_ATTACK) {
		dx = i;
		dy = p->destParam2;
	} else if (p->destAction == ACTION_ATTACKMON) {
		dx = monster[i]._mfutx;
		dy = monster[i]._mfuty;
		if (abs(p->_px - dx) > 1 || abs(p->_py - dy) > 1)
			return FALSE;
		if (monster[i].mtalkmsg != 0 && monster[i].mtalkmsg != TEXT_VILE14) {
			TalktoMonster(i);
			return TRUE;
		}
	} else if (p->destAction == ACTION_ATTACKPLR) {
		dx = plr[i]._pfutx;
		dy = plr[i]._pfuty;
		if (abs(p->_px - dx) > 1 || abs(p->_py - dy) > 1)
			return FALSE;
	} else {
		assert(p->destAction == ACTION_OPERATE || p->destAction == ACTION_DISARM);
		dx = p->destParam2;
		dy = p->destParam3;
		if (abs(p->_px - dx) > 1 || abs(p->_py - dy) > 1)
			return FALSE;
		if (abs(dObject[dx][dy]) != i + 1) // this should always be false, but never trust the internet
			return FALSE;
		if (object[i]._oBreak != 1) {
			if (p->destAction == ACTION_DISARM)
				DisarmObject(pnum, i);
			OperateObject(pnum, i, FALSE);
			return TRUE;
		}
		if (p->destAction == ACTION_DISARM && pnum == myplr)
			NewCursor(CURSOR_HAND);
	}

	dir = GetDirection(p->_px, p->_py, dx, dy);
	p->_pmode = PM_ATTACK;
	p->_pVar7 = 0; // 'flags' of sfx and hit
	p->_pVar8 = 0; // speed helper

	if (!(p->_pGFXLoad & PFILE_ATTACK)) {
		LoadPlrGFX(pnum, PFILE_ATTACK);
	}
	NewPlrAnim(pnum, p->_pAAnim, dir, p->_pAFrames, 0, p->_pAWidth);

	FixPlayerLocation(pnum);
	return TRUE;
}

static void StartRangeAttack(int pnum)
{
	PlayerStruct *p;
	int i, dx, dy, dir;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("StartRangeAttack: illegal player %d", pnum);
	}

	p = &plr[pnum];
	if (p->_pHitPoints < (1 << 6)) {
		SyncPlrKill(pnum, -1); // BUGFIX: is this really necessary?
		return;
	}

	i = p->destParam1;
	if (p->destAction == ACTION_RATTACK) {
		dx = i;
		dy = p->destParam2;
	} else if (p->destAction == ACTION_RATTACKMON) {
		if (monster[i].mtalkmsg != 0 && monster[i].mtalkmsg != TEXT_VILE14) {
			TalktoMonster(i);
			return;
		}
		dx = monster[i]._mfutx;
		dy = monster[i]._mfuty;
	} else {
		assert(p->destAction == ACTION_RATTACKPLR);
		dx = plr[i]._pfutx;
		dy = plr[i]._pfuty;
	}
	p->_pVar1 = dx;
	p->_pVar2 = dy;
	p->_pVar8 = 0; // speed helper
	p->_pmode = PM_RATTACK;

	dir = GetDirection(p->_px, p->_py, dx, dy);

	if (!(p->_pGFXLoad & PFILE_ATTACK)) {
		LoadPlrGFX(pnum, PFILE_ATTACK);
	}
	NewPlrAnim(pnum, p->_pAAnim, dir, p->_pAFrames, 0, p->_pAWidth);

	FixPlayerLocation(pnum);
}

static void StartBlock(int pnum, int dir)
{
	PlayerStruct *p;
	if ((DWORD)pnum >= MAX_PLRS) {
		dev_fatal("StartBlock: illegal player %d", pnum);
	}

	p = &plr[pnum];
	if (p->_pHitPoints < (1 << 6)) {
		SyncPlrKill(pnum, -1); // BUGFIX: is this really necessary?
		return;
	}

	p->_pmode = PM_BLOCK;
	p->_pVar1 = 0; // extended blocking
	if (!(p->_pGFXLoad & PFILE_BLOCK)) {
		LoadPlrGFX(pnum, PFILE_BLOCK);
	}
	NewPlrAnim(pnum, p->_pBAnim, dir, p->_pBFrames, 2, p->_pBWidth);

	FixPlayerLocation(pnum);
}

static void StartSpell(int pnum)
{
	PlayerStruct *p;
	int i, dx, dy, spllvl;
	player_graphic gfx;
	unsigned char **anim;
	SpellData *sd;

	if ((DWORD)pnum >= MAX_PLRS)
		app_fatal("StartSpell: illegal player %d", pnum);

	p = &plr[pnum];
	if (p->_pHitPoints < (1 << 6)) {
		SyncPlrKill(pnum, -1); // BUGFIX: is this really necessary?
		return;
	}

	i = p->destParam1;
	if (p->destAction == ACTION_SPELL) {
		dx = p->destParam2;
		dy = p->destParam3;
		spllvl = i;
	} else if (p->destAction == ACTION_SPELLMON) {
		dx = monster[i]._mfutx;
		dy = monster[i]._mfuty;
		spllvl = p->destParam2;
	} else {
		assert(p->destAction == ACTION_SPELLPLR);
		dx = plr[i]._pfutx;
		dy = plr[i]._pfuty;
		spllvl = p->destParam2;
	}

	p->_pVar1 = dx;
	p->_pVar2 = dy;
	p->_pVar3 = p->_pSpell;
	p->_pVar4 = spllvl;
	p->_pVar5 = p->_pSplFrom;
	p->_pVar7 = FALSE; // 'flag' of cast
	p->_pVar8 = 0; // speed helper
	p->_pmode = PM_SPELL;

	sd = &spelldata[p->_pSpell];
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

	PlaySfxLoc(sd->sSFX, p->_px, p->_py);

	FixPlayerLocation(pnum);
}

void RemovePlrFromMap(int pnum)
{
	int pp, pn;
	int dx, dy, y, x;

	if ((DWORD)pnum >= MAX_PLRS) {
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

void StartPlrHit(int pnum, int dam, BOOL forcehit)
{
	PlayerStruct *p;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("StartPlrHit: illegal player %d", pnum);
	}

	p = &plr[pnum];
	if (p->_pHitPoints < (1 << 6)) {
		SyncPlrKill(pnum, -1); // BUGFIX: is this really necessary?
		return;
	}

	PlaySfxLoc(sgSFXSets[SFXS_PLR_69][p->_pClass], p->_px, p->_py, 2);

	if (!forcehit) {
#ifdef HELLFIRE
		if (p->_pClass == PC_BARBARIAN) {
			if (dam >> 6 < p->_pLevel + p->_pLevel / 4)
				return;
		} else
#endif
			if (dam >> 6 < p->_pLevel)
				return;
	}

	if (!(p->_pGFXLoad & PFILE_HIT)) {
		LoadPlrGFX(pnum, PFILE_HIT);
	}
	NewPlrAnim(pnum, p->_pHAnim, p->_pdir, p->_pHFrames, 0, p->_pHWidth);

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
	if (FindGetItem(is->_iIdx, is->_iCreateInfo, is->_iSeed) >= 0) {
		DrawInvMsg("A duplicate item has been detected.  Destroying duplicate...");
		SyncGetItem(xx, yy, is->_iIdx, is->_iCreateInfo, is->_iSeed);
	}

	i = itemavail[0];
	dItem[xx][yy] = i + 1;
	itemavail[0] = itemavail[MAXITEMS - numitems - 1];
	itemactive[numitems] = i;
	copy_pod(item[i], *is);
	item[i]._ix = xx;
	item[i]._iy = yy;
	RespawnItem(i, TRUE);
	numitems++;
	NetSendCmdPItem(FALSE, CMD_RESPAWNITEM, is, xx, yy);

	is->_itype = ITYPE_NONE;
}

#if defined(__clang__) || defined(__GNUC__)
__attribute__((no_sanitize("shift-base")))
#endif
void StartPlrKill(int pnum, int earflag)
{
	BOOL diablolevel;
	int i;
	PlayerStruct *p;
	ItemStruct ear;
	ItemStruct *pi;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("StartPlrKill: illegal player %d", pnum);
	}

	p = &plr[pnum];
	if (p->_pmode == PM_DEATH) {
		return;
	}

	if (myplr == pnum) {
		NetSendCmdParam1(TRUE, CMD_PLRDEAD, earflag);
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
		CalcPlrInv(pnum, FALSE);
	}

	if (p->plrlevel == currlevel) {
		RemovePlrFromMap(pnum);
		dFlags[p->_px][p->_py] |= BFLAG_DEAD_PLAYER;
		FixPlayerLocation(pnum);

		if (pnum == myplr) {
			deathdelay = 30;

			if (pcurs >= CURSOR_FIRSTITEM) {
				PlrDeadItem(&p->HoldItem, p);
				NewCursor(CURSOR_HAND);
			}

			if (!diablolevel) {
				DropHalfPlayersGold(pnum);
				if (earflag != -1) {
					if (earflag != 0) {
						CreateBaseItem(&ear, IDI_EAR);
						snprintf(ear._iName, sizeof(ear._iName), "Ear of %s", p->_pName);
						const int earSets[NUM_CLASSES] = {
								ICURS_EAR_WARRIOR, ICURS_EAR_ROGUE, ICURS_EAR_SORCEROR
#ifdef HELLFIRE
								, ICURS_EAR_SORCEROR, ICURS_EAR_ROGUE, ICURS_EAR_WARRIOR
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
						pi = &p->InvBody[0];
						for (i = NUM_INVLOC; i != 0; i--, pi++) {
							PlrDeadItem(pi, p);
						}

						CalcPlrInv(pnum, FALSE);
					}
				}
			}
		}
	}
	PlrSetHp(pnum, 0);
}

void DropHalfPlayersGold(int pnum)
{
	PlayerStruct *p;
	ItemStruct *pi, *holditem;
	int i, hGold, limit, value;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("DropHalfPlayersGold: illegal player %d", pnum);
	}

	p = &plr[pnum];
	hGold = p->_pGold >> 1;
	p->_pGold -= hGold;
	holditem = &p->HoldItem;
#ifdef HELLFIRE
	limit = MaxGold;
#else
	limit = GOLD_MAX_LIMIT;
#endif
	for (i = 0; i < p->_pNumInv && hGold > 0; i++) {
		pi = &p->InvList[i];
		if (pi->_itype != ITYPE_GOLD)
			continue;
		value = pi->_ivalue;
		if (value == limit)
			continue;
		hGold -= value;
		if (hGold < 0) {
			SetGoldItemValue(pi, -hGold);
			value += hGold;
		} else {
			RemoveInvItem(pnum, i);
			i--;
		}
		CreateBaseItem(holditem, IDI_GOLD);
		SetGoldItemValue(holditem, value);
		PlrDeadItem(holditem, p);
	}
	for (i = 0; i < p->_pNumInv && hGold > 0; i++) {
		pi = &p->InvList[i];
		if (pi->_itype != ITYPE_GOLD)
			continue;
		value = pi->_ivalue;
		hGold -= value;
		if (hGold < 0) {
			SetGoldItemValue(pi, -hGold);
			value += hGold;
		} else {
			RemoveInvItem(pnum, i);
			i--;
		}
		CreateBaseItem(holditem, IDI_GOLD);
		SetGoldItemValue(holditem, value);
		PlrDeadItem(holditem, p);
	}
}

#ifdef HELLFIRE
void StripTopGold(int pnum)
{
	PlayerStruct *p;
	ItemStruct *pi, *holditem;
	ItemStruct tmpItem;
	int i, val;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("StripTopGold: illegal player %d", pnum);
	}
	p = &plr[pnum];
	holditem = &p->HoldItem;
	copy_pod(tmpItem, *holditem);

	pi = p->InvList;
	for (i = p->_pNumInv; i > 0; i--, pi++) {
		if (pi->_itype == ITYPE_GOLD) {
			val = pi->_ivalue - MaxGold;
			if (val > 0) {
				SetGoldItemValue(pi, MaxGold);
				CreateBaseItem(holditem, IDI_GOLD);
				SetGoldItemValue(holditem, val);
				if (!GoldAutoPlace(pnum, holditem))
					PlrDeadItem(holditem, p);
			}
		}
	}
	CalculateGold(pnum);
	copy_pod(*holditem, tmpItem);
}
#endif

void SyncPlrKill(int pnum, int earflag)
{
	if (currlevel == 0) {
		PlrSetHp(pnum, 64);
		return;
	}

	StartPlrKill(pnum, earflag);
}

void RemovePlrMissiles(int pnum)
{
	int i, mi;

	if (currlevel != 0 && pnum == myplr && (monster[myplr]._mx != 1 || monster[myplr]._my != 0)) {
		MonStartKill(myplr, myplr);
		AddDead(myplr);
		DeleteMonsterList();
	}

	for (i = 0; i < nummissiles; i++) {
		mi = missileactive[i];
		if (missile[mi]._miSource != pnum)
			continue;
		if (missile[mi]._miType == MIS_STONE) {
			monster[missile[mi]._miVar2]._mmode = missile[mi]._miVar1;
		} else if (missile[mi]._miType == MIS_ETHEREALIZE) {
			ClearMissileSpot(mi);
			DeleteMissile(mi, i);
			i--;
		}
	}
}

static void InitLevelChange(int pnum)
{
	PlayerStruct *p;

	RemovePlrMissiles(pnum);
	if (pnum == myplr && qtextflag) {
		qtextflag = FALSE;
		stream_stop();
	}

	RemovePlrFromMap(pnum);
	ClrPlrPath(pnum);
	p = &plr[pnum];
	SetPlayerOld(p);
	if (pnum == myplr) {
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

	if ((DWORD)pnum >= MAX_PLRS) {
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
		setlvlnum = lvl;
		break;
	case WM_DIABTWARPUP:
		if (pnum == myplr)
			plr[pnum].pTownWarps |= 1 << (leveltype - 2);
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
			NetSendCmdParam2(TRUE, CMD_NEWLVL, fom, lvl);
		}
	}
}

void RestartTownLvl(int pnum)
{
	InitLevelChange(pnum);
	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("RestartTownLvl: illegal player %d", pnum);
	}

	plr[pnum].plrlevel = 0;
	plr[pnum]._pInvincible = FALSE;

	PlrSetHp(pnum, 64);
	PlrSetMana(pnum, 0);

	CalcPlrInv(pnum, FALSE);

	if (pnum == myplr) {
		deathflag = FALSE;
		gamemenu_off();
		plr[pnum]._pmode = PM_NEWLVL;
		plr[pnum]._pInvincible = TRUE;
		PostMessage(WM_DIABRETOWN, 0, 0);
	}
}

void StartWarpLvl(int pnum, int pidx)
{
	InitLevelChange(pnum);

	if (gbMaxPlayers != 1) {
		if (plr[pnum].plrlevel != 0) {
			plr[pnum].plrlevel = 0;
		} else {
			plr[pnum].plrlevel = portal[pidx].level;
		}
	}

	if (pnum == myplr) {
		SetCurrentPortal(pidx);
		plr[pnum]._pmode = PM_NEWLVL;
		plr[pnum]._pInvincible = TRUE;
		PostMessage(WM_DIABWARPLVL, 0, 0);
	}
}

static BOOL PlrDoStand(int pnum)
{
	return FALSE;
}

static BOOL PlrDoWalk(int pnum)
{
	PlayerStruct *p;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("PlrDoWalk: illegal player %d", pnum);
	}

	p = &plr[pnum];
	p->_pVar8++;
	if (p->_pIFlags2 & ISPH_FASTESTWALK) {
		p->_pAnimFrame++;
	} else if (p->_pIFlags2 & ISPH_FASTERWALK) {
		if ((p->_pVar8 & 1) == 1)
			p->_pAnimFrame++;
	} else if (p->_pIFlags2 & ISPH_FASTWALK) {
		if ((p->_pVar8 & 3) == 2)
			p->_pAnimFrame++;
	}

	if ((p->_pAnimFrame & 3) == 3) {
		PlaySfxLoc(PS_WALK1, p->_px, p->_py);
	}

	if (p->_pAnimFrame < p->_pWFrames) {
		PlrChangeOffset(pnum);
		return FALSE;
	}

	dPlayer[p->_px][p->_py] = 0;
	p->_px += p->_pVar1;
	p->_py += p->_pVar2;
	dPlayer[p->_px][p->_py] = pnum + 1;

	if (leveltype != DTYPE_TOWN) {
		ChangeLightXY(p->_plid, p->_px, p->_py);
		ChangeVisionXY(p->_pvid, p->_px, p->_py);
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

	if (leveltype != DTYPE_TOWN) {
		ChangeLightOff(p->_plid, 0, 0);
	}
	return TRUE;
}

static BOOL PlrDoWalk2(int pnum)
{
	PlayerStruct *p;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("PlrDoWalk2: illegal player %d", pnum);
	}
	p = &plr[pnum];
	p->_pVar8++;
	if (p->_pIFlags2 & ISPH_FASTESTWALK) {
		p->_pAnimFrame++;
	} else if (p->_pIFlags2 & ISPH_FASTERWALK) {
		if ((p->_pVar8 & 1) == 1)
			p->_pAnimFrame++;
	} else if (p->_pIFlags2 & ISPH_FASTWALK) {
		if ((p->_pVar8 & 3) == 2)
			p->_pAnimFrame++;
	}

	if ((p->_pAnimFrame & 3) == 3) {
		PlaySfxLoc(PS_WALK1, p->_px, p->_py);
	}

	if (p->_pAnimFrame < p->_pWFrames) {
		PlrChangeOffset(pnum);
		return FALSE;
	}
	dPlayer[p->_pVar1][p->_pVar2] = 0;

	if (leveltype != DTYPE_TOWN) {
		ChangeLightXY(p->_plid, p->_px, p->_py);
		ChangeVisionXY(p->_pvid, p->_px, p->_py);
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
	if (leveltype != DTYPE_TOWN) {
		ChangeLightOff(p->_plid, 0, 0);
	}
	return TRUE;
}

static BOOL PlrDoWalk3(int pnum)
{
	PlayerStruct *p;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("PlrDoWalk3: illegal player %d", pnum);
	}
	p = &plr[pnum];
	p->_pVar8++;
	if (p->_pIFlags2 & ISPH_FASTESTWALK) {
		p->_pAnimFrame++;
	} else if (p->_pIFlags2 & ISPH_FASTERWALK) {
		if ((p->_pVar8 & 1) == 1)
			p->_pAnimFrame++;
	} else if (p->_pIFlags2 & ISPH_FASTWALK) {
		if ((p->_pVar8 & 3) == 2)
			p->_pAnimFrame++;
	}
	if ((p->_pAnimFrame & 3) == 3) {
		PlaySfxLoc(PS_WALK1, p->_px, p->_py);
	}

	if (p->_pAnimFrame < p->_pWFrames) {
		PlrChangeOffset(pnum);
		return FALSE;
	}
	dPlayer[p->_px][p->_py] = 0;
	dFlags[p->_pVar4][p->_pVar5] &= ~BFLAG_PLAYERLR;
	p->_px = p->_pVar1;
	p->_py = p->_pVar2;
	dPlayer[p->_px][p->_py] = pnum + 1;

	if (leveltype != DTYPE_TOWN) {
		ChangeLightXY(p->_plid, p->_px, p->_py);
		ChangeVisionXY(p->_pvid, p->_px, p->_py);
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

	if (leveltype != DTYPE_TOWN) {
		ChangeLightOff(p->_plid, 0, 0);
	}
	return TRUE;
}

static BOOL WeaponDur(int pnum, int durrnd)
{
	PlayerStruct *p;
	ItemStruct *pi;
	if (pnum != myplr) {
		return FALSE;
	}

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("WeaponDur: illegal player %d", pnum);
	}

	p = &plr[pnum];
#ifdef HELLFIRE
	pi = &p->InvBody[INVLOC_HAND_LEFT];
	if (pi->_itype != ITYPE_NONE && pi->_iClass == ICLASS_WEAPON && pi->_iFlags2 & ISPH_DECAY) {
		pi->_iPLDam -= 5;
		if (pi->_iPLDam <= -100) {
			NetSendCmdDelItem(TRUE, INVLOC_HAND_LEFT);
			pi->_itype = ITYPE_NONE;
			CalcPlrInv(pnum, TRUE);
			return TRUE;
		}
		CalcPlrInv(pnum, TRUE);
	}

	pi = &p->InvBody[INVLOC_HAND_RIGHT];
	if (pi->_itype != ITYPE_NONE && pi->_iClass == ICLASS_WEAPON && pi->_iFlags2 & ISPH_DECAY) {
		pi->_iPLDam -= 5;
		if (pi->_iPLDam <= -100) {
			NetSendCmdDelItem(TRUE, INVLOC_HAND_RIGHT);
			pi->_itype = ITYPE_NONE;
			CalcPlrInv(pnum, TRUE);
			return TRUE;
		}
		CalcPlrInv(pnum, TRUE);
	}
#endif

	if (random_(3, durrnd) != 0) {
		return FALSE;
	}

	pi = &p->InvBody[INVLOC_HAND_LEFT];
	if (pi->_itype != ITYPE_NONE && pi->_iClass == ICLASS_WEAPON) {
		if (pi->_iDurability == DUR_INDESTRUCTIBLE) {
			return FALSE;
		}

		pi->_iDurability--;
		if (pi->_iDurability == 0) {
			NetSendCmdDelItem(TRUE, INVLOC_HAND_LEFT);
			pi->_itype = ITYPE_NONE;
			CalcPlrInv(pnum, TRUE);
			return TRUE;
		}
	}

	pi = &p->InvBody[INVLOC_HAND_RIGHT];
	if (pi->_itype != ITYPE_NONE && pi->_iClass == ICLASS_WEAPON) {
		if (pi->_iDurability == DUR_INDESTRUCTIBLE) {
			return FALSE;
		}

		pi->_iDurability--;
		if (pi->_iDurability == 0) {
			NetSendCmdDelItem(TRUE, INVLOC_HAND_RIGHT);
			pi->_itype = ITYPE_NONE;
			CalcPlrInv(pnum, TRUE);
			return TRUE;
		}
	}

	if (p->InvBody[INVLOC_HAND_LEFT]._itype == ITYPE_NONE && pi->_itype == ITYPE_SHIELD) {
		if (pi->_iDurability == DUR_INDESTRUCTIBLE) {
			return FALSE;
		}

		pi->_iDurability--;
		if (pi->_iDurability == 0) {
			NetSendCmdDelItem(TRUE, INVLOC_HAND_RIGHT);
			pi->_itype = ITYPE_NONE;
			CalcPlrInv(pnum, TRUE);
			return TRUE;
		}
	}

	pi = &p->InvBody[INVLOC_HAND_LEFT];
	if (p->InvBody[INVLOC_HAND_RIGHT]._itype == ITYPE_NONE && pi->_itype == ITYPE_SHIELD) {
		if (pi->_iDurability == DUR_INDESTRUCTIBLE) {
			return FALSE;
		}

		pi->_iDurability--;
		if (pi->_iDurability == 0) {
			NetSendCmdDelItem(TRUE, INVLOC_HAND_LEFT);
			pi->_itype = ITYPE_NONE;
			CalcPlrInv(pnum, TRUE);
			return TRUE;
		}
	}

	return FALSE;
}

int PlrAtkDam(int pnum)
{
	PlayerStruct *p;
	int dam;

	p = &plr[pnum];
	dam = RandRange(p->_pIMinDam, p->_pIMaxDam);
	dam += dam * p->_pIBonusDam / 100;
	dam += p->_pDamageMod + p->_pIBonusDamMod;
#ifdef HELLFIRE
	if (p->_pClass == PC_WARRIOR || p->_pClass == PC_BARBARIAN) {
#else
	if (p->_pClass == PC_WARRIOR) {
#endif
		if (random_(6, 100) < p->_pLevel) {
			dam <<= 1;
		}
	}
	return dam;
}

static BOOL PlrHitMonst(int pnum, int mnum)
{
	PlayerStruct *p;
	MonsterStruct *mon;
	BOOL ret;
	int hper, tmac, dam, skdam, phanditype;
#ifdef HELLFIRE
	BOOL adjacentDamage = FALSE;
#endif

	if ((DWORD)mnum >= MAXMONSTERS) {
		app_fatal("PlrHitMonst: illegal monster %d", mnum);
	}

	if (CheckMonsterHit(mnum, &ret)) {
		return ret;
	}

#ifdef HELLFIRE
	if (pnum < 0) {
		adjacentDamage = TRUE;
		pnum = -(pnum + 1);
	}
#endif

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("PlrHitMonst: illegal player %d", pnum);
	}

	p = &plr[pnum];
	mon = &monster[mnum];

	tmac = mon->mArmorClass;
#ifdef HELLFIRE
	if (p->_pIEnAc > 0) {
		int _pIEnAc = p->_pIEnAc - 1;
		if (_pIEnAc > 0)
			tmac >>= _pIEnAc;
		else
			tmac -= tmac >> 2;

		if (p->_pClass == PC_BARBARIAN) {
			tmac -= mon->mArmorClass / 8;
		}

		if (tmac < 0)
			tmac = 0;
	}
#else
	tmac -= p->_pIEnAc;
#endif
	hper = 50 + (p->_pDexterity >> 1) + p->_pIBonusToHit + p->_pLevel
		- tmac;

#ifdef HELLFIRE
	if (adjacentDamage) {
		if (p->_pLevel > 20)
			hper -= 30;
		else
			hper -= (35 - p->_pLevel) * 2;
	}
#endif

	if (p->_pClass == PC_WARRIOR)
		hper += 20;
	if (hper < 5)
		hper = 5;
	if (hper > 95)
		hper = 95;
	if (random_(4, 100) >= hper && mon->_mmode != MM_STONE)
#ifdef _DEBUG
		if (!debug_mode_god_mode)
#endif
			return FALSE;

	dam = PlrAtkDam(pnum);

#ifdef HELLFIRE
	int dam2 = dam;
#endif

	phanditype = ITYPE_NONE;
	if (p->InvBody[INVLOC_HAND_LEFT]._itype == ITYPE_SWORD || p->InvBody[INVLOC_HAND_RIGHT]._itype == ITYPE_SWORD) {
		phanditype = ITYPE_SWORD;
	}
	if (p->InvBody[INVLOC_HAND_LEFT]._itype == ITYPE_MACE || p->InvBody[INVLOC_HAND_RIGHT]._itype == ITYPE_MACE) {
		phanditype = ITYPE_MACE;
	}

	switch (mon->MData->mMonstClass) {
	case MC_UNDEAD:
		if (phanditype == ITYPE_SWORD) {
			dam -= dam >> 1;
		}
		if (phanditype == ITYPE_MACE) {
			dam += dam >> 1;
		}
		break;
	case MC_DEMON:
		if (p->_pIFlags & ISPL_3XDAMVDEM) {
			dam *= 3;
		}
		break;
	case MC_ANIMAL:
		if (phanditype == ITYPE_MACE) {
			dam -= dam >> 1;
		}
		if (phanditype == ITYPE_SWORD) {
			dam += dam >> 1;
		}
		break;
	}

#ifdef HELLFIRE
	if (p->_pIFlags2 & ISPH_DEVASTATION && random_(6, 100) < 5) {
		dam *= 3;
	}

	if ((p->_pIFlags2 & ISPH_DOPPELGANGER) && mon->MType->mtype != MT_DIABLO && mon->_uniqtype == 0 && random_(6, 100) < 10) {
		MonDoppel(mnum);
	}
#endif

	dam <<= 6;

#ifdef HELLFIRE
	if (p->_pIFlags2 & ISPH_JESTERS) {
		int r = random_(6, 257);
		if (r >= 128)
			r = 128 + (r - 128) * 5;
		dam = dam * r / 128;
	}

	if (adjacentDamage)
		dam >>= 2;
#endif

	if (pnum == myplr) {
#ifdef HELLFIRE
		if (p->_pIFlags2 & ISPH_PERIL) {
			dam2 += p->_pIGetHit;
			if (dam2 >= 0 && !p->_pInvincible) {
				dam2 <<= 6;
				if (p->_pHitPoints > dam2) {
					PlrDecHp(pnum, dam2, 0);
				} else {
					PlrSetHp(pnum, 64);
				}
			}
			dam <<= 1;
		}
#endif

		mon->_mhitpoints -= dam;
	}

	if (p->_pIFlags & ISPL_RNDSTEALLIFE) {
		skdam = random_(7, dam >> 3);
		PlrIncHp(pnum, skdam);
	}
	if (p->_pIFlags & (ISPL_STEALLIFE_3 | ISPL_STEALLIFE_5)) {
		if (p->_pIFlags & ISPL_STEALLIFE_5) {
			skdam = 5 * dam / 100;
		} else {
			skdam = 3 * dam / 100;
		}
		PlrIncHp(pnum, skdam);
	}
	if (p->_pIFlags & (ISPL_STEALMANA_3 | ISPL_STEALMANA_5)) {
		if (p->_pIFlags & ISPL_STEALMANA_5) {
			skdam = 5 * dam / 100;
		} else {
			skdam = 3 * dam / 100;
		}
		PlrIncMana(pnum, skdam);
	}
	if (p->_pIFlags & ISPL_NOHEALMON) {
		mon->_mFlags |= MFLAG_NOHEAL;
	}
#ifdef _DEBUG
	if (debug_mode_god_mode) {
		mon->_mhitpoints = 0; /* double check */
	}
#endif
	if ((mon->_mhitpoints >> 6) <= 0) {
		MonStartKill(mnum, pnum);
	} else {
		if (mon->_mmode != MM_STONE && p->_pIFlags & ISPL_KNOCKBACK) {
			MonGetKnockback(mnum);
		}
		MonStartHit(mnum, pnum, dam);
	}
	return TRUE;
}

static BOOL PlrHitPlr(int offp, char defp)
{
	PlayerStruct *ops, *dps;
	int hper, blkper, dir, dam, skdam;

	if ((DWORD)defp >= MAX_PLRS) {
		app_fatal("PlrHitPlr: illegal target player %d", defp);
	}

	dps = &plr[defp];
	if (dps->_pInvincible) {
		return FALSE;
	}

	if (dps->_pSpellFlags & PSE_ETHERALIZED) {
		return FALSE;
	}

	if ((DWORD)offp >= MAX_PLRS) {
		app_fatal("PlrHitPlr: illegal attacking player %d", offp);
	}
	ops = &plr[offp];
	hper = 50 + (ops->_pDexterity >> 1) + ops->_pLevel + ops->_pIBonusToHit
		- (dps->_pIBonusAC + dps->_pIAC + dps->_pDexterity / 5);
	if (ops->_pClass == PC_WARRIOR)
		hper += 20;
	if (hper < 5)
		hper = 5;
	if (hper > 95)
		hper = 95;
	if (random_(4, 100) >= hper)
		return FALSE;

	if (dps->_pBlockFlag
	 && (dps->_pmode == PM_STAND || dps->_pmode == PM_BLOCK)) {
		blkper = dps->_pDexterity + dps->_pBaseToBlk
			+ (dps->_pLevel << 1)
			- (ops->_pLevel << 1);
		if (blkper >= 100 || blkper > random_(5, 100)) {
			dir = GetDirection(dps->_px, dps->_py, ops->_px, ops->_py);
			PlrStartBlock(defp, dir);
			return TRUE;
		}
	}

	dam = PlrAtkDam(offp);
	dam <<= 6;
	if (ops->_pIFlags & ISPL_RNDSTEALLIFE) {
		skdam = random_(7, dam >> 3);
		PlrIncHp(offp, skdam);
	}
	if (offp == myplr) {
		NetSendCmdDwParam2(TRUE, CMD_PLRDAMAGE, defp, dam);
	}
	StartPlrHit(defp, dam, FALSE);
	return TRUE;
}

static BOOL PlrTryHit(int pnum, int dx, int dy)
{
	int mpo;

	mpo = dMonster[dx][dy];
	if (mpo != 0) {
		mpo = mpo >= 0 ? mpo - 1 : -(mpo + 1);
		return !CanTalkToMonst(mpo) && PlrHitMonst(pnum, mpo);
	}
	if (pnum < 0)
		return FALSE;
	mpo = dPlayer[dx][dy];
	if (mpo != 0 && !FriendlyMode) {
		mpo = mpo >= 0 ? mpo - 1 : -(mpo + 1);
		return PlrHitPlr(pnum, mpo);
	}
	mpo = dObject[dx][dy];
	if (mpo != 0) {
		mpo = mpo >= 0 ? mpo - 1 : -(mpo + 1);
		if (object[mpo]._oBreak == 1) {
			BreakObject(pnum, mpo);
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL PlrDoAttack(int pnum)
{
	PlayerStruct *p;
	int dx, dy;
	BOOL didhit;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("PlrDoAttack: illegal player %d", pnum);
	}

	p = &plr[pnum];
	p->_pVar8++;
	if (p->_pIFlags & ISPL_FASTESTATTACK) {
		p->_pAnimFrame++;
	} else if (p->_pIFlags & ISPL_FASTERATTACK) {
		if ((p->_pVar8 & 1) == 1)
			p->_pAnimFrame++;
	} else if (p->_pIFlags & ISPL_FASTATTACK) {
		if ((p->_pVar8 & 3) == 2)
			p->_pAnimFrame++;
	} else if (p->_pIFlags & ISPL_QUICKATTACK) {
		if ((p->_pVar8 & 7) == 4)
			p->_pAnimFrame++;
	}
	if (p->_pAnimFrame < p->_pAFNum - 1)
		return FALSE;
	if (p->_pVar7 == 0) {
		p->_pVar7++;
		PlaySfxLoc(PS_SWING, p->_px, p->_py, 2);
	}
	if (p->_pAnimFrame == p->_pAFNum - 1) {
		return FALSE;
	}
	if (p->_pVar7 == 1) {
		p->_pVar7++;
		dx = p->_px + offset_x[p->_pdir];
		dy = p->_py + offset_y[p->_pdir];

		if (p->_pIFlags & ISPL_FIREDAM) {
			AddMissile(dx, dy, 0, 0, 0, MIS_WEAPFEXP, 0, pnum, 0, 0);
		}
		if (p->_pIFlags & ISPL_LIGHTDAM) {
			AddMissile(dx, dy, 0, 0, 0, MIS_WEAPLEXP, 0, pnum, 0, 0);
		}
#ifdef HELLFIRE
		if (p->_pIFlags & ISPL_SPECDAM) {
			int midam = RandRange(p->_pILMinDam, p->_pILMaxDam);
			AddMissile(p->_px, p->_py, dx, dy, p->_pdir, MIS_SPECARROW, 0, pnum, midam, MIS_CBOLTARROW);
		}
#endif

		didhit = PlrTryHit(pnum, dx, dy);
#ifdef HELLFIRE
		if (p->_pIFlags2 & ISPH_SWIPE) {
			dx = p->_px + offset_x[(p->_pdir + 1) % 8];
			dy = p->_py + offset_y[(p->_pdir + 1) % 8];
			didhit |= PlrTryHit(-(pnum + 1), dx, dy);

			dx = p->_px + offset_x[(p->_pdir + 7) % 8];
			dy = p->_py + offset_y[(p->_pdir + 7) % 8];
			didhit |= PlrTryHit(-(pnum + 1), dx, dy);
		}
#endif

		if (didhit && WeaponDur(pnum, 30)) {
			PlrStartStand(pnum, p->_pdir);
			ClearPlrPVars(pnum);
			return TRUE;
		}
	}

	if (p->_pAnimFrame < p->_pAFrames)
		return FALSE;

	PlrStartStand(pnum, p->_pdir);
	ClearPlrPVars(pnum);
	return TRUE;
}

static BOOL PlrDoRangeAttack(int pnum)
{
	PlayerStruct *p;
	int mitype;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("PlrDoRangeAttack: illegal player %d", pnum);
	}

	p = &plr[pnum];
	p->_pVar8++;
	if (p->_pIFlags & ISPL_FASTESTATTACK) {
		p->_pAnimFrame++;
	} else if (p->_pIFlags & ISPL_FASTERATTACK) {
		if ((p->_pVar8 & 1) == 1)
			p->_pAnimFrame++;
	} else if (p->_pIFlags & ISPL_FASTATTACK) {
		if ((p->_pVar8 & 3) == 2)
			p->_pAnimFrame++;
	} else if (p->_pIFlags & ISPL_QUICKATTACK) {
		if ((p->_pVar8 & 7) == 4)
			p->_pAnimFrame++;
	}
	if (p->_pAnimFrame >= p->_pAFNum) {
		mitype = MIS_ARROW;
		if (p->_pIFlags & ISPL_FIRE_ARROWS) {
			mitype = MIS_FARROW;
		}
		if (p->_pIFlags & ISPL_LIGHT_ARROWS) {
			mitype = MIS_LARROW;
		}
		AddMissile(p->_px, p->_py, p->_pVar1, p->_pVar2, p->_pdir, mitype, 0, pnum, 0, 0);

		PlaySfxLoc(PS_BFIRE, p->_px, p->_py);

		if (WeaponDur(pnum, 40)) {
			PlrStartStand(pnum, p->_pdir);
			ClearPlrPVars(pnum);
			return TRUE;
		}
	}

	if (p->_pAnimFrame < p->_pAFrames)
		return FALSE;

	PlrStartStand(pnum, p->_pdir);
	ClearPlrPVars(pnum);
	return TRUE;
}

static void ShieldDur(int pnum)
{
	ItemStruct *pi;

	if (pnum != myplr) {
		return;
	}

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("ShieldDur: illegal player %d", pnum);
	}

	pi = &plr[pnum].InvBody[INVLOC_HAND_LEFT];
	if (pi->_itype == ITYPE_SHIELD) {
		if (pi->_iDurability == DUR_INDESTRUCTIBLE) {
			return;
		}

		pi->_iDurability--;
		if (pi->_iDurability == 0) {
			NetSendCmdDelItem(TRUE, INVLOC_HAND_LEFT);
			pi->_itype = ITYPE_NONE;
			CalcPlrInv(pnum, TRUE);
		}
	}

	pi = &plr[pnum].InvBody[INVLOC_HAND_RIGHT];
	if (pi->_itype == ITYPE_SHIELD) {
		if (pi->_iDurability != DUR_INDESTRUCTIBLE) {
			pi->_iDurability--;
			if (pi->_iDurability == 0) {
				NetSendCmdDelItem(TRUE, INVLOC_HAND_RIGHT);
				pi->_itype = ITYPE_NONE;
				CalcPlrInv(pnum, TRUE);
			}
		}
	}
}

void PlrStartBlock(int pnum, int dir)
{
	if ((DWORD)pnum >= MAX_PLRS) {
		dev_fatal("PlrStartBlock: illegal player %d", pnum);
	}

	if (plr[pnum]._pmode != PM_BLOCK)
		StartBlock(pnum, dir);

	PlaySfxLoc(IS_ISWORD, plr[pnum]._px, plr[pnum]._py);
	if (random_(3, 10) == 0) {
		ShieldDur(pnum);
	}
}

static BOOL PlrDoBlock(int pnum)
{
	PlayerStruct *p;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("PlrDoBlock: illegal player %d", pnum);
	}
	p = &plr[pnum];
	if (p->_pIFlags & ISPL_FASTBLOCK && p->_pAnimCnt == 0 && p->_pAnimFrame != 1) {
		p->_pAnimFrame++;
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
				return TRUE;
			}
		}
		p->_pVar1--;
	}

	return FALSE;
}

static void ArmorDur(int pnum)
{
	ItemStruct *pi, *pio;
	inv_body_loc loc;

	if (pnum != myplr) {
		return;
	}

	if ((DWORD)pnum >= MAX_PLRS) {
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

	NetSendCmdDelItem(TRUE, loc);
	pi->_itype = ITYPE_NONE;
	CalcPlrInv(pnum, TRUE);
}

static BOOL PlrDoSpell(int pnum)
{
	PlayerStruct *p;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("PlrDoSpell: illegal player %d", pnum);
	}
	p = &plr[pnum];
	p->_pVar8++;
	if (p->_pIFlags2 & ISPH_FASTESTCAST) {
		p->_pAnimFrame++;
	} else if (p->_pIFlags2 & ISPH_FASTERCAST) {
		if ((p->_pVar8 & 1) == 1)
			p->_pAnimFrame++;
	} else if (p->_pIFlags2 & ISPH_FASTCAST) {
		if ((p->_pVar8 & 3) == 2)
			p->_pAnimFrame++;
	}

	if (p->_pAnimFrame < p->_pSFNum)
		return FALSE;

	if (!p->_pVar7) {
		p->_pVar7 = TRUE;

		if (HasMana(pnum, p->_pVar3, p->_pVar5)
		 && AddMissile(p->_px, p->_py, p->_pVar1, p->_pVar2, p->_pdir,
				spelldata[p->_pVar3].sMissile, 0, pnum, 0, p->_pVar4) != -1) {
			UseMana(pnum, p->_pVar3, p->_pVar5);
		}
	}

	if (p->_pAnimFrame < p->_pSFrames)
		return FALSE;

	PlrStartStand(pnum, p->_pdir);
	ClearPlrPVars(pnum);
	return TRUE;
}

static BOOL PlrDoGotHit(int pnum)
{
	PlayerStruct *p;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("PlrDoGotHit: illegal player %d", pnum);
	}
	p = &plr[pnum];
	p->_pVar8++;
	if (p->_pIFlags & ISPL_FASTESTRECOVER) {
		p->_pAnimFrame++;
	} else if (p->_pIFlags & ISPL_FASTERRECOVER) {
		if ((p->_pVar8 & 1) == 1)
			p->_pAnimFrame++;
	} else if (p->_pIFlags & ISPL_FASTRECOVER) {
		if ((p->_pVar8 & 3) == 2)
			p->_pAnimFrame++;
	}

	if (p->_pAnimFrame < p->_pHFrames)
		return FALSE;
	PlrStartStand(pnum, p->_pdir);
	ClearPlrPVars(pnum);
	if (random_(3, 4) != 0) {
		ArmorDur(pnum);
	}

	return TRUE;
}

static BOOL PlrDoDeath(int pnum)
{
	PlayerStruct *p;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("PlrDoDeath: illegal player %d", pnum);
	}

	p = &plr[pnum];
	if ((unsigned)p->_pVar8 >= 2 * p->_pDFrames) {
		if (deathdelay > 1 && pnum == myplr) {
			deathdelay--;
			if (deathdelay == 1) {
				deathflag = TRUE;
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

	return FALSE;
}

static BOOL PlrDoNewLvl(int pnum)
{
	return FALSE;
}

static void CheckNewPath(int pnum)
{
	PlayerStruct *p;
	int i, x, y;
	int xvel3, xvel, yvel;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("CheckNewPath: illegal player %d", pnum);
	}
	p = &plr[pnum];
	if (p->destAction == ACTION_ATTACKMON) {
		MakePlrPath(pnum, monster[p->destParam1]._mfutx, monster[p->destParam1]._mfuty, FALSE);
	} else if (p->destAction == ACTION_ATTACKPLR) {
		MakePlrPath(pnum, plr[p->destParam1]._pfutx, plr[p->destParam1]._pfuty, FALSE);
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

			if (currlevel != 0) {
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
				x = abs(p->_px - item[i]._ix);
				y = abs(p->_py - item[i]._iy);
				if (x <= 1 && y <= 1 && pcurs == CURSOR_HAND && !item[i]._iRequest) {
					NetSendCmdGItem(TRUE, CMD_REQUESTGITEM, myplr, myplr, i);
					item[i]._iRequest = TRUE;
				}
			}
			break;
		case ACTION_PICKUPAITEM:
			if (pnum == myplr) {
				i = p->destParam1;
				x = abs(p->_px - item[i]._ix);
				y = abs(p->_py - item[i]._iy);
				if (x <= 1 && y <= 1 && pcurs == CURSOR_HAND) {
					NetSendCmdGItem(TRUE, CMD_REQUESTAGITEM, myplr, myplr, i);
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
			OperateObject(pnum, i, TRUE);
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

static BOOL PlrDeathModeOK(int pnum)
{
	PlayerStruct *p;

	if (pnum != myplr) {
		return TRUE;
	}

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("PlrDeathModeOK: illegal player %d", pnum);
	}

	p = &plr[pnum];
	return p->_pmode == PM_DEATH || p->_pmode == PM_QUIT || p->_pmode == PM_NEWLVL;
}

static void ValidatePlayer()
{
	PlayerStruct *p;
	ItemStruct *pi;
	__int64 msk;
	int gt, pc, i, limit;

	if ((DWORD)myplr >= MAX_PLRS) {
		app_fatal("ValidatePlayer: illegal player %d", myplr);
	}
	p = &plr[myplr];
	if (p->_pLevel > MAXCHARLEVEL)
		p->_pLevel = MAXCHARLEVEL;
	if (p->_pExperience > p->_pNextExper)
		p->_pExperience = p->_pNextExper;

#ifdef HELLFIRE
	limit = auricGold; // BUGFIX: change to MaxGold? Why would auricGold be used here?
#else
	limit = GOLD_MAX_LIMIT;
#endif
	gt = 0;
	pi = p->InvList;
	for (i = p->_pNumInv; i != 0; i--, pi++) {
		if (pi->_itype == ITYPE_GOLD) {
			if (pi->_ivalue > limit)
				pi->_ivalue = limit;
			gt += pi->_ivalue;
		}
	}
	p->_pGold = gt;

	pc = p->_pClass;
	if (p->_pBaseStr > MaxStats[pc][ATTRIB_STR]) {
		p->_pBaseStr = MaxStats[pc][ATTRIB_STR];
	}
	if (p->_pBaseMag > MaxStats[pc][ATTRIB_MAG]) {
		p->_pBaseMag = MaxStats[pc][ATTRIB_MAG];
	}
	if (p->_pBaseDex > MaxStats[pc][ATTRIB_DEX]) {
		p->_pBaseDex = MaxStats[pc][ATTRIB_DEX];
	}
	if (p->_pBaseVit > MaxStats[pc][ATTRIB_VIT]) {
		p->_pBaseVit = MaxStats[pc][ATTRIB_VIT];
	}

	msk = 0;
	for (i = 1; i < NUM_SPELLS; i++) {
		if (spelldata[i].sBookLvl != SPELL_NA) {
			msk |= SPELL_MASK(i);
			if (p->_pSplLvl[i] > MAXSPLLEVEL)
				p->_pSplLvl[i] = MAXSPLLEVEL;
		}
	}

	p->_pMemSpells &= msk;
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
	BOOL tplayer;

	if ((DWORD)myplr >= MAX_PLRS) {
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
		if (plr[pnum].plractive && currlevel == plr[pnum].plrlevel && (pnum == myplr || !plr[pnum]._pLvlChanging)) {
			CheckCheatStats(pnum);

			if (!PlrDeathModeOK(pnum) && plr[pnum]._pHitPoints < (1 << 6)) {
				SyncPlrKill(pnum, -1);
			}

			if (pnum == myplr) {
				if ((plr[pnum]._pIFlags & ISPL_DRAINLIFE) && currlevel != 0 && !plr[pnum]._pInvincible) {
					PlrDecHp(pnum, 4, 0);
				}
				if (plr[pnum]._pIFlags & ISPL_NOMANA && plr[pnum]._pMana > 0) {
					PlrSetMana(pnum, 0);
				}
			}

			tplayer = FALSE;
			do {
				switch (plr[pnum]._pmode) {
				case PM_STAND:
					tplayer = PlrDoStand(pnum);
					break;
				case PM_WALK:
					tplayer = PlrDoWalk(pnum);
					break;
				case PM_WALK2:
					tplayer = PlrDoWalk2(pnum);
					break;
				case PM_WALK3:
					tplayer = PlrDoWalk3(pnum);
					break;
				case PM_ATTACK:
					tplayer = PlrDoAttack(pnum);
					break;
				case PM_RATTACK:
					tplayer = PlrDoRangeAttack(pnum);
					break;
				case PM_BLOCK:
					tplayer = PlrDoBlock(pnum);
					break;
				case PM_SPELL:
					tplayer = PlrDoSpell(pnum);
					break;
				case PM_GOTHIT:
					tplayer = PlrDoGotHit(pnum);
					break;
				case PM_DEATH:
					tplayer = PlrDoDeath(pnum);
					break;
				case PM_NEWLVL:
					tplayer = PlrDoNewLvl(pnum);
					break;
				}
				CheckNewPath(pnum);
			} while (tplayer);

			plr[pnum]._pAnimCnt++;
			if (plr[pnum]._pAnimCnt > plr[pnum]._pAnimDelay) {
				plr[pnum]._pAnimCnt = 0;
				plr[pnum]._pAnimFrame++;
				if (plr[pnum]._pAnimFrame > plr[pnum]._pAnimLen) {
					plr[pnum]._pAnimFrame = 1;
				}
			}
		}
	}
}

void ClrPlrPath(int pnum)
{
	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("ClrPlrPath: illegal player %d", pnum);
	}

	memset(plr[pnum].walkpath, WALK_NONE, sizeof(plr[pnum].walkpath));
}

BOOL PosOkPlayer(int pnum, int x, int y)
{
	int mpo;

	if (IN_DUNGEON_AREA(x, y) && dPiece[x][y] != 0 && !nSolidTable[dPiece[x][y]]) {
		mpo = dPlayer[x][y];
		if (mpo != 0) {
			mpo = mpo >= 0 ? mpo - 1 : -(mpo + 1);
			if (mpo != pnum && plr[mpo]._pHitPoints >= (1 << 6)) {
				return FALSE;
			}
		}
		mpo = dMonster[x][y];
		if (mpo != 0) {
			if (mpo < 0) {
				return FALSE;
			}
			if ((monster[mpo - 1]._mhitpoints >> 6) > 0) {
				return FALSE;
			}
			if (currlevel == 0) {
				return FALSE;
			}
		}
		mpo = dObject[x][y];
		if (mpo != 0) {
			mpo = mpo >= 0 ? mpo - 1 : -(mpo + 1);
			if (object[mpo]._oSolidFlag) {
				return FALSE;
			}
		}

		return TRUE;
	}

	return FALSE;
}

void MakePlrPath(int pnum, int xx, int yy, BOOL endspace)
{
	int path;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("MakePlrPath: illegal player %d", pnum);
	}

	plr[pnum]._ptargx = xx;
	plr[pnum]._ptargy = yy;
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

		plr[pnum]._ptargx = xx;
		plr[pnum]._ptargy = yy;
	}

	plr[pnum].walkpath[path] = WALK_NONE;
}

void SyncPlrAnim(int pnum)
{
	PlayerStruct *p;
	int sType;
	unsigned char** anim;

	if ((DWORD)pnum >= MAX_PLRS) {
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
			sType = spelldata[p->_pSpell].sType;
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
	plr[pnum]._ptargx = plr[pnum]._px;
	plr[pnum]._ptargy = plr[pnum]._py;

	if (gbMaxPlayers == 1 || plr[pnum].plrlevel != currlevel)
		return;

	if (PlacePlayer(pnum))
		FixPlayerLocation(pnum);
	dPlayer[plr[pnum]._px][plr[pnum]._py] = pnum + 1;
}

void SyncInitPlr(int pnum)
{
	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("SyncInitPlr: illegal player %d", pnum);
	}

	SetPlrAnims(pnum);
	SyncInitPlrPos(pnum);
}

/*void CheckStats(int pnum)
{
	PlayerStruct *p;
	const int *stats;

	if ((DWORD)pnum >= MAX_PLRS) {
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
	if ((DWORD)pnum >= MAX_PLRS) {
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
	if ((DWORD)pnum >= MAX_PLRS) {
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
	if ((DWORD)pnum >= MAX_PLRS) {
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
	if ((DWORD)pnum >= MAX_PLRS) {
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

BOOL PlrDecHp(int pnum, int hp, int earflag)
{
	PlayerStruct *p;

	assert(hp >= 0);
	p = &plr[pnum];
	if (p->pManaShield != 0) {
#ifdef HELLFIRE
		int div = 19 - (std::min((int)p->pManaShield, 8) << 1);
#else
		int div = 3;
#endif
		hp -= hp / div;
		if (p->_pMana >= hp) {
			PlrDecMana(pnum, hp);
			return FALSE;
		}
		hp -= p->_pMana;
		PlrSetMana(pnum, 0);
		if (pnum == myplr)
			NetSendCmd(TRUE, CMD_REMSHIELD);
	}
	p->_pHPBase -= hp;
	p->_pHitPoints -= hp;
	if (p->_pHitPoints < (1 << 6)) {
		SyncPlrKill(pnum, earflag);
		return TRUE;
	}
	if (pnum == myplr)
		gbRedrawFlags |= REDRAW_HP_FLASK;
	return FALSE;
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

void ModifyPlrStr(int pnum, int v)
{
	PlayerStruct *p;
	int val;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("ModifyPlrStr: illegal player %d", pnum);
	}
	p = &plr[pnum];
	val = p->_pBaseStr;
	v = std::max(std::min(v, MaxStats[p->_pClass][ATTRIB_STR] - val), -val);

	p->_pStrength += v;
	p->_pBaseStr += v;

	CalcPlrInv(pnum, TRUE);

	if (pnum == myplr) {
		NetSendCmdParam1(FALSE, CMD_SETSTR, p->_pBaseStr); //60
	}
}

void ModifyPlrMag(int pnum, int v)
{
	PlayerStruct *p;
	int val, ms;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("ModifyPlrMag: illegal player %d", pnum);
	}
	p = &plr[pnum];
	val = p->_pBaseMag;
	v = std::max(std::min(v, MaxStats[p->_pClass][ATTRIB_MAG] - val), -val);

	p->_pMagic += v;
	p->_pBaseMag += v;

	ms = v << 6;
	if (p->_pClass == PC_SORCERER) {
		ms <<= 1;
#ifdef HELLFIRE
	} else if (p->_pClass == PC_BARD) {
		ms += ms >> 1;
#endif
	}

	p->_pMaxManaBase += ms;
	p->_pMaxMana += ms;
	if (!(p->_pIFlags & ISPL_NOMANA)) {
		p->_pManaBase += ms;
		p->_pMana += ms;
	}

	CalcPlrInv(pnum, TRUE);

	if (pnum == myplr) {
		NetSendCmdParam1(FALSE, CMD_SETMAG, p->_pBaseMag);
	}
}

void ModifyPlrDex(int pnum, int v)
{
	PlayerStruct *p;
	int val;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("ModifyPlrDex: illegal player %d", pnum);
	}
	p = &plr[pnum];
	val = p->_pBaseDex;
	v = std::max(std::min(v, MaxStats[p->_pClass][ATTRIB_DEX] - val), -val);

	p->_pDexterity += v;
	p->_pBaseDex += v;

	CalcPlrInv(pnum, TRUE);

	if (pnum == myplr) {
		NetSendCmdParam1(FALSE, CMD_SETDEX, p->_pBaseDex);
	}
}

void ModifyPlrVit(int pnum, int v)
{
	PlayerStruct *p;
	int val, ms;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("ModifyPlrVit: illegal player %d", pnum);
	}
	p = &plr[pnum];
	val = p->_pBaseVit;
	v = std::max(std::min(v, MaxStats[p->_pClass][ATTRIB_VIT] - val), -val);

	p->_pVitality += v;
	p->_pBaseVit += v;

	ms = v << 6;
	if (p->_pClass == PC_WARRIOR) {
		ms <<= 1;
#ifdef HELLFIRE
	} else if (p->_pClass == PC_BARBARIAN) {
		ms <<= 1;
#endif
	}

	p->_pHPBase += ms;
	p->_pMaxHPBase += ms;
	p->_pHitPoints += ms;
	p->_pMaxHP += ms;

	CalcPlrInv(pnum, TRUE);

	if (pnum == myplr) {
		NetSendCmdParam1(FALSE, CMD_SETVIT, p->_pBaseVit);
	}
}

void SetPlrStr(int pnum, int v)
{
	PlayerStruct *p;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("SetPlrStr: illegal player %d", pnum);
	}
	p = &plr[pnum];

	v = std::max(0, std::min(v, MaxStats[p->_pClass][ATTRIB_STR]));

	p->_pBaseStr = v;

	CalcPlrInv(pnum, TRUE);
}

void SetPlrMag(int pnum, int v)
{
	PlayerStruct *p;
	int m;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("SetPlrMag: illegal player %d", pnum);
	}
	p = &plr[pnum];

	v = std::max(0, std::min(v, MaxStats[p->_pClass][ATTRIB_MAG]));

	p->_pBaseMag = v;

	m = v << 6;
	if (p->_pClass == PC_SORCERER) {
		m <<= 1;
#ifdef HELLFIRE
	} else if (p->_pClass == PC_BARD) {
		m += m >> 1;
#endif
	}

	p->_pMaxManaBase = m;
	p->_pMaxMana = m;

	CalcPlrInv(pnum, TRUE);
}

void SetPlrDex(int pnum, int v)
{
	PlayerStruct *p;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("SetPlrDex: illegal player %d", pnum);
	}
	p = &plr[pnum];

	v = std::max(0, std::min(v, MaxStats[p->_pClass][ATTRIB_DEX]));

	p->_pBaseDex = v;

	CalcPlrInv(pnum, TRUE);
}

void SetPlrVit(int pnum, int v)
{
	PlayerStruct *p;
	int hp;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("SetPlrVit: illegal player %d", pnum);
	}
	p = &plr[pnum];

	v = std::max(0, std::min(v, MaxStats[p->_pClass][ATTRIB_VIT]));

	p->_pBaseVit = v;

	hp = v << 6;
	if (p->_pClass == PC_WARRIOR) {
		hp <<= 1;
#ifdef HELLFIRE
	} else if (p->_pClass == PC_BARBARIAN) {
		hp <<= 1;
#endif
	}

	p->_pHPBase = hp;
	p->_pMaxHPBase = hp;

	CalcPlrInv(pnum, TRUE);
}

void InitDungMsgs(int pnum)
{
	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("InitDungMsgs: illegal player %d", pnum);
	}

	plr[pnum].pDungMsgs = 0;
#ifdef HELLFIRE
	plr[pnum].pDungMsgs2 = 0;
#endif
}

void PlayDungMsgs()
{
	if ((DWORD)myplr >= MAX_PLRS) {
		app_fatal("PlayDungMsgs: illegal player %d", myplr);
	}

	sfxdelay = 0;

	if (gbMaxPlayers != 1)
		return;

	if (currlevel == 1 && !plr[myplr]._pLvlVisited[1] && !(plr[myplr].pDungMsgs & DMSG_CATHEDRAL)) {
		sfxdelay = 40;
		sfxdnum = sgSFXSets[SFXS_PLR_97][plr[myplr]._pClass];
		plr[myplr].pDungMsgs |= DMSG_CATHEDRAL;
	} else if (currlevel == 5 && !plr[myplr]._pLvlVisited[5] && !(plr[myplr].pDungMsgs & DMSG_CATACOMBS)) {
		sfxdelay = 40;
		sfxdnum = sgSFXSets[SFXS_PLR_96][plr[myplr]._pClass];
		plr[myplr].pDungMsgs |= DMSG_CATACOMBS;
	} else if (currlevel == 9 && !plr[myplr]._pLvlVisited[9] && !(plr[myplr].pDungMsgs & DMSG_CAVES)) {
		sfxdelay = 40;
		sfxdnum = sgSFXSets[SFXS_PLR_98][plr[myplr]._pClass];
		plr[myplr].pDungMsgs |= DMSG_CAVES;
	} else if (currlevel == 13 && !plr[myplr]._pLvlVisited[13] && !(plr[myplr].pDungMsgs & DMSG_HELL)) {
		sfxdelay = 40;
		sfxdnum = sgSFXSets[SFXS_PLR_99][plr[myplr]._pClass];
		plr[myplr].pDungMsgs |= DMSG_HELL;
	} else if (currlevel == 16 && !plr[myplr]._pLvlVisited[15] && !(plr[myplr].pDungMsgs & DMSG_DIABLO)) { // BUGFIX: _pLvlVisited should check 16 or this message will never play
		sfxdelay = 40;
		sfxdnum = PS_DIABLVLINT;
		plr[myplr].pDungMsgs |= DMSG_DIABLO;
#ifdef HELLFIRE
	} else if (currlevel == 17 && !plr[myplr]._pLvlVisited[17] && !(plr[myplr].pDungMsgs2 & DMSG2_DEFILER)) {
		sfxdelay = 10;
		sfxdnum = USFX_DEFILER1;
		quests[Q_DEFILER]._qactive = QUEST_ACTIVE;
		quests[Q_DEFILER]._qlog = TRUE;
		quests[Q_DEFILER]._qmsg = TEXT_DEFILER1;
		plr[myplr].pDungMsgs2 |= DMSG2_DEFILER;
	} else if (currlevel == 19 && !plr[myplr]._pLvlVisited[19] && !(plr[myplr].pDungMsgs2 & DMSG2_DEFILER1)) {
		sfxdelay = 10;
		sfxdnum = USFX_DEFILER3;
		plr[myplr].pDungMsgs2 |= DMSG2_DEFILER1;
	} else if (currlevel == 21 && !plr[myplr]._pLvlVisited[21] && !(plr[myplr].pDungMsgs & DMSG2_DEFILER2)) {
		sfxdelay = 30;
		sfxdnum = sgSFXSets[SFXS_PLR_92][plr[myplr]._pClass];
		plr[myplr].pDungMsgs |= DMSG2_DEFILER2;
#endif
	}
}

DEVILUTION_END_NAMESPACE
