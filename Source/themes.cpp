/**
 * @file themes.cpp
 *
 * Implementation of the theme room placing algorithms.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

int numthemes;
static bool _gbShrineFlag;
static bool _gbSkelRoomFlag;
static bool _gbGoatFlag;
static bool _gbWeaponFlag;
static bool _gbArmorFlag;
static bool _gbTreasureFlag;
static bool _gbCauldronFlag;
static bool _gbMFountainFlag;
static bool _gbTFountainFlag;
static bool _gbPFountainFlag;
static bool _gbBFountainFlag;
int zharlib;
ThemeStruct themes[MAXTHEMES];

/** Specifies the set of special theme IDs from which one will be selected at random. */
static const int ThemeGood[4] = { THEME_GOATSHRINE, THEME_SHRINE, THEME_SKELROOM, THEME_LIBRARY };
/** Specifies a 5x5 area to fit theme objects. */
static const int trm5x[] = {
	0, 0, 0, 0, 0,
	-1, -1, -1, -1, -1,
	-2, -2, -2, -2, -2,
	1, 1, 1, 1, 1,
	2, 2, 2, 2, 2
};
/** Specifies a 5x5 area to fit theme objects. */
static const int trm5y[] = {
	-2, -1, 0, 1, 2,
	-2, -1, 0, 1, 2,
	-2, -1, 0, 1, 2,
	-2, -1, 0, 1, 2,
	-2, -1, 0, 1, 2
};
/** Specifies a 3x3 area to fit theme objects. */
static const int trm3x[] = {
	0, 0, 0,
	-1, -1, -1,
	1, 1, 1
};
/** Specifies a 3x3 area to fit theme objects. */
static const int trm3y[] = {
	-1, 0, 1,
	-1, 0, 1,
	-1, 0, 1
};

static int TFit_Shrine(int themeId)
{
	int xx, yy, numMatches;
	const ThemeStruct &theme = themes[themeId];

	numMatches = 0;
	yy = theme._tsy1;
	for (xx = theme._tsx1 + 1; xx < theme._tsx2 - 1; xx++) {
		// assert(dTransVal[xx][yy] == theme._tsTransVal && dTransVal[xx - 1][yy] == theme._tsTransVal && dTransVal[xx + 1][yy] == theme._tsTransVal);
		if (/*dTransVal[xx][yy] == tv &&*/ !nSolidTable[dPiece[xx][yy]]) {
			if ((nSpecTrapTable[dPiece[xx][yy - 1]] & PST_TRAP_TYPE) == PST_RIGHT
			 // make sure the place is wide enough
			 // - on the inside
			 && !nSolidTable[dPiece[xx - 1][yy]]
			 && !nSolidTable[dPiece[xx + 1][yy]]
			 // - on the wall (to avoid doors)
			 && nSolidTable[dPiece[xx - 1][yy - 1]]
			 && nSolidTable[dPiece[xx + 1][yy - 1]]
			 // make sure it is in the same room
			 /*&& dTransVal[xx - 1][yy] == tv
			 && dTransVal[xx + 1][yy] == tv*/) {
				// assert(dObject[xx][yy] == 0);
				// assert(dObject[xx - 1][yy] == 0);
				// assert(dObject[xx + 1][yy] == 0);
				drlg.thLocs[numMatches].tpdx = xx;
				drlg.thLocs[numMatches].tpdy = yy;
				drlg.thLocs[numMatches].tpdvar1 = 1;
				drlg.thLocs[numMatches].tpdvar2 = 0;
				numMatches++;
				static_assert(lengthof(drlg.thLocs) >= (10 - 2 - (1 + 1)) * (10 - 2 - (1 + 1)), "TFit_Shrine skips limit checks assuming enough thLocs entries I.");
				// if (numMatches == lengthof(drlg.thLocs))
				//	goto done;
			}
		}
	}
	xx = theme._tsx1;
	for (yy = theme._tsy1 + 1; yy < theme._tsy2 - 1; yy++) {
		// assert(dTransVal[xx][yy] == theme._tsTransVal && dTransVal[xx][yy - 1] == theme._tsTransVal && dTransVal[xx][yy + 1] == theme._tsTransVal);
		if (/*dTransVal[xx][yy] == tv &&*/ !nSolidTable[dPiece[xx][yy]]) {
			if ((nSpecTrapTable[dPiece[xx - 1][yy]] & PST_TRAP_TYPE) == PST_LEFT
			 // make sure the place is wide enough
			 // - on the inside
			 && !nSolidTable[dPiece[xx][yy - 1]]
			 && !nSolidTable[dPiece[xx][yy + 1]]
			 // - on the wall (to avoid doors)
			 && nSolidTable[dPiece[xx - 1][yy - 1]]
			 && nSolidTable[dPiece[xx - 1][yy + 1]]
			 // make sure it is in the same room
			 /*&& dTransVal[xx][yy - 1] == tv
			 && dTransVal[xx][yy + 1] == tv*/) {
				// assert(dObject[xx][yy] == 0);
				// assert(dObject[xx][yy - 1] == 0);
				// assert(dObject[xx][yy + 1] == 0);
				drlg.thLocs[numMatches].tpdx = xx;
				drlg.thLocs[numMatches].tpdy = yy;
				drlg.thLocs[numMatches].tpdvar1 = 0;
				drlg.thLocs[numMatches].tpdvar2 = 0;
				numMatches++;
				static_assert(lengthof(drlg.thLocs) >= (10 - 2 - (1 + 1)) * (10 - 2 - (1 + 1)), "TFit_Shrine skips limit checks assuming enough thLocs entries II.");
				// if (numMatches == lengthof(drlg.thLocs))
				//	goto done;
			}
		}
	}
// done:
	if (numMatches == 0)
		return -1;
	static_assert(lengthof(drlg.thLocs) < 0x7FFF, "TFit_Shrine uses random_low to select a matching location.");
	return random_low(0, numMatches);
}

