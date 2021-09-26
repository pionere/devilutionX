/**
 * @file themes.cpp
 *
 * Implementation of the theme room placing algorithms.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

int numthemes;
static bool _gbArmorFlag;
static bool _gbWeaponFlag;
static bool _gbTreasureFlag;
static bool _gbMFountainFlag;
static bool _gbCauldronFlag;
static bool _gbTFountainFlag;
static bool _gbPFountainFlag;
static bool _gbBFountainFlag;
int zharlib;
static int themex;
static int themey;
static int themeVar1;
ThemeStruct themes[MAXTHEMES];

/** Specifies the set of special theme IDs from which one will be selected at random. */
static const int ThemeGood[4] = { THEME_GOATSHRINE, THEME_SHRINE, THEME_SKELROOM, THEME_LIBRARY };
/** Specifies a 5x5 area to fit theme objects. */
static const int trm5x[] = {
	-2, -1, 0, 1, 2,
	-2, -1, 0, 1, 2,
	-2, -1, 0, 1, 2,
	-2, -1, 0, 1, 2,
	-2, -1, 0, 1, 2
};
/** Specifies a 5x5 area to fit theme objects. */
static const int trm5y[] = {
	-2, -2, -2, -2, -2,
	-1, -1, -1, -1, -1,
	0, 0, 0, 0, 0,
	1, 1, 1, 1, 1,
	2, 2, 2, 2, 2
};
/** Specifies a 3x3 area to fit theme objects. */
static const int trm3x[] = {
	-1, 0, 1,
	-1, 0, 1,
	-1, 0, 1
};
/** Specifies a 3x3 area to fit theme objects. */
static const int trm3y[] = {
	-1, -1, -1,
	0, 0, 0,
	1, 1, 1
};

typedef struct PosDir {
	int x;
	int y;
	int dir;
} PosDir;

static bool TFit_Shrine(int tidx)
{
	int xx, yy, i, numMatches;
	PosDir matches[5];
	const BYTE tv = themes[tidx].ttval;

	xx = DBORDERX;
	yy = DBORDERY;
	numMatches = 0;
	while (TRUE) {
		if (dTransVal[xx][yy] == tv && !nSolidTable[dPiece[xx][yy]]) {
			if (nTrapTable[dPiece[xx][yy - 1]]
			 // make sure the place is wide enough
			 && !nSolidTable[dPiece[xx - 1][yy]]
			 && !nSolidTable[dPiece[xx + 1][yy]]
			 // make sure it is in the same room
			 && dTransVal[xx - 1][yy] == tv
			 && dTransVal[xx + 1][yy] == tv
			 // make sure there is no door on the wall
			 && dObject[xx - 1][yy - 1] == 0
			 && dObject[xx + 1][yy - 1] == 0) {
				// assert(dObject[xx][yy] == 0);
				// assert(dObject[xx - 1][yy] == 0);
				// assert(dObject[xx + 1][yy] == 0);
				matches[numMatches] = { xx, yy, 1 };
				if (++numMatches == lengthof(matches))
					break;
			}
			if (nTrapTable[dPiece[xx - 1][yy]]
			 // make sure the place is wide enough
			 && !nSolidTable[dPiece[xx][yy - 1]]
			 && !nSolidTable[dPiece[xx][yy + 1]]
			 // make sure it is in the same room
			 && dTransVal[xx][yy - 1] == tv
			 && dTransVal[xx][yy + 1] == tv
			 // make sure there is no door on the wall
			 && dObject[xx - 1][yy - 1] == 0
			 && dObject[xx - 1][yy + 1] == 0) {
				// assert(dObject[xx][yy] == 0);
				// assert(dObject[xx][yy - 1] == 0);
				// assert(dObject[xx][yy + 1] == 0);
				matches[numMatches] = { xx, yy, 2 };
				if (++numMatches == lengthof(matches))
					break;
			}
		}
		if (++xx == DBORDERX + DSIZEX) {
			xx = DBORDERX;
			if (++yy == DBORDERY + DSIZEY)
				break;
		}
	}
	if (numMatches == 0)
		return false;

	i = random_(0, numMatches);

	themex = matches[i].x;
	themey = matches[i].y;
	themeVar1 = matches[i].dir;
	return true;
}

