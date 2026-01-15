/**
 * @file player.cpp
 *
 * Implementation of player functionality, leveling, actions, creation, loading, etc.
 */
#include "all.h"
#include "plrctrls.h"
#include "engine/render/render.h"

DEVILUTION_BEGIN_NAMESPACE

NONETCONST int mypnum = 0;
PlayerStruct players[MAX_PLRS];
/* Whether the current player is changing the level. */
bool gbLvlLoad;
/** The current player while processing the players. */
static BYTE gbGameLogicPnum;
/** Cache the maximum sizes of the player gfx files. */
static unsigned _guPlrFrameSize[NUM_PGXS + 1];
/** Whether the _guPlrFrameSize array is initalized. */
static bool _gbPlrGfxSizeLoaded = false;

/** Maps from armor animation to letter used in graphic files. */
static const char ArmorChar[4] = {
	'L', // light
	'M', // medium
	'H', // heavy
	0
};
/** Maps from weapon animation to letter used in graphic files. */
static const char WepChar[10] = {
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
static const char CharChar[NUM_CLASSES] = { 'W', 'R', 'S',
#ifdef HELLFIRE
//	'M', 'B', 'C'
	'M', 'R', 'W'
#endif
};
/** Maps from player class to the folder of graphic files. */
static const char* const gfxClassTbl[NUM_CLASSES] = { "Warrior", "Rogue", "Sorceror",
#ifdef HELLFIRE
//	"Monk", "Bard", "Barbarian"
	"Monk", "Rogue", "Warrior"
#endif
};
/** Maps from player class to the string shown to the player. */
const char* const ClassStrTbl[NUM_CLASSES] = { "Warrior", "Rogue", "Sorceror",
#ifdef HELLFIRE
	"Monk", "Bard", "Barbarian"
#endif
};
/*
 * Specifies the X and Y offsets to try when a player is entering the level or resurrected.
 * The base position is the location of the portal or the body of the dead player.
 */
static const int plrxoff2[NUM_DIRS + 1] = { 0, 1, 1, 0, -1, 0, -1, 1, -1 };
static const int plryoff2[NUM_DIRS + 1] = { 0, 1, 0, 1, -1, -1, 0, -1, 1 };
/* Data related to the player-animation types. */
static const PlrAnimType PlrAnimTypes[NUM_PGTS] = {
	// clang-format off
	{ "ST", PGX_STAND },     // PGT_STAND_TOWN
	{ "AS", PGX_STAND },     // PGT_STAND_DUNGEON
	{ "WL", PGX_WALK },      // PGT_WALK_TOWN
	{ "AW", PGX_WALK },      // PGT_WALK_DUNGEON
	{ "AT", PGX_ATTACK },    // PGT_ATTACK
	{ "FM", PGX_FIRE },      // PGT_FIRE
	{ "LM", PGX_LIGHTNING }, // PGT_LIGHTNING
	{ "QM", PGX_MAGIC },     // PGT_MAGIC
	{ "BL", PGX_BLOCK },     // PGT_BLOCK
	{ "HT", PGX_GOTHIT },    // PGT_GOTHIT
	{ "DT", PGX_DEATH },     // PGT_DEATH
	// clang-format on
};
/**
 * Specifies the number of frames of each animation for each player class.
   STAND, WALK, ATTACK, SPELL, BLOCK, GOTHIT, DEATH
 */
static const BYTE PlrGFXAnimLens[NUM_CLASSES][NUM_PLR_ANIMS] = {
	// clang-format off
	{ 10, 8, 16, 20, 2, 6, 20 },
	{  8, 8, 18, 16, 4, 7, 20 },
	{  8, 8, 16, 12, 6, 8, 20 },
#ifdef HELLFIRE
	{  8, 8, 16, 18, 3, 6, 20 },
	{  8, 8, 18, 16, 4, 7, 20 },
	{ 10, 8, 16, 20, 2, 6, 20 },
#endif
	// clang-format on
};
/** Specifies the frame of attack and spell animation for which the action is triggered, for each player class. */
static const BYTE PlrGFXAnimActFrames[NUM_CLASSES][2] = {
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
/** Specifies the length of a frame for each animation (player_graphic_idx). */
const BYTE PlrAnimFrameLens[NUM_PGXS] = { 4, 1, 1, 1, 1, 1, 3, 1, 2 };

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
	SPL_REPAIR, SPL_DISARM, SPL_RECHARGE,
#ifdef HELLFIRE
	SPL_WHITTLE, SPL_IDENTIFY, SPL_BUCKLE,
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
static const int8_t dir2sdir[NUM_DIRS] = { SDIR_S, SDIR_SW, SDIR_W, SDIR_NW, SDIR_N, SDIR_NE, SDIR_E, SDIR_SE };
#endif

static inline void GetPlrGFXCells(int pc, const char** szCel, const char** cs)
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

static void LoadPlrGFX(int pnum, unsigned gfxflag)
{
	char prefix[4];
	char pszName[DATA_ARCHIVE_MAX_PATH];
	const char *szCel, *chrClass, *strClass;
	unsigned mask;

	GetPlrGFXCells(plr._pClass, &chrClass, &strClass);
	prefix[0] = *chrClass;
	prefix[1] = ArmorChar[plr._pgfxnum >> 4];
	prefix[2] = WepChar[plr._pgfxnum & 0xF];
	prefix[3] = '\0';

	mask = gfxflag;
	if (currLvl._dType != DTYPE_TOWN)
		mask &= ~(PGF_STAND_TOWN | PGF_WALK_TOWN);
	else
		mask &= ~(PGF_STAND_DUNGEON | PGF_WALK_DUNGEON);
	if (prefix[2] != 'U' && prefix[2] != 'D' && prefix[2] != 'H')
/*#ifdef HELLFIRE
if (plr._pClass != PC_MONK || prefix[1] == 'A' || prefix[1] == 'B')
#endif*/
		mask &= ~PGF_BLOCK;
	for (auto pAnimType = &PlrAnimTypes[0]; mask != 0; pAnimType++, mask >>= 1) {
		if (!(mask & 1))
			continue;

		szCel = pAnimType->patTxt;
		assert(pAnimType->patGfxIdx != PGX_DEATH || plr._pgfxnum == ANIM_ID_UNARMED);
		// assert(pAnimType->patGfxIdx != PGX_DEATH || (plr._pGFXLoad == 0 && mask == 1));// MEM_DEATH

		snprintf(pszName, sizeof(pszName), "PlrGFX\\%s\\%s\\%s%s.CL2", strClass, prefix, prefix, szCel);
		LoadFileWithMem(pszName, plr._pAnimFileData[pAnimType->patGfxIdx]);
		LoadFrameGroups(plr._pAnimFileData[pAnimType->patGfxIdx], plr._pAnims[pAnimType->patGfxIdx].paAnimData);
		plr._pGFXLoad |= 1 << (pAnimType - &PlrAnimTypes[0]);
	}
}

void InitPlayerGFX(int pnum)
{
	unsigned gfx2load, gfxvalid;
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("InitPlayerGFX: illegal player %d", pnum);
	}
	gfxvalid = plr._pGFXLoad;
	// reset gfx-flags which are no longer usable
	if (currLvl._dType == DTYPE_TOWN)
		gfxvalid &= ~(PGF_STAND_DUNGEON | PGF_WALK_DUNGEON);
	else
		gfxvalid &= ~(PGF_STAND_TOWN | PGF_WALK_TOWN);
	// select appropriate flags based on player-status and location
	if (plr._pHitPoints != 0) {
		gfx2load = PGF_NONDEATH;
		// commented out because it is preferable to load everything at once
		//if (currLvl._dType == DTYPE_TOWN)
		//	gfx2load &= ~(PGF_ATTACK | PGF_GOTHIT | PGF_FIRE | PGF_LIGHTNING | PGF_BLOCK);
		//else if (!(plr._pSkillFlags & SFLAG_BLOCK))
		//	gfx2load &= ~PGF_BLOCK;
		gfxvalid &= PGF_NONDEATH; // MEM_DEATH: _pAnimFileData is either for death or non-death animations
	} else {
		gfx2load = PGF_DEATH;
		// protect against warping deads
		if (plr._pgfxnum != ANIM_ID_UNARMED) {
			plr._pgfxnum = ANIM_ID_UNARMED;
			gfxvalid = 0;
		}
		gfxvalid &= PGF_DEATH; // MEM_DEATH
	}
	// update gfx-flags
	plr._pGFXLoad = gfxvalid;
	// mask gfx-flags which are already loaded
	gfx2load &= ~gfxvalid;

	LoadPlrGFX(pnum, gfx2load);
}

static unsigned GetPlrGFXSize(const char* szCel)
{
	int c;
	const char *chrArmor, *chrWeapon, *chrClass, *strClass;
	DWORD dwSize, dwMaxSize;
	char pszName[DATA_ARCHIVE_MAX_PATH];
	char prefix[4];

	dwMaxSize = 0;

	for (c = 0; c < NUM_CLASSES; c++) {
		GetPlrGFXCells(c, &chrClass, &strClass);
		for (chrArmor = &ArmorChar[0]; *chrArmor != '\0'; chrArmor++) {
			for (chrWeapon = &WepChar[0]; *chrWeapon != '\0'; chrWeapon++) { // BUGFIX loads non-existing animations; DT is only for N, BL is only for U, D & H (fixed)
				if (szCel[0] == 'D' /*&& szCel[1] == 'T'*/ && (*chrArmor != 'L' || *chrWeapon != 'N')) {
					continue; //Death has no weapon or armor
				}
				if (szCel[0] == 'B' /*&& szCel[1] == 'L'*/ && (*chrWeapon != 'U' && *chrWeapon != 'D' && *chrWeapon != 'H')) {
/* BUGFIX monks can block without shield (fixed)
#ifdef HELLFIRE
if (c !=  PC_MONK || *chrWeapon == 'A' || *chrWeapon == 'B')
#endif
*/
					continue; //No block without shield
				}
				prefix[0] = *chrClass;
				prefix[1] = *chrArmor;
				prefix[2] = *chrWeapon;
				prefix[3] = '\0';
				snprintf(pszName, sizeof(pszName), "PlrGFX\\%s\\%s\\%s%s.CL2", strClass, prefix, prefix, szCel);
				dwSize = SFileReadFileEx(pszName, NULL);
				if (dwMaxSize < dwSize) {
					dwMaxSize = dwSize;
				}
			}
		}
	}

	dwMaxSize += sizeof(int) - 1;
	dwMaxSize -= dwMaxSize % sizeof(int);

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
		_guPlrFrameSize[PGX_STAND] = std::max(GetPlrGFXSize("ST"), GetPlrGFXSize("AS"));
		// WALK -- WL: TOWN, AW: DUNGEON
		_guPlrFrameSize[PGX_WALK] = std::max(GetPlrGFXSize("WL"), GetPlrGFXSize("AW"));
		_guPlrFrameSize[PGX_ATTACK] = GetPlrGFXSize("AT");
		_guPlrFrameSize[PGX_FIRE] = GetPlrGFXSize("FM");
		_guPlrFrameSize[PGX_LIGHTNING] = GetPlrGFXSize("LM");
		_guPlrFrameSize[PGX_MAGIC] = GetPlrGFXSize("QM");
		_guPlrFrameSize[PGX_BLOCK] = GetPlrGFXSize("BL");
		_guPlrFrameSize[PGX_GOTHIT] = GetPlrGFXSize("HT");
		_guPlrFrameSize[PGX_DEATH] = GetPlrGFXSize("DT");
		static_assert((int)PGX_DEATH + 1 == NUM_PGXS, "PGX_DEATH must be the last player_graphic_idx to reuse memory for the death animation (MEM_DEATH)");
		for (int i = 0; i < PGX_DEATH; i++) {
			_guPlrFrameSize[NUM_PGXS] += _guPlrFrameSize[i];
		}
	}

	assert(plr._pAnimFileData[0] == NULL);
	BYTE* animFileData = DiabloAllocPtr(_guPlrFrameSize[NUM_PGXS]);
	plr._pAnimFileData[PGX_DEATH] = animFileData; // MEM_DEATH
	for (int i = 0; i < PGX_DEATH; i++) {
		plr._pAnimFileData[i] = animFileData;
		animFileData += _guPlrFrameSize[i];
	}

	// assert(plr._pGFXLoad == 0 || !plr._pActive);
}

