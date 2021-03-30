/**
 * @file themes.cpp
 *
 * Implementation of the theme room placing algorithms.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

int numthemes;
bool _gbArmorFlag;
bool _gbWeaponFlag;
bool _gbTreasureFlag;
bool _gbMFountainFlag;
bool _gbCauldronFlag;
bool _gbTFountainFlag;
bool _gbPFountainFlag;
bool _gbBFountainFlag;
bool _gbBCrossFlag;
int zharlib;
int themex;
int themey;
int themeVar1;
ThemeStruct themes[MAXTHEMES];

/** Specifies the set of special theme IDs from which one will be selected at random. */
int ThemeGood[4] = { THEME_GOATSHRINE, THEME_SHRINE, THEME_SKELROOM, THEME_LIBRARY };
/** Specifies a 5x5 area to fit theme objects. */
int trm5x[] = {
	-2, -1, 0, 1, 2,
	-2, -1, 0, 1, 2,
	-2, -1, 0, 1, 2,
	-2, -1, 0, 1, 2,
	-2, -1, 0, 1, 2
};
/** Specifies a 5x5 area to fit theme objects. */
int trm5y[] = {
	-2, -2, -2, -2, -2,
	-1, -1, -1, -1, -1,
	0, 0, 0, 0, 0,
	1, 1, 1, 1, 1,
	2, 2, 2, 2, 2
};
/** Specifies a 3x3 area to fit theme objects. */
int trm3x[] = {
	-1, 0, 1,
	-1, 0, 1,
	-1, 0, 1
};
/** Specifies a 3x3 area to fit theme objects. */
int trm3y[] = {
	-1, -1, -1,
	0, 0, 0,
	1, 1, 1
};

static bool TFit_Shrine(int tidx)
{
	int xx, yy, found;
	const BYTE tv = themes[tidx].ttval;

	xx = DBORDERX;
	yy = DBORDERY;
	while (TRUE) {
		if (dTransVal[xx][yy] == tv) {
			if (nTrapTable[dPiece[xx][yy - 1]]
			 && !nSolidTable[dPiece[xx - 1][yy]]
			 && !nSolidTable[dPiece[xx + 1][yy]]
			 && dTransVal[xx - 1][yy] == tv
			 && dTransVal[xx + 1][yy] == tv
			 && dObject[xx - 1][yy - 1] == 0
			 && dObject[xx + 1][yy - 1] == 0) {
				found = 1;
				break;
			}
			if (nTrapTable[dPiece[xx - 1][yy]]
			 && !nSolidTable[dPiece[xx][yy - 1]]
			 && !nSolidTable[dPiece[xx][yy + 1]]
			 && dTransVal[xx][yy - 1] == tv
			 && dTransVal[xx][yy + 1] == tv
			 && dObject[xx - 1][yy - 1] == 0
			 && dObject[xx - 1][yy + 1] == 0) {
				found = 2;
				break;
			}
		}
		xx++;
		if (xx == DBORDERX + DSIZEX) {
			xx = DBORDERX;
			yy++;
			if (yy == DBORDERY + DSIZEY)
				return false;
		}
	}
	themex = xx;
	themey = yy;
	themeVar1 = found;
	return true;
}

static bool TFit_Obj5(int tidx)
{
	int xx, yy;
	int i, r, rs;
	bool found;
	const BYTE tv = themes[tidx].ttval;

	xx = DBORDERX;
	yy = DBORDERY;
	r = RandRange(1, 5);
	rs = r;
	while (r > 0) {
		found = false;
		if (dTransVal[xx][yy] == tv && !nSolidTable[dPiece[xx][yy]]) {
			found = true;
			static_assert(lengthof(trm5x) == lengthof(trm5y), "Mismatching trm5 tables.");
			for (i = 0; found && i < lengthof(trm5x); i++) {
				if (nSolidTable[dPiece[xx + trm5x[i]][yy + trm5y[i]]]) {
					found = false;
				}
				if (dTransVal[xx + trm5x[i]][yy + trm5y[i]] != tv) {
					found = false;
				}
			}
		}

		if (!found) {
			xx++;
			if (xx == DBORDERX + DSIZEX) {
				xx = DBORDERX;
				yy++;
				if (yy == DBORDERY + DSIZEY) {
					if (r == rs) {
						return false;
					}
					yy = DBORDERY;
				}
			}
			continue;
		}

		r--;
	}

	themex = xx;
	themey = yy;

	return true;
}

