/**
 * @file player.cpp
 *
 * Implementation of player functionality, leveling, actions, creation, loading, etc.
 */
#include "all.h"

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
int plrxoff[9] = { 0, 2, 0, 2, 1, 0, 1, 2, 1 };
/** Specifies the Y-coordinate delta from the player start location in Tristram. */
int plryoff[9] = { 0, 2, 2, 0, 1, 1, 0, 1, 2 };
/** Specifies the X-coordinate delta from a player, used for instanced when casting resurrect. */
int plrxoff2[9] = { 0, 1, 0, 1, 2, 0, 1, 2, 2 };
/** Specifies the Y-coordinate delta from a player, used for instanced when casting resurrect. */
int plryoff2[9] = { 0, 0, 1, 1, 0, 2, 2, 1, 2 };
/** Specifies the frame of each animation for which an action is triggered, for each player class. */
char PlrGFXAnimLens[NUM_CLASSES][11] = {
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
int PWVel[NUM_CLASSES][3] = {
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
int AnimLenFromClass[NUM_CLASSES] = {
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
int StrengthTbl[NUM_CLASSES] = {
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
int MagicTbl[NUM_CLASSES] = {
	10,
	15,
	35,
#ifdef HELLFIRE
	15,
	20,
	 0,
#endif
};
/** Maps from player_class to starting stat in dexterity. */
int DexterityTbl[NUM_CLASSES] = {
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
int VitalityTbl[NUM_CLASSES] = {
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
int ToBlkTbl[NUM_CLASSES] = {
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
int MaxStats[NUM_CLASSES][4] = {
	{ 250,  50,  60, 100 },
	{  55,  70, 250,  80 },
	{  45, 250,  85,  80 },
#ifdef HELLFIRE
	{ 150,  80, 150,  80 },
	{ 120, 120, 120, 100 },
	{ 255,   0,  55, 150 },
#endif
};
int Abilities[NUM_CLASSES] = {
	SPL_REPAIR, SPL_DISARM, SPL_RECHARGE
#ifdef HELLFIRE
	, SPL_SEARCH, SPL_IDENTIFY, SPL_BLODBOIL
#endif
};

/** Specifies the experience point limit of each level. */
int ExpLvlsTbl[MAXCHARLEVEL + 1] = {
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
/** Unused local of PlrChangeLightOff, originally for computing light radius. */
BYTE fix[9] = { 0, 0, 3, 3, 3, 6, 6, 6, 8 };

void SetPlayerGPtrs(BYTE *pData, BYTE **pAnim)
{
	int i;

	for (i = 0; i < 8; i++) {
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

void LoadPlrGFX(int pnum, player_graphic gfxflag)
{
	char prefix[16];
	char pszName[256];
	const char *szCel, *cs;
	PlayerStruct *p;
	BYTE *pData, *pAnim;
	DWORD i;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("LoadPlrGFX: illegal player %d", pnum);
	}

	p = &plr[pnum];
	GetPlrGFXCells(p->_pClass, &szCel, &cs);
	sprintf(prefix, "%c%c%c", *szCel, ArmourChar[p->_pgfxnum >> 4], WepChar[p->_pgfxnum & 0xF]);

	for (i = 1; i <= PFILE_NONDEATH; i <<= 1) {
		if (!(i & gfxflag)) {
			continue;
		}

		switch (i) {
		case PFILE_STAND:
			szCel = "AS";
			if (leveltype == DTYPE_TOWN) {
				szCel = "ST";
			}
			pData = p->_pNData;
			pAnim = (BYTE *)p->_pNAnim;
			break;
		case PFILE_WALK:
			szCel = "AW";
			if (leveltype == DTYPE_TOWN) {
				szCel = "WL";
			}
			pData = p->_pWData;
			pAnim = (BYTE *)p->_pWAnim;
			break;
		case PFILE_ATTACK:
			if (leveltype == DTYPE_TOWN) {
				continue;
			}
			szCel = "AT";
			pData = p->_pAData;
			pAnim = (BYTE *)p->_pAAnim;
			break;
		case PFILE_HIT:
			if (leveltype == DTYPE_TOWN) {
				continue;
			}
			szCel = "HT";
			pData = p->_pHData;
			pAnim = (BYTE *)p->_pHAnim;
			break;
		case PFILE_LIGHTNING:
			if (leveltype == DTYPE_TOWN) {
				continue;
			}
			szCel = "LM";
			pData = p->_pLData;
			pAnim = (BYTE *)p->_pLAnim;
			break;
		case PFILE_FIRE:
			if (leveltype == DTYPE_TOWN) {
				continue;
			}
			szCel = "FM";
			pData = p->_pFData;
			pAnim = (BYTE *)p->_pFAnim;
			break;
		case PFILE_MAGIC:
			if (leveltype == DTYPE_TOWN) {
				continue;
			}
			szCel = "QM";
			pData = p->_pTData;
			pAnim = (BYTE *)p->_pTAnim;
			break;
		case PFILE_DEATH:
			if (p->_pgfxnum & 0xF) {
				continue;
			}
			szCel = "DT";
			pData = p->_pDData;
			pAnim = (BYTE *)p->_pDAnim;
			break;
		case PFILE_BLOCK:
			if (leveltype == DTYPE_TOWN) {
				continue;
			}
			if (!p->_pBlockFlag) {
				continue;
			}

			szCel = "BL";
			pData = p->_pBData;
			pAnim = (BYTE *)p->_pBAnim;
			break;
		default:
			app_fatal("PLR:2");
			break;
		}

		sprintf(pszName, "PlrGFX\\%s\\%s\\%s%s.CL2", cs, prefix, prefix, szCel);
		LoadFileWithMem(pszName, pData);
		SetPlayerGPtrs((BYTE *)pData, (BYTE **)pAnim);
		p->_pGFXLoad |= i;
	}
}

void InitPlayerGFX(int pnum)
{
	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("InitPlayerGFX: illegal player %d", pnum);
	}

	if (plr[pnum]._pHitPoints >> 6 == 0) {
		plr[pnum]._pgfxnum = 0;
		LoadPlrGFX(pnum, PFILE_DEATH);
	} else {
		LoadPlrGFX(pnum, PFILE_NONDEATH);
	}
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

DWORD GetPlrGFXSize(const char *szCel)
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
				sprintf(Type, "%c%c%c", *cc, *a, *w);
				sprintf(pszName, "PlrGFX\\%s\\%s\\%s%s.CL2", cst, Type, Type, szCel);
				if (WOpenFile(pszName, &hsFile, TRUE)) {
					/// ASSERT: assert(hsFile);
					dwSize = WGetFileSize(hsFile, NULL, pszName);
					WCloseFile(hsFile);
					if (dwMaxSize <= dwSize) {
						dwMaxSize = dwSize;
					}
				}
			}
		}
	}

	return dwMaxSize;
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

void NewPlrAnim(int pnum, BYTE *Peq, int numFrames, int Delay, int width)
{
	PlayerStruct *p;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("NewPlrAnim: illegal player %d", pnum);
	}
	p = &plr[pnum];
	p->_pAnimData = Peq;
	p->_pAnimLen = numFrames;
	p->_pAnimFrame = 1;
	p->_pAnimCnt = 0;
	p->_pAnimDelay = Delay;
	p->_pAnimWidth = width;
	p->_pAnimWidth2 = (width - 64) >> 1;
}

void ClearPlrPVars(int pnum)
{
	PlayerStruct *p;

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
	p->_pVar8 = 0;
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

void ClearPlrRVars(PlayerStruct *p)
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
	if (c == PC_WARRIOR) {
		hp <<= 1;
	} else if (c == PC_ROGUE) {
		hp += hp >> 1;
	}
	p->_pHitPoints = p->_pMaxHP = p->_pHPBase = p->_pMaxHPBase = hp;

	mana = p->_pMagic << 6;
	if (c == PC_SORCERER) {
		mana <<= 1;
	} else if (c == PC_ROGUE) {
		mana += mana >> 1;
	}
	p->_pMana = p->_pMaxMana = p->_pManaBase = p->_pMaxManaBase = mana;

	p->_pLevel = 1;
	p->_pMaxLvl = 1;
	p->_pNextExper = ExpLvlsTbl[1];
	p->_pLightRad = 10;

	p->_pAblSpells = (__int64)1 << (Abilities[c] - 1);

	if (c == PC_SORCERER) {
		p->_pSplLvl[SPL_FIREBOLT] = 2;
		p->_pMemSpells = (__int64)1 << (SPL_FIREBOLT - 1);
	}

	// interestingly, only the first three hotkeys are reset
	// TODO: BUGFIX: clear all 4 hotkeys instead of 3 (demo leftover)
	for (i = 0; i < 3; i++) {
		p->_pSplHotKey[i] = -1;
	}

	// TODO: BUGFIX: is this necessary? does not seem to work with hellfire...
	if (c == PC_WARRIOR) {
		p->_pgfxnum = ANIM_ID_SWORD_SHIELD;
	} else if (c == PC_ROGUE) {
		p->_pgfxnum = ANIM_ID_BOW;
	} else if (c == PC_SORCERER) {
		p->_pgfxnum = ANIM_ID_STAFF;
	}

	InitDungMsgs(pnum);
	CreatePlrItems(pnum);
	SetRndSeed(0);
}

int CalcStatDiff(int pnum)
{
	int *stats;

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
	p->_pMaxLvl++;

#ifdef HELLFIRE
	CalcPlrInv(pnum, TRUE);
#endif

	if (CalcStatDiff(pnum) < 5) {
		p->_pStatPts = CalcStatDiff(pnum);
	} else {
		p->_pStatPts += 5;
	}

	p->_pNextExper = ExpLvlsTbl[p->_pLevel];

	hp = p->_pClass == PC_SORCERER ? 64 : 128;
	if (gbMaxPlayers == 1) {
		hp++;
	}
	p->_pMaxHP += hp;
	p->_pHitPoints = p->_pMaxHP;
	p->_pMaxHPBase += hp;
	p->_pHPBase = p->_pMaxHPBase;

	if (pnum == myplr) {
		drawhpflag = TRUE;
	}

	if (p->_pClass == PC_WARRIOR)
		mana = 64;
#ifdef HELLFIRE
	else if (p->_pClass == PC_BARBARIAN)
		mana = 0;
#endif
	else
		mana = 128;

	if (gbMaxPlayers == 1) {
		mana++;
	}
	p->_pMaxMana += mana;
	p->_pMaxManaBase += mana;

	if (!(p->_pIFlags & ISPL_NOMANA)) {
		p->_pMana = p->_pMaxMana;
		p->_pManaBase = p->_pMaxManaBase;
	}

	if (pnum == myplr) {
#ifdef HELLFIRE
		if (p->_pMana > 0)
#endif
			drawmanaflag = TRUE;
	}

	if (sgbControllerActive)
		FocusOnCharInfo();
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
	if (p->_pHitPoints <= 0) {
		return;
	}

	// Adjust xp based on difference in level between player and monster
	exp *= 1 + ((double)lvl - p->_pLevel) / 10;
	if (exp < 0) {
		exp = 0;
	}

	// Prevent power leveling
	if (gbMaxPlayers != 1) {
		powerLvlCap = p->_pLevel < 0 ? 0 : p->_pLevel;
		if (powerLvlCap >= 50) {
			powerLvlCap = 50;
		}
		// cap to 1/20 of current levels xp
		if (exp >= ExpLvlsTbl[powerLvlCap] / 20) {
			exp = ExpLvlsTbl[powerLvlCap] / 20;
		}
		// cap to 200 * current level
		expCap = 200 * powerLvlCap;
		if (exp >= expCap) {
			exp = expCap;
		}
	}

	p->_pExperience += exp;
	if ((DWORD)p->_pExperience > MAXEXP) {
		p->_pExperience = MAXEXP;
	}

	if (p->_pExperience >= ExpLvlsTbl[49]) {
		p->_pLevel = 50;
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
		p->_pSBkSpell = SPL_INVALID;
		// TODO: BUGFIX: does not seem to be the best place to set this
		if ((p->_pgfxnum & 0xF) == ANIM_ID_BOW) {
			p->_pwtype = WT_RANGED;
		} else {
			p->_pwtype = WT_MELEE;
		}
		p->pManaShield = FALSE;

		p->_pBaseToBlk = ToBlkTbl[p->_pClass];
		p->_pAblSpells = (unsigned __int64)1 << (Abilities[p->_pClass] - 1);
		p->_pNextExper = ExpLvlsTbl[p->_pLevel];
	}

	if (p->plrlevel == currlevel || leveldebug) {

		SetPlrAnims(pnum);

		p->_pxoff = 0;
		p->_pyoff = 0;
		p->_pxvel = 0;
		p->_pyvel = 0;

		ClearPlrPVars(pnum);

		if (p->_pHitPoints >> 6 > 0) {
			p->_pmode = PM_STAND;
			NewPlrAnim(pnum, p->_pNAnim[DIR_S], p->_pNFrames, 3, p->_pNWidth);
			p->_pAnimFrame = random_(2, p->_pNFrames - 1) + 1;
			p->_pAnimCnt = random_(2, 3);
		} else {
			p->_pmode = PM_DEATH;
			NewPlrAnim(pnum, p->_pDAnim[DIR_S], p->_pDFrames, 1, p->_pDWidth);
			p->_pAnimFrame = p->_pAnimLen - 1;
			p->_pVar8 = 2 * p->_pAnimLen;
		}

		p->_pdir = DIR_S;

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
	if (debug_mode_dollar_sign && FirstTime) {
		p->_pMemSpells |= 1 << (SPL_TELEPORT - 1);
		if (plr[myplr]._pSplLvl[SPL_TELEPORT] == 0) {
			plr[myplr]._pSplLvl[SPL_TELEPORT] = 1;
		}
	}
	if (debug_mode_key_inverted_v && FirstTime) {
		p->_pMemSpells = SPL_INVALID;
	}
#endif

	// TODO: BUGFIX: should be set on FirstTime only?
	p->_pInvincible = FALSE;

	if (pnum == myplr) {
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

BOOL PlrDirOK(int pnum, int dir)
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

void SetPlayerOld(int pnum)
{
	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("SetPlayerOld: illegal player %d", pnum);
	}

	plr[pnum]._poldx = plr[pnum]._px;
	plr[pnum]._poldy = plr[pnum]._py;
}

void FixPlayerLocation(int pnum, int dir)
{
	PlayerStruct *p;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("FixPlayerLocation: illegal player %d", pnum);
	}
	p = &plr[pnum];
	p->_pfutx = p->_px;
	p->_pfuty = p->_py;
	p->_ptargx = p->_px;
	p->_ptargy = p->_py;
	p->_pxoff = 0;
	p->_pyoff = 0;
	p->_pdir = dir;
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
	if (!p->_pInvincible || p->_pHitPoints != 0 || pnum != myplr) {
		if (!(p->_pGFXLoad & PFILE_STAND)) {
			LoadPlrGFX(pnum, PFILE_STAND);
		}

		NewPlrAnim(pnum, p->_pNAnim[dir], p->_pNFrames, 3, p->_pNWidth);
		p->_pmode = PM_STAND;
		FixPlayerLocation(pnum, dir);
		FixPlrWalkTags(pnum);
		dPlayer[p->_px][p->_py] = pnum + 1;
		SetPlayerOld(pnum);
	} else {
		SyncPlrKill(pnum, -1);
	}
}

void StartWalkStand(int pnum)
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

void PlrChangeLightOff(int pnum)
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

void PlrChangeOffset(int pnum)
{
	PlayerStruct *p;
	int px, py;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("PlrChangeOffset: illegal player %d", pnum);
	}

	p = &plr[pnum];
	p->_pVar8++;
	px = p->_pVar6 / 256;
	py = p->_pVar7 / 256;

	p->_pVar6 += p->_pxvel;
	p->_pVar7 += p->_pyvel;

#ifdef HELLFIRE
	if (currlevel == 0 && jogging_opt) {
		p->_pVar6 += p->_pxvel;
		p->_pVar7 += p->_pyvel;
	}
#endif

	p->_pxoff = p->_pVar6 >> 8;
	p->_pyoff = p->_pVar7 >> 8;

	px -= p->_pVar6 >> 8;
	py -= p->_pVar7 >> 8;

	if (pnum == myplr && ScrollInfo._sdir != SDIR_NONE) {
		ScrollInfo._sxoff += px;
		ScrollInfo._syoff += py;
	}

	PlrChangeLightOff(pnum);
}

void StartWalk(int pnum, int xvel, int yvel, int xadd, int yadd, int EndDir, int sdir)
{
	PlayerStruct *p;
	int px, py;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("StartWalk: illegal player %d", pnum);
	}

	p = &plr[pnum];
	if (p->_pInvincible && p->_pHitPoints == 0 && pnum == myplr) {
		SyncPlrKill(pnum, -1);
		return;
	}

	SetPlayerOld(pnum);

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

	if (!(p->_pGFXLoad & PFILE_WALK)) {
		LoadPlrGFX(pnum, PFILE_WALK);
	}

	NewPlrAnim(pnum, p->_pWAnim[EndDir], p->_pWFrames, 0, p->_pWWidth);

	p->_pdir = EndDir;
	p->_pVar6 = 0;
	p->_pVar7 = 0;
	p->_pVar8 = 0;

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
void StartWalk2(int pnum, int xvel, int yvel, int xoff, int yoff, int xadd, int yadd, int EndDir, int sdir)
{
	PlayerStruct *p;
	int px, py;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("StartWalk2: illegal player %d", pnum);
	}

	p = &plr[pnum];
	if (p->_pInvincible && p->_pHitPoints == 0 && pnum == myplr) {
		SyncPlrKill(pnum, -1);
		return;
	}

	SetPlayerOld(pnum);
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

	dPlayer[p->_px][p->_py] = -1 - pnum;
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

	if (!(p->_pGFXLoad & PFILE_WALK)) {
		LoadPlrGFX(pnum, PFILE_WALK);
	}
	NewPlrAnim(pnum, p->_pWAnim[EndDir], p->_pWFrames, 0, p->_pWWidth);

	p->_pdir = EndDir;
	p->_pVar8 = 0;

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
void StartWalk3(int pnum, int xvel, int yvel, int xoff, int yoff, int xadd, int yadd, int mapx, int mapy, int EndDir, int sdir)
{
	PlayerStruct *p;
	int px, py, x, y;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("StartWalk3: illegal player %d", pnum);
	}

	p = &plr[pnum];
	if (p->_pInvincible && p->_pHitPoints == 0 && pnum == myplr) {
		SyncPlrKill(pnum, -1);
		return;
	}

	SetPlayerOld(pnum);
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

	dPlayer[p->_px][p->_py] = -1 - pnum;
	dPlayer[px][py] = -1 - pnum;
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

	if (!(p->_pGFXLoad & PFILE_WALK)) {
		LoadPlrGFX(pnum, PFILE_WALK);
	}
	NewPlrAnim(pnum, p->_pWAnim[EndDir], p->_pWFrames, 0, p->_pWWidth);

	p->_pdir = EndDir;
	p->_pVar8 = 0;

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

void StartAttack(int pnum, int dir)
{
	PlayerStruct *p;
	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("StartAttack: illegal player %d", pnum);
	}

	p = &plr[pnum];
	if (p->_pInvincible && p->_pHitPoints == 0 && pnum == myplr) {
		SyncPlrKill(pnum, -1);
		return;
	}

	if (!(p->_pGFXLoad & PFILE_ATTACK)) {
		LoadPlrGFX(pnum, PFILE_ATTACK);
	}

	NewPlrAnim(pnum, p->_pAAnim[dir], p->_pAFrames, 0, p->_pAWidth);
	p->_pmode = PM_ATTACK;
	FixPlayerLocation(pnum, dir);
	SetPlayerOld(pnum);
}

void StartRangeAttack(int pnum, int dir, int cx, int cy)
{
	PlayerStruct *p;
	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("StartRangeAttack: illegal player %d", pnum);
	}

	p = &plr[pnum];
	if (p->_pInvincible && p->_pHitPoints == 0 && pnum == myplr) {
		SyncPlrKill(pnum, -1);
		return;
	}

	if (!(p->_pGFXLoad & PFILE_ATTACK)) {
		LoadPlrGFX(pnum, PFILE_ATTACK);
	}
	NewPlrAnim(pnum, p->_pAAnim[dir], p->_pAFrames, 0, p->_pAWidth);

	p->_pmode = PM_RATTACK;
	FixPlayerLocation(pnum, dir);
	SetPlayerOld(pnum);
	p->_pVar1 = cx;
	p->_pVar2 = cy;
}

void PlrStartBlock(int pnum, int dir)
{
	PlayerStruct *p;
	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("PlrStartBlock: illegal player %d", pnum);
	}

	p = &plr[pnum];
	if (p->_pInvincible && p->_pHitPoints == 0 && pnum == myplr) {
		SyncPlrKill(pnum, -1);
		return;
	}

	PlaySfxLoc(IS_ISWORD, p->_px, p->_py);

	if (!(p->_pGFXLoad & PFILE_BLOCK)) {
		LoadPlrGFX(pnum, PFILE_BLOCK);
	}
	NewPlrAnim(pnum, p->_pBAnim[dir], p->_pBFrames, 2, p->_pBWidth);

	p->_pmode = PM_BLOCK;
	FixPlayerLocation(pnum, dir);
	SetPlayerOld(pnum);
}

void StartSpell(int pnum, int dir, int cx, int cy)
{
	PlayerStruct *p;
	if ((DWORD)pnum >= MAX_PLRS)
		app_fatal("StartSpell: illegal player %d", pnum);

	p = &plr[pnum];
	if (p->_pInvincible && p->_pHitPoints == 0 && pnum == myplr) {
		SyncPlrKill(pnum, -1);
		return;
	}

	if (leveltype != DTYPE_TOWN) {
		switch (spelldata[p->_pSpell].sType) {
		case STYPE_FIRE:
			if (!(p->_pGFXLoad & PFILE_FIRE)) {
				LoadPlrGFX(pnum, PFILE_FIRE);
			}
			NewPlrAnim(pnum, p->_pFAnim[dir], p->_pSFrames, 0, p->_pSWidth);
			break;
		case STYPE_LIGHTNING:
			if (!(p->_pGFXLoad & PFILE_LIGHTNING)) {
				LoadPlrGFX(pnum, PFILE_LIGHTNING);
			}
			NewPlrAnim(pnum, p->_pLAnim[dir], p->_pSFrames, 0, p->_pSWidth);
			break;
		case STYPE_MAGIC:
			if (!(p->_pGFXLoad & PFILE_MAGIC)) {
				LoadPlrGFX(pnum, PFILE_MAGIC);
			}
			NewPlrAnim(pnum, p->_pTAnim[dir], p->_pSFrames, 0, p->_pSWidth);
			break;
		}
	}

	PlaySfxLoc(spelldata[p->_pSpell].sSFX, p->_px, p->_py);

	p->_pmode = PM_SPELL;

	FixPlayerLocation(pnum, dir);
	SetPlayerOld(pnum);

	p->_pVar1 = cx;
	p->_pVar2 = cy;
	p->_pVar4 = GetSpellLevel(pnum, p->_pSpell);
	p->_pVar8 = 1;
}

void FixPlrWalkTags(int pnum)
{
	int pp, pn;
	int dx, dy, y, x;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("FixPlrWalkTags: illegal player %d", pnum);
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

void RemovePlrFromMap(int pnum)
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
}

void StartPlrHit(int pnum, int dam, BOOL forcehit)
{
	PlayerStruct *p;
	int pd;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("StartPlrHit: illegal player %d", pnum);
	}

	p = &plr[pnum];
	if (p->_pInvincible && p->_pHitPoints == 0 && pnum == myplr) {
		SyncPlrKill(pnum, -1);
		return;
	}

	if (p->_pClass == PC_WARRIOR) {
		PlaySfxLoc(PS_WARR69, p->_px, p->_py);
	} else if (p->_pClass == PC_ROGUE) {
		PlaySfxLoc(PS_ROGUE69, p->_px, p->_py);
	} else if (p->_pClass == PC_SORCERER) {
		PlaySfxLoc(PS_MAGE69, p->_px, p->_py);
	}

	drawhpflag = TRUE;
	if (dam >> 6 >= p->_pLevel || forcehit) {
		pd = p->_pdir;

		if (!(p->_pGFXLoad & PFILE_HIT)) {
			LoadPlrGFX(pnum, PFILE_HIT);
		}
		NewPlrAnim(pnum, p->_pHAnim[pd], p->_pHFrames, 0, p->_pHWidth);

		p->_pmode = PM_GOTHIT;
		FixPlayerLocation(pnum, pd);
		p->_pVar8 = 1;
		FixPlrWalkTags(pnum);
		dPlayer[p->_px][p->_py] = pnum + 1;
		SetPlayerOld(pnum);
	}
}

void RespawnDeadItem(ItemStruct *is, int x, int y)
{
	int ii;

	if (numitems >= MAXITEMS) {
		return;
	}

	if (FindGetItem(is->IDidx, is->_iCreateInfo, is->_iSeed) >= 0) {
		DrawInvMsg("A duplicate item has been detected.  Destroying duplicate...");
		SyncGetItem(x, y, is->IDidx, is->_iCreateInfo, is->_iSeed);
	}

	ii = itemavail[0];
	dItem[x][y] = ii + 1;
	itemavail[0] = itemavail[MAXITEMS - numitems - 1];
	itemactive[numitems] = ii;
	item[ii] = *is;
	item[ii]._ix = x;
	item[ii]._iy = y;
	RespawnItem(ii, TRUE);
	numitems++;
	is->_itype = ITYPE_NONE;
}

#if defined(__clang__) || defined(__GNUC__)
__attribute__((no_sanitize("shift-base")))
#endif
void StartPlrKill(int pnum, int earflag)
{
	BOOL diablolevel;
	int i, pdd;
	PlayerStruct *p;
	ItemStruct ear;
	ItemStruct *pi;

	p = &plr[pnum];
	if (p->_pHitPoints <= 0 && p->_pmode == PM_DEATH) {
		return;
	}

	if (myplr == pnum) {
		NetSendCmdParam1(TRUE, CMD_PLRDEAD, earflag);
	}

	diablolevel = gbMaxPlayers != 1 && p->plrlevel == 16;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("StartPlrKill: illegal player %d", pnum);
	}

	if (p->_pClass == PC_WARRIOR) {
		PlaySfxLoc(PS_DEAD, p->_px, p->_py); // BUGFIX: should use `PS_WARR71` like other classes
	} else if (p->_pClass == PC_ROGUE) {
		PlaySfxLoc(PS_ROGUE71, p->_px, p->_py);
	} else if (p->_pClass == PC_SORCERER) {
		PlaySfxLoc(PS_MAGE71, p->_px, p->_py);
#ifdef HELLFIRE
	} else if (p->_pClass == PC_MONK) {
		PlaySfxLoc(PS_MONK71, p->_px, p->_py);
	} else if (p->_pClass == PC_BARD) {
		PlaySfxLoc(PS_ROGUE71, p->_px, p->_py);
	} else if (p->_pClass == PC_BARBARIAN) {
		PlaySfxLoc(PS_WARR71, p->_px, p->_py);
#endif
	}

	if (p->_pgfxnum) {
		p->_pgfxnum = 0;
		p->_pGFXLoad = 0;
		SetPlrAnims(pnum);
	}

	if (!(p->_pGFXLoad & PFILE_DEATH)) {
		LoadPlrGFX(pnum, PFILE_DEATH);
	}

	NewPlrAnim(pnum, p->_pDAnim[p->_pdir], p->_pDFrames, 1, p->_pDWidth);

	p->_pBlockFlag = FALSE;
	p->_pmode = PM_DEATH;
	p->_pInvincible = TRUE;
	SetPlayerHitPoints(pnum, 0);
	p->_pVar8 = 1;

	if (pnum != myplr && !earflag && !diablolevel) {
		for (i = 0; i < NUM_INVLOC; i++) {
			p->InvBody[i]._itype = ITYPE_NONE;
		}
		CalcPlrInv(pnum, FALSE);
	}

	if (p->plrlevel == currlevel) {
		FixPlayerLocation(pnum, p->_pdir);
		RemovePlrFromMap(pnum);
		dFlags[p->_px][p->_py] |= BFLAG_DEAD_PLAYER;
		SetPlayerOld(pnum);

		if (pnum == myplr) {
			drawhpflag = TRUE;
			deathdelay = 30;

			if (pcurs >= CURSOR_FIRSTITEM) {
				PlrDeadItem(pnum, &p->HoldItem, 0, 0);
				NewCursor(CURSOR_HAND);
			}

			if (!diablolevel) {
				DropHalfPlayersGold(pnum);
				if (earflag != -1) {
					if (earflag != 0) {
						SetPlrHandItem(&ear, IDI_EAR);
						sprintf(ear._iName, "Ear of %s", p->_pName);
						if (p->_pClass == PC_SORCERER) {
							ear._iCurs = ICURS_EAR_SORCEROR;
						} else if (p->_pClass == PC_WARRIOR) {
							ear._iCurs = ICURS_EAR_WARRIOR;
						} else {
							ear._iCurs = ICURS_EAR_ROGUE;
						}

						ear._iCreateInfo = p->_pName[0] << 8 | p->_pName[1];
						ear._iSeed = p->_pName[2] << 24 | p->_pName[3] << 16 | p->_pName[4] << 8 | p->_pName[5];
						ear._ivalue = p->_pLevel;

						if (FindGetItem(IDI_EAR, ear._iCreateInfo, ear._iSeed) == -1) {
							PlrDeadItem(pnum, &ear, 0, 0);
						}
					} else {
						pi = &p->InvBody[0];
						i = NUM_INVLOC;
						while (i--) {
							pdd = (i + p->_pdir) & 7;
							PlrDeadItem(pnum, pi, offset_x[pdd], offset_y[pdd]);
							pi++;
						}

						CalcPlrInv(pnum, FALSE);
					}
				}
			}
		}
	}
#ifndef HELLFIRE
	SetPlayerHitPoints(pnum, 0);
#endif
}