static int TFit_Obj5(int themeId)
{
	int xx, yy, i, numMatches;
	const ThemeStruct &theme = themes[themeId];

	numMatches = 0;
	for (xx = theme._tsx1 + 2; xx < theme._tsx2 - 2; xx++) {
		for (yy = theme._tsy1 + 2; yy < theme._tsy2 - 2; yy++) {
			// assert(dTransVal[xx][yy] == theme._tsTransVal);
			if (/*dTransVal[xx][yy] == tv &&*/ !nSolidTable[dPiece[xx][yy]]) {
				static_assert(lengthof(trm5x) == lengthof(trm5y), "Mismatching trm5 tables.");
				for (i = 0; i < lengthof(trm5x); i++) {
					if (nSolidTable[dPiece[xx + trm5x[i]][yy + trm5y[i]]]) {
						break;
					}
					//assert(dTransVal[xx + trm5x[i]][yy + trm5y[i]] == theme._tsTransVal);
					//if (dTransVal[xx + trm5x[i]][yy + trm5y[i]] != tv) {
					//	break;
					//}
				}
				if (i == lengthof(trm5x)) {
					drlg.thLocs[numMatches].tpdx = xx;
					drlg.thLocs[numMatches].tpdy = yy;
					drlg.thLocs[numMatches].tpdvar1 = 0;
					drlg.thLocs[numMatches].tpdvar2 = 0;
					numMatches++;
					static_assert(lengthof(drlg.thLocs) >= (10 - 2 - (3 + 3)) * (10 - 2 - (3 + 3)), "TFit_Obj5 skips limit checks assuming enough thLocs entries II.");
					// if (numMatches == lengthof(drlg.thLocs))
					//	goto done;
				}
			}
		}
	}
// done:
	if (numMatches == 0)
		return -1;
	static_assert(lengthof(drlg.thLocs) < 0x7FFF, "TFit_Obj5 uses random_low to select a matching location.");
	return random_low(0, numMatches);
}

static bool CheckThemeObj3(int x, int y)
{
	int i, xx, yy;

	static_assert(lengthof(trm3x) == lengthof(trm3y), "Mismatching trm3 tables.");
	for (i = 0; i < lengthof(trm3x); i++) {
		xx = x + trm3x[i];
		yy = y + trm3y[i];
		//if (xx < 0 || yy < 0)
		//	return false;
		//assert(dTransVal[xx][yy] == themes[themeId]._tsTransVal);
		//if (dTransVal[xx][yy] != tv)
		//	return false;
		if ((nSolidTable[dPiece[xx][yy]] | dObject[xx][yy]) != 0)
			return false;
	}

	return true;
}

static int TFit_Obj3(int themeId)
{
	int xx, yy, numMatches;
	const ThemeStruct &theme = themes[themeId];

	numMatches = 0;
	for (xx = theme._tsx1 + 1; xx < theme._tsx2 - 1; xx++) {
		for (yy = theme._tsy1 + 1; yy < theme._tsy2 - 1; yy++) {
			if (CheckThemeObj3(xx, yy)) {
				drlg.thLocs[numMatches].tpdx = xx;
				drlg.thLocs[numMatches].tpdy = yy;
				drlg.thLocs[numMatches].tpdvar1 = 0;
				drlg.thLocs[numMatches].tpdvar2 = 0;
				numMatches++;
				static_assert(lengthof(drlg.thLocs) >= (10 - 2 - (2 + 2)) * (10 - 2 - (2 + 2)), "TFit_Obj3 skips limit checks assuming enough thLocs entries II.");
				// if (numMatches == lengthof(drlg.thLocs))
				//	goto done;
			}
		}
	}
// done:
	if (numMatches == 0)
		return -1;
	static_assert(lengthof(drlg.thLocs) < 0x7FFF, "TFit_Obj3 uses random_low to select a matching location.");
	return random_low(0, numMatches);
}