void FreePlayerGFX(int pnum)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("FreePlayerGFX: illegal player %d", pnum);
	}

	MemFreeDbg(plr._pAnimFileData[0]);

	// plr._pGFXLoad = 0;
}

/**
 * @brief Sets the new Player Animation with all relevant information for rendering

 * @param pnum Player Id
 * @param animIdx type of the animation to select (player_graphic_idx)
 * @param dir the direction of the player (direction)
*/
static void NewPlrAnim(int pnum, unsigned animIdx, int dir)
{
	PlrAnimStruct* anim;

	anim = &plr._pAnims[animIdx];

	plr._pdir = dir;
	plr._pAnimData = anim->paAnimData[dir];
	plr._pAnimLen = anim->paFrames;
	plr._pAnimFrame = 1;
	plr._pAnimCnt = (gbGameLogicProgress < GLP_PLAYERS_DONE && gbGameLogicPnum <= pnum) ? -1 : 0;
	plr._pAnimFrameLen = PlrAnimFrameLens[animIdx];
	plr._pAnimWidth = anim->paAnimWidth;
	plr._pAnimXOffset = (anim->paAnimWidth - TILE_WIDTH) >> 1;
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
	plr._pAnims[PGX_STAND].paAnimWidth = 96 * ASSET_MPL;
	plr._pAnims[PGX_WALK].paAnimWidth = 96 * ASSET_MPL;
	plr._pAnims[PGX_ATTACK].paAnimWidth = 128 * ASSET_MPL;
	plr._pAnims[PGX_FIRE].paAnimWidth = 96 * ASSET_MPL;
	plr._pAnims[PGX_LIGHTNING].paAnimWidth = 96 * ASSET_MPL;
	plr._pAnims[PGX_MAGIC].paAnimWidth = 96 * ASSET_MPL;
	plr._pAnims[PGX_BLOCK].paAnimWidth = 96 * ASSET_MPL;
	plr._pAnims[PGX_GOTHIT].paAnimWidth = 96 * ASSET_MPL;
	plr._pAnims[PGX_DEATH].paAnimWidth = 128 * ASSET_MPL;

	pc = plr._pClass;
	plr._pAFNum = PlrGFXAnimActFrames[pc][0];
	plr._pSFNum = PlrGFXAnimActFrames[pc][1];

	plr._pAnims[PGX_STAND].paFrames = PlrGFXAnimLens[pc][PA_STAND];
	plr._pAnims[PGX_WALK].paFrames = PlrGFXAnimLens[pc][PA_WALK];
	plr._pAnims[PGX_ATTACK].paFrames = PlrGFXAnimLens[pc][PA_ATTACK];
	plr._pAnims[PGX_FIRE].paFrames = PlrGFXAnimLens[pc][PA_SPELL];
	plr._pAnims[PGX_LIGHTNING].paFrames = PlrGFXAnimLens[pc][PA_SPELL];
	plr._pAnims[PGX_MAGIC].paFrames = PlrGFXAnimLens[pc][PA_SPELL];
	plr._pAnims[PGX_BLOCK].paFrames = PlrGFXAnimLens[pc][PA_BLOCK];
	plr._pAnims[PGX_GOTHIT].paFrames = PlrGFXAnimLens[pc][PA_GOTHIT];
	plr._pAnims[PGX_DEATH].paFrames = PlrGFXAnimLens[pc][PA_DEATH];

	gn = plr._pgfxnum & 0xF;
	switch (pc) {
	case PC_WARRIOR:
		if (gn == ANIM_ID_BOW) {
			plr._pAnims[PGX_STAND].paFrames = 8;
			plr._pAnims[PGX_ATTACK].paAnimWidth = 96 * ASSET_MPL;
			// plr._pAnims[PGX_ATTACK].paFrames = 16;
			plr._pAFNum = 11;
		} else if (gn == ANIM_ID_AXE) {
			plr._pAnims[PGX_ATTACK].paFrames = 20;
			plr._pAFNum = 10;
		} else if (gn == ANIM_ID_STAFF) {
			// plr._pAnims[PGX_ATTACK].paFrames = 16;
			plr._pAFNum = 11;
		}
		break;
	case PC_ROGUE:
		if (gn == ANIM_ID_AXE) {
			plr._pAnims[PGX_ATTACK].paFrames = 22;
			plr._pAFNum = 13;
		} else if (gn == ANIM_ID_BOW) {
			plr._pAnims[PGX_ATTACK].paFrames = 12;
			plr._pAFNum = 7;
		} else if (gn == ANIM_ID_STAFF) {
			plr._pAnims[PGX_ATTACK].paFrames = 16;
			plr._pAFNum = 11;
		}
		break;
	case PC_SORCERER:
		plr._pAnims[PGX_FIRE].paAnimWidth = 128 * ASSET_MPL;
		plr._pAnims[PGX_LIGHTNING].paAnimWidth = 128 * ASSET_MPL;
		plr._pAnims[PGX_MAGIC].paAnimWidth = 128 * ASSET_MPL;
		if (gn == ANIM_ID_UNARMED) {
			plr._pAnims[PGX_ATTACK].paFrames = 20;
		} else if (gn == ANIM_ID_UNARMED_SHIELD) {
			// plr._pAnims[PGX_ATTACK].paFrames = 16;
			plr._pAFNum = 9;
		} else if (gn == ANIM_ID_BOW) {
			plr._pAnims[PGX_ATTACK].paFrames = 20;
			plr._pAFNum = 16;
		} else if (gn == ANIM_ID_AXE) {
			plr._pAnims[PGX_ATTACK].paFrames = 24;
			plr._pAFNum = 16;
		}
		break;
#ifdef HELLFIRE
	case PC_MONK:
		plr._pAnims[PGX_STAND].paAnimWidth = 112 * ASSET_MPL;
		plr._pAnims[PGX_WALK].paAnimWidth = 112 * ASSET_MPL;
		plr._pAnims[PGX_ATTACK].paAnimWidth = 130 * ASSET_MPL;
		plr._pAnims[PGX_FIRE].paAnimWidth = 114 * ASSET_MPL;
		plr._pAnims[PGX_LIGHTNING].paAnimWidth = 114 * ASSET_MPL;
		plr._pAnims[PGX_MAGIC].paAnimWidth = 114 * ASSET_MPL;
		plr._pAnims[PGX_BLOCK].paAnimWidth = 98 * ASSET_MPL;
		plr._pAnims[PGX_GOTHIT].paAnimWidth = 98 * ASSET_MPL;
		plr._pAnims[PGX_DEATH].paAnimWidth = 160 * ASSET_MPL;

		switch (gn) {
		case ANIM_ID_UNARMED:
		case ANIM_ID_UNARMED_SHIELD:
			plr._pAnims[PGX_ATTACK].paFrames = 12;
			plr._pAFNum = 7;
			break;
		case ANIM_ID_BOW:
			plr._pAnims[PGX_ATTACK].paFrames = 20;
			plr._pAFNum = 14;
			break;
		case ANIM_ID_AXE:
			plr._pAnims[PGX_ATTACK].paFrames = 23;
			plr._pAFNum = 14;
			break;
		case ANIM_ID_STAFF:
			plr._pAnims[PGX_ATTACK].paFrames = 13;
			plr._pAFNum = 8;
			break;
		}
		break;
	case PC_BARD:
		if (gn == ANIM_ID_AXE) {
			plr._pAnims[PGX_ATTACK].paFrames = 22;
			plr._pAFNum = 13;
		} else if (gn == ANIM_ID_BOW) {
			plr._pAnims[PGX_ATTACK].paFrames = 12;
			plr._pAFNum = 11;
		} else if (gn == ANIM_ID_STAFF) {
			plr._pAnims[PGX_ATTACK].paFrames = 16;
			plr._pAFNum = 11;
		} else if (gn == ANIM_ID_SWORD_SHIELD || gn == ANIM_ID_SWORD) {
			plr._pAnims[PGX_ATTACK].paFrames = 10; // TODO: check for onehanded swords or daggers?
		}
		break;
	case PC_BARBARIAN:
		if (gn == ANIM_ID_AXE) {
			plr._pAnims[PGX_ATTACK].paFrames = 20;
			plr._pAFNum = 8;
		} else if (gn == ANIM_ID_BOW) {
			plr._pAnims[PGX_STAND].paFrames = 8;
			plr._pAnims[PGX_ATTACK].paAnimWidth = 96 * ASSET_MPL;
			plr._pAFNum = 11;
		} else if (gn == ANIM_ID_STAFF) {
			// plr._pAnims[PGX_ATTACK].paFrames = 16;
			plr._pAFNum = 11;
		} else if (gn == ANIM_ID_MACE || gn == ANIM_ID_MACE_SHIELD) {
			// plr._pAnims[PGX_ATTACK].paFrames = 16;
			plr._pAFNum = 8;
		}
		break;
#endif
	default:
		ASSUME_UNREACHABLE
		break;
	}
	if (currLvl._dType == DTYPE_TOWN) {
		plr._pAnims[PGX_STAND].paFrames = 20;
		// plr._pAnims[PGX_WALK].paFrames = 8;
	}
}