static bool TFit_SkelRoom(int tidx)
{
	int i;

	for (i = 0; i < nummtypes; i++) {
		if (IsSkel(Monsters[i].mtype)) {
			themeVar1 = i;
			return TFit_Obj5(tidx);
		}
	}

	return false;
}

static bool TFit_GoatShrine(int tidx)
{
	int i;

	for (i = 0; i < nummtypes; i++) {
		if (IsGoat(Monsters[i].mtype)) {
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
	const char objrnds[4] = { 4, 4, 3, 5 };
	const char objrnd = objrnds[currLvl._dDunType - 1]; // TODO: use dType instead?

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

static bool CheckThemeRoom(int tv)
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
	_gbBCrossFlag = false;
	_gbWeaponFlag = true;
	zharlib = -1;

	if (currLvl._dDunType == DTYPE_CATHEDRAL) { // TODO: use dType instead?
		for (i = 0; i < 256 && numthemes < MAXTHEMES; i++) {
			if (CheckThemeRoom(i)) {
				themes[numthemes].ttval = i;
				j = ThemeGood[random_(0, 4)];
				while (!SpecialThemeFit(numthemes, j))
					j = random_(0, NUM_THEMES);
				themes[numthemes].ttype = j;
				numthemes++;
			}
		}
	} else {
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
			if (zharlib == -1) {
				quests[Q_ZHAR]._qactive = QUEST_NOTAVAIL;
				// TODO: RemoveMonsterType(UniqMonst[UMT_ZHAR].mtype); ?
			}
		}
		for (i = 0; i < themeCount; i++) {
			if (themes[i].ttype == THEME_NONE) {
				j = ThemeGood[random_(0, 4)];
				while (!SpecialThemeFit(i, j))
					j = random_(0, NUM_THEMES);
				themes[i].ttype = j;
			}
		}
		numthemes += themeCount;
	}
}

/**
 * @brief HoldThemeRooms marks theme rooms as populated.
 */
void HoldThemeRooms()
{
	int i, xx, yy;
	BYTE v;

	if (currLvl._dLevelIdx == DLV_HELL4)
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
		DRLG_HoldThemeRooms();
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
		if (Monsters[i].mPlaceScatter) {
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
					if (random_(0, barrnd) == 0) {
						r = OBJ_BARREL;
					} else {
						r = OBJ_BARRELEX;
					}
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
	const char monstrnds[4] = { 6, 6, 3, 9 };

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
	const char monstrnds[4] = { 6, 7, 3, 9 };
	char monstrnd;

	if (!TFit_SkelRoom(tidx))
		return;

	xx = themex;
	yy = themey;

	AddObject(OBJ_SKFIRE, xx, yy);

	monstrnd = monstrnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	if (random_(0, monstrnd) != 0) {
		i = PreSpawnSkeleton();
		SpawnSkeleton(i, xx - 1, yy - 1);
	} else {
		AddObject(OBJ_BANNERL, xx - 1, yy - 1);
	}

	i = PreSpawnSkeleton();
	SpawnSkeleton(i, xx, yy - 1);

	if (random_(0, monstrnd) != 0) {
		i = PreSpawnSkeleton();
		SpawnSkeleton(i, xx + 1, yy - 1);
	} else {
		AddObject(OBJ_BANNERR, xx + 1, yy - 1);
	}
	if (random_(0, monstrnd) != 0) {
		i = PreSpawnSkeleton();
		SpawnSkeleton(i, xx - 1, yy);
	} else {
		AddObject(OBJ_BANNERM, xx - 1, yy);
	}
	if (random_(0, monstrnd) != 0) {
		i = PreSpawnSkeleton();
		SpawnSkeleton(i, xx + 1, yy);
	} else {
		AddObject(OBJ_BANNERM, xx + 1, yy);
	}
	if (random_(0, monstrnd) != 0) {
		i = PreSpawnSkeleton();
		SpawnSkeleton(i, xx - 1, yy + 1);
	} else {
		AddObject(OBJ_BANNERR, xx - 1, yy + 1);
	}

	i = PreSpawnSkeleton();
	SpawnSkeleton(i, xx, yy + 1);

	if (random_(0, monstrnd) != 0) {
		i = PreSpawnSkeleton();
		SpawnSkeleton(i, xx + 1, yy + 1);
	} else {
		AddObject(OBJ_BANNERL, xx + 1, yy + 1);
	}

	if (dObject[xx][yy - 3] == 0) {
		AddObject(OBJ_SKELBOOK, xx, yy - 2);
	}
	if (dObject[xx][yy + 3] == 0) {
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
				}
				// BUGFIX: should probably be `random_(0, 2*treasrnd...) == 0`
				if (random_(0, treasrnd) == 0) {
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
	const char librnds[4] = { 1, 2, 2, 5 };
	const char monstrnds[4] = { 5, 7, 3, 9 };
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
					object[oi]._oSelFlag = 0;
					object[oi]._oAnimFrame += 2;
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
	int xx, yy;
	const BYTE tortrnds[4] = { 6, 8, 3, 8 };
	const BYTE monstrnds[4] = { 6, 8, 3, 9 };
	const BYTE tortrnd = tortrnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	const BYTE monstrnd = monstrnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	const BYTE tv = themes[tidx].ttval;

	for (yy = DBORDERY + 1; yy < DBORDERY + DSIZEY - 1; yy++) {
		for (xx = DBORDERX + 1; xx < DBORDERX + DSIZEX - 1; xx++) {
			if (dTransVal[xx][yy] == tv && !nSolidTable[dPiece[xx][yy]]) {
				if (CheckThemeObj3(xx, yy, tidx, -1)) {
					if (random_(0, tortrnd) == 0) {
						AddObject(OBJ_TNUDEM2, xx, yy);
					}
				}
			}
		}
	}
	PlaceThemeMonsts(tidx, monstrnd);
}

/**
 * Theme_BloodFountain initializes the blood fountain theme.
 * @param tidx Theme number (index into themes array).
 */
static void Theme_BloodFountain(int tidx)
{
	const char monstrnds[4] = { 6, 8, 3, 9 };

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
	int xx, yy;
	const BYTE decaprnds[4] = { 6, 8, 3, 8 };
	const BYTE monstrnds[4] = { 6, 8, 3, 9 };
	const BYTE decaprnd = decaprnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	const BYTE monstrnd = monstrnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	const BYTE tv = themes[tidx].ttval;

	for (yy = DBORDERY + 1; yy < DBORDERY + DSIZEY - 1; yy++) {
		for (xx = DBORDERX + 1; xx < DBORDERX + DSIZEX - 1; xx++) {
			if (dTransVal[xx][yy] == tv && !nSolidTable[dPiece[xx][yy]]) {
				if (CheckThemeObj3(xx, yy, tidx, -1)) {
					if (random_(0, decaprnd) == 0) {
						AddObject(OBJ_DECAP, xx, yy);
					}
				}
			}
		}
	}
	PlaceThemeMonsts(tidx, monstrnd);
}

/**
 * Theme_PurifyingFountain initializes the purifying fountain theme.
 *
 * @param tidx theme number (index into themes array).
 */
static void Theme_PurifyingFountain(int tidx)
{
	const char monstrnds[4] = { 6, 7, 3, 9 };

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
	int xx, yy;
	const BYTE armorrnds[4] = { 6, 8, 3, 8 };
	const BYTE monstrnds[4] = { 6, 7, 3, 9 };
	const BYTE armorrnd = armorrnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	const BYTE monstrnd = monstrnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	BYTE tv;

	if (_gbArmorFlag) {
		if (TFit_Obj3(tidx))
			AddObject(OBJ_ARMORSTAND, themex, themey);
	}
	tv = themes[tidx].ttval;
	for (yy = DBORDERY + 1; yy < DBORDERY + DSIZEY - 1; yy++) {
		for (xx = DBORDERX + 1; xx < DBORDERX + DSIZEX - 1; xx++) {
			if (dTransVal[xx][yy] == tv && !nSolidTable[dPiece[xx][yy]]) {
				if (CheckThemeObj3(xx, yy, tidx, -1)) {
					if (random_(0, armorrnd) == 0) {
						AddObject(OBJ_ARMORSTANDN, xx, yy);
					}
				}
			}
		}
	}
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
	const char monstrnds[4] = { 6, 7, 3, 9 };

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
	const char monstrnds[4] = { 6, 7, 3, 9 };

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
	const char monstrnds[4] = { 6, 7, 3, 9 };

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
	int xx, yy;
	const BYTE monstrnds[4] = { 6, 8, 3, 9 };
	const BYTE bcrossrnds[4] = { 5, 7, 3, 8 };
	const BYTE bcrossrnd = bcrossrnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	const BYTE monstrnd = monstrnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	const BYTE tv = themes[tidx].ttval;

	for (yy = DBORDERY + 1; yy < DBORDERY + DSIZEY - 1; yy++) {
		for (xx = DBORDERX + 1; xx < DBORDERX + DSIZEX - 1; xx++) {
			if (dTransVal[xx][yy] == tv && !nSolidTable[dPiece[xx][yy]]) {
				if (CheckThemeObj3(xx, yy, tidx, -1)) {
					if (random_(0, bcrossrnd) == 0) {
						AddObject(OBJ_TBCROSS, xx, yy);
					}
				}
			}
		}
	}
	PlaceThemeMonsts(tidx, monstrnd);
	_gbBCrossFlag = true;
}

/**
 * Theme_WeaponRack initializes the weapon rack theme.
 *
 * @param tidx theme number (index into themes array).
 */
static void Theme_WeaponRack(int tidx)
{
	int xx, yy, type;
	const BYTE weaponrnds[4] = { 6, 8, 5, 8 };
	const BYTE monstrnds[4] = { 6, 7, 3, 9 };
	const BYTE weaponrnd = weaponrnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	const BYTE monstrnd = monstrnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	const BYTE tv = themes[tidx].ttval;

	static_assert(OBJ_WEAPONRACKL + 2 == OBJ_WEAPONRACKR, "Theme_WeaponRack depends on the order of WEAPONRACKL/R");
	type = OBJ_WEAPONRACKL + 2 * random_(0, 2);
	if (_gbWeaponFlag) {
		if (TFit_Obj3(tidx))
			AddObject(type, themex, themey);
	}
	static_assert(OBJ_WEAPONRACKL + 1 == OBJ_WEAPONRACKLN, "Theme_WeaponRack depends on the order of WEAPONRACKL(N)");
	static_assert(OBJ_WEAPONRACKR + 1 == OBJ_WEAPONRACKRN, "Theme_WeaponRack depends on the order of WEAPONRACKR(N)");
	type += 1;
	for (yy = DBORDERY + 1; yy < DBORDERY + DSIZEY - 1; yy++) {
		for (xx = DBORDERX + 1; xx < DBORDERX + DSIZEX - 1; xx++) {
			if (dTransVal[xx][yy] == tv && !nSolidTable[dPiece[xx][yy]]) {
				if (CheckThemeObj3(xx, yy, tidx, -1)) {
					if (random_(0, weaponrnd) == 0) {
						AddObject(type, xx, yy);
					}
				}
			}
		}
	}
	PlaceThemeMonsts(tidx, monstrnd);
	_gbWeaponFlag = false;
}

/**
 * UpdateL4Trans sets each value of the transparency map to 1.
 */
static void UpdateL4Trans()
{
	int i;
	BYTE *pTmp;

	static_assert(sizeof(dTransVal) == MAXDUNX * MAXDUNY, "Linear traverse of dTransVal does not work in UpdateL4Trans.");
	pTmp = &dTransVal[0][0];
	for (i = 0; i < MAXDUNX * MAXDUNY; i++, pTmp++)
		if (*pTmp != 0)
			*pTmp = 1;
}

/**
 * CreateThemeRooms adds thematic elements to rooms.
 */
void CreateThemeRooms()
{
	int i;

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
	if (currLvl._dType == DTYPE_HELL && themeCount > 0) {
		UpdateL4Trans();
	}
}

DEVILUTION_END_NAMESPACE