static bool SpecialThemeFit(int themeId, int themeType)
{
	bool rv;
	BYTE req;
	int loc;

	switch (themeType) {
	case THEME_BARREL:
	case THEME_MONSTPIT:
		rv = true;
		req = 0;
		break;
	case THEME_SHRINE:
	case THEME_LIBRARY:
		rv = _gbShrineFlag;
		req = 1;
		break;
	case THEME_SKELROOM:
		rv = _gbSkelRoomFlag;
		req = 3;
		break;
	case THEME_BLOODFOUNTAIN:
		rv = _gbBFountainFlag;
		req = 3;
		_gbBFountainFlag = false;
		break;
	case THEME_PURIFYINGFOUNTAIN:
		rv = _gbPFountainFlag;
		req = 3;
		_gbPFountainFlag = false;
		break;
	case THEME_MURKYFOUNTAIN:
		rv = _gbMFountainFlag;
		req = 3;
		_gbMFountainFlag = false;
		break;
	case THEME_TEARFOUNTAIN:
		rv = _gbTFountainFlag;
		req = 3;
		_gbTFountainFlag = false;
		break;
	case THEME_CAULDRON:
		rv = _gbCauldronFlag;
		req = 3;
		_gbCauldronFlag = false;
		break;
	case THEME_GOATSHRINE:
		rv = _gbGoatFlag;
		req = 3;
		break;
	case THEME_WEAPONRACK:
		rv = _gbWeaponFlag;
		req = 2;
		break;
	case THEME_ARMORSTAND:
		rv = _gbArmorFlag;
		req = 2;
		break;
	case THEME_TORTURE:
	case THEME_DECAPITATED:
	case THEME_BRNCROSS:
		rv = true;
		req = 2;
		break;
	case THEME_TREASURE:
		rv = _gbTreasureFlag;
		req = 0;
		_gbTreasureFlag = false;
		break;
	default:
		ASSUME_UNREACHABLE
		rv = false;
		req = 0;
		break;
	}
	loc = -1;
	if (rv) {
		switch (req) {
		case 0:
			loc = 0;
			break;
		case 1:
			loc = TFit_Shrine(themeId);
			break;
		case 2:
			loc = TFit_Obj3(themeId);
			break;
		case 3:
			loc = TFit_Obj5(themeId);
			break;
		default:
			ASSUME_UNREACHABLE
		}
	}
	if (loc < 0)
		return false;
	themes[themeId]._tsType = themeType;
	themes[themeId]._tsObjX = drlg.thLocs[loc].tpdx;
	themes[themeId]._tsObjY = drlg.thLocs[loc].tpdy;
	themes[themeId]._tsObjVar1 = drlg.thLocs[loc].tpdvar1;
	// themes[themeId]._tsObjVar2 = drlg.thLocs[loc].tpdvar2;
	return true;
}

void InitLvlThemes()
{
	numthemes = 0;
	zharlib = -1;
}