static bool TFit_Obj5(int tidx)
{
	int xx, yy, i, numMatches;
	POS32 matches[5];
	const BYTE tv = themes[tidx].ttval;

	xx = DBORDERX;
	yy = DBORDERY;
	numMatches = 0;
	while (TRUE) {
		if (dTransVal[xx][yy] == tv && !nSolidTable[dPiece[xx][yy]]) {
			static_assert(lengthof(trm5x) == lengthof(trm5y), "Mismatching trm5 tables.");
			for (i = 0; i < lengthof(trm5x); i++) {
				if (nSolidTable[dPiece[xx + trm5x[i]][yy + trm5y[i]]]) {
					break;
				}
				if (dTransVal[xx + trm5x[i]][yy + trm5y[i]] != tv) {
					break;
				}
			}
			if (i == lengthof(trm5x)) {
				matches[numMatches] = { xx, yy };
				if (++numMatches == lengthof(matches))
					break;
			}
		}

		if (++xx == DBORDERX + DSIZEX) {
			xx = DBORDERX;
			if (++yy == DBORDERY + DSIZEY)
				break;
		}
	}

	if (numMatches == 0)
		return false;

	i = random_(0, numMatches);

	themex = matches[i].x;
	themey = matches[i].y;
	return true;
}

static bool TFit_SkelRoom(int tidx)
{
	int i;

	for (i = 0; i < nummtypes; i++) {
		if (IsSkel(mapMonTypes[i].cmType)) {
			// themeVar1 = i;
			return TFit_Obj5(tidx);
		}
	}

	return false;
}

static bool TFit_GoatShrine(int tidx)
{
	int i;

	for (i = 0; i < nummtypes; i++) {
		if (IsGoat(mapMonTypes[i].cmType)) {
			themeVar1 = i;
			return TFit_Obj5(tidx);
		}
	}

	return false;
}

static bool CheckThemeObj3(int x, int y, int tidx, int rndfrq)
{
	int i, xx, yy;
	const BYTE tv = themes[tidx].ttval;

	static_assert(lengthof(trm3x) == lengthof(trm3y), "Mismatching trm3 tables.");
	for (i = 0; i < lengthof(trm3x); i++) {
		xx = x + trm3x[i];
		yy = y + trm3y[i];
		//if (xx < 0 || yy < 0)
		//	return false;
		if ((nSolidTable[dPiece[xx][yy]] | dObject[xx][yy]) != 0)
			return false;
		if (dTransVal[xx][yy] != tv)
			return false;
		if (rndfrq != -1 && random_(0, rndfrq) == 0)
			return false;
	}

	return true;
}

static bool TFit_Obj3(int tidx)
{
	int xx, yy;
	const BYTE objrnds[4] = { 4, 4, 3, 5 };
	const BYTE objrnd = objrnds[currLvl._dDunType - 1]; // TODO: use dType instead?

	for (yy = DBORDERY + 1; yy < DBORDERY + DSIZEY - 1; yy++) {
		for (xx = DBORDERX + 1; xx < DBORDERX + DSIZEX - 1; xx++) {
			if (CheckThemeObj3(xx, yy, tidx, objrnd)) {
				themex = xx;
				themey = yy;
				return true;
			}
		}
	}

	return false;
}

static bool CheckThemeReqs(int theme)
{
	switch (theme) {
	case THEME_BARREL:
	case THEME_MONSTPIT:
	case THEME_TORTURE:
	case THEME_DECAPITATED:
	case THEME_GOATSHRINE:
	case THEME_BRNCROSS:
		return true;
	case THEME_SHRINE:
	case THEME_SKELROOM:
	case THEME_LIBRARY: // TODO: use dType instead
		return currLvl._dDunType != DTYPE_CAVES && currLvl._dDunType != DTYPE_HELL;
	case THEME_BLOODFOUNTAIN:
		return _gbBFountainFlag;
	case THEME_PURIFYINGFOUNTAIN:
		return _gbPFountainFlag;
	case THEME_ARMORSTAND:
		return currLvl._dDunType != DTYPE_CATHEDRAL; // TODO: use dType instead
	case THEME_CAULDRON: // TODO: use dType instead
		return currLvl._dDunType == DTYPE_HELL && _gbCauldronFlag;
	case THEME_MURKYFOUNTAIN:
		return _gbMFountainFlag;
	case THEME_TEARFOUNTAIN:
		return _gbTFountainFlag;
	case THEME_WEAPONRACK: // TODO: use dType instead
		return currLvl._dDunType != DTYPE_CATHEDRAL;
	case THEME_TREASURE:
		return _gbTreasureFlag;
	default:
		ASSUME_UNREACHABLE
		return true;
	}
}