/**
 * @param c plr_classes value
 */
void CreatePlayer(const _uiheroinfo& heroinfo)
{
	int val, hp, mana;
	int pnum = 0;

	memset(&plr, 0, sizeof(PlayerStruct));
	SetRndSeed(SDL_GetTicks()); // used by CreatePlrItems / CreateBaseItem

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
	/*plr._pHitPoints = plr._pMaxHP =*/ plr._pHPBase = plr._pMaxHPBase = hp;

	val = heroinfo.hiMagic;
	//plr._pMagic = val;
	plr._pBaseMag = val;

	mana = val << (6 + 1);
	/*plr._pMana = plr._pMaxMana =*/ plr._pManaBase = plr._pMaxManaBase = mana;

	//plr._pNextExper = PlrExpLvlsTbl[1];
	plr._pLightRad = 10;

	//plr._pMainSkill = { { SPL_ATTACK, RSPLTYPE_ABILITY } , { SPL_WALK, RSPLTYPE_ABILITY } };
	//plr._pAltSkill = { { SPL_NULL, 0 } , SPL_NULL, 0 } };
	static_assert((int)SPL_NULL == 0, "CreatePlayer fails to initialize the skillhotkeys I.");
	static_assert(offsetof(PlayerStruct, _pAltSkillSwapKey) - offsetof(PlayerStruct, _pSkillHotKey) == sizeof(plr._pSkillHotKey) + sizeof(plr._pAltSkillHotKey) + sizeof(plr._pSkillSwapKey),
		"CreatePlayer fails to initialize the skillhotkeys II.");
	static_assert(offsetof(PlayerStruct, _pAltSkillHotKey) > offsetof(PlayerStruct, _pSkillHotKey) && offsetof(PlayerStruct, _pAltSkillHotKey) < offsetof(PlayerStruct, _pAltSkillSwapKey),
		"CreatePlayer fails to initialize the skillhotkeys III.");
	static_assert(offsetof(PlayerStruct, _pSkillSwapKey) > offsetof(PlayerStruct, _pSkillHotKey) && offsetof(PlayerStruct, _pSkillSwapKey) < offsetof(PlayerStruct, _pAltSkillSwapKey),
		"CreatePlayer fails to initialize the skillhotkeys IV.");
	memset(plr._pSkillHotKey, 0, offsetof(PlayerStruct, _pAltSkillSwapKey) - offsetof(PlayerStruct, _pSkillHotKey) + sizeof(plr._pAltSkillSwapKey));

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
	const int8_t* cr;

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
		static_assert(DBORDERX >= 15 && DBORDERY >= 15, "PlacePlayer expects a large enough border.");
		static_assert(lengthof(CrawlNum) > 15, "PlacePlayer uses CrawlTable/CrawlNum up to radius 16.");
		for (i = 2; i <= 15; i++) {
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
		plr._pSkillFlags |= SFLAG_DUNGEON;
	} else {
		plr._pSkillFlags &= ~SFLAG_DUNGEON;
	}

	CalcPlrItemVals(pnum, false);

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
		// assert(plr._pAnims[PGX_STAND].paFrames == plr._pAnimLen);
		// plr._pAnimFrame = RandRange(1, plr._pAnimLen - 1);
		// plr._pAnimCnt = random_(2, 3);

		plr._pDestAction = ACTION_NONE;
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
	if (currLvl._dLevelIdx != DLV_TOWN) {
		plr._pvid = AddVision(plr._poldx, plr._poldy, std::max(PLR_MIN_VISRAD, (int)plr._pLightRad), pnum == mypnum);
	} else {
		plr._pvid = NO_VISION;
	}
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
			MonKill(pnum, pnum);
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

	//if (plr._pHitPoints == 0) {
	//	return;
	//}
	// assert(plr._pmode != PM_DEATH && plr._pmode != PM_DYING);

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

static void StartPlrKill(int pnum, int dmgtype)
{
	if (plr._pmode == PM_DEATH || plr._pmode == PM_DYING)
		return;

	plr._pmode = PM_DYING;
	plr._pInvincible = TRUE;

	plr._pManaShield = 0;
	plr._pTimer[PLTR_INFRAVISION] = 0;
	plr._pTimer[PLTR_RAGE] = 0;

	if (pnum == mypnum) {
		gbDeathflag = MDM_DYING;
		gbActionBtnDown = 0;
		NetSendCmdBParam1(CMD_PLRDEAD, dmgtype);
	}

	if (plr._pDunLevel == currLvl._dLevelIdx) {
		if (plr._pgfxnum != ANIM_ID_UNARMED) {
			plr._pgfxnum = ANIM_ID_UNARMED;
			plr._pGFXLoad = 0;
			SetPlrAnims(pnum);
		}

		if (!(plr._pGFXLoad & PGF_DEATH)) {
			plr._pGFXLoad = 0; // MEM_DEATH: reset _pGFXLoad to make death and non-death animations exclusive
			LoadPlrGFX(pnum, PGF_DEATH);
		}

		NewPlrAnim(pnum, PGX_DEATH, plr._pdir);

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
		myview.x = plr._px; // - ScrollInfo._sdx;
		myview.y = plr._py; // - ScrollInfo._sdy;
	}
}

static void AssertFixPlayerLocation(int pnum)
{
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
		assert(myview.x == plr._px); // - ScrollInfo._sdx;
		assert(myview.y == plr._py); // - ScrollInfo._sdy;
	}
}

static void StartStand(int pnum)
{
	plr._pVar1 = PM_STAND; // STAND_PREV_MODE -- TODO: plr._pmode?
	plr._pmode = PM_STAND;

	if (!(plr._pGFXLoad & PGF_STAND)) {
		LoadPlrGFX(pnum, PGF_STAND);
	}

	NewPlrAnim(pnum, PGX_STAND, plr._pdir);
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

static void PlrChangeOffset(int pnum)
{
	// int px, py;

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
		// if (gbActionBtnDown != 0 && (px | py) != 0 && MON_VALID(pcursmonst))
		//	SetCursorPos(MousePos.x + px, MousePos.y + py);
	}

	//if (plr._plid != NO_LIGHT)
		CondChangeLightScreenOff(plr._plid, plr._pxoff, plr._pyoff);
}

/**
 * @brief Start a move action
 */
static void StartWalk1(int pnum, int xvel, int yvel, int dir)
{
	int px, py;

	plr._pmode = PM_WALK;
	plr._pVar4 = xvel; // WALK_XVEL : velocity of the player in the X-direction
	plr._pVar5 = yvel; // WALK_YVEL : velocity of the player in the Y-direction
	plr._pxoff = 0;
	plr._pyoff = 0;
	//plr._pVar3 = dir;  // Player's direction when ending movement.
	plr._pVar6 = 0;    // WALK_XOFF : _pxoff value in a higher range
	plr._pVar7 = 0;    // WALK_YOFF : _pyoff value in a higher range
	plr._pVar8 = 0;    // WALK_TICK : speed helper

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
 * @brief Start a move action and shift to the future position
 */
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
		myview.x = plr._px;
		myview.y = plr._py;
		ScrollInfo._sxoff = -plr._pxoff;
		ScrollInfo._syoff = -plr._pyoff;
	}
	//if (plr._plid != NO_LIGHT) {
		ChangeLightXY(plr._plid, plr._px, plr._py);
		ChangeLightScreenOff(plr._plid, plr._pxoff, plr._pyoff);
	//}
}