void InitThemes()
{
	int sh, i, j, x, y, x1, y1, x2, y2;

	// assert(currLvl._dType != DTYPE_TOWN);
	if (numthemes == 0)
		return;
	// assert(currLvl._dLevelNum < DLV_HELL4 || (currLvl._dDynLvl && currLvl._dLevelNum == DLV_HELL4)); // there are no themes in hellfire (and on diablo-level)
	sh = (currLvl._dDunType == DGT_CAVES ? 1 : 0); // shift location of the room where the walls are on the southern side of the tile (fences in caves)
	for (i = 0; i < numthemes; i++) {
		x1 = themes[i]._tsx1;
		y1 = themes[i]._tsy1;
		x2 = themes[i]._tsx2;
		y2 = themes[i]._tsy2;
		// convert to subtile-coordinates and select the internal subtiles of the room [p0;p1)
		x1 = DBORDERX + 2 * x1 + 1 + sh;
		y1 = DBORDERY + 2 * y1 + 1 + sh;
		x2 = DBORDERX + 2 * x2 + sh;
		y2 = DBORDERY + 2 * y2 + sh;
		themes[i]._tsx1 = x1;
		themes[i]._tsy1 = y1;
		themes[i]._tsx2 = x2;
		themes[i]._tsy2 = y2;
		// select transval
		themes[i]._tsTransVal = dTransVal[x1 + 1][y1 + 1];
		assert(themes[i]._tsTransVal != 0);
		// protect themes with dFlags - TODO: extend the protection +1 to prevent overdrawn shrine and torch? unlikely + protection would prevent torches in theme rooms...
		// v = themes[i]._tsTransVal;
		for (x = x1; x < x2; x++) {
			for (y = y1; y < y2; y++) {
				// if (dTransVal[x][y] == v) { -- wall?
					dFlags[x][y] |= BFLAG_MON_PROTECT | BFLAG_OBJ_PROTECT;
				// }
			}
		}
	}

	// select theme types
	// TODO: use dType instead
	_gbShrineFlag = currLvl._dDunType != DGT_CAVES && currLvl._dDunType != DGT_HELL;
	_gbSkelRoomFlag = _gbShrineFlag && numSkelTypes != 0;
	_gbGoatFlag = numGoatTypes != 0;
	_gbWeaponFlag = currLvl._dDunType != DGT_CATHEDRAL;
	_gbArmorFlag = currLvl._dDunType != DGT_CATHEDRAL;
	_gbCauldronFlag = currLvl._dDunType == DGT_HELL;
	_gbBFountainFlag = true;
	_gbMFountainFlag = true;
	_gbPFountainFlag = true;
	_gbTFountainFlag = true;
	_gbTreasureFlag = true;

	if (QuestStatus(Q_ZHAR)) {
		for (i = 0; i < numthemes; i++) {
			if (SpecialThemeFit(i, THEME_LIBRARY)) {
				zharlib = i;
				break;
			}
		}
	}
	for (i = 0; i < numthemes; i++) {
		if (i != zharlib) {
			j = ThemeGood[random_(0, lengthof(ThemeGood))];
			while (!SpecialThemeFit(i, j))
				j = random_(0, NUM_THEMES);
		}
	}
}

/*
 * Place a theme object with the specified frequency.
 * @param theme: the theme in which the object should be placed
 * @param type: the type of the object to place
 * @param rndfrq: the frequency to place the object
 */
static void Place_Obj3(const ThemeStruct &theme, int type, int rndfrq)
{
	int xx, yy;
	// assert(rndfrq > 0);
	for (xx = theme._tsx1 + 1; xx < theme._tsx2 - 1; xx++) {
		for (yy = theme._tsy1 + 1; yy < theme._tsy2 - 1; yy++) {
			if (CheckThemeObj3(xx, yy) && random_low(0, rndfrq) == 0) {
				AddObject(type, xx, yy);
			}
		}
	}
}
/**
 * PlaceThemeMonsts places theme monsters with the specified frequency.
 *
 * @param theme: the theme in which the monsters should be placed
 */
static void PlaceThemeMonsts(const ThemeStruct &theme)
{
	int mtidx, xx, yy;
	const BYTE monstrnds[4] = { 6, 7, 3, 9 };
	const BYTE rndfrq = monstrnds[currLvl._dDunType - 1]; // TODO: use dType instead?

	// assert(numScaTypes != 0);
	mtidx = mapScaTypes[random_low(0, numScaTypes)];
	for (xx = theme._tsx1; xx < theme._tsx2; xx++) {
		for (yy = theme._tsy1; yy < theme._tsy2; yy++) {
			// assert(dTransVal[xx][yy] == theme._tsTransVal);
			if (/*dTransVal[xx][yy] == tv &&*/ (nSolidTable[dPiece[xx][yy]] | dItem[xx][yy] | dObject[xx][yy]) == 0) {
				if (random_low(0, rndfrq) == 0) {
					AddMonster(mtidx, xx, yy);
				}
			}
		}
	}
}

/**
 * Theme_Barrel initializes the barrel theme.
 *
 * @param themeId: theme id.
 */
static void Theme_Barrel(int themeId)
{
	int r, xx, yy;
	const BYTE barrnds[4] = { 2, 6, 4, 8 };
	const BYTE barrnd = barrnds[currLvl._dDunType - 1];     // TODO: use dType instead?
	const ThemeStruct &theme = themes[themeId];

	for (xx = theme._tsx1; xx < theme._tsx2; xx++) {
		for (yy = theme._tsy1; yy < theme._tsy2; yy++) {
			// assert(dTransVal[xx][yy] == theme._tsTransVal);
			if (/*dTransVal[xx][yy] == tv &&*/ !nSolidTable[dPiece[xx][yy]]) {
				if (random_low(0, barrnd) == 0) {
					r = random_low(0, barrnd) == 0 ? OBJ_BARREL : OBJ_BARRELEX;
					AddObject(r, xx, yy);
				}
			}
		}
	}
	PlaceThemeMonsts(theme);
}