static bool SpecialThemeFit(int tidx, int theme)
{
	bool rv;

	if (!CheckThemeReqs(theme))
		return false;

	switch (theme) {
	case THEME_BARREL:
	case THEME_MONSTPIT:
		rv = true;
		break;
	case THEME_SHRINE:
	case THEME_LIBRARY:
		rv = TFit_Shrine(tidx);
		break;
	case THEME_SKELROOM:
		rv = TFit_SkelRoom(tidx);
		break;
	case THEME_BLOODFOUNTAIN:
		rv = TFit_Obj5(tidx);
		if (rv) {
			_gbBFountainFlag = false;
		}
		break;
	case THEME_PURIFYINGFOUNTAIN:
		rv = TFit_Obj5(tidx);
		if (rv) {
			_gbPFountainFlag = false;
		}
		break;
	case THEME_MURKYFOUNTAIN:
		rv = TFit_Obj5(tidx);
		if (rv) {
			_gbMFountainFlag = false;
		}
		break;
	case THEME_TEARFOUNTAIN:
		rv = TFit_Obj5(tidx);
		if (rv) {
			_gbTFountainFlag = false;
		}
		break;
	case THEME_CAULDRON:
		rv = TFit_Obj5(tidx);
		if (rv) {
			_gbCauldronFlag = false;
		}
		break;
	case THEME_GOATSHRINE:
		rv = TFit_GoatShrine(tidx);
		break;
	case THEME_TORTURE:
	case THEME_DECAPITATED:
	case THEME_ARMORSTAND:
	case THEME_BRNCROSS:
	case THEME_WEAPONRACK:
		rv = TFit_Obj3(tidx);
		break;
	case THEME_TREASURE:
		rv = true;
		_gbTreasureFlag = false;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	return rv;
}

static bool CheckThemeRoom(BYTE tv)
{
	int i, j, tarea;

	for (i = 0; i < numtrigs; i++) {
		if (dTransVal[trigs[i]._tx][trigs[i]._ty] == tv)
			return false;
	}

	tarea = 0;
	for (j = DBORDERY; j < DBORDERY + DSIZEY; j++) {
		for (i = DBORDERX; i < DBORDERX + DSIZEX; i++) {
			if (dTransVal[i][j] != tv)
				continue;
			if (dFlags[i][j] & BFLAG_POPULATED)
				return false;

			tarea++;
		}
	}

	assert(currLvl._dType == DTYPE_CATHEDRAL);
	if (tarea < 9 || tarea > 100)
		return false;

	for (j = DBORDERY; j < DBORDERY + DSIZEY; j++) {
		for (i = DBORDERX; i < DBORDERX + DSIZEX; i++) {
			if (dTransVal[i][j] != tv || nSolidTable[dPiece[i][j]])
				continue;
			if (dTransVal[i - 1][j] != tv && !nSolidTable[dPiece[i - 1][j]])
				return false;
			if (dTransVal[i + 1][j] != tv && !nSolidTable[dPiece[i + 1][j]])
				return false;
			if (dTransVal[i][j - 1] != tv && !nSolidTable[dPiece[i][j - 1]])
				return false;
			if (dTransVal[i][j + 1] != tv && !nSolidTable[dPiece[i][j + 1]])
				return false;
		}
	}

	return true;
}

void InitThemes()
{
	int i, j;

	numthemes = 0;
	// assert(currLvl._dType != DTYPE_TOWN);
	if (currLvl._dLevelIdx >= DLV_HELL4) // there are no themes in hellfire (and on diablo-level)
		return;

	_gbArmorFlag = true;
	_gbBFountainFlag = true;
	_gbCauldronFlag = true;
	_gbMFountainFlag = true;
	_gbPFountainFlag = true;
	_gbTFountainFlag = true;
	_gbTreasureFlag = true;
	_gbWeaponFlag = true;
	zharlib = -1;

	if (currLvl._dDunType == DTYPE_CATHEDRAL) { // TODO: use dType instead?
		themeCount = 0;
		for (i = 0; i < numtrans && themeCount < MAXTHEMES; i++) {
			if (CheckThemeRoom(i)) {
				themeLoc[themeCount].ttval = i;
				themeCount++;
			}
		}
	}
	for (i = 0; i < themeCount; i++) {
		themes[i].ttype = THEME_NONE;
		themes[i].ttval = themeLoc[i].ttval;
	}
	if (QuestStatus(Q_ZHAR)) {
		for (i = 0; i < themeCount; i++) {
			if (SpecialThemeFit(i, THEME_LIBRARY)) {
				themes[i].ttype = THEME_LIBRARY;
				zharlib = i;
				break;
			}
		}
	}
	for (i = 0; i < themeCount; i++) {
		if (themes[i].ttype == THEME_NONE) {
			j = ThemeGood[random_(0, lengthof(ThemeGood))];
			while (!SpecialThemeFit(i, j))
				j = random_(0, NUM_THEMES);
			themes[i].ttype = j;
		}
	}
	numthemes = themeCount;
}

void HoldThemeRooms()
{
	int i, xx, yy;
	BYTE v;
	// assert(currLvl._dType != DTYPE_TOWN);
	if (currLvl._dLevelIdx >= DLV_HELL4) // there are no themes in hellfire (and on diablo-level)
		return;

	if (currLvl._dDunType == DTYPE_CATHEDRAL) { // TODO: use dType instead?
		for (i = 0; i < numthemes; i++) {
			v = themes[i].ttval;
			for (yy = DBORDERY; yy < DBORDERY + DSIZEY; yy++) {
				for (xx = DBORDERX; xx < DBORDERX + DSIZEX; xx++) {
					if (dTransVal[xx][yy] == v) {
						dFlags[xx][yy] |= BFLAG_POPULATED;
					}
				}
			}
		}
	} else {
		//assert(numthemes == themeCount);
		DRLG_HoldThemeRooms();
	}
}

/*
 * Place a theme object with the specified frequency.
 * @param tidx: the index of the selected theme
 * @param type: the type of the object to place
 * @param rndfrq: the frequency to place the object
 */
static void Place_Obj3(int tidx, int type, int rndfrq)
{
	int xx, yy;
	const BYTE tv = themes[tidx].ttval;

	for (yy = DBORDERY + 1; yy < DBORDERY + DSIZEY - 1; yy++) {
		for (xx = DBORDERX + 1; xx < DBORDERX + DSIZEX - 1; xx++) {
			if (dTransVal[xx][yy] == tv && !nSolidTable[dPiece[xx][yy]]) {
				if (CheckThemeObj3(xx, yy, tidx, -1)) {
					if (random_(0, rndfrq) == 0) {
						AddObject(type, xx, yy);
					}
				}
			}
		}
	}
}
/**
 * PlaceThemeMonsts places theme monsters with the specified frequency.
 *
 * @param tidx theme number (index into themes array).
 * @param rndfrq frequency (1/f likelihood of adding monster).
 */
static void PlaceThemeMonsts(int tidx, int rndfrq)
{
	int xx, yy;
	int scattertypes[MAX_LVLMTYPES];
	int numscattypes, mtype, i;
	BYTE tv;

	numscattypes = 0;
	for (i = 0; i < nummtypes; i++) {
		if (mapMonTypes[i].cmPlaceScatter) {
			scattertypes[numscattypes] = i;
			numscattypes++;
		}
	}
	mtype = scattertypes[random_(0, numscattypes)];
	tv = themes[tidx].ttval;
	for (yy = DBORDERY; yy < DBORDERY + DSIZEY; yy++) {
		for (xx = DBORDERX; xx < DBORDERX + DSIZEX; xx++) {
			if (dTransVal[xx][yy] == tv && (nSolidTable[dPiece[xx][yy]] | dItem[xx][yy] | dObject[xx][yy]) == 0) {
				if (random_(0, rndfrq) == 0) {
					AddMonster(xx, yy, random_(0, 8), mtype, true);
				}
			}
		}
	}
}

/**
 * Theme_Barrel initializes the barrel theme.
 *
 * @param tidx theme number (index into themes array).
 */
static void Theme_Barrel(int tidx)
{
	int r, xx, yy;
	const BYTE barrnds[4] = { 2, 6, 4, 8 };
	const BYTE monstrnds[4] = { 5, 7, 3, 9 };
	const BYTE barrnd = barrnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	const BYTE monstrnd = monstrnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	const BYTE tv = themes[tidx].ttval;

	for (yy = DBORDERY; yy < DBORDERY + DSIZEY; yy++) {
		for (xx = DBORDERX; xx < DBORDERX + DSIZEX; xx++) {
			if (dTransVal[xx][yy] == tv && !nSolidTable[dPiece[xx][yy]]) {
				if (random_(0, barrnd) == 0) {
					r = random_(0, barrnd) == 0 ? OBJ_BARREL : OBJ_BARRELEX;
					AddObject(r, xx, yy);
				}
			}
		}
	}
	PlaceThemeMonsts(tidx, monstrnd);
}

/**
 * Theme_Shrine initializes the shrine theme.
 *
 * @param tidx theme number (index into themes array).
 */
static void Theme_Shrine(int tidx)
{
	const BYTE monstrnds[4] = { 6, 6, 3, 9 };

	if (TFit_Shrine(tidx)) {
		if (themeVar1 == 1) {
			AddObject(OBJ_CANDLE2, themex - 1, themey);
			AddObject(OBJ_SHRINER, themex, themey);
			AddObject(OBJ_CANDLE2, themex + 1, themey);
		} else {
			AddObject(OBJ_CANDLE2, themex, themey - 1);
			AddObject(OBJ_SHRINEL, themex, themey);
			AddObject(OBJ_CANDLE2, themex, themey + 1);
		}
	}
	PlaceThemeMonsts(tidx, monstrnds[currLvl._dDunType - 1]); // TODO: use dType instead?
}

/**
 * Theme_MonstPit initializes the monster pit theme.
 *
 * @param tidx theme number (index into themes array).
 */
static void Theme_MonstPit(int tidx)
{
	int r, xx, yy;
	const BYTE monstrnds[4] = { 6, 7, 3, 9 };
	const BYTE tv = themes[tidx].ttval;

	r = RandRange(1, 100);
	xx = DBORDERX;
	yy = DBORDERY;
	while (TRUE) {
		if (dTransVal[xx][yy] == tv && !nSolidTable[dPiece[xx][yy]]) {
			--r;
			if (r <= 0)
				break;
		}
		xx++;
		if (xx == DBORDERX + DSIZEX) {
			xx = DBORDERX;
			yy++;
			if (yy == DBORDERY + DSIZEY) {
				yy = DBORDERY;
			}
		}
	}
	CreateRndItem(xx, yy, true, false, true);
	ItemNoFlippy();
	PlaceThemeMonsts(tidx, monstrnds[currLvl._dDunType - 1]); // TODO: use dType instead?
}

/**
 * Theme_SkelRoom initializes the skeleton room theme.
 *
 * @param tidx theme number (index into themes array).
 */
static void Theme_SkelRoom(int tidx)
{
	int xx, yy, i;
	const BYTE monstrnds[4] = { 6, 7, 3, 9 };
	char monstrnd;

	if (!TFit_SkelRoom(tidx))
		return;

	xx = themex;
	yy = themey;

	AddObject(OBJ_SKFIRE, xx, yy);

	monstrnd = monstrnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	if (random_(0, monstrnd) != 0) {
		i = PreSpawnSkeleton();
		SpawnSkeleton(i, xx - 1, yy - 1, DIR_NONE);
	} else {
		AddObject(OBJ_BANNERL, xx - 1, yy - 1);
	}

	i = PreSpawnSkeleton();
	SpawnSkeleton(i, xx, yy - 1, DIR_NONE);

	if (random_(0, monstrnd) != 0) {
		i = PreSpawnSkeleton();
		SpawnSkeleton(i, xx + 1, yy - 1, DIR_NONE);
	} else {
		AddObject(OBJ_BANNERR, xx + 1, yy - 1);
	}
	if (random_(0, monstrnd) != 0) {
		i = PreSpawnSkeleton();
		SpawnSkeleton(i, xx - 1, yy, DIR_NONE);
	} else {
		AddObject(OBJ_BANNERM, xx - 1, yy);
	}
	if (random_(0, monstrnd) != 0) {
		i = PreSpawnSkeleton();
		SpawnSkeleton(i, xx + 1, yy, DIR_NONE);
	} else {
		AddObject(OBJ_BANNERM, xx + 1, yy);
	}
	if (random_(0, monstrnd) != 0) {
		i = PreSpawnSkeleton();
		SpawnSkeleton(i, xx - 1, yy + 1, DIR_NONE);
	} else {
		AddObject(OBJ_BANNERR, xx - 1, yy + 1);
	}

	i = PreSpawnSkeleton();
	SpawnSkeleton(i, xx, yy + 1, DIR_NONE);

	if (random_(0, monstrnd) != 0) {
		i = PreSpawnSkeleton();
		SpawnSkeleton(i, xx + 1, yy + 1, DIR_NONE);
	} else {
		AddObject(OBJ_BANNERL, xx + 1, yy + 1);
	}

	if (dObject[xx][yy - 3] == 0) {
		assert(dObject[xx][yy - 2] == 0);
		AddObject(OBJ_SKELBOOK, xx, yy - 2);
	}
	if (dObject[xx][yy + 3] == 0) {
		assert(dObject[xx][yy + 2] == 0);
		AddObject(OBJ_SKELBOOK, xx, yy + 2);
	}
}

/**
 * Theme_Treasure initializes the treasure theme.
 *
 * @param tidx theme number (index into themes array).
 */
static void Theme_Treasure(int tidx)
{
	int xx, yy;
	const BYTE treasrnds[4] = { 4, 9, 7, 10 };
	const BYTE monstrnds[4] = { 6, 8, 3, 7 };
	const BYTE treasrnd = treasrnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	const BYTE monstrnd = monstrnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	const BYTE tv = themes[tidx].ttval;

	for (yy = DBORDERY; yy < DBORDERY + DSIZEY; yy++) {
		for (xx = DBORDERX; xx < DBORDERX + DSIZEX; xx++) {
			if (dTransVal[xx][yy] == tv && !nSolidTable[dPiece[xx][yy]]) {
				if (random_(0, treasrnd) == 0) {
					CreateTypeItem(xx, yy, false, ITYPE_GOLD, IMISC_NONE, false, true);
					ItemNoFlippy();
				} else if (random_(0, treasrnd) == 0) {
					CreateRndItem(xx, yy, false, false, true);
					ItemNoFlippy();
				}
			}
		}
	}
	PlaceThemeMonsts(tidx, monstrnd);
}

/**
 * Theme_Library initializes the library theme.
 *
 * @param tidx theme number (index into themes array).
 */
static void Theme_Library(int tidx)
{
	int xx, yy, oi;
	const BYTE librnds[4] = { 1, 2, 2, 5 };
	const BYTE monstrnds[4] = { 5, 7, 3, 9 };
	char librnd, monstrnd;

	if (TFit_Shrine(tidx)) {
		if (themeVar1 == 1) {
			AddObject(OBJ_BOOKCANDLE, themex - 1, themey);
			AddObject(OBJ_BOOKCASER, themex, themey);
			AddObject(OBJ_BOOKCANDLE, themex + 1, themey);
		} else {
			AddObject(OBJ_BOOKCANDLE, themex, themey - 1);
			AddObject(OBJ_BOOKCASEL, themex, themey);
			AddObject(OBJ_BOOKCANDLE, themex, themey + 1);
		}
	}

	librnd = librnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	monstrnd = monstrnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	for (yy = DBORDERY + 1; yy < DBORDERY + DSIZEY - 1; yy++) {
		for (xx = DBORDERX + 1; xx < DBORDERX + DSIZEX - 1; xx++) {
			if (CheckThemeObj3(xx, yy, tidx, -1) && dMonster[xx][yy] == 0 && random_(0, librnd) == 0) {
				oi = AddObject(OBJ_BOOKSTAND, xx, yy);
				if (random_(0, 2 * librnd) != 0 && oi != -1) { /// BUGFIX: check AddObject succeeded (fixed)
					objects[oi]._oSelFlag = 0;
					objects[oi]._oAnimFrame += 2;
				}
			}
		}
	}

	if (QuestStatus(Q_ZHAR) && tidx == zharlib)
		return;

	PlaceThemeMonsts(tidx, monstrnd);
}

/**
 * Theme_Torture initializes the torture theme.
 *
 * @param tidx theme number (index into themes array).
 */
static void Theme_Torture(int tidx)
{
	const BYTE tortrnds[4] = { 6 * 2, 8 * 2, 3 * 2, 8 * 2};
	const BYTE monstrnds[4] = { 6, 8, 3, 9 };
	const BYTE tortrnd = tortrnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	const BYTE monstrnd = monstrnds[currLvl._dDunType - 1]; // TODO: use dType instead?

	Place_Obj3(tidx, OBJ_TNUDEM, tortrnd);
	Place_Obj3(tidx, OBJ_TNUDEW, tortrnd);
	PlaceThemeMonsts(tidx, monstrnd);
}

/**
 * Theme_BloodFountain initializes the blood fountain theme.
 * @param tidx Theme number (index into themes array).
 */
static void Theme_BloodFountain(int tidx)
{
	const BYTE monstrnds[4] = { 6, 8, 3, 9 };

	if (TFit_Obj5(tidx))
		AddObject(OBJ_BLOODFTN, themex, themey);
	PlaceThemeMonsts(tidx, monstrnds[currLvl._dDunType - 1]); // TODO: use dType instead?
}

/**
 * Theme_Decap initializes the decapitated theme.
 *
 * @param tidx theme number (index into themes array).
 */
static void Theme_Decap(int tidx)
{
	const BYTE decaprnds[4] = { 6, 8, 3, 8 };
	const BYTE monstrnds[4] = { 6, 8, 3, 9 };
	const BYTE decaprnd = decaprnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	const BYTE monstrnd = monstrnds[currLvl._dDunType - 1]; // TODO: use dType instead?

	Place_Obj3(tidx, OBJ_DECAP, decaprnd);
	PlaceThemeMonsts(tidx, monstrnd);
}

/**
 * Theme_PurifyingFountain initializes the purifying fountain theme.
 *
 * @param tidx theme number (index into themes array).
 */
static void Theme_PurifyingFountain(int tidx)
{
	const BYTE monstrnds[4] = { 6, 7, 3, 9 };

	if (TFit_Obj5(tidx))
		AddObject(OBJ_PURIFYINGFTN, themex, themey);
	PlaceThemeMonsts(tidx, monstrnds[currLvl._dDunType - 1]); // TODO: use dType instead?
}

/**
 * Theme_ArmorStand initializes the armor stand theme.
 *
 * @param tidx theme number (index into themes array).
 */
static void Theme_ArmorStand(int tidx)
{
	const BYTE armorrnds[4] = { 6, 8, 3, 8 };
	const BYTE monstrnds[4] = { 6, 7, 3, 9 };
	const BYTE armorrnd = armorrnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	const BYTE monstrnd = monstrnds[currLvl._dDunType - 1]; // TODO: use dType instead?

	if (_gbArmorFlag) {
		if (TFit_Obj3(tidx))
			AddObject(OBJ_ARMORSTAND, themex, themey);
	}
	Place_Obj3(tidx, OBJ_ARMORSTANDN, armorrnd);
	PlaceThemeMonsts(tidx, monstrnd);
	_gbArmorFlag = false;
}

/**
 * Theme_GoatShrine initializes the goat shrine theme.
 *
 * @param tidx theme number (index into themes array).
 */
static void Theme_GoatShrine(int tidx)
{
	int xx, yy;
	BYTE tv;

	if (!TFit_GoatShrine(tidx))
		return;
	AddObject(OBJ_GOATSHRINE, themex, themey);
	tv = themes[tidx].ttval;
	for (yy = themey - 1; yy <= themey + 1; yy++) {
		for (xx = themex - 1; xx <= themex + 1; xx++) {
			if (dTransVal[xx][yy] == tv && !nSolidTable[dPiece[xx][yy]] && (xx != themex || yy != themey)) {
				AddMonster(xx, yy, DIR_SW, themeVar1, true);
			}
		}
	}
}

/**
 * Theme_Cauldron initializes the cauldron theme.
 *
 * @param tidx theme number (index into themes array).
 */
static void Theme_Cauldron(int tidx)
{
	const BYTE monstrnds[4] = { 6, 7, 3, 9 };

	if (TFit_Obj5(tidx))
		AddObject(OBJ_CAULDRON, themex, themey);
	PlaceThemeMonsts(tidx, monstrnds[currLvl._dDunType - 1]); // TODO: use dType instead?
}

/**
 * Theme_MurkyFountain initializes the murky fountain theme.
 *
 * @param tidx theme number (index into themes array).
 */
static void Theme_MurkyFountain(int tidx)
{
	const BYTE monstrnds[4] = { 6, 7, 3, 9 };

	if (TFit_Obj5(tidx))
		AddObject(OBJ_MURKYFTN, themex, themey);
	PlaceThemeMonsts(tidx, monstrnds[currLvl._dDunType - 1]); // TODO: use dType instead?
}

/**
 * Theme_TearFountain initializes the tear fountain theme.
 *
 * @param tidx theme number (index into themes array).
 */
static void Theme_TearFountain(int tidx)
{
	const BYTE monstrnds[4] = { 6, 7, 3, 9 };

	if (TFit_Obj5(tidx))
		AddObject(OBJ_TEARFTN, themex, themey);
	PlaceThemeMonsts(tidx, monstrnds[currLvl._dDunType - 1]); // TODO: use dType instead?
}

/**
 * Theme_BrnCross initializes the burning cross theme.
 *
 * @param tidx theme number (index into themes array).
 */
static void Theme_BrnCross(int tidx)
{
	const BYTE monstrnds[4] = { 6, 8, 3, 9 };
	const BYTE bcrossrnds[4] = { 5, 7, 3, 8 };
	const BYTE bcrossrnd = bcrossrnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	const BYTE monstrnd = monstrnds[currLvl._dDunType - 1]; // TODO: use dType instead?

	Place_Obj3(tidx, OBJ_TBCROSS, bcrossrnd);
	PlaceThemeMonsts(tidx, monstrnd);
}

/**
 * Theme_WeaponRack initializes the weapon rack theme.
 *
 * @param tidx theme number (index into themes array).
 */
static void Theme_WeaponRack(int tidx)
{
	int type;
	const BYTE weaponrnds[4] = { 6, 8, 5, 8 };
	const BYTE monstrnds[4] = { 6, 7, 3, 9 };
	const BYTE weaponrnd = weaponrnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	const BYTE monstrnd = monstrnds[currLvl._dDunType - 1]; // TODO: use dType instead?

	static_assert(OBJ_WEAPONRACKL + 2 == OBJ_WEAPONRACKR, "Theme_WeaponRack depends on the order of WEAPONRACKL/R");
	type = OBJ_WEAPONRACKL + 2 * random_(0, 2);
	if (_gbWeaponFlag) {
		if (TFit_Obj3(tidx))
			AddObject(type, themex, themey);
	}
	static_assert(OBJ_WEAPONRACKL + 1 == OBJ_WEAPONRACKLN, "Theme_WeaponRack depends on the order of WEAPONRACKL(N)");
	static_assert(OBJ_WEAPONRACKR + 1 == OBJ_WEAPONRACKRN, "Theme_WeaponRack depends on the order of WEAPONRACKR(N)");
	type += 1;
	Place_Obj3(tidx, type, weaponrnd);
	PlaceThemeMonsts(tidx, monstrnd);
	_gbWeaponFlag = false;
}

/**
 * UpdateL4Trans sets each value of the transparency map to 1.
 */
/*static void UpdateL4Trans()
{
	int i;
	BYTE *pTmp;

	static_assert(sizeof(dTransVal) == MAXDUNX * MAXDUNY, "Linear traverse of dTransVal does not work in UpdateL4Trans.");
	pTmp = &dTransVal[0][0];
	for (i = 0; i < MAXDUNX * MAXDUNY; i++, pTmp++)
		if (*pTmp != 0)
			*pTmp = 1;
}*/

void CreateThemeRooms()
{
	int i;
	// assert(currLvl._dType != DTYPE_TOWN);
	if (currLvl._dLevelIdx >= DLV_HELL4) // there are no themes in hellfire (and on diablo-level)
		return;

	//gbInitObjFlag = true;
	for (i = 0; i < numthemes; i++) {
		switch (themes[i].ttype) {
		case THEME_BARREL:
			Theme_Barrel(i);
			break;
		case THEME_SHRINE:
			Theme_Shrine(i);
			break;
		case THEME_MONSTPIT:
			Theme_MonstPit(i);
			break;
		case THEME_SKELROOM:
			Theme_SkelRoom(i);
			break;
		case THEME_TREASURE:
			Theme_Treasure(i);
			break;
		case THEME_LIBRARY:
			Theme_Library(i);
			break;
		case THEME_TORTURE:
			Theme_Torture(i);
			break;
		case THEME_BLOODFOUNTAIN:
			Theme_BloodFountain(i);
			break;
		case THEME_DECAPITATED:
			Theme_Decap(i);
			break;
		case THEME_PURIFYINGFOUNTAIN:
			Theme_PurifyingFountain(i);
			break;
		case THEME_ARMORSTAND:
			Theme_ArmorStand(i);
			break;
		case THEME_GOATSHRINE:
			Theme_GoatShrine(i);
			break;
		case THEME_CAULDRON:
			Theme_Cauldron(i);
			break;
		case THEME_MURKYFOUNTAIN:
			Theme_MurkyFountain(i);
			break;
		case THEME_TEARFOUNTAIN:
			Theme_TearFountain(i);
			break;
		case THEME_BRNCROSS:
			Theme_BrnCross(i);
			break;
		case THEME_WEAPONRACK:
			Theme_WeaponRack(i);
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
	}
	//gbInitObjFlag = false;
	// TODO: why was this necessary in the vanilla code?
	//if (currLvl._dType == DTYPE_HELL && numthemes > 0) {
	//	UpdateL4Trans();
	//}
}

DEVILUTION_END_NAMESPACE