void PlrDeadItem(int pnum, ItemStruct *is, int xx, int yy)
{
	PlayerStruct *p;
	int x, y;
	int i, j, k;

	if (is->_itype == ITYPE_NONE)
		return;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("PlrDeadItem: illegal player %d", pnum);
	}

	p = &plr[pnum];
	x = xx + p->_px;
	y = yy + p->_py;
	if ((xx || yy) && ItemSpaceOk(x, y)) {
		RespawnDeadItem(is, x, y);
		p->HoldItem = *is;
		NetSendCmdPItem(FALSE, CMD_RESPAWNITEM, x, y);
		return;
	}

	for (k = 1; k < 50; k++) {
		for (j = -k; j <= k; j++) {
			y = j + p->_py;
			for (i = -k; i <= k; i++) {
				x = i + p->_px;
				if (ItemSpaceOk(x, y)) {
					RespawnDeadItem(is, x, y);
					p->HoldItem = *is;
					NetSendCmdPItem(FALSE, CMD_RESPAWNITEM, x, y);
					return;
				}
			}
		}
	}
}

void DropHalfPlayersGold(int pnum)
{
	PlayerStruct *p;
	ItemStruct *holditem;
	int i, hGold;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("DropHalfPlayersGold: illegal player %d", pnum);
	}

	p = &plr[pnum];
	holditem = &p->HoldItem;
	hGold = p->_pGold >> 1;
	for (i = 0; i < MAXBELTITEMS && hGold > 0; i++) {
		if (p->SpdList[i]._itype == ITYPE_GOLD &&
#ifdef HELLFIRE
			p->SpdList[i]._ivalue != MaxGold) {
#else
			p->SpdList[i]._ivalue != GOLD_MAX_LIMIT) {
#endif
			if (hGold < p->SpdList[i]._ivalue) {
				SetGoldItemValue(&p->SpdList[i], p->SpdList[i]._ivalue - hGold);
				SetPlrHandItem(holditem, IDI_GOLD);
				GetGoldSeed(pnum, holditem);
				SetGoldItemValue(holditem, hGold);
				PlrDeadItem(pnum, holditem, 0, 0);
				hGold = 0;
			} else {
				hGold -= p->SpdList[i]._ivalue;
				RemoveSpdBarItem(pnum, i);
				SetPlrHandItem(holditem, IDI_GOLD);
				GetGoldSeed(pnum, holditem);
				SetGoldItemValue(holditem, p->SpdList[i]._ivalue);
				PlrDeadItem(pnum, &p->HoldItem, 0, 0);
				i = -1;
			}
		}
	}
	if (hGold > 0) {
		for (i = 0; i < MAXBELTITEMS && hGold > 0; i++) {
			if (p->SpdList[i]._itype == ITYPE_GOLD) {
				if (hGold < p->SpdList[i]._ivalue) {
					SetGoldItemValue(&p->SpdList[i], p->SpdList[i]._ivalue - hGold);
					SetPlrHandItem(holditem, IDI_GOLD);
					GetGoldSeed(pnum, holditem);
					SetGoldItemValue(holditem, hGold);
					PlrDeadItem(pnum, holditem, 0, 0);
					hGold = 0;
				} else {
					hGold -= p->SpdList[i]._ivalue;
					RemoveSpdBarItem(pnum, i);
					SetPlrHandItem(holditem, IDI_GOLD);
					GetGoldSeed(pnum, holditem);
					SetGoldItemValue(holditem, p->SpdList[i]._ivalue);
					PlrDeadItem(pnum, holditem, 0, 0);
					i = -1;
				}
			}
		}
	}
	force_redraw = 255;
	if (hGold > 0) {
		for (i = 0; i < p->_pNumInv && hGold > 0; i++) {
			if (p->InvList[i]._itype == ITYPE_GOLD &&
#ifdef HELLFIRE
				p->InvList[i]._ivalue != MaxGold) {
#else
				p->InvList[i]._ivalue != GOLD_MAX_LIMIT) {
#endif
				if (hGold < p->InvList[i]._ivalue) {
					SetGoldItemValue(&p->InvList[i], p->InvList[i]._ivalue - hGold);
					SetPlrHandItem(holditem, IDI_GOLD);
					GetGoldSeed(pnum, holditem);
					SetGoldItemValue(holditem, hGold);
					PlrDeadItem(pnum, holditem, 0, 0);
					hGold = 0;
				} else {
					hGold -= p->InvList[i]._ivalue;
					RemoveInvItem(pnum, i);
					SetPlrHandItem(holditem, IDI_GOLD);
					GetGoldSeed(pnum, holditem);
					SetGoldItemValue(holditem, p->InvList[i]._ivalue);
					PlrDeadItem(pnum, holditem, 0, 0);
					i = -1;
				}
			}
		}
	}
	if (hGold > 0) {
		for (i = 0; i < p->_pNumInv && hGold > 0; i++) {
			if (p->InvList[i]._itype == ITYPE_GOLD) {
				if (hGold < p->InvList[i]._ivalue) {
					SetGoldItemValue(&p->InvList[i], p->InvList[i]._ivalue - hGold);
					SetPlrHandItem(holditem, IDI_GOLD);
					GetGoldSeed(pnum, holditem);
					SetGoldItemValue(holditem, hGold);
					PlrDeadItem(pnum, holditem, 0, 0);
					hGold = 0;
				} else {
					hGold -= p->InvList[i]._ivalue;
					RemoveInvItem(pnum, i);
					SetPlrHandItem(holditem, IDI_GOLD);
					GetGoldSeed(pnum, holditem);
					SetGoldItemValue(holditem, p->InvList[i]._ivalue);
					PlrDeadItem(pnum, holditem, 0, 0);
					i = -1;
				}
			}
		}
	}
	p->_pGold = CalculateGold(pnum);
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
	tmpItem = *holditem;

	pi = p->InvList;
	for (i = p->_pNumInv; i > 0; i--, pi++) {
		if (pi->_itype == ITYPE_GOLD) {
			if (pi->_ivalue > MaxGold) {
				val = pi->_ivalue - MaxGold;
				SetGoldItemValue(pi, MaxGold);
				SetPlrHandItem(holditem, 0);
				GetGoldSeed(pnum, holditem);
				SetGoldItemValue(holditem, val);
				if (!GoldAutoPlace(pnum))
					PlrDeadItem(pnum, holditem, 0, 0);
			}
		}
	}
	p->_pGold = CalculateGold(pnum);
	*holditem = tmpItem;
}