/**
 * Theme_Shrine initializes the shrine theme.
 *
 * @param themeId: theme id.
 */
static void Theme_Shrine(int themeId)
{
	int xx, yy;
	const ThemeStruct &theme = themes[themeId];

	xx = theme._tsObjX;
	yy = theme._tsObjY;
	if (theme._tsObjVar1 != 0) {
		AddObject(OBJ_CANDLE2, xx - 1, yy);
		AddObject(OBJ_SHRINER, xx, yy);
		AddObject(OBJ_CANDLE2, xx + 1, yy);
	} else {
		AddObject(OBJ_CANDLE2, xx, yy - 1);
		AddObject(OBJ_SHRINEL, xx, yy);
		AddObject(OBJ_CANDLE2, xx, yy + 1);
	}
	PlaceThemeMonsts(theme);
}

/**
 * Theme_MonstPit initializes the monster pit theme.
 *
 * @param themeId: theme id.
 */
static void Theme_MonstPit(int themeId)
{
	int r, xx, yy;
	const ThemeStruct &theme = themes[themeId];

	r = random_(11, (theme._tsx2 - theme._tsx1) * (theme._tsy2 - theme._tsy1));
	while (true) {
		for (xx = theme._tsx1; xx < theme._tsx2; xx++) {
			for (yy = theme._tsy1; yy < theme._tsy2; yy++) {
				// assert(dTransVal[xx][yy] == theme._tsTransVal);
				if (/*dTransVal[xx][yy] == tv &&*/ !nSolidTable[dPiece[xx][yy]] && --r < 0) {
					CreateRndItem(xx, yy, CFDQ_GOOD, ICM_DELTA);
					goto done;
				}
			}
		}
	}
done:

	PlaceThemeMonsts(theme);
}

static void AddSkelMonOrBanner(BYTE rnd, int x, int y)
{
	assert(PosOkActor(x, y));
	if (rnd == 0 || random_low(0, rnd) != 0) {
		AddMonster(mapSkelTypes[random_low(136, numSkelTypes)], x, y);
	} else {
		AddObject(OBJ_BANNER, x, y);
	}
}

/**
 * Theme_SkelRoom initializes the skeleton room theme.
 *
 * @param themeId: theme id.
 */
static const int8_t SkelPatterns[][2] = {
	{  0,  0 },
	{ -1,  0 },
	{  0, -1 },
	{ -1, -1 },
	{  1, -1 },
	{ -1,  1 },
};
static void Theme_SkelRoom(int themeId)
{
	int xx, yy;
	const BYTE monstrnds[4] = { 6, 7, 3, 9 };
	BYTE monstrnd;
	const ThemeStruct &theme = themes[themeId];
	int8_t objs[2];

	xx = theme._tsObjX;
	yy = theme._tsObjY;

	AddObject(OBJ_SKFIRE, xx, yy);

	monstrnd = monstrnds[currLvl._dDunType - 1]; // TODO: use dType instead?

	AddSkelMonOrBanner(monstrnd, xx - 1, yy - 1);

	AddSkelMonOrBanner(0, xx, yy - 1);

	AddSkelMonOrBanner(monstrnd, xx + 1, yy - 1);

	AddSkelMonOrBanner(monstrnd, xx - 1, yy);

	AddSkelMonOrBanner(monstrnd, xx + 1, yy);

	AddSkelMonOrBanner(monstrnd, xx - 1, yy + 1);

	AddSkelMonOrBanner(0, xx, yy + 1);

	AddSkelMonOrBanner(monstrnd, xx + 1, yy + 1);

	unsigned ver = random_(0, 8);
	if (ver >= (unsigned)lengthof(SkelPatterns)) {
		ver = 0;
	}
	objs[0] = SkelPatterns[ver][0];
	objs[1] = SkelPatterns[ver][1];

	if ((dObject[xx][yy - 3] == 0 || !objects[dObject[xx][yy - 3] - 1]._oDoorFlag)   // not a door
	 && (nSolidTable[dPiece[xx][yy - 3]] || !nSolidTable[dPiece[xx + 1][yy - 3]])) { // or a single path to NE TODO: allow if !nSolidTable[dPiece[xx - 1][yy - 3]]?
		// assert(dObject[xx][yy - 2] == 0);
	} else {
		objs[0] = -1;
	}
	if ((dObject[xx][yy + 3] == 0 || !objects[dObject[xx][yy + 3] - 1]._oDoorFlag)   // not a door
	 && (nSolidTable[dPiece[xx][yy + 3]] || !nSolidTable[dPiece[xx + 1][yy + 3]])) { // or a single path to SW TODO: allow if !nSolidTable[dPiece[xx - 1][yy + 3]]?
		// assert(dObject[xx][yy + 2] == 0);
	} else {
		objs[1] = -1;
	}
	if (objs[0] >= 0) {
		AddObject(objs[0] == 0 ? OBJ_BOOK2R : OBJ_BOOK1R, xx, yy - 2);
	}
	if (objs[1] >= 0) {
		AddObject(objs[1] == 0 ? OBJ_BOOK2R : OBJ_BOOK1R, xx, yy + 2);
	}
}