static void StartWalk(int pnum, int dir)
{
	int mwi;

	static_assert(TILE_WIDTH / TILE_HEIGHT == 2, "StartWalk relies on fix width/height ratio of the floor-tile.");
	static_assert(PLR_WALK_SHIFT == MON_WALK_SHIFT, "To reuse MWVel in StartWalk, PLR_WALK_SHIFT must be equal to MON_WALK_SHIFT.");
	// assert(PlrGFXAnimLens[plr._pClass][PA_WALK] == PlrGFXAnimLens[PC_WARRIOR][PA_WALK]);
	assert(PlrGFXAnimLens[PC_WARRIOR][PA_WALK] <= lengthof(MWVel));
	assert(PlrGFXAnimLens[PC_WARRIOR][PA_WALK] == 8); // StartWalk relies on fix walk-animation length to calculate the x/y velocity
	mwi = MWVel[PlrGFXAnimLens[PC_WARRIOR][PA_WALK] - (plr._pIWalkSpeed == 0 ? 0 : (1 + plr._pIWalkSpeed)) - 1];
	switch (dir) {
	case DIR_N:
		StartWalk1(pnum, 0, -(mwi >> 1), dir);
		break;
	case DIR_NE:
		StartWalk1(pnum, (mwi >> 1), -(mwi >> 2), dir);
		break;
	case DIR_E:
		StartWalk2(pnum, mwi, 0, -TILE_WIDTH, 0, dir);
		break;
	case DIR_SE:
		StartWalk2(pnum, (mwi >> 1), (mwi >> 2), -TILE_WIDTH/2, -TILE_HEIGHT/2, dir);
		break;
	case DIR_S:
		StartWalk2(pnum, 0, (mwi >> 1), 0, -TILE_HEIGHT, dir);
		break;
	case DIR_SW:
		StartWalk2(pnum, -(mwi >> 1), (mwi >> 2), TILE_WIDTH/2, -TILE_HEIGHT/2, dir);
		break;
	case DIR_W:
		StartWalk1(pnum, -mwi, 0, dir);
		break;
	case DIR_NW:
		StartWalk1(pnum, -(mwi >> 1), -(mwi >> 2), dir);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	if (!(plr._pGFXLoad & PGF_WALK)) {
		LoadPlrGFX(pnum, PGF_WALK);
	}

	NewPlrAnim(pnum, PGX_WALK, dir);

	if (pnum == mypnum) {
		// assert(ScrollInfo._sdx == 0);
		// assert(ScrollInfo._sdy == 0);
		// assert(plr._poldx == myview.x);
		// assert(plr._poldy == myview.y);
		// ScrollInfo._sdx = plr._poldx - myview.x;
		// ScrollInfo._sdy = plr._poldy - myview.y;

#if DEBUG_MODE
		for (int i = 0; i < lengthof(dir2sdir); i++)
			assert(dir2sdir[i] == 1 + i);
#endif
		dir = 1 + dir; // == dir2sdir[dir];
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
}

static void StartAttack(int pnum)
{
	int i, dx, dy, sn, sl, dir, ss;

	i = plr._pDestParam1;
	switch (plr._pDestAction) {
	case ACTION_ATTACK:
		dx = i;
		dy = plr._pDestParam2;
		break;
	case ACTION_ATTACKMON:
		dx = monsters[i]._mfutx;
		dy = monsters[i]._mfuty;
		break;
	case ACTION_ATTACKPLR:
		dx = plx(i)._pfutx;
		dy = plx(i)._pfuty;
		break;
	case ACTION_OPERATE:
		dx = i;
		dy = plr._pDestParam2;
		i = plr._pDestParam4;
		assert(abs(dObject[dx][dy]) == i + 1);
		if (objects[i]._oBreak == OBM_UNBREAKABLE) {
			OperateObject(pnum, i, false);
			return; // true;
		}
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	sn = plr._pDestParam3;
	sl = plr._pDestParam4;
	dir = GetDirection(plr._px, plr._py, dx, dy);
	ss = plr._pIBaseAttackSpeed;
	if (sn == SPL_WHIPLASH) {
		ss += 3;
		if (ss > 4)
			ss = 4;
	} else if (sn == SPL_WALLOP) {
		ss -= 3;
	}

	plr._pmode = PM_ATTACK;
	plr._pVar4 = ss; // ATTACK_SPEED
	plr._pVar5 = sn; // ATTACK_SKILL
	plr._pVar6 = sl; // ATTACK_SKILL_LEVEL
	plr._pVar7 = 0;  // ATTACK_ACTION_PROGRESS : 'flags' of sfx and hit
	plr._pVar8 = 0;  // ATTACK_TICK : speed helper

	if (!(plr._pGFXLoad & PGF_ATTACK)) {
		LoadPlrGFX(pnum, PGF_ATTACK);
	}
	NewPlrAnim(pnum, PGX_ATTACK, dir);

	AssertFixPlayerLocation(pnum);
	// return true;
}

static void StartRangeAttack(int pnum)
{
	int i, dx, dy, sn, sl, dir, ss;

	i = plr._pDestParam1;
	switch (plr._pDestAction) {
	case ACTION_RATTACK:
		dx = i;
		dy = plr._pDestParam2;
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
	sn = plr._pDestParam3;
	sl = plr._pDestParam4;
	ss = plr._pIBaseAttackSpeed;

	plr._pVar1 = dx;    // RATTACK_TARGET_X
	plr._pVar2 = dy;    // RATTACK_TARGET_Y
	plr._pVar4 = ss;    // RATTACK_SPEED
	plr._pVar5 = sn;    // RATTACK_SKILL
	plr._pVar6 = sl;    // RATTACK_SKILL_LEVEL
	plr._pVar7 = FALSE; // RATTACK_ACTION_PROGRESS : 'flag' of launch
	plr._pVar8 = 0;     // RATTACK_TICK : speed helper
	plr._pmode = PM_RATTACK;

	dir = GetDirection(plr._px, plr._py, dx, dy);

	if (!(plr._pGFXLoad & PGF_ATTACK)) {
		LoadPlrGFX(pnum, PGF_ATTACK);
	}
	NewPlrAnim(pnum, PGX_ATTACK, dir);

	AssertFixPlayerLocation(pnum);
}

static void StartTurn(int pnum, int dir)
{
	// assert(plr._pmode == PM_STAND);
	plr._pdir = dir;
	plr._pAnimData = plr._pAnims[PGX_STAND].paAnimData[dir];
	// StartStand(pnum);
}

static void StartBlock(int pnum, int dir)
{
	plr._pmode = PM_BLOCK;
	plr._pVar1 = 8 - plr._pAnims[PGX_BLOCK].paFrames; // BASE_BLOCK_EXTENSION
	plr._pVar2 = 0; // BLOCK_EXTENSION : extended blocking
	if (!(plr._pGFXLoad & PGF_BLOCK)) {
		LoadPlrGFX(pnum, PGF_BLOCK);
	}
	NewPlrAnim(pnum, PGX_BLOCK, dir);

	AssertFixPlayerLocation(pnum);
}

static void StartSpell(int pnum)
{
	int i, dx, dy, gfx, animIdx;
	const SpellData* sd;

	i = plr._pDestParam1;
	switch (plr._pDestAction) {
	case ACTION_SPELL:
		dx = i;
		dy = plr._pDestParam2;
		break;
	case ACTION_SPELLMON:
		dx = monsters[i]._mfutx;
		dy = monsters[i]._mfuty;
		break;
	case ACTION_SPELLPLR:
		// preserve target information for the resurrect spell
		if (plr._pDestParam3 == SPL_RESURRECT) // SPELL_NUM
			plr._pDestParam4 = i;              // SPELL_LEVEL
		dx = plx(i)._pfutx;
		dy = plx(i)._pfuty;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	plr._pVar1 = dx;               // SPELL_TARGET_X
	plr._pVar2 = dy;               // SPELL_TARGET_Y
	plr._pVar5 = plr._pDestParam3; // SPELL_NUM : the spell to be cast
	plr._pVar6 = plr._pDestParam4; // SPELL_LEVEL
	plr._pVar7 = FALSE;            // SPELL_ACTION_PROGRESS : 'flag' of cast
	plr._pVar8 = 0;                // SPELL_TICK : speed helper
	plr._pmode = PM_SPELL;

	sd = &spelldata[plr._pVar5]; // SPELL_NUM
	if (sd->sSkillFlags & SDFLAG_TARGETED)
		plr._pdir = GetDirection(plr._px, plr._py, dx, dy);

	static_assert((int)PGX_LIGHTNING - (int)PGX_FIRE == (int)STYPE_LIGHTNING - (int)STYPE_FIRE, "StartSpell expects ordered player_graphic_idx and magic_type I.");
	static_assert((int)PGX_MAGIC - (int)PGX_FIRE == (int)STYPE_MAGIC - (int)STYPE_FIRE, "StartSpell expects ordered player_graphic_idx and magic_type II.");
	animIdx = PGX_FIRE + sd->sType - STYPE_FIRE;
	static_assert((int)PGX_FIRE + 2 == (int)PGT_FIRE, "StartSpell calculates player_graphic_flag I.");
	static_assert((int)PGX_LIGHTNING + 2 == (int)PGT_LIGHTNING, "StartSpell calculates player_graphic_flag II.");
	static_assert((int)PGX_MAGIC + 2 == (int)PGT_MAGIC, "StartSpell calculates player_graphic_flag III.");
	static_assert((int)PGF_FIRE == 1 << (int)PGT_FIRE, "StartSpell calculates player_graphic_flag.");
	gfx = 1 << (animIdx + 2);

	if (!(plr._pGFXLoad & gfx)) {
		LoadPlrGFX(pnum, gfx);
	}
	NewPlrAnim(pnum, animIdx, plr._pdir);

	PlaySfxLoc(sd->sSFX, plr._px, plr._py);

	AssertFixPlayerLocation(pnum);
}

static void StartPickItem(int pnum)
{
	if (pnum == mypnum && pcursicon == CURSOR_HAND) {
		NetSendCmdGItem(!gbInvflag ? CMD_AUTOGETITEM : CMD_GETITEM, plr._pDestParam4);
	}
}

static void StartTalk(int pnum)
{
	int mnum = plr._pDestParam1;

	if (currLvl._dLevelIdx == DLV_TOWN)
		TalkToTowner(mnum, pnum);
	else
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
	assert(dx >= 1 && dx < MAXDUNX - 1);
	assert(dy >= 1 && dy < MAXDUNY - 1);
	static_assert(DBORDERX >= 1 && DBORDERY >= 1, "RemovePlrFromMap expects a large enough border.");

	pp = pnum + 1;
	for (x = dx - 1; x <= dx + 1; x++) {
		for (y = dy - 1; y <= dy + 1; y++) {
			if (abs(dPlayer[x][y]) == pp) {
				dPlayer[x][y] = 0;
			}
		}
	}
}

static void PlrStartGetHit(int pnum, int dir)
{
	if (!(plr._pGFXLoad & PGF_GOTHIT)) {
		LoadPlrGFX(pnum, PGF_GOTHIT);
	}
	NewPlrAnim(pnum, PGX_GOTHIT, dir);

	plr._pmode = PM_GOTHIT;
	plr._pVar8 = 0; // GOTHIT_TICK
	RemovePlrFromMap(pnum);
	dPlayer[plr._px][plr._py] = pnum + 1;
	FixPlayerLocation(pnum);
}

static void PlrGetKnockback(int pnum, int dir)
{
	int oldx, oldy, newx, newy;
	// assert(plr._pHitPoints != 0);
	// AssertFixPlayerLocation(pnum);

	oldx = plr._px;
	oldy = plr._py;
	if (PathWalkable(oldx, oldy, dir2pdir[dir])) {
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
}

void PlrHitByAny(int pnum, int mpnum, int dam, unsigned hitflags, int dir)
{
	bool knockback, stun;
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrHitByAny: illegal player %d", pnum);
	}
	// assert(plr._pHitPoints != 0 && dam >= 0);
	if (plr._pManaShield != 0) {
		hitflags &= (ISPL_FAKE_FORCE_STUN | ISPL_KNOCKBACK);
		if (hitflags == 0)
			return;
		// dam = 0;
	}

	PlaySfxLocN(sgSFXSets[SFXS_PLR_69][plr._pClass], plr._px, plr._py, 2);

	static_assert(MAX_PLRS <= MAX_MINIONS, "PlrHitByAny uses a single int to store player and monster sources.");
	if (!(plr._pIFlags & ISPL_NO_BLEED) && (hitflags & ISPL_FAKE_CAN_BLEED)
	 && ((hitflags & ISPL_BLEED) ? random_(47, 64) == 0 : random_(48, 128) == 0))
		AddMissile(0, 0, 0, 0, 0, MIS_BLEED, mpnum < MAX_PLRS ? (mpnum < 0 ? MST_OBJECT : MST_PLAYER) : MST_MONSTER, mpnum, pnum); // TODO: prevent golems from acting like a player?
	knockback = (hitflags & ISPL_KNOCKBACK) != 0;
	stun = (hitflags & ISPL_FAKE_FORCE_STUN) || (dam << ((hitflags & ISPL_STUN) ? 3 : 2)) >= plr._pMaxHP;
	if (knockback || stun) {
		if (stun || plr._pmode != PM_GOTHIT)
			PlrStartGetHit(pnum, OPPOSITE(dir));
		if (knockback)
			PlrGetKnockback(pnum, dir);
	}
}

void SyncPlrKill(int pnum)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("SyncPlrKill: illegal player %d", pnum);
	}

	if (plr._pmode == PM_DEATH) {
		return;
	}

	if (plr._pmode != PM_DYING) {
		StartPlrKill(pnum, DMGTYPE_UNKNOWN);
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

	plr._pDestAction = ACTION_NONE;
	plr._pmode = PM_STAND;
	plr._pInvincible = FALSE;
	plr._pGFXLoad = 0; // MEM_DEATH

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

	plr._pDestAction = ACTION_NONE;
	plr._pLvlChanging = TRUE;
	plr._pmode = PM_NEWLVL;
	plr._pInvincible = TRUE;
}

#if defined(__clang__) || defined(__GNUC__)
__attribute__((no_sanitize("shift-base")))
#endif
void StartNewLvl(int pnum, int fom, int lvl)
{
	// assert((unsigned)pnum < MAX_PLRS);

	InitLevelChange(pnum);

	// net_assert(lvl < NUM_LEVELS);
	plr._pDunLevel = lvl;
	if (pnum == mypnum) {
		if (fom == DVL_DWM_TWARPUP) {
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
		PostMessage(fom);
	}
}

void RestartTownLvl(int pnum)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("RestartTownLvl: illegal player %d", pnum);
	}
	InitLevelChange(pnum);

	plr._pDunLevel = DLV_TOWN;
	// plr._pGFXLoad = 0; // MEM_DEATH

	PlrSetHp(pnum, (1 << 6));

	// CalcPlrInv(pnum, false); -- no need to calc, InitLvlPlayer should take care about it

	if (pnum == mypnum) {
		gbDeathflag = MDM_ALIVE;
		gamemenu_off();
		PostMessage(DVL_DWM_RETOWN);
	}
}

void UseTownPortal(int pnum, int pidx)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("UseTownPortal: illegal player %d", pnum);
	}
	InitLevelChange(pnum);

	if (plr._pDunLevel != DLV_TOWN) {
		plr._pDunLevel = DLV_TOWN;
	} else {
		plr._pDunLevel = portals[pidx]._rlevel;
		if (pidx == pnum) {
			DeactivatePortal(pidx);
		}
	}

	if (pnum == mypnum) {
		UseCurrentPortal(pidx);
		PostMessage(DVL_DWM_PORTLVL);
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
	bool stepAnim;

	plr._pVar8++; // WALK_TICK
	switch (plr._pIWalkSpeed) {
	case 0:
		stepAnim = false;
		break;
	case 1:
		stepAnim = (plr._pVar8 & 3) == 2;
		break;
	case 2:
		stepAnim = (plr._pVar8 & 1) == 1;
		break;
	case 3:
		stepAnim = true;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	if (stepAnim) {
		PlrStepAnim(pnum);
		// assert(PlrAnimFrameLens[PGX_WALK] == 1);
		// PlrChangeOffset(pnum); -- unnecessary in case the velocity is based on _pIWalkSpeed
	}
	assert(PlrAnimFrameLens[PGX_WALK] == 1);
	if ((plr._pAnimFrame & 3) == 3) {
		PlayWalkSfx(pnum);
	}

	assert(PlrAnimFrameLens[PGX_WALK] == 1);
	// assert(plr._pAnims[PGX_WALK].paFrames == plr._pAnimLen);
	if (plr._pAnimFrame < plr._pAnimLen) {
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
	//PlrStartStand(pnum);
	StartStand(pnum);
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
		NetSendCmdBParam1(CMD_DELPLRITEM, iLoc);
}

static void WeaponDur(int pnum, int durrnd)
{
	ItemStruct* pi;

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

	mon = &monsters[mnum];

	hper = plr._pIHitChance - mon->_mArmorClass;
	if (sn == SPL_SWIPE) {
		hper -= 30 - sl * 2;
	}
	if (!CheckHit(hper) && mon->_mmode != MM_STONE)
		return false;

	if (!CheckMonsterHit(mnum, &ret))
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

	switch (sn) {
	case SPL_ATTACK:
		break;
	case SPL_SWIPE:
		dam = (dam * (48 + sl)) >> 6;
		break;
	case SPL_WALLOP:
		dam = (dam * (112 + sl)) >> 6;
		break;
	case SPL_WHIPLASH:
		dam = (dam * (24 + sl)) >> 6;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
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
	int adam = plr._pIAMaxDam;
	if (adam != 0)
		adam = CalcMonsterDam(mon->_mMagicRes, MISR_ACID, plr._pIAMinDam, adam, false);

	dam += AddElementalExplosion(fdam, ldam, mdam, adam, true, mnum);

	//if (pnum == mypnum) {
		mon->_mhitpoints -= dam;
	//}

	if (mon->_mhitpoints < (1 << 6)) {
		MonKill(mnum, pnum);
	} else {
		hitFlags = (plr._pIFlags & ISPL_HITFLAGS_MASK) | ISPL_FAKE_CAN_BLEED;
		//if (hitFlags & ISPL_NOHEALMON)
		//	mon->_mFlags |= MFLAG_NOHEAL;
		MonHitByPlr(mnum, pnum, dam, hitFlags, plr._pdir);
	}
	return true;
}

static bool PlrHitPlr(int offp, int sn, int sl, int pnum)
{
	int hper, dam, damsl, dambl, dampc;
	unsigned tmp, hitFlags;

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

	if (PlrCheckBlock(pnum, 2 * plx(offp)._pLevel, OPPOSITE(plx(offp)._pdir)))
		return true;

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

	tmp = sn == SPL_SWIPE ? 800 : 200;
	if (random_low(6, tmp) < plx(offp)._pICritChance) {
		dam <<= 1;
	}

	switch (sn) {
	case SPL_ATTACK:
		break;
	case SPL_SWIPE:
		dam = (dam * (48 + sl)) >> 6;
		break;
	case SPL_WALLOP:
		dam = (dam * (112 + sl)) >> 6;
		break;
	case SPL_WHIPLASH:
		dam = (dam * (24 + sl)) >> 6;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	dam -= plr._pIAbsAnyHit + plr._pIAbsPhyHit;
	if (dam > 0 && plx(offp)._pILifeSteal != 0) {
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
		dam += AddElementalExplosion(fdam, ldam, mdam, adam, false, pnum);
	}
	if (dam <= 0) {
		dam = 1;
	}

	if (!PlrDecHp(pnum, dam, DMGTYPE_PLAYER)) {
		hitFlags = (plx(offp)._pIFlags & ISPL_HITFLAGS_MASK) | ISPL_FAKE_CAN_BLEED;
		PlrHitByAny(pnum, offp, dam, hitFlags, plx(offp)._pdir);
	}
	return true;
}

static int PlrTryHit(int pnum, int dir)
{
	int dx, dy, mpo, sn, sl;

	plr._pdir = dir;
	dx = plr._px + offset_x[dir];
	dy = plr._py + offset_y[dir];
	sn = plr._pVar5; // ATTACK_SKILL
	sl = plr._pVar6, // ATTACK_SKILL_LEVEL

	mpo = dMonster[dx][dy];
	if (mpo != 0) {
		mpo = CheckMonCol(mpo);
		return (mpo >= 0 && PlrHitMonst(pnum, sn, sl, mpo)) ? 1 : 0;
	}
	mpo = dPlayer[dx][dy];
	if (mpo != 0) {
		mpo = CheckPlrCol(mpo);
		return (mpo >= 0 && PlrHitPlr(pnum, sn, sl, mpo)) ? 1 : 0;
	}
	mpo = dObject[dx][dy];
	if (mpo != 0) {
		mpo = mpo >= 0 ? mpo - 1 : -(mpo + 1);
		if (objects[mpo]._oBreak == OBM_BREAKABLE) {
			OperateObject(pnum, mpo, false);
			return 0; // do not reduce the durability if the target is an object
		}
	}
	return 0;
}

static void PlrDoAttack(int pnum)
{
	int dir, hitcnt;
	bool stepAnim = false;

	plr._pVar8++;         // ATTACK_TICK
	switch (plr._pVar4) { // ATTACK_SPEED
	/*case -4:
		if ((plr._pVar8 & 1) == 1)
			plr._pAnimCnt--;
		break;*/
	case -3:
		if ((plr._pVar8 % 3u) == 0)
			plr._pAnimCnt--;
		break;
	case -2:
		if ((plr._pVar8 & 3) == 2)
			plr._pAnimCnt--;
		break;
	case -1:
		if ((plr._pVar8 & 7) == 4)
			plr._pAnimCnt--;
		break;
	case 0:
		break;
	case 1:
		stepAnim = (plr._pVar8 & 7) == 4;
		break;
	case 2:
		stepAnim = (plr._pVar8 & 3) == 2;
		break;
	case 3:
		stepAnim = (plr._pVar8 & 1) == 1;
		break;
	case 4:
		stepAnim = true;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	if (stepAnim) {
		PlrStepAnim(pnum);
	}

	if (plr._pAnimFrame < plr._pAFNum - 1)
		return;
	if (plr._pVar7 == 0) { // ATTACK_ACTION_PROGRESS
		plr._pVar7 = 1;
		PlaySfxLocN(PS_SWING, plr._px, plr._py, 2);
	}
	if (plr._pAnimFrame == plr._pAFNum - 1) {
		return;
	}
	if (plr._pVar7 == 1) {
		plr._pVar7 = 2;
		dir = plr._pdir;
		hitcnt = PlrTryHit(pnum, dir);
		if (plr._pVar5 == SPL_SWIPE) {
			hitcnt += PlrTryHit(pnum, (dir + 1) & 7);
			hitcnt += PlrTryHit(pnum, (dir + 7) & 7);
		}

		if (hitcnt != 0) {
			WeaponDur(pnum, 40 - hitcnt * 8);
		}
		// return early if the weapon is lost or triggered a got-hit/death animation
		if (plr._pmode != PM_ATTACK) {
			return;
		}
		plr._pdir = dir;
	}
	assert(PlrAnimFrameLens[PGX_ATTACK] == 1);
	// assert(plr._pAnims[PGX_ATTACK].paFrames == plr._pAnimLen);
	if (plr._pAnimFrame < plr._pAnimLen) {
		if (plr._pAnimFrame > plr._pAFNum
		 && (plr._pDestAction == ACTION_ATTACK
		  || plr._pDestAction == ACTION_ATTACKMON
		  || plr._pDestAction == ACTION_ATTACKPLR
		  || (plr._pDestAction == ACTION_OPERATE && objects[plr._pDestParam4]._oBreak != OBM_UNBREAKABLE))) {
			// assert(plr._pmode == PM_ATTACK);
			plr._pVar1 = PM_ATTACK; // STAND_PREV_MODE
			plr._pmode = PM_STAND;
		}
	} else {
		//PlrStartStand(pnum);
		StartStand(pnum);
		//ClearPlrPVars(pnum);
	}
}

static void PlrDoRangeAttack(int pnum)
{
	bool stepAnim = false;
	int numarrows, sx, sy, dx, dy;

	plr._pVar8++;         // RATTACK_TICK
	switch (plr._pVar4) { // RATTACK_SPEED
	case -4:
		if ((plr._pVar8 & 1) == 1)
			plr._pAnimCnt--;
		break;
	case -3:
		if ((plr._pVar8 % 3u) == 0)
			plr._pAnimCnt--;
		break;
	case -2:
		if ((plr._pVar8 & 3) == 2)
			plr._pAnimCnt--;
		break;
	case -1:
		if ((plr._pVar8 & 7) == 4)
			plr._pAnimCnt--;
		break;
	case 0:
		break;
	case 1:
		stepAnim = (plr._pVar8 & 7) == 4;
		break;
	case 2:
		stepAnim = (plr._pVar8 & 3) == 2;
		break;
	case 3:
		stepAnim = (plr._pVar8 & 1) == 1;
		break;
	case 4:
		stepAnim = true;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	if (stepAnim) {
		PlrStepAnim(pnum);
	}

	if (plr._pAnimFrame < plr._pAFNum)
		return;

	if (!plr._pVar7) { // RATTACK_ACTION_PROGRESS
		plr._pVar7 = TRUE;

		numarrows = plr._pVar5 == SPL_MULTI_SHOT ? 3 : 1; // RATTACK_SKILL
		sx = plr._px;
		sy = plr._py;
		dx = plr._pVar1; // RATTACK_TARGET_X
		dy = plr._pVar2; // RATTACK_TARGET_Y

		while (--numarrows >= 0) {
			int xoff = 0;
			int yoff = 0;
			if (numarrows != 0) {
				int angle = numarrows == 2 ? -1 : 1;
				int x = dx - sx;
				if (x != 0)
					yoff = x < 0 ? angle : -angle;
				int y = dy - sy;
				if (y != 0)
					xoff = y < 0 ? -angle : angle;

			}
			AddMissile(sx, sy, dx + xoff, dy + yoff, plr._pdir,
				spelldata[plr._pVar5].sMissile, MST_PLAYER, pnum, plr._pVar6); // RATTACK_SKILL, RATTACK_SKILL_LEVEL
		}

		WeaponDur(pnum, 40);
	}
	assert(PlrAnimFrameLens[PGX_ATTACK] == 1);
	// assert(plr._pAnims[PGX_ATTACK].paFrames == plr._pAnimLen);
	if (plr._pAnimFrame < plr._pAnimLen) {
		if (plr._pAnimFrame > plr._pAFNum
		 && (plr._pDestAction == ACTION_RATTACK
		  || plr._pDestAction == ACTION_RATTACKMON
		  || plr._pDestAction == ACTION_RATTACKPLR)) {
			// assert(plr._pmode == PM_RATTACK);
			plr._pVar1 = PM_RATTACK; // STAND_PREV_MODE
			plr._pmode = PM_STAND;
		}
	} else {
		//PlrStartStand(pnum);
		StartStand(pnum);
		//ClearPlrPVars(pnum);
	}
}

static void ShieldDur(int pnum)
{
	ItemStruct* pi;

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

bool PlrCheckBlock(int pnum, int bmod, int dir)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrCheckBlock: illegal player %d", pnum);
	}

	bool result = false;
	int blkper = plr._pIBlockChance;
	if (blkper != 0
	 && (plr._pmode == PM_STAND || plr._pmode == PM_BLOCK)) {
		// assert(plr._pSkillFlags & SFLAG_BLOCK);
		blkper = blkper - bmod * 2;
		if (CheckHit(blkper)) {
			if (plr._pmode == PM_STAND) {
				AssertFixPlayerLocation(pnum);
				StartBlock(pnum, dir);
			}

			PlaySfxLoc(IS_ISWORD, plr._px, plr._py);
			if (random_(3, 10) == 0) {
				ShieldDur(pnum);
			}
			result = true;
		}
	}

	return result;
}

static void PlrDoBlock(int pnum)
{
	int extlen;

	if (plr._pVar2 != 0) { // BLOCK_EXTENSION
		plr._pVar2--;
		plr._pAnimCnt--;
		return;
	}

	if (plr._pIFlags & ISPL_FASTBLOCK) {
		PlrStepAnim(pnum);
	}
	// assert(plr._pAnims[PGX_BLOCK].paFrames == plr._pAnimLen);
	if (plr._pAnimFrame > plr._pAnimLen || (plr._pAnimFrame == plr._pAnimLen && plr._pAnimCnt >= PlrAnimFrameLens[PGX_BLOCK] - 1)) {
		if (plr._pDestAction == ACTION_BLOCK) {
			// extend the blocking animation
			plr._pDestAction = ACTION_NONE;
			// StartBlock(pnum, plr._pDestParam1);
			plr._pdir = plr._pDestParam1;
			plr._pAnimData = plr._pAnims[PGX_BLOCK].paAnimData[plr._pDestParam1];
			// jump to the last frame
			plr._pAnimFrame = plr._pAnimLen; //  plr._pAnims[PGX_BLOCK].paFrames;
			plr._pAnimCnt = PlrAnimFrameLens[PGX_BLOCK] - 2;
			// extend the blocking duration with a fixed amount
			extlen = plr._pAnimLen * 4; // plr._pAnims[PGX_BLOCK].paFrames * 4;
			if (plr._pIFlags & ISPL_FASTBLOCK) {
				extlen >>= 1;
			}
			plr._pVar2 = extlen; // BLOCK_EXTENSION
			// restore the base extension
			plr._pVar1 = 8 - plr._pAnimLen; // BASE_BLOCK_EXTENSION
		} else if (plr._pDestAction == ACTION_NONE && plr._pVar1 > 0) { // BASE_BLOCK_EXTENSION
			plr._pVar1--;
			// jump to the last frame
			plr._pAnimFrame = plr._pAnimLen;
			plr._pAnimCnt = PlrAnimFrameLens[PGX_BLOCK] - 2;
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
	bool stepAnim;

	plr._pVar8++; // SPELL_TICK
	switch (plr._pIBaseCastSpeed) {
	case 0:
		stepAnim = false;
		break;
	case 1:
		stepAnim = (plr._pVar8 & 3) == 2;
		break;
	case 2:
		stepAnim = (plr._pVar8 & 1) == 1;
		break;
	case 3:
		stepAnim = true;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	if (stepAnim) {
		PlrStepAnim(pnum);
	}

	if (plr._pAnimFrame < plr._pSFNum)
		return;

	if (!plr._pVar7) { // SPELL_ACTION_PROGRESS
		plr._pVar7 = TRUE;

		AddMissile(plr._px, plr._py, plr._pVar1, plr._pVar2, plr._pdir,    // SPELL_TARGET_X, SPELL_TARGET_Y
			spelldata[plr._pVar5].sMissile, MST_PLAYER, pnum, plr._pVar6); // SPELL_NUM, SPELL_LEVEL
	}
	assert(PlrAnimFrameLens[PGX_FIRE] == 1 && PlrAnimFrameLens[PGX_LIGHTNING] == 1 && PlrAnimFrameLens[PGX_MAGIC] == 1);
	// assert(plr._pAnims[PGX_FIRE].paFrames == plr._pAnimLen || plr._pAnims[PGX_LIGHTNING].paFrames == plr._pAnimLen || plr._pAnims[PGX_MAGIC].paFrames == plr._pAnimLen);
	if (plr._pAnimFrame < plr._pAnimLen) {
		if (plr._pAnimFrame > plr._pSFNum
		 && (plr._pDestAction == ACTION_SPELL
		  || plr._pDestAction == ACTION_SPELLMON
		  || plr._pDestAction == ACTION_SPELLPLR)) {
			// assert(plr._pmode == PM_SPELL);
			plr._pVar1 = PM_SPELL; // STAND_PREV_MODE
			plr._pmode = PM_STAND;
		}
	} else {
		//PlrStartStand(pnum);
		StartStand(pnum);
		//ClearPlrPVars(pnum);
	}
}

static void PlrDoGotHit(int pnum)
{
	bool stepAnim;

	plr._pVar8++; // GOTHIT_TICK
	switch (plr._pIRecoverySpeed) {
	case 0:
		stepAnim = false;
		break;
	case 1:
		stepAnim = (plr._pVar8 & 3) == 2;
		break;
	case 2:
		stepAnim = (plr._pVar8 & 1) == 1;
		break;
	case 3:
		stepAnim = true;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	if (stepAnim) {
		PlrStepAnim(pnum);
	}

	assert(PlrAnimFrameLens[PGX_GOTHIT] == 1);
	// assert(plr._pAnims[PGX_GOTHIT].paFrames == plr._pAnimLen);
	if (plr._pAnimFrame < plr._pAnimLen)
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
	// assert(plr._pAnims[PGX_DEATH].paFrames == plr._pAnimLen);
	if (plr._pAnimFrame == plr._pAnimLen) {
		assert(PlrAnimFrameLens[PGX_DEATH] > 1);
		plr._pAnimCnt = 0;
		if (plr._pVar7 > 0 && --plr._pVar7 == 0) { // DEATH_DELAY
			if (pnum == mypnum) {
				gbDeathflag = MDM_DEAD;
				if (!IsMultiGame) {
					// close temporary windows
					PressEscKey();
					// activate the menu
					gamemenu_on();
				}
			}
		}
		//dFlags[plr._px][plr._py] |= BFLAG_DEAD_PLAYER;
	}
}

static int MakePlrPath(int pnum, int xx, int yy, bool endspace)
{
	int md = -1;
	int8_t path[MAX_PATH_LENGTH];

	if (!endspace || PosOkPlayer(pnum, xx, yy))
		md = FindPath(PosOkPlayer, pnum, plr._pfutx, plr._pfuty, xx, yy, path);
	if (md < 0) {
		return md;
	}

	if (md != 0 && !endspace) {
		md--;
	}

	path[md] = DIR_NONE;
	return path[0];
}

static void CheckNewPath(int pnum)
{
	int dir = DIR_NONE;

	if (plr._pmode != PM_STAND) {
		return;
	}
	if (plr._pDestAction == ACTION_NONE) {
		return;
	}
	if (plr._pDestAction == ACTION_WALK) {
		dir = MakePlrPath(pnum, plr._pDestParam1, plr._pDestParam2, true);
	} else if (plr._pDestAction == ACTION_WALKDIR) {
		if (PathWalkable(plr._pfutx, plr._pfuty, dir2pdir[plr._pDestParam1])) // Don't start backtrack around obstacles
			dir = MakePlrPath(pnum, plr._pfutx + offset_x[plr._pDestParam1], plr._pfuty + offset_y[plr._pDestParam1], true);
		else
			dir = -1;
	} else if (plr._pDestAction == ACTION_ATTACKMON || plr._pDestAction == ACTION_TALK) {
		if (!(monsters[plr._pDestParam1]._mFlags & MFLAG_HIDDEN))
			dir = MakePlrPath(pnum, monsters[plr._pDestParam1]._mfutx, monsters[plr._pDestParam1]._mfuty, false);
		else
			dir = -1;
	} else if (plr._pDestAction == ACTION_ATTACKPLR) {
		dir = MakePlrPath(pnum, plx(plr._pDestParam1)._pfutx, plx(plr._pDestParam1)._pfuty, false);
	} else if (plr._pDestAction == ACTION_PICKUPITEM) {
		dir = MakePlrPath(pnum, plr._pDestParam1, plr._pDestParam2, false);
	} else if (plr._pDestAction == ACTION_OPERATE || (plr._pDestAction == ACTION_SPELL && plr._pDestParam3 == SPL_DISARM)) {
		static_assert((int)ODT_NONE == 0, "BitOr optimization of CheckNewPath expects ODT_NONE to be zero.");
		dir = MakePlrPath(pnum, plr._pDestParam1, plr._pDestParam2, !(objects[plr._pDestParam4]._oSolidFlag | objects[plr._pDestParam4]._oDoorFlag));
	}
	static_assert((int)DIR_NONE >= 0, "CheckNewPath uses negative value to define an invalid path.");
	if (dir < 0) {
		if (plr._pVar1 == PM_STAND) { // STAND_PREV_MODE
			// inaccessible after the last action is finished -> skip the action
			plr._pDestAction = ACTION_NONE;
		} else {
			// inaccessible while trying to repeat an action -> restore the mode
			plr._pmode = plr._pVar1; // STAND_PREV_MODE
		}
		return;
	}
	if (dir != DIR_NONE) {
		if (plr._pVar1 == PM_STAND) { // STAND_PREV_MODE
			// walk is necessary after the last action is finished -> start walking
			if (plr._pDestAction == ACTION_WALKDIR) { // || (plr._pDestAction == ACTION_WALK && path[1] == DIR_NONE)) {
				plr._pDestAction = ACTION_NONE;
			}
			StartWalk(pnum, dir);
		} else {
			// walk is necessary while trying to repeat an action -> restore the mode
			plr._pmode = plr._pVar1; // STAND_PREV_MODE
		}
		return;
	}

	switch (plr._pDestAction) {
	case ACTION_WALK:
	case ACTION_WALKDIR:
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
	case ACTION_TURN:
		StartTurn(pnum, plr._pDestParam1);
		break;
	case ACTION_BLOCK:
		StartBlock(pnum, plr._pDestParam1);
		break;
	case ACTION_PICKUPITEM:
		StartPickItem(pnum);
		break;
	case ACTION_TALK:
		StartTalk(pnum);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	AssertFixPlayerLocation(pnum);
	plr._pDestAction = ACTION_NONE;
}

#if DEBUG_MODE || DEV_MODE
static void ValidatePlayer(int pnum)
{
	ItemStruct* pi;
	uint64_t msk;
	int gt, i;

	//if (plr._pLevel > MAXCHARLEVEL)
	//	plr._pLevel = MAXCHARLEVEL;
	assert(plr._pLevel <= MAXCHARLEVEL);
	//if (plr._pExperience > plr._pNextExper)
	//	plr._pExperience = plr._pNextExper;
	assert(plr._pExperience <= plr._pNextExper);

	int numerrors = 0;
	gt = 0;
	pi = plr._pInvList;
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
					EventPlrMsg("InvItem %d bad ph of %d", i, pnum);
					numerrors++;
				} else {
					ItemStruct* is = &plr._pInvList[par];
					if (is->_itype == ITYPE_NONE) {
						EventPlrMsg("InvItem %d null ph of %d", i, pnum);
						numerrors++;
					} else if (is->_itype == ITYPE_PLACEHOLDER) {
						EventPlrMsg("InvItem %d ph ph of %d", i, pnum);
						numerrors++;
					} else {
						int xx = InvItemWidth[is->_iCurs + CURSOR_FIRSTITEM] / INV_SLOT_SIZE_PX;
						int yy = InvItemHeight[is->_iCurs + CURSOR_FIRSTITEM] / INV_SLOT_SIZE_PX;
						int x = i % 10;
						int y = i / 10;
						int px = par % 10;
						int py = par / 10;
						if (x < px || x >= px + xx) {
							EventPlrMsg("InvI %d lx %d:%d par%d:%d s%d:%d of %d", i, x, y, px, py, xx, yy, pnum);
							numerrors++;
						} else if (y > py || y < py - yy) {
							EventPlrMsg("InvI %d ly %d:%d par%d:%d s%d:%d of %d", i, x, y, px, py, xx, yy, pnum);
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
							ItemStruct* is = &plr._pInvList[i + ii - jj * 10];
							if (is->_itype != ITYPE_PLACEHOLDER) {
								EventPlrMsg("InvItem %d no ph %d %d of %d", i, ii, jj, pnum);
								numerrors++;
								ii = xx;
								jj = yy;
							} else if (is->_iPHolder != i) {
								EventPlrMsg("InvItem %d bad ph %d %d vs %d of %d", i, ii, jj, is->_iPHolder, pnum);
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
	//plr._pGold = gt;
	if (plr._pGold != gt) {
		EventPlrMsg("Gold %d vs calcGold %d of %d", plr._pGold, gt, pnum);
	}
	//assert(plr._pGold == gt);

	msk = 0;
	for (i = 1; i < NUM_SPELLS; i++) {
		if (spelldata[i].sBookLvl != SPELL_NA) {
			msk |= SPELL_MASK(i);
			//if (plr._pSkillLvlBase[i] > MAXSPLLEVEL)
			//	plr._pSkillLvlBase[i] = MAXSPLLEVEL;
			assert(plr._pSkillLvlBase[i] <= MAXSPLLEVEL);
		}
	}
	//plr._pMemSkills &= msk;
	assert((plr._pMemSkills & ~msk) == 0);
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

	if ((unsigned)mypnum >= MAX_PLRS) {
		dev_fatal("ProcessPlayers: illegal player %d", mypnum);
	}

#ifndef NOSOUND
	if (gnSfxDelay > 0) {
		gnSfxDelay--;
		if (gnSfxDelay == 0) {
			StartQTextMsg(gnSfxNum);
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
			if (plr._pDestAction != ACTION_NONE)
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
				//if (plr._pTimer[PLTR_INFRAVISION] == 0) {
				//	CalcPlrItemVals(pnum, false); // last parameter should not matter
				//}
			}

			int16_t lastTimer = plr._pTimer[PLTR_RAGE];
			if (lastTimer != 0) {
				plr._pTimer[PLTR_RAGE] = lastTimer + (lastTimer < 0 ? 1 : -1);
				if (plr._pTimer[PLTR_RAGE] == 0) {
					if (lastTimer >= 0) {
						plr._pTimer[PLTR_RAGE] = -RAGE_COOLDOWN_TICK;
						PlaySfxLoc(sgSFXSets[SFXS_PLR_72][plr._pClass], plr._px, plr._py);
					}
					CalcPlrItemVals(pnum, false); // last parameter should not matter
				}
			}

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
			CheckNewPath(pnum);

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

void PlrHinder(int pnum, int spllvl, unsigned tick)
{
	int effect;
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrHinder: illegal player %d", pnum);
	}
	if ((plr._pmode < PM_WALK || plr._pmode > PM_WALK2 /*|| plr._pAnimFrame == plr._pAnimLen*/) && plr._pmode != PM_CHARGE)
		return;
	effect = spllvl * 4 - plr._pLevel;
	effect = effect > 10 ? 2 : (effect > 0 ? 3 : (effect > -10 ? 4 : 0));
	if (effect != 0 && ((unsigned)tick % (unsigned)effect) == 0) {
		if (plr._pmode != PM_CHARGE) {
			plr._pAnimCnt--;
			plr._pVar6 -= plr._pVar4; // WALK_XOFF <- WALK_XVEL
			plr._pVar7 -= plr._pVar5; // WALK_YOFF <- WALK_YVEL
			plr._pVar8--; // WALK_TICK
		} else {
			PlrStartStand(pnum);
		}
	}
}

void MissToPlr(int mi, bool hit)
{
	MissileStruct* mis;
	MonsterStruct* mon;
	int pnum, oldx, oldy, mpnum, dist, minbl, maxbl, dam, hper;
	unsigned hitFlags;
	bool ret;

	if ((unsigned)mi >= MAXMISSILES) {
		dev_fatal("MissToPlr: illegal missile %d", mi);
	}
	mis = &missile[mi];
	pnum = mis->_miSource;
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("MissToPlr: illegal player %d", pnum);
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
	if (!hit || plr._pHitPoints == 0) {
		PlrStartStand(pnum);
		return;
	}
	//if (mis->_miSpllvl < 10)
		PlrHitByAny(pnum, -1, 0, ISPL_FAKE_FORCE_STUN, OPPOSITE(plr._pdir));
	//else
	//	PlaySfxLoc(IS_BHIT, x, y);
	dist = (int)mis->_miRange - 24; // MISRANGE
	// if (dist < 0)
	//	return;
	if (dist > 32)
		dist = 32;
	minbl = plr._pIChMinDam;
	maxbl = plr._pIChMaxDam;
	//if (maxbl != 0) {
		minbl = ((64 + dist) * minbl) >> 5;
		maxbl = ((64 + dist) * maxbl) >> 5;
	//}
	if (maxbl <= 0)
		return;
	//if (minbl < 0)
	//	minbl = 0;

	oldx = mis->_mix;
	oldy = mis->_miy;
	mpnum = dMonster[oldx][oldy];
	if (mpnum != 0) {
		mpnum = CheckMonCol(mpnum);
		if (/*mpnum < 0 ||*/ mpnum < MAX_MINIONS)
			return;
		//PlrHitMonst(pnum, SPL_CHARGE, mis->_miSpllvl, mpnum);
		mon = &monsters[mpnum];

		hper = mis->_miSpllvl * 16 - mon->_mArmorClass;
		if (!CheckHit(hper) && mon->_mmode != MM_STONE)
			return;

		if (!CheckMonsterHit(mpnum, &ret))
			return;

		dam = CalcMonsterDam(mon->_mMagicRes, MISR_BLUNT, minbl, maxbl, false);

		//if (random_(151, 200) < plr._pICritChance)
		//	dam <<= 1;

		//if (pnum == mypnum) {
			mon->_mhitpoints -= dam;
		//}

		if (mon->_mhitpoints < (1 << 6)) {
			MonKill(mpnum, pnum);
		} else {
			hitFlags = (plr._pIFlags & ISPL_HITFLAGS_MASK) | ISPL_STUN;
			//if (hitFlags & ISPL_NOHEALMON)
			//	mon->_mFlags |= MFLAG_NOHEAL;
			MonHitByPlr(mpnum, pnum, dam, hitFlags, plr._pdir);
		}
		return;
	}
	mpnum = dPlayer[oldx][oldy];
	if (mpnum != 0) {
		mpnum = CheckPlrCol(mpnum);
		if (mpnum < 0)
			return;
		//PlrHitPlr(pnum, SPL_CHARGE, mis->_miSpllvl, mpnum);
		if (plx(mpnum)._pTeam == plr._pTeam || plx(mpnum)._pInvincible)
			return;

		hper = mis->_miSpllvl * 16 - plx(mpnum)._pIAC;
		if (!CheckHit(hper))
			return;

		if (PlrCheckBlock(mpnum, 2 * plr._pLevel + 16, OPPOSITE(plr._pdir)))
			return;
		dam = CalcPlrDam(mpnum, MISR_BLUNT, minbl, maxbl);

		//if (random_(151, 200) < plr._pICritChance)
		//	dam <<= 1;
		if (!PlrDecHp(mpnum, dam, DMGTYPE_PLAYER)) {
			hitFlags = (plr._pIFlags & ISPL_HITFLAGS_MASK) | ISPL_STUN;
			PlrHitByAny(mpnum, pnum, dam, hitFlags, plr._pdir);
		}
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
			//if (monsters[mpo - 1]._mhitpoints != 0) {
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
			return mpo == pnum /*|| plx(mpo)._pHitPoints == 0*/;
		}

		return true;
	}

	return false;
}

void SyncPlrAnim(int pnum)
{
	PlayerStruct* p;
	unsigned animIdx;
	PlrAnimStruct* anim;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("SyncPlrAnim: illegal player %d", pnum);
	}
	p = &plr;
	switch (p->_pmode) {
	case PM_STAND:
	case PM_NEWLVL:
		animIdx = PGX_STAND;
		break;
	case PM_WALK:
	case PM_WALK2:
	case PM_CHARGE: // TODO: this should be PGX_MAGIC, but does not really matter...
		animIdx = PGX_WALK;
		break;
	case PM_ATTACK:
	case PM_RATTACK:
		animIdx = PGX_ATTACK;
		break;
	case PM_BLOCK:
		animIdx = PGX_BLOCK;
		break;
	case PM_GOTHIT:
		animIdx = PGX_GOTHIT;
		break;
	case PM_DYING:
	case PM_DEATH:
		animIdx = PGX_DEATH;
		break;
	case PM_SPELL:
		static_assert((int)PGX_LIGHTNING - (int)PGX_FIRE == (int)STYPE_LIGHTNING - (int)STYPE_FIRE, "SyncPlrAnim expects ordered player_graphic_idx and magic_type I.");
		static_assert((int)PGX_MAGIC - (int)PGX_FIRE == (int)STYPE_MAGIC - (int)STYPE_FIRE, "SyncPlrAnim expects ordered player_graphic_idx and magic_type II.");
		animIdx = PGX_FIRE + spelldata[p->_pVar5].sType - STYPE_FIRE; // SPELL_NUM
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	anim = &p->_pAnims[animIdx];
	p->_pAnimFrameLen = PlrAnimFrameLens[animIdx];
	p->_pAnimLen = anim->paFrames;
	p->_pAnimWidth = anim->paAnimWidth;
	p->_pAnimXOffset = (anim->paAnimWidth - TILE_WIDTH) >> 1;
	p->_pAnimData = anim->paAnimData[p->_pdir];
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
		gbRedrawFlags |= REDRAW_RECALC_HP;
}

void PlrSetMana(int pnum, int val)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrSetMana: illegal player %d", pnum);
	}
	// assert(val == 0);
	// if (plr._pIFlags & ISPL_NOMANA)
		val = 0;
	plr._pMana = val;
	plr._pManaBase = val - (plr._pMaxMana - plr._pMaxManaBase);

	if (pnum == mypnum)
		gbRedrawFlags |= REDRAW_RECALC_MANA;
}

void PlrFillHp(int pnum)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrFillHp: illegal player %d", pnum);
	}
	// assert(plr._pHitPoints != 0);
	plr._pHitPoints = plr._pMaxHP;
	plr._pHPBase = plr._pMaxHPBase;
	if (pnum == mypnum)
		gbRedrawFlags |= REDRAW_RECALC_HP;
}

void PlrFillMana(int pnum)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrFillMana: illegal player %d", pnum);
	}
	// assert(plr._pHitPoints != 0);
	if (plr._pIFlags & ISPL_NOMANA)
		return;
	plr._pMana = plr._pMaxMana;
	plr._pManaBase = plr._pMaxManaBase;
	if (pnum == mypnum)
		gbRedrawFlags |= REDRAW_RECALC_MANA;
}

void PlrIncHp(int pnum, int hp)
{
	assert(hp >= 0);
	if (plr._pHitPoints == 0)
		return;
	plr._pHitPoints += hp;
	if (plr._pHitPoints > plr._pMaxHP)
		plr._pHitPoints = plr._pMaxHP;
	plr._pHPBase += hp;
	if (plr._pHPBase > plr._pMaxHPBase)
		plr._pHPBase = plr._pMaxHPBase;
	if (pnum == mypnum)
		gbRedrawFlags |= REDRAW_RECALC_HP;
}

void PlrIncMana(int pnum, int mana)
{
	assert(mana >= 0);
	if (plr._pHitPoints == 0 || plr._pIFlags & ISPL_NOMANA)
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
		gbRedrawFlags |= REDRAW_RECALC_MANA;
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
		gbRedrawFlags |= REDRAW_RECALC_HP;
	return false;
}

void PlrDecMana(int pnum, int mana)
{
	// assert(mana >= 0);
	// assert(mana == 0 || !(plr._pIFlags & ISPL_NOMANA));
	plr._pMana -= mana;
	plr._pManaBase -= mana;
	if (pnum == mypnum) {
		if (plr._pMana <= 0 && plr._pManaShield != 0)
			NetSendCmd(CMD_REMSHIELD);
		gbRedrawFlags |= REDRAW_RECALC_MANA;
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

void DecreasePlrMaxHp(int pnum)
{
	int tmp;
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("DecreasePlrMaxHp: illegal player %d", pnum);
	}

	if (plr._pMaxHPBase > (1 << 6) && plr._pMaxHP > (1 << 6)) {
		tmp = plr._pMaxHP - (1 << 6);
		plr._pMaxHP = tmp;
		if (plr._pHitPoints > tmp) {
			plr._pHitPoints = tmp;
		}
		tmp = plr._pMaxHPBase - (1 << 6);
		plr._pMaxHPBase = tmp;
		if (plr._pHPBase > tmp) {
			plr._pHPBase = tmp;
		}
	}
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