#endif
void SyncPlrKill(int pnum, int earflag)
{
	MissileStruct *mis;
	int i;

#ifdef HELLFIRE
	if (plr[pnum]._pHitPoints <= 0 && currlevel == 0) {
#else
	if (plr[pnum]._pHitPoints == 0 && currlevel == 0) {
#endif
		SetPlayerHitPoints(pnum, 64);
		return;
	}

	for (i = 0; i < nummissiles; i++) {
		mis = &missile[missileactive[i]];
		if (mis->_mitype == MIS_MANASHIELD && mis->_misource == pnum && !mis->_miDelFlag) {
			if (earflag != -1) {
				mis->_miVar8 = earflag;
			}

			return;
		}
	}

	SetPlayerHitPoints(pnum, 0);
	StartPlrKill(pnum, earflag);
}

void RemovePlrMissiles(int pnum)
{
	int i, mi;
	int mx, my;

	if (currlevel != 0 && pnum == myplr && (monster[myplr]._mx != 1 || monster[myplr]._my != 0)) {
		MonStartKill(myplr, myplr);
		AddDead(monster[myplr]._mx, monster[myplr]._my, (monster[myplr].MType)->mdeadval, monster[myplr]._mdir);
		mx = monster[myplr]._mx;
		my = monster[myplr]._my;
		dMonster[mx][my] = 0;
		monster[myplr]._mDelFlag = TRUE;
		DeleteMonsterList();
	}

	for (i = 0; i < nummissiles; i++) {
		mi = missileactive[i];
		if (missile[mi]._misource != pnum)
			continue;
		if (missile[mi]._mitype == MIS_STONE) {
			monster[missile[mi]._miVar2]._mmode = missile[mi]._miVar1;
		} else if (missile[mi]._mitype == MIS_MANASHIELD
				|| missile[mi]._mitype == MIS_ETHEREALIZE) {
			ClearMissileSpot(mi);
			DeleteMissile(mi, i);
		}
	}
}

void InitLevelChange(int pnum)
{
	PlayerStruct *p;

	RemovePlrMissiles(pnum);
	if (pnum == myplr && qtextflag) {
		qtextflag = FALSE;
		stream_stop();
	}

	RemovePlrFromMap(pnum);
	SetPlayerOld(pnum);
	ClrPlrPath(pnum);
	p = &plr[pnum];
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
		plr[myplr].pTownWarps |= 1 << (leveltype - 2);
		plr[pnum].plrlevel = lvl;
		break;
	case WM_DIABRETOWN:
		break;
	default:
		app_fatal("StartNewLvl");
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

	SetPlayerHitPoints(pnum, 64);

	plr[pnum]._pMana = 0;
	plr[pnum]._pManaBase = plr[pnum]._pMana - (plr[pnum]._pMaxMana - plr[pnum]._pMaxManaBase);

	CalcPlrInv(pnum, FALSE);

	if (pnum == myplr) {
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

BOOL PlrDoStand(int pnum)
{
	return FALSE;
}

BOOL PlrDoWalk(int pnum)
{
	PlayerStruct *p;
	int anim_len;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("PlrDoWalk: illegal player %d", pnum);
	}

	p = &plr[pnum];
#ifdef HELLFIRE
	if (currlevel == 0 && jogging_opt) {
		if (p->_pAnimFrame % 2 == 0) {
			p->_pAnimFrame++;
			p->_pVar8++;
		}
		if (p->_pAnimFrame >= p->_pWFrames) {
			p->_pAnimFrame = 0;
		}
	}
#else
	if (p->_pAnimFrame == 3
	    || (p->_pWFrames == 8 && p->_pAnimFrame == 7)
	    || (p->_pWFrames != 8 && p->_pAnimFrame == 4)) {
		PlaySfxLoc(PS_WALK1, p->_px, p->_py);
	}
#endif

	anim_len = 8;
	if (currlevel != 0) {
		anim_len = AnimLenFromClass[p->_pClass];
	}

#ifdef HELLFIRE
	if (p->_pVar8 >= anim_len) {
#else
	if (p->_pVar8 == anim_len) {
#endif
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
	} else {
		PlrChangeOffset(pnum);
		return FALSE;
	}
}

BOOL PlrDoWalk2(int pnum)
{
	PlayerStruct *p;
	int anim_len;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("PlrDoWalk2: illegal player %d", pnum);
	}
	p = &plr[pnum];
#ifdef HELLFIRE
	if (currlevel == 0 && jogging_opt) {
		if (p->_pAnimFrame % 2 == 0) {
			p->_pAnimFrame++;
			p->_pVar8++;
		}
		if (p->_pAnimFrame >= p->_pWFrames) {
			p->_pAnimFrame = 0;
		}
	}
#else
	if (p->_pAnimFrame == 3
	    || (p->_pWFrames == 8 && p->_pAnimFrame == 7)
	    || (p->_pWFrames != 8 && p->_pAnimFrame == 4)) {
		PlaySfxLoc(PS_WALK1, p->_px, p->_py);
	}
#endif

	anim_len = 8;
	if (currlevel != 0) {
		anim_len = AnimLenFromClass[p->_pClass];
	}

#ifdef HELLFIRE
	if (p->_pVar8 >= anim_len) {
#else
	if (p->_pVar8 == anim_len) {
#endif
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
	} else {
		PlrChangeOffset(pnum);
		return FALSE;
	}
}

BOOL PlrDoWalk3(int pnum)
{
	PlayerStruct *p;
	int anim_len;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("PlrDoWalk3: illegal player %d", pnum);
	}
	p = &plr[pnum];
#ifdef HELLFIRE
	if (currlevel == 0 && jogging_opt) {
		if (p->_pAnimFrame % 2 == 0) {
			p->_pAnimFrame++;
			p->_pVar8++;
		}
		if (p->_pAnimFrame >= p->_pWFrames) {
			p->_pAnimFrame = 0;
		}
	}
#else
	if (p->_pAnimFrame == 3
	    || (p->_pWFrames == 8 && p->_pAnimFrame == 7)
	    || (p->_pWFrames != 8 && p->_pAnimFrame == 4)) {
		PlaySfxLoc(PS_WALK1, p->_px, p->_py);
	}
#endif

	anim_len = 8;
	if (currlevel != 0) {
		anim_len = AnimLenFromClass[p->_pClass];
	}

#ifdef HELLFIRE
	if (p->_pVar8 >= anim_len) {
#else
	if (p->_pVar8 == anim_len) {
#endif
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
	} else {
		PlrChangeOffset(pnum);
		return FALSE;
	}
}

BOOL WeaponDur(int pnum, int durrnd)
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
	if (pi->_itype != ITYPE_NONE && pi->_iClass == ICLASS_WEAPON && pi->_iDamAcFlags & ISPH_DECAY) {
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
	if (pi->_itype != ITYPE_NONE && pi->_iClass == ICLASS_WEAPON && pi->_iDamAcFlags & ISPH_DECAY) {
		pi->_iPLDam -= 5;
		if (pi->_iPLDam <= -100) {
			NetSendCmdDelItem(TRUE, INVLOC_HAND_LEFT); // BUGFIX: INVLOC_HAND_RIGHT
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
#ifdef HELLFIRE
		if (pi->_iDurability <= 0) {
#else
		if (pi->_iDurability == 0) {
#endif
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
	int mind, maxd, dam;

	p = &plr[pnum];
	mind = p->_pIMinDam;
	maxd = p->_pIMaxDam;
	dam = random_(5, maxd - mind + 1) + mind;
	dam += dam * p->_pIBonusDam / 100;
	dam += p->_pDamageMod + p->_pIBonusDamMod;
	if (p->_pClass == PC_WARRIOR) {
		if (random_(6, 100) < p->_pLevel) {
			dam <<= 1;
		}
	}
	return dam;
}

BOOL PlrHitMonst(int pnum, int mnum)
{
	PlayerStruct *p;
	MonsterStruct *mon;
	BOOL ret;
	int hit, hper, dam, skdam, phanditype;

	if ((DWORD)mnum >= MAXMONSTERS) {
		app_fatal("PlrHitMonst: illegal monster %d", mnum);
	}

	if (CheckMonsterHit(mnum, &ret)) {
		return ret;
	}

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("PlrHitMonst: illegal player %d", pnum);
	}

	p = &plr[pnum];
	mon = &monster[mnum];
	hit = mon->_mmode == MM_STONE ? 0 : random_(4, 100);

	hper = (p->_pDexterity >> 1) + p->_pIBonusToHit + p->_pLevel - (mon->mArmorClass - p->_pIEnAc) + 50;
	if (p->_pClass == PC_WARRIOR) {
		hper += 20;
	}
	if (hper < 5) {
		hper = 5;
	}
	if (hper > 95) {
		hper = 95;
	}

#ifdef _DEBUG
	if (hit < hper || debug_mode_key_inverted_v || debug_mode_dollar_sign) {
#else
	if (hit < hper) {
#endif
		dam = PlrAtkDam(pnum);

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

		dam <<= 6;
		if (pnum == myplr) {
			mon->_mhitpoints -= dam;
		}

		if (p->_pIFlags & ISPL_RNDSTEALLIFE) {
			skdam = random_(7, dam >> 3);
			p->_pHitPoints += skdam;
			if (p->_pHitPoints > p->_pMaxHP) {
				p->_pHitPoints = p->_pMaxHP;
			}
			p->_pHPBase += skdam;
			if (p->_pHPBase > p->_pMaxHPBase) {
				p->_pHPBase = p->_pMaxHPBase;
			}
			drawhpflag = TRUE;
		}
		if (p->_pIFlags & (ISPL_STEALMANA_3 | ISPL_STEALMANA_5) && !(p->_pIFlags & ISPL_NOMANA)) {
			if (p->_pIFlags & ISPL_STEALMANA_5) {
				skdam = 5 * dam / 100;
			} else {
				skdam = 3 * dam / 100;
			}
			p->_pMana += skdam;
			if (p->_pMana > p->_pMaxMana) {
				p->_pMana = p->_pMaxMana;
			}
			p->_pManaBase += skdam;
			if (p->_pManaBase > p->_pMaxManaBase) {
				p->_pManaBase = p->_pMaxManaBase;
			}
			drawmanaflag = TRUE;
		}
		if (p->_pIFlags & (ISPL_STEALLIFE_3 | ISPL_STEALLIFE_5)) {
			if (p->_pIFlags & ISPL_STEALLIFE_5) {
				skdam = 5 * dam / 100;
			} else {
				skdam = 3 * dam / 100;
			}
			p->_pHitPoints += skdam;
			if (p->_pHitPoints > p->_pMaxHP) {
				p->_pHitPoints = p->_pMaxHP;
			}
			p->_pHPBase += skdam;
			if (p->_pHPBase > p->_pMaxHPBase) {
				p->_pHPBase = p->_pMaxHPBase;
			}
			drawhpflag = TRUE;
		}
		if (p->_pIFlags & ISPL_NOHEALPLR) {
			mon->_mFlags |= MFLAG_NOHEAL;
		}
#ifdef _DEBUG
		if (debug_mode_dollar_sign || debug_mode_key_inverted_v) {
			mon->_mhitpoints = 0; /* double check */
		}
#endif
		if ((mon->_mhitpoints >> 6) <= 0) {
			if (mon->_mmode == MM_STONE) {
				MonStartKill(mnum, pnum);
				mon->_mmode = MM_STONE;
			} else {
				MonStartKill(mnum, pnum);
			}
		} else {
			if (mon->_mmode == MM_STONE) {
				MonStartHit(mnum, pnum, dam);
				mon->_mmode = MM_STONE;
			} else {
				if (p->_pIFlags & ISPL_KNOCKBACK) {
					MonGetKnockback(mnum);
				}
				MonStartHit(mnum, pnum, dam);
			}
		}
		return TRUE;
	}

	return FALSE;
}

BOOL PlrHitPlr(int offp, char defp)
{
	PlayerStruct *ops, *dps;
	int hit, hper, blk, blkper, dir, dam, skdam;

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
	hit = random_(4, 100);

	hper = (ops->_pDexterity >> 1) + ops->_pLevel + 50 - (dps->_pIBonusAC + dps->_pIAC + dps->_pDexterity / 5);

	if (ops->_pClass == PC_WARRIOR) {
		hper += 20;
	}
	hper += ops->_pIBonusToHit;
	if (hper < 5) {
		hper = 5;
	}
	if (hper > 95) {
		hper = 95;
	}

	if ((dps->_pmode == PM_STAND || dps->_pmode == PM_ATTACK) && dps->_pBlockFlag) {
		blk = random_(5, 100);
	} else {
		blk = 100;
	}

	blkper = dps->_pDexterity + dps->_pBaseToBlk + (dps->_pLevel << 1) - (ops->_pLevel << 1);
	if (blkper < 0) {
		blkper = 0;
	}
	if (blkper > 100) {
		blkper = 100;
	}

	if (hit < hper) {
		if (blk < blkper) {
			dir = GetDirection(dps->_px, dps->_py, ops->_px, ops->_py);
			PlrStartBlock(defp, dir);
		} else {
			dam = PlrAtkDam(offp);
			dam <<= 6;
			if (ops->_pIFlags & ISPL_RNDSTEALLIFE) {
				skdam = random_(7, dam >> 3);
				ops->_pHitPoints += skdam;
				if (ops->_pHitPoints > ops->_pMaxHP) {
					ops->_pHitPoints = ops->_pMaxHP;
				}
				ops->_pHPBase += skdam;
				if (ops->_pHPBase > ops->_pMaxHPBase) {
					ops->_pHPBase = ops->_pMaxHPBase;
				}
				drawhpflag = TRUE;
			}
			if (offp == myplr) {
				NetSendCmdDamage(TRUE, defp, dam);
			}
			StartPlrHit(defp, dam, FALSE);
		}

		return TRUE;
	}

	return FALSE;
}

BOOL PlrHitObj(int pnum, int mx, int my)
{
	int oi;

	oi = dObject[mx][my];
	oi = oi >= 0 ? oi - 1 : -(oi + 1);

	if (object[oi]._oBreak == 1) {
		BreakObject(pnum, oi);
		return TRUE;
	}

	return FALSE;
}

BOOL PlrDoAttack(int pnum)
{
	PlayerStruct *p;
	int frame, dir, dx, dy, mp;
	BOOL didhit;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("PlrDoAttack: illegal player %d", pnum);
	}

	p = &plr[pnum];
	frame = p->_pAnimFrame;
	if (p->_pIFlags & ISPL_QUICKATTACK && frame == 1) {
		p->_pAnimFrame++;
	}
	if (p->_pIFlags & ISPL_FASTATTACK && (frame == 1 || frame == 3)) {
		p->_pAnimFrame++;
	}
	if (p->_pIFlags & ISPL_FASTERATTACK && (frame == 1 || frame == 3 || frame == 5)) {
		p->_pAnimFrame++;
	}
	if (p->_pIFlags & ISPL_FASTESTATTACK && (frame == 1 || frame == 4)) {
		p->_pAnimFrame += 2;
	}
	if (p->_pAnimFrame == p->_pAFNum - 1) {
		PlaySfxLoc(PS_SWING, p->_px, p->_py);
	}

	if (p->_pAnimFrame == p->_pAFNum) {
		dir = p->_pdir;
		dx = p->_px + offset_x[dir];
		dy = p->_py + offset_y[dir];

		mp = dMonster[dx][dy];
		if (mp != 0) {
			mp = mp >= 0 ? mp - 1 : -(mp + 1);
			if (CanTalkToMonst(mp)) {
				p->_pVar1 = 0;
				return FALSE;
			}
		}

		if (p->_pIFlags & ISPL_FIREDAM) {
			AddMissile(dx, dy, 1, 0, 0, MIS_WEAPEXP, 0, pnum, 0, 0);
		}
		if (p->_pIFlags & ISPL_LIGHTDAM) {
			AddMissile(dx, dy, 2, 0, 0, MIS_WEAPEXP, 0, pnum, 0, 0);
		}

		didhit = FALSE;
		mp = dMonster[dx][dy];
		if (mp != 0) {
			mp = mp >= 0 ? mp - 1 : -(mp + 1);
			didhit = PlrHitMonst(pnum, mp);
		} else if (dPlayer[dx][dy] != 0 && !FriendlyMode) {
			mp = dPlayer[dx][dy];
			mp = mp >= 0 ? mp - 1 : -(mp + 1);
			didhit = PlrHitPlr(pnum, mp);
		} else if (dObject[dx][dy] > 0) {
			didhit = PlrHitObj(pnum, dx, dy);
		}

		if (didhit && WeaponDur(pnum, 30)) {
			PlrStartStand(pnum, p->_pdir);
			ClearPlrPVars(pnum);
			return TRUE;
		}
	}

	if (p->_pAnimFrame == p->_pAFrames) {
		PlrStartStand(pnum, p->_pdir);
		ClearPlrPVars(pnum);
		return TRUE;
	} else {
		return FALSE;
	}
}

BOOL PlrDoRangeAttack(int pnum)
{
	PlayerStruct *p;
	int origFrame, mitype;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("PlrDoRangeAttack: illegal player %d", pnum);
	}
	p = &plr[pnum];
	origFrame = p->_pAnimFrame;
	if (p->_pIFlags & ISPL_QUICKATTACK && origFrame == 1) {
		p->_pAnimFrame++;
	}
	if (p->_pIFlags & ISPL_FASTATTACK && (origFrame == 1 || origFrame == 3)) {
		p->_pAnimFrame++;
	}

	if (p->_pAnimFrame == p->_pAFNum) {
		mitype = MIS_ARROW;
		if (p->_pIFlags & ISPL_FIRE_ARROWS) {
			mitype = MIS_FARROW;
		}
		if (p->_pIFlags & ISPL_LIGHT_ARROWS) {
			mitype = MIS_LARROW;
		}
		AddMissile(
		    p->_px,
		    p->_py,
		    p->_pVar1,
		    p->_pVar2,
		    p->_pdir,
		    mitype,
		    0,
		    pnum,
		    4,
		    0);

		PlaySfxLoc(PS_BFIRE, p->_px, p->_py);

		if (WeaponDur(pnum, 40)) {
			PlrStartStand(pnum, p->_pdir);
			ClearPlrPVars(pnum);
			return TRUE;
		}
	}

	if (p->_pAnimFrame >= p->_pAFrames) {
		PlrStartStand(pnum, p->_pdir);
		ClearPlrPVars(pnum);
		return TRUE;
	} else {
		return FALSE;
	}
}

void ShieldDur(int pnum)
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

BOOL PlrDoBlock(int pnum)
{
	PlayerStruct *p;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("PlrDoBlock: illegal player %d", pnum);
	}
	p = &plr[pnum];
	if (p->_pIFlags & ISPL_FASTBLOCK && p->_pAnimFrame != 1) {
		p->_pAnimFrame = p->_pBFrames;
	}

	if (p->_pAnimFrame >= p->_pBFrames) {
		PlrStartStand(pnum, p->_pdir);
		ClearPlrPVars(pnum);

		if (random_(3, 10) == 0) {
			ShieldDur(pnum);
		}
		return TRUE;
	}

	return FALSE;
}

BOOL PlrDoSpell(int pnum)
{
	PlayerStruct *p;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("PlrDoSpell: illegal player %d", pnum);
	}
	p = &plr[pnum];
	if (p->_pVar8 == p->_pSFNum) {
		CastSpell(
		    pnum,
		    p->_pSpell,
		    p->_px,
		    p->_py,
		    p->_pVar1,
		    p->_pVar2,
		    0,
		    p->_pVar4);

		if (p->_pSplFrom == 0) {
			if (p->_pRSplType == RSPLTYPE_SCROLL) {
				if (!(p->_pScrlSpells
				        & (unsigned __int64)1 << (p->_pRSpell - 1))) {
					p->_pRSpell = SPL_INVALID;
					p->_pRSplType = RSPLTYPE_INVALID;
					force_redraw = 255;
				}
			}

			if (p->_pRSplType == RSPLTYPE_CHARGES) {
				if (!(p->_pISpells
				        & (unsigned __int64)1 << (p->_pRSpell - 1))) {
					p->_pRSpell = SPL_INVALID;
					p->_pRSplType = RSPLTYPE_INVALID;
					force_redraw = 255;
				}
			}
		}
	}

	p->_pVar8++;

	if (leveltype == DTYPE_TOWN) {
		if (p->_pVar8 > p->_pSFrames) {
			StartWalkStand(pnum);
			ClearPlrPVars(pnum);
			return TRUE;
		}
	} else if (p->_pAnimFrame == p->_pSFrames) {
		PlrStartStand(pnum, p->_pdir);
		ClearPlrPVars(pnum);
		return TRUE;
	}

	return FALSE;
}

BOOL PlrDoGotHit(int pnum)
{
	PlayerStruct *p;
	int frame;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("PlrDoGotHit: illegal player %d", pnum);
	}
	p = &plr[pnum];
#ifdef HELLFIRE
	if (p->_pIFlags & (ISPL_FASTRECOVER | ISPL_FASTERRECOVER | ISPL_FASTESTRECOVER)) {
		frame = 3;
		if (p->_pIFlags & ISPL_FASTERRECOVER)
			frame = 4;
		if (p->_pIFlags & ISPL_FASTESTRECOVER)
			frame = 5;
		if (p->_pVar8 > 1 && p->_pVar8 < frame) {
			p->_pVar8 = frame;
		}
		if (p->_pVar8 > p->_pHFrames)
			p->_pVar8 = p->_pHFrames;
	}

	if (p->_pVar8 == p->_pHFrames) {
#else
	frame = p->_pAnimFrame;
	if (p->_pIFlags & ISPL_FASTRECOVER && frame == 3) {
		p->_pAnimFrame++;
	}
	if (p->_pIFlags & ISPL_FASTERRECOVER && (frame == 3 || frame == 5)) {
		p->_pAnimFrame++;
	}
	if (p->_pIFlags & ISPL_FASTESTRECOVER && (frame == 1 || frame == 3 || frame == 5)) {
		p->_pAnimFrame++;
	}

	if (p->_pAnimFrame >= p->_pHFrames) {
#endif
		PlrStartStand(pnum, p->_pdir);
		ClearPlrPVars(pnum);
		if (random_(3, 4)) {
			ArmorDur(pnum);
		}

		return TRUE;
	}

#ifdef HELLFIRE
	p->_pVar8++;
#endif
	return FALSE;
}

void ArmorDur(int pnum)
{
	int a;
	ItemStruct *pi;
	PlayerStruct *p;

	if (pnum != myplr) {
		return;
	}

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("ArmorDur: illegal player %d", pnum);
	}

	p = &plr[pnum];
	if (p->InvBody[INVLOC_CHEST]._itype == ITYPE_NONE && p->InvBody[INVLOC_HEAD]._itype == ITYPE_NONE) {
		return;
	}

	a = random_(8, 3);
	if (p->InvBody[INVLOC_CHEST]._itype != ITYPE_NONE && p->InvBody[INVLOC_HEAD]._itype == ITYPE_NONE) {
		a = 1;
	}
	if (p->InvBody[INVLOC_CHEST]._itype == ITYPE_NONE && p->InvBody[INVLOC_HEAD]._itype != ITYPE_NONE) {
		a = 0;
	}

	if (a != 0) {
		pi = &p->InvBody[INVLOC_CHEST];
	} else {
		pi = &p->InvBody[INVLOC_HEAD];
	}
	if (pi->_iDurability == DUR_INDESTRUCTIBLE) {
		return;
	}

	pi->_iDurability--;
	if (pi->_iDurability != 0) {
		return;
	}

	if (a != 0) {
		NetSendCmdDelItem(TRUE, INVLOC_CHEST);
	} else {
		NetSendCmdDelItem(TRUE, INVLOC_HEAD);
	}
	pi->_itype = ITYPE_NONE;
	CalcPlrInv(pnum, TRUE);
}

BOOL PlrDoDeath(int pnum)
{
	PlayerStruct *p;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("PlrDoDeath: illegal player %d", pnum);
	}

	p = &plr[pnum];
	if (p->_pVar8 >= 2 * p->_pDFrames) {
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
	}

	if (p->_pVar8 < 100) {
		p->_pVar8++;
	}

	return FALSE;
}

BOOL PlrDoNewLvl(int pnum)
{
	return FALSE;
}

void CheckNewPath(int pnum)
{
	PlayerStruct *p;
	int i, x, y, d;
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
					i = p->destParam1;

					if (p->destAction == ACTION_ATTACKMON) {
						x = abs(p->_pfutx - monster[i]._mfutx);
						y = abs(p->_pfuty - monster[i]._mfuty);
						d = GetDirection(p->_pfutx, p->_pfuty, monster[i]._mfutx, monster[i]._mfuty);
					} else {
						x = abs(p->_pfutx - plr[i]._pfutx);
						y = abs(p->_pfuty - plr[i]._pfuty);
						d = GetDirection(p->_pfutx, p->_pfuty, plr[i]._pfutx, plr[i]._pfuty);
					}

					if (x < 2 && y < 2) {
						ClrPlrPath(pnum);
						if (monster[i].mtalkmsg && monster[i].mtalkmsg != TEXT_VILE14) {
							TalktoMonster(i);
						} else {
							StartAttack(pnum, d);
						}
						p->destAction = ACTION_NONE;
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
			}

			for (i = 1; i < MAX_PATH_LENGTH; i++) {
				p->walkpath[i - 1] = p->walkpath[i];
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
			d = GetDirection(p->_px, p->_py, p->destParam1, p->destParam2);
			StartAttack(pnum, d);
			break;
		case ACTION_ATTACKMON:
			i = p->destParam1;
			x = abs(p->_px - monster[i]._mfutx);
			y = abs(p->_py - monster[i]._mfuty);
			if (x <= 1 && y <= 1) {
				d = GetDirection(p->_pfutx, p->_pfuty, monster[i]._mfutx, monster[i]._mfuty);
				if (monster[i].mtalkmsg && monster[i].mtalkmsg != TEXT_VILE14) {
					TalktoMonster(i);
				} else {
					StartAttack(pnum, d);
				}
			}
			break;
		case ACTION_ATTACKPLR:
			i = p->destParam1;
			x = abs(p->_px - plr[i]._pfutx);
			y = abs(p->_py - plr[i]._pfuty);
			if (x <= 1 && y <= 1) {
				d = GetDirection(p->_pfutx, p->_pfuty, plr[i]._pfutx, plr[i]._pfuty);
				StartAttack(pnum, d);
			}
			break;
		case ACTION_RATTACK:
			d = GetDirection(p->_px, p->_py, p->destParam1, p->destParam2);
			StartRangeAttack(pnum, d, p->destParam1, p->destParam2);
			break;
		case ACTION_RATTACKMON:
			i = p->destParam1;
			d = GetDirection(p->_pfutx, p->_pfuty, monster[i]._mfutx, monster[i]._mfuty);
			if (monster[i].mtalkmsg && monster[i].mtalkmsg != TEXT_VILE14) {
				TalktoMonster(i);
			} else {
				StartRangeAttack(pnum, d, monster[i]._mfutx, monster[i]._mfuty);
			}
			break;
		case ACTION_RATTACKPLR:
			i = p->destParam1;
			d = GetDirection(p->_pfutx, p->_pfuty, plr[i]._pfutx, plr[i]._pfuty);
			StartRangeAttack(pnum, d, plr[i]._pfutx, plr[i]._pfuty);
			break;
		case ACTION_SPELL:
			d = GetDirection(p->_px, p->_py, p->destParam1, p->destParam2);
			StartSpell(pnum, d, p->destParam1, p->destParam2);
			p->_pVar4 = p->destParam3;
			break;
		case ACTION_SPELLWALL:
			StartSpell(pnum, p->destParam3, p->destParam1, p->destParam2);
			p->_pVar3 = p->destParam3;
			p->_pVar4 = p->destParam4;
			break;
		case ACTION_SPELLMON:
			i = p->destParam1;
			d = GetDirection(p->_px, p->_py, monster[i]._mfutx, monster[i]._mfuty);
			StartSpell(pnum, d, monster[i]._mfutx, monster[i]._mfuty);
			p->_pVar4 = p->destParam2;
			break;
		case ACTION_SPELLPLR:
			i = p->destParam1;
			d = GetDirection(p->_px, p->_py, plr[i]._pfutx, plr[i]._pfuty);
			StartSpell(pnum, d, plr[i]._pfutx, plr[i]._pfuty);
			p->_pVar4 = p->destParam2;
			break;
		case ACTION_OPERATE:
			i = p->destParam1;
			x = abs(p->_px - object[i]._ox);
			y = abs(p->_py - object[i]._oy);
			if (y > 1 && dObject[object[i]._ox][object[i]._oy - 1] == -1 - i) {
				y = abs(p->_py - object[i]._oy + 1);
			}
			if (x <= 1 && y <= 1) {
				if (object[i]._oBreak == 1) {
					d = GetDirection(p->_px, p->_py, object[i]._ox, object[i]._oy);
					StartAttack(pnum, d);
				} else {
					OperateObject(pnum, i, FALSE);
				}
			}
			break;
		case ACTION_DISARM:
			i = p->destParam1;
			x = abs(p->_px - object[i]._ox);
			y = abs(p->_py - object[i]._oy);
			if (y > 1 && dObject[object[i]._ox][object[i]._oy - 1] == -1 - i) {
				y = abs(p->_py - object[i]._oy + 1);
			}
			if (x <= 1 && y <= 1) {
				if (object[i]._oBreak == 1) {
					d = GetDirection(p->_px, p->_py, object[i]._ox, object[i]._oy);
					StartAttack(pnum, d);
				} else {
					TryDisarm(pnum, i);
					OperateObject(pnum, i, FALSE);
				}
			}
			break;
		case ACTION_OPERATETK:
			i = p->destParam1;
			if (object[i]._oBreak != 1) {
				OperateObject(pnum, i, TRUE);
			}
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
		}

		FixPlayerLocation(pnum, p->_pdir);
		p->destAction = ACTION_NONE;

		return;
	}

	if (p->_pmode == PM_ATTACK && p->_pAnimFrame > plr[myplr]._pAFNum) {
		if (p->destAction == ACTION_ATTACK) {
			d = GetDirection(p->_pfutx, p->_pfuty, p->destParam1, p->destParam2);
			StartAttack(pnum, d);
			p->destAction = ACTION_NONE;
		} else if (p->destAction == ACTION_ATTACKMON) {
			i = p->destParam1;
			x = abs(p->_px - monster[i]._mfutx);
			y = abs(p->_py - monster[i]._mfuty);
			if (x <= 1 && y <= 1) {
				d = GetDirection(p->_pfutx, p->_pfuty, monster[i]._mfutx, monster[i]._mfuty);
				StartAttack(pnum, d);
			}
			p->destAction = ACTION_NONE;
		} else if (p->destAction == ACTION_ATTACKPLR) {
			i = p->destParam1;
			x = abs(p->_px - plr[i]._pfutx);
			y = abs(p->_py - plr[i]._pfuty);
			if (x <= 1 && y <= 1) {
				d = GetDirection(p->_pfutx, p->_pfuty, plr[i]._pfutx, plr[i]._pfuty);
				StartAttack(pnum, d);
			}
			p->destAction = ACTION_NONE;
		} else if (p->destAction == ACTION_OPERATE) {
			i = p->destParam1;
			x = abs(p->_px - object[i]._ox);
			y = abs(p->_py - object[i]._oy);
			if (y > 1 && dObject[object[i]._ox][object[i]._oy - 1] == -1 - i) {
				y = abs(p->_py - object[i]._oy + 1);
			}
			if (x <= 1 && y <= 1) {
				if (object[i]._oBreak == 1) {
					d = GetDirection(p->_px, p->_py, object[i]._ox, object[i]._oy);
					StartAttack(pnum, d);
				} else {
					OperateObject(pnum, i, FALSE);
				}
			}
		}
	}

	if (p->_pmode == PM_RATTACK && p->_pAnimFrame > plr[myplr]._pAFNum) {
		if (p->destAction == ACTION_RATTACK) {
			d = GetDirection(p->_px, p->_py, p->destParam1, p->destParam2);
			StartRangeAttack(pnum, d, p->destParam1, p->destParam2);
			p->destAction = ACTION_NONE;
		} else if (p->destAction == ACTION_RATTACKMON) {
			i = p->destParam1;
			d = GetDirection(p->_px, p->_py, monster[i]._mfutx, monster[i]._mfuty);
			StartRangeAttack(pnum, d, monster[i]._mfutx, monster[i]._mfuty);
			p->destAction = ACTION_NONE;
		} else if (p->destAction == ACTION_RATTACKPLR) {
			i = p->destParam1;
			d = GetDirection(p->_px, p->_py, plr[i]._pfutx, plr[i]._pfuty);
			StartRangeAttack(pnum, d, plr[i]._pfutx, plr[i]._pfuty);
			p->destAction = ACTION_NONE;
		}
	}

	if (p->_pmode == PM_SPELL && p->_pAnimFrame > p->_pSFNum) {
		if (p->destAction == ACTION_SPELL) {
			d = GetDirection(p->_px, p->_py, p->destParam1, p->destParam2);
			StartSpell(pnum, d, p->destParam1, p->destParam2);
			p->destAction = ACTION_NONE;
		} else if (p->destAction == ACTION_SPELLMON) {
			i = p->destParam1;
			d = GetDirection(p->_px, p->_py, monster[i]._mfutx, monster[i]._mfuty);
			StartSpell(pnum, d, monster[i]._mfutx, monster[i]._mfuty);
			p->destAction = ACTION_NONE;
		} else if (p->destAction == ACTION_SPELLPLR) {
			i = p->destParam1;
			d = GetDirection(p->_px, p->_py, plr[i]._pfutx, plr[i]._pfuty);
			StartSpell(pnum, d, plr[i]._pfutx, plr[i]._pfuty);
			p->destAction = ACTION_NONE;
		}
	}
}