/**
 * Theme_Treasure initializes the treasure theme.
 *
 * @param themeId: theme id.
 */
static void Theme_Treasure(int themeId)
{
	int xx, yy;
	const BYTE treasrnds[4] = { 6, 9, 7, 10 };
	const BYTE treasrnd = treasrnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	const ThemeStruct &theme = themes[themeId];

	for (xx = theme._tsx1; xx < theme._tsx2; xx++) {
		for (yy = theme._tsy1; yy < theme._tsy2; yy++) {
			// assert(dTransVal[xx][yy] == theme._tsTransVal);
			if (/*dTransVal[xx][yy] == tv &&*/ !nSolidTable[dPiece[xx][yy]]) {
				if (random_low(0, treasrnd) == 0) {
					CreateTypeItem(xx, yy, CFDQ_NORMAL, ITYPE_GOLD, IMISC_NONE, ICM_DELTA);
				} else if (random_low(0, treasrnd) == 0) {
					CreateRndItem(xx, yy, CFDQ_NORMAL, ICM_DELTA);
				}
			}
		}
	}
	PlaceThemeMonsts(theme);
}

/**
 * Theme_Library initializes the library theme.
 *
 * @param themeId: theme id.
 */
static const int8_t LibPatterns[][3] = {
	{ -1, 0, 1 },
	{  1, 0,-1 },
	{  1, 0, 1 },
	{  0, 1, 0 },
};
static int NextLibraryObj(int8_t type, bool notzhar, unsigned ver, int dx)
{
	ver &= 7;
	return type < 0 ? type : (type == 0 ? ((ver & 3) ? OBJ_CANDLE2 : -1) : ((ver == 1 && notzhar) ? -1 : (((!notzhar || ver > 1) ? OBJ_BOOKCASEL : OBJ_BOOKSHELFL) + dx)));
}
static void Theme_Library(int themeId)
{
	int xx, yy, dx, dy, type, t0, t1, t2;
	const BYTE librnds[4] = { 1, 2, 2, 5 };
	BYTE librnd;
	const ThemeStruct &theme = themes[themeId];
	const bool placemonsters = /*QuestStatus(Q_ZHAR) &&*/ themeId != zharlib;
	const int8_t* ptrn;

	xx = theme._tsObjX;
	yy = theme._tsObjY;
	dx = theme._tsObjVar1;
	dy = 1 - dx;
	static_assert(OBJ_BOOKCASEL + 1 == OBJ_BOOKCASER, "Theme_Library depends on the order of OBJ_BOOKCASEL/R");
	static_assert(OBJ_BOOKSHELFL + 1 == OBJ_BOOKSHELFR, "Theme_Library depends on the order of OBJ_BOOKSHELFL/R");
	unsigned ver = random_(0, 1 << (2 + 3 + 3 + 3));
	if (placemonsters
		// make sure the place is wide enough
		// - on the inside
		&& !nSolidTable[dPiece[xx - 2 * dx][yy - 2 * dy]]
		&& !nSolidTable[dPiece[xx + 2 * dx][yy + 2 * dy]]
		 // - on the wall (to avoid doors)
		&& !nSolidTable[dPiece[xx - 2 * dx - dy][yy - 2 * dy - dx]]
		&& !nSolidTable[dPiece[xx - 2 * dx - dy][yy - 2 * dy - dx]]) {
		type = ver & 3;
	} else {
		type = 3;
	}
	ptrn = &LibPatterns[type][0];
	ver >>= 2;
	t0 = NextLibraryObj(ptrn[0], placemonsters, ver, dx);
	ver >>= 3;
	if (t0 >= 0)
		AddObject(t0, xx - dx, yy - dy);
	t1 = NextLibraryObj(ptrn[1], placemonsters, ver, dx);
	ver >>= 3;
	if (t1 >= 0)
		AddObject(t1, xx, yy);
	t2 = NextLibraryObj(ptrn[2], placemonsters, ver, dx);
	// ver >>= 3;
	if (t2 >= 0)
		AddObject(t2, xx + dx, yy + dy);

	static_assert(DTYPE_CATHEDRAL == 1 && DTYPE_CATACOMBS == 2, "Theme_Library uses dungeon_type as an array-index.");
	// assert(currLvl._dDunType == 1 /* DTYPE_CATHEDRAL */ || currLvl._dDunType == 2 /* DTYPE_CATACOMBS */);
	librnd = librnds[currLvl._dDunType - 1];     // TODO: use dType instead?
	static_assert(OBJ_BOOK2L - 2 == OBJ_BOOK2R, "Theme_Library depends on the order of OBJ_BOOK2L/R");
	type = OBJ_BOOK2L - 2 * random_(0, 2);
	static_assert(OBJ_BOOK2L - 1 == OBJ_BOOK2LN, "Theme_Library depends on the order of OBJ_BOOK2L(N)");
	static_assert(OBJ_BOOK2R - 1 == OBJ_BOOK2RN, "Theme_Library depends on the order of OBJ_BOOK2R(N)");
	for (xx = theme._tsx1 + 1; xx < theme._tsx2 - 1; xx++) {
		for (yy = theme._tsy1 + 1; yy < theme._tsy2 - 1; yy++) {
			if (CheckThemeObj3(xx, yy) && dMonster[xx][yy] == 0 && random_low(0, librnd) == 0) {
				AddObject(type - (random_low(0, 2 * librnd) != 0 ? 1 : 0), xx, yy);
			}
		}
	}

	if (placemonsters)
		PlaceThemeMonsts(theme);
}

/**
 * Theme_Torture initializes the torture theme.
 *
 * @param themeId: theme id.
 */
static void Theme_Torture(int themeId)
{
	const BYTE tortrnds[4] = { 6 * 2, 8 * 2, 3 * 2, 8 * 2 };
	const BYTE tortrnd = tortrnds[currLvl._dDunType - 1];   // TODO: use dType instead?
	const ThemeStruct &theme = themes[themeId];

	AddObject(random_(46, 2) ? OBJ_TNUDEW : OBJ_TNUDEM, theme._tsObjX, theme._tsObjY);
	Place_Obj3(theme, OBJ_TNUDEM, tortrnd);
	Place_Obj3(theme, OBJ_TNUDEW, tortrnd);
	PlaceThemeMonsts(theme);
}

/**
 * Theme_BloodFountain initializes the blood fountain theme.
 *
 * @param themeId: theme id.
 */
static void Theme_BloodFountain(int themeId)
{
	const ThemeStruct &theme = themes[themeId];

	AddObject(OBJ_BLOODFTN, theme._tsObjX, theme._tsObjY);
	PlaceThemeMonsts(theme);
}

/**
 * Theme_Decap initializes the decapitated theme.
 *
 * @param themeId: theme id.
 */
static void Theme_Decap(int themeId)
{
	const BYTE decaprnds[4] = { 6, 8, 3, 8 };
	const BYTE decaprnd = decaprnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	const ThemeStruct &theme = themes[themeId];

	AddObject(OBJ_DECAP, theme._tsObjX, theme._tsObjY);
	Place_Obj3(theme, OBJ_DECAP, decaprnd);
	PlaceThemeMonsts(theme);
}

/**
 * Theme_PurifyingFountain initializes the purifying fountain theme.
 *
 * @param themeId: theme id.
 */
static void Theme_PurifyingFountain(int themeId)
{
	const ThemeStruct &theme = themes[themeId];

	AddObject(OBJ_PURIFYINGFTN, theme._tsObjX, theme._tsObjY);
	PlaceThemeMonsts(theme);
}

/**
 * Theme_ArmorStand initializes the armor stand theme.
 *
 * @param themeId: theme id.
 */
static void Theme_ArmorStand(int themeId)
{
	int type;
	const BYTE armorrnds[4] = { 6, 8, 3, 8 };
	const BYTE armorrnd = armorrnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	const ThemeStruct &theme = themes[themeId];

	static_assert(OBJ_ARMORSTANDL - 2 == OBJ_ARMORSTANDR, "Theme_ArmorStand depends on the order of ARMORSTANDL/R");
	type = OBJ_ARMORSTANDL - 2 * random_(0, 2);
	static_assert(OBJ_ARMORSTANDL - 1 == OBJ_ARMORSTANDLN, "Theme_ArmorStand depends on the order of ARMORSTANDL(N)");
	static_assert(OBJ_ARMORSTANDR - 1 == OBJ_ARMORSTANDRN, "Theme_ArmorStand depends on the order of ARMORSTANDR(N)");
	AddObject(type - (_gbArmorFlag ? 0 : 1), theme._tsObjX, theme._tsObjY);
	_gbArmorFlag = false;
	type -= 1;
	Place_Obj3(theme, type, armorrnd);
	PlaceThemeMonsts(theme);
}