BOOL PlrDeathModeOK(int pnum)
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

void ValidatePlayer()
{
	PlayerStruct *p;
	ItemStruct *pi;
	__int64 msk;
	int gt, pc, i;

	if ((DWORD)myplr >= MAX_PLRS) {
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
#ifdef HELLFIRE
			if (pi->_ivalue > auricGold) { // BUGFIX: change to MaxGold? Why would auricGold be used here?
				pi->_ivalue = auricGold;   // BUGFIX: change to MaxGold? Why would auricGold be used here?
#else
			if (pi->_ivalue > GOLD_MAX_LIMIT) {
				pi->_ivalue = GOLD_MAX_LIMIT;
#endif
			}
			gt += pi->_ivalue;
		}
	}
	if (gt != p->_pGold)
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
	for (i = 1; i < MAX_SPELLS; i++) {
		if (spelldata[i].sBookLvl != -1) {
			msk |= (__int64)1 << (i - 1);
			if (p->_pSplLvl[i] > MAXSPLLEVEL)
				p->_pSplLvl[i] = MAXSPLLEVEL;
		}
	}

	p->_pMemSpells &= msk;
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
#endif
			PlaySFX(sfxdnum);
#ifdef HELLFIRE
			}
#endif
		}
	}

	ValidatePlayer();

	for (pnum = 0; pnum < MAX_PLRS; pnum++) {
		if (plr[pnum].plractive && currlevel == plr[pnum].plrlevel && (pnum == myplr || !plr[pnum]._pLvlChanging)) {
			CheckCheatStats(pnum);

			if (!PlrDeathModeOK(pnum) && (plr[pnum]._pHitPoints >> 6) <= 0) {
				SyncPlrKill(pnum, -1);
			}

			if (pnum == myplr) {
				if ((plr[pnum]._pIFlags & ISPL_DRAINLIFE) && currlevel != 0) {
					plr[pnum]._pHitPoints -= 4;
					plr[pnum]._pHPBase -= 4;
					if ((plr[pnum]._pHitPoints >> 6) <= 0) {
						SyncPlrKill(pnum, 0);
					}
					drawhpflag = TRUE;
				}
				if (plr[pnum]._pIFlags & ISPL_NOMANA && plr[pnum]._pManaBase > 0) {
					plr[pnum]._pManaBase -= plr[pnum]._pMana;
					plr[pnum]._pMana = 0;
					drawmanaflag = TRUE;
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

void CheckCheatStats(int pnum)
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
			if (mpo != pnum && mpo < MAX_PLRS && plr[mpo]._pHitPoints != 0) {
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
	if (!path) {
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
		}

		plr[pnum]._ptargx = xx;
		plr[pnum]._ptargy = yy;
	}

	plr[pnum].walkpath[path] = WALK_NONE;
}

void CheckPlrSpell()
{
	BOOL addflag;
	int rspell, sd, sl;

	if ((DWORD)myplr >= MAX_PLRS) {
		app_fatal("CheckPlrSpell: illegal player %d", myplr);
	}

	rspell = plr[myplr]._pRSpell;
	if (rspell == SPL_INVALID) {
		if (plr[myplr]._pClass == PC_WARRIOR) {
			PlaySFX(PS_WARR34);
		} else if (plr[myplr]._pClass == PC_ROGUE) {
			PlaySFX(PS_ROGUE34);
		} else if (plr[myplr]._pClass == PC_SORCERER) {
			PlaySFX(PS_MAGE34);
		}
		return;
	}

	if (leveltype == DTYPE_TOWN && !spelldata[rspell].sTownSpell) {
		if (plr[myplr]._pClass == PC_WARRIOR) {
			PlaySFX(PS_WARR27);
		} else if (plr[myplr]._pClass == PC_ROGUE) {
			PlaySFX(PS_ROGUE27);
		} else if (plr[myplr]._pClass == PC_SORCERER) {
			PlaySFX(PS_MAGE27);
		}
		return;
	}

	if (!sgbControllerActive) {
		if (pcurs != CURSOR_HAND
		    || (MouseY >= PANEL_TOP && MouseX >= PANEL_LEFT && MouseX <= RIGHT_PANEL)     // inside main panel
		    || ((chrflag || questlog) && MouseX < SPANEL_WIDTH && MouseY < SPANEL_HEIGHT) // inside left panel
		    || ((invflag || sbookflag) && MouseX > RIGHT_PANEL && MouseY < SPANEL_HEIGHT) // inside right panel
		        && rspell != SPL_HEAL
		        && rspell != SPL_IDENTIFY
		        && rspell != SPL_REPAIR
		        && rspell != SPL_INFRA
		        && rspell != SPL_RECHARGE) {
			return;
		}
	}

	addflag = FALSE;
	switch (plr[myplr]._pRSplType) {
	case RSPLTYPE_SKILL:
	case RSPLTYPE_SPELL:
		addflag = CheckSpell(myplr, rspell, plr[myplr]._pRSplType, FALSE);
		break;
	case RSPLTYPE_SCROLL:
		addflag = UseScroll();
		break;
	case RSPLTYPE_CHARGES:
		addflag = UseStaff();
		break;
	}

	if (addflag) {
		sl = GetSpellLevel(myplr, rspell);
		if (rspell == SPL_FIREWALL) {
			sd = GetDirection(plr[myplr]._px, plr[myplr]._py, cursmx, cursmy);
			NetSendCmdLocParam3(TRUE, CMD_SPELLXYD, cursmx, cursmy, rspell, sd, sl);
		} else if (pcursmonst != -1) {
			NetSendCmdParam3(TRUE, CMD_SPELLID, pcursmonst, rspell, sl);
		} else if (pcursplr != -1) {
			NetSendCmdParam3(TRUE, CMD_SPELLPID, pcursplr, rspell, sl);
		} else { //145
			NetSendCmdLocParam2(TRUE, CMD_SPELLXY, cursmx, cursmy, rspell, sl);
		}
		return;
	}

	if (plr[myplr]._pRSplType == RSPLTYPE_SPELL) {
		if (plr[myplr]._pClass == PC_WARRIOR) {
			PlaySFX(PS_WARR35);
		} else if (plr[myplr]._pClass == PC_ROGUE) {
			PlaySFX(PS_ROGUE35);
		} else if (plr[myplr]._pClass == PC_SORCERER) {
			PlaySFX(PS_MAGE35);
		}
	}
}

void SyncPlrAnim(int pnum)
{
	PlayerStruct *p;
	int dir, sType;

	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("SyncPlrAnim: illegal player %d", pnum);
	}
	p = &plr[pnum];
	dir = p->_pdir;
	switch (p->_pmode) {
	case PM_STAND:
		p->_pAnimData = p->_pNAnim[dir];
		break;
	case PM_WALK:
	case PM_WALK2:
	case PM_WALK3:
		p->_pAnimData = p->_pWAnim[dir];
		break;
	case PM_ATTACK:
		p->_pAnimData = p->_pAAnim[dir];
		break;
	case PM_RATTACK:
		p->_pAnimData = p->_pAAnim[dir];
		break;
	case PM_BLOCK:
		p->_pAnimData = p->_pBAnim[dir];
		break;
	case PM_SPELL:
		if (pnum == myplr)
			sType = spelldata[p->_pSpell].sType;
		else
			sType = STYPE_FIRE;
		if (sType == STYPE_FIRE)
			p->_pAnimData = p->_pFAnim[dir];
		if (sType == STYPE_LIGHTNING)
			p->_pAnimData = p->_pLAnim[dir];
		if (sType == STYPE_MAGIC)
			p->_pAnimData = p->_pTAnim[dir];
		break;
	case PM_GOTHIT:
		p->_pAnimData = p->_pHAnim[dir];
		break;
	case PM_NEWLVL:
		p->_pAnimData = p->_pNAnim[dir];
		break;
	case PM_DEATH:
		p->_pAnimData = p->_pDAnim[dir];
		break;
	case PM_QUIT:
		p->_pAnimData = p->_pNAnim[dir];
		break;
	default:
		app_fatal("SyncPlrAnim");
		break;
	}
}

void SyncInitPlrPos(int pnum)
{
	PlayerStruct *p;
	int x, y, xx, yy, i;
	BOOL posOk;

	p = &plr[pnum];
	p->_ptargx = p->_px;
	p->_ptargy = p->_py;

	if (gbMaxPlayers == 1 || p->plrlevel != currlevel) {
		return;
	}

	for (i = 0; i < 8; i++) {
		x = p->_px + plrxoff2[i];
		y = p->_py + plryoff2[i];
		if (PosOkPlayer(pnum, x, y)) {
			break;
		}
	}

#ifdef HELLFIRE
	p->_px += plrxoff2[i];
	p->_py += plryoff2[i];
	dPlayer[p->_px][p->_py] = pnum + 1;
#else
	if (!PosOkPlayer(pnum, x, y)) {
		posOk = FALSE;
		for (i = 1; i < 50 && !posOk; i++) {
			for (yy = -i; yy <= i && !posOk; yy++) {
				y = yy + p->_py;
				for (xx = -i; xx <= i && !posOk; xx++) {
					x = xx + p->_px;
					if (PosOkPlayer(pnum, x, y) && !PosOkPortal(currlevel, x, y)) {
						posOk = TRUE;
					}
				}
			}
		}
	}

	p->_px = x;
	p->_py = y;
	dPlayer[x][y] = pnum + 1;

	if (pnum == myplr) {
		p->_pfutx = x;
		p->_pfuty = y;
		p->_ptargx = x;
		p->_ptargy = y;
		ViewX = x;
		ViewY = y;
	}
#endif
}

void SyncInitPlr(int pnum)
{
	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("SyncInitPlr: illegal player %d", pnum);
	}

	SetPlrAnims(pnum);
	SyncInitPlrPos(pnum);
}

void CheckStats(int pnum)
{
	PlayerStruct *p;
	int *stats;

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

void SetPlayerHitPoints(int pnum, int val)
{
	PlayerStruct *p;
	if ((DWORD)pnum >= MAX_PLRS) {
		app_fatal("SetPlayerHitPoints: illegal player %d", pnum);
	}
	p = &plr[pnum];
	p->_pHitPoints = val;
	p->_pHPBase = val + p->_pMaxHPBase - p->_pMaxHP;

	if (pnum == myplr) {
		drawhpflag = TRUE;
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
		if (plr[myplr]._pClass == PC_WARRIOR) {
			sfxdnum = PS_WARR97;
		} else if (plr[myplr]._pClass == PC_ROGUE) {
			sfxdnum = PS_ROGUE97;
		} else if (plr[myplr]._pClass == PC_SORCERER) {
			sfxdnum = PS_MAGE97;
#ifdef HELLFIRE
		} else if (plr[myplr]._pClass == PC_MONK) {
			sfxdnum = PS_MONK97;
		} else if (plr[myplr]._pClass == PC_BARD) {
			sfxdnum = PS_ROGUE97;
		} else if (plr[myplr]._pClass == PC_BARBARIAN) {
			sfxdnum = PS_WARR97;
#endif
		}
		plr[myplr].pDungMsgs |= DMSG_CATHEDRAL;
	} else if (currlevel == 5 && !plr[myplr]._pLvlVisited[5] && !(plr[myplr].pDungMsgs & DMSG_CATACOMBS)) {
		sfxdelay = 40;
		if (plr[myplr]._pClass == PC_WARRIOR) {
			sfxdnum = PS_WARR96B;
		} else if (plr[myplr]._pClass == PC_ROGUE) {
			sfxdnum = PS_ROGUE96;
		} else if (plr[myplr]._pClass == PC_SORCERER) {
			sfxdnum = PS_MAGE96;
#ifdef HELLFIRE
		} else if (plr[myplr]._pClass == PC_MONK) {
			sfxdnum = PS_MONK96;
		} else if (plr[myplr]._pClass == PC_BARD) {
			sfxdnum = PS_ROGUE96;
		} else if (plr[myplr]._pClass == PC_BARBARIAN) {
			sfxdnum = PS_WARR96B;
#endif
		}
		plr[myplr].pDungMsgs |= DMSG_CATACOMBS;
	} else if (currlevel == 9 && !plr[myplr]._pLvlVisited[9] && !(plr[myplr].pDungMsgs & DMSG_CAVES)) {
		sfxdelay = 40;
		if (plr[myplr]._pClass == PC_WARRIOR) {
			sfxdnum = PS_WARR98;
		} else if (plr[myplr]._pClass == PC_ROGUE) {
			sfxdnum = PS_ROGUE98;
		} else if (plr[myplr]._pClass == PC_SORCERER) {
			sfxdnum = PS_MAGE98;
#ifdef HELLFIRE
		} else if (plr[myplr]._pClass == PC_MONK) {
			sfxdnum = PS_MONK98;
		} else if (plr[myplr]._pClass == PC_BARD) {
			sfxdnum = PS_ROGUE98;
		} else if (plr[myplr]._pClass == PC_BARBARIAN) {
			sfxdnum = PS_WARR98;
#endif
		}
		plr[myplr].pDungMsgs |= DMSG_CAVES;
	} else if (currlevel == 13 && !plr[myplr]._pLvlVisited[13] && !(plr[myplr].pDungMsgs & DMSG_HELL)) {
		sfxdelay = 40;
		if (plr[myplr]._pClass == PC_WARRIOR) {
			sfxdnum = PS_WARR99;
		} else if (plr[myplr]._pClass == PC_ROGUE) {
			sfxdnum = PS_ROGUE99;
		} else if (plr[myplr]._pClass == PC_SORCERER) {
			sfxdnum = PS_MAGE99;
#ifdef HELLFIRE
		} else if (plr[myplr]._pClass == PC_MONK) {
			sfxdnum = PS_MONK99;
		} else if (plr[myplr]._pClass == PC_BARD) {
			sfxdnum = PS_ROGUE99;
		} else if (plr[myplr]._pClass == PC_BARBARIAN) {
			sfxdnum = PS_WARR99;
#endif
		}
		plr[myplr].pDungMsgs |= DMSG_HELL;
	} else if (currlevel == 16 && !plr[myplr]._pLvlVisited[15] && !(plr[myplr].pDungMsgs & DMSG_DIABLO)) { // BUGFIX: _pLvlVisited should check 16 or this message will never play
		sfxdelay = 40;
#ifdef HELLFIRE
		if (plr[myplr]._pClass == PC_WARRIOR || plr[myplr]._pClass == PC_ROGUE || plr[myplr]._pClass == PC_SORCERER || plr[myplr]._pClass == PC_MONK || plr[myplr]._pClass == PC_BARD || plr[myplr]._pClass == PC_BARBARIAN) {
#else
		if (plr[myplr]._pClass == PC_WARRIOR || plr[myplr]._pClass == PC_ROGUE || plr[myplr]._pClass == PC_SORCERER) {
#endif
			sfxdnum = PS_DIABLVLINT;
		}
		plr[myplr].pDungMsgs |= DMSG_DIABLO;
#ifdef HELLFIRE
	} else if (currlevel == 17 && !plr[myplr]._pLvlVisited[17] && !(plr[myplr].pDungMsgs2 & 1)) {
		sfxdelay = 10;
		sfxdnum = USFX_DEFILER1;
		quests[Q_DEFILER]._qactive = QUEST_ACTIVE;
		quests[Q_DEFILER]._qlog = TRUE;
		quests[Q_DEFILER]._qmsg = TEXT_DEFILER1;
		plr[myplr].pDungMsgs2 |= 1;
	} else if (currlevel == 19 && !plr[myplr]._pLvlVisited[19] && !(plr[myplr].pDungMsgs2 & 4)) {
		sfxdelay = 10;
		sfxdnum = USFX_DEFILER3;
		plr[myplr].pDungMsgs2 |= 4;
	} else if (currlevel == 21 && !plr[myplr]._pLvlVisited[21] && !(plr[myplr].pDungMsgs & 32)) {
		sfxdelay = 30;
		if (plr[myplr]._pClass == PC_WARRIOR) {
			sfxdnum = PS_WARR92;
		} else if (plr[myplr]._pClass == PC_ROGUE) {
			sfxdnum = PS_ROGUE92;
		} else if (plr[myplr]._pClass == PC_SORCERER) {
			sfxdnum = PS_MAGE92;
		} else if (plr[myplr]._pClass == PC_MONK) {
			sfxdnum = PS_MONK92;
		} else if (plr[myplr]._pClass == PC_BARD) {
			sfxdnum = PS_ROGUE92;
		} else if (plr[myplr]._pClass == PC_BARBARIAN) {
			sfxdnum = PS_WARR92;
		}
		plr[myplr].pDungMsgs |= 32;
#endif
	}
}

#ifdef HELLFIRE
int player_45EFA1(int pc)
{
	return MaxStats[pc][ATTRIB_STR];
}

int player_45EFAB(int pc)
{
	return MaxStats[pc][ATTRIB_MAG];
}

int player_45EFB5(int pc)
{
	return MaxStats[pc][ATTRIB_DEX];
}
#endif

DEVILUTION_END_NAMESPACE