/**
 * Theme_GoatShrine initializes the goat shrine theme.
 *
 * @param themeId: theme id.
 */
static void Theme_GoatShrine(int themeId)
{
	int i, xx, yy, mtidx, x, y;
	const ThemeStruct &theme = themes[themeId];

	xx = theme._tsObjX;
	yy = theme._tsObjY;
	AddObject(OBJ_GOATSHRINE, xx, yy);
	mtidx = mapGoatTypes[random_low(136, numGoatTypes)];
	for (i = 0; i < lengthof(offset_x); i++) {
		x = xx + offset_x[i];
		y = yy + offset_y[i];
		// assert(dTransVal[x][y] == theme._tsTransVal && !nSolidTable[dPiece[x][y]]);
		AddMonster(mtidx, x, y); // OPPOSITE(i)
	}
}

/**
 * Theme_Cauldron initializes the cauldron theme.
 *
 * @param themeId: theme id.
 */
static void Theme_Cauldron(int themeId)
{
	const ThemeStruct &theme = themes[themeId];

	AddObject(OBJ_CAULDRON, theme._tsObjX, theme._tsObjY);
	PlaceThemeMonsts(theme);
}

/**
 * Theme_MurkyFountain initializes the murky fountain theme.
 *
 * @param themeId: theme id.
 */
static void Theme_MurkyFountain(int themeId)
{
	const ThemeStruct &theme = themes[themeId];

	AddObject(OBJ_MURKYFTN, theme._tsObjX, theme._tsObjY);
	PlaceThemeMonsts(theme);
}

/**
 * Theme_TearFountain initializes the tear fountain theme.
 *
 * @param themeId: theme id.
 */
static void Theme_TearFountain(int themeId)
{
	const ThemeStruct &theme = themes[themeId];

	AddObject(OBJ_TEARFTN, theme._tsObjX, theme._tsObjY);
	PlaceThemeMonsts(theme);
}

/**
 * Theme_BrnCross initializes the burning cross theme.
 *
 * @param themeId: theme id.
 */
static void Theme_BrnCross(int themeId)
{
	const BYTE bcrossrnds[4] = { 5, 7, 3, 8 };
	const BYTE bcrossrnd = bcrossrnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	const ThemeStruct &theme = themes[themeId];

	AddObject(OBJ_TBCROSS, theme._tsObjX, theme._tsObjY);
	Place_Obj3(theme, OBJ_TBCROSS, bcrossrnd);
	PlaceThemeMonsts(theme);
}

/**
 * Theme_WeaponRack initializes the weapon rack theme.
 *
 * @param themeId: theme id.
 */
static void Theme_WeaponRack(int themeId)
{
	int type;
	const BYTE weaponrnds[4] = { 6, 8, 5, 8 };
	const BYTE weaponrnd = weaponrnds[currLvl._dDunType - 1]; // TODO: use dType instead?
	const ThemeStruct &theme = themes[themeId];

	static_assert(OBJ_WEAPONRACKL - 2 == OBJ_WEAPONRACKR, "Theme_WeaponRack depends on the order of WEAPONRACKL/R");
	type = OBJ_WEAPONRACKL - 2 * random_(0, 2);
	static_assert(OBJ_WEAPONRACKL - 1 == OBJ_WEAPONRACKLN, "Theme_WeaponRack depends on the order of WEAPONRACKL(N)");
	static_assert(OBJ_WEAPONRACKR - 1 == OBJ_WEAPONRACKRN, "Theme_WeaponRack depends on the order of WEAPONRACKR(N)");
	AddObject(type - (_gbWeaponFlag ? 0 : 1), theme._tsObjX, theme._tsObjY);
	_gbWeaponFlag = false;
	type -= 1;
	Place_Obj3(theme, type, weaponrnd);
	PlaceThemeMonsts(theme);
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
	// assert(currLvl._dLevelNum < DLV_HELL4 || (currLvl._dDynLvl && currLvl._dLevelNum == DLV_HELL4) || numthemes == 0); // there are no themes in hellfire (and on diablo-level)
	//gbInitObjFlag = true;
	for (i = 0; i < numthemes; i++) {
		switch (themes[i]._tsType) {
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
