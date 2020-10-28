/**
 * @file themes.cpp
 *
 * Implementation of the theme room placing algorithms.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

int numthemes;
BOOL armorFlag;
BOOL ThemeGoodIn[4];
BOOL weaponFlag;
BOOL treasureFlag;
BOOL mFountainFlag;
BOOL cauldronFlag;
BOOL tFountainFlag;
int zharlib;
int themex;
int themey;
int themeVar1;
ThemeStruct themes[MAXTHEMES];
BOOL pFountainFlag;
BOOL bFountainFlag;
BOOL bCrossFlag;

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

BOOL TFit_Shrine(int tidx)
{
	int xp, yp, found;
	const char tv = themes[tidx].ttval;

	xp = 0;
	yp = 0;
	while (TRUE) {
		if (dTransVal[xp][yp] == tv) {
			if (nTrapTable[dPiece[xp][yp - 1]]
			    && !nSolidTable[dPiece[xp - 1][yp]]
			    && !nSolidTable[dPiece[xp + 1][yp]]
			    && dTransVal[xp - 1][yp] == tv
			    && dTransVal[xp + 1][yp] == tv
			    && dObject[xp - 1][yp - 1] == 0
			    && dObject[xp + 1][yp - 1] == 0) {
				found = 1;
				break;
			}
			if (nTrapTable[dPiece[xp - 1][yp]]
			    && !nSolidTable[dPiece[xp][yp - 1]]
			    && !nSolidTable[dPiece[xp][yp + 1]]
			    && dTransVal[xp][yp - 1] == tv
			    && dTransVal[xp][yp + 1] == tv
			    && dObject[xp - 1][yp - 1] == 0
			    && dObject[xp - 1][yp + 1] == 0) {
				found = 2;
				break;
			}
		}
		xp++;
		if (xp == MAXDUNX) {
			xp = 0;
			yp++;
			if (yp == MAXDUNY)
				return FALSE;
		}
	}
	themex = xp;
	themey = yp;
	themeVar1 = found;
	return TRUE;
}

BOOL TFit_Obj5(int tidx)
{
	int xp, yp;
	int i, r, rs;
	BOOL found;
	const char tv = themes[tidx].ttval;

	xp = 0;
	yp = 0;
	r = random_(0, 5) + 1;
	rs = r;
	while (r > 0) {
		found = FALSE;
		if (dTransVal[xp][yp] == tv && !nSolidTable[dPiece[xp][yp]]) {
			found = TRUE;
			for (i = 0; found && i < 25; i++) {
				if (nSolidTable[dPiece[xp + trm5x[i]][yp + trm5y[i]]]) {
					found = FALSE;
				}
				if (dTransVal[xp + trm5x[i]][yp + trm5y[i]] != tv) {
					found = FALSE;
				}
			}
		}

		if (!found) {
			xp++;
			if (xp == MAXDUNX) {
				xp = 0;
				yp++;
				if (yp == MAXDUNY) {
					if (r == rs) {
						return FALSE;
					}
					yp = 0;
				}
			}
			continue;
		}

		r--;
	}

	themex = xp;
	themey = yp;

	return TRUE;
}

BOOL TFit_SkelRoom(int tidx)
{
	int i;

	if (leveltype != DTYPE_CATHEDRAL && leveltype != DTYPE_CATACOMBS) {
		return FALSE;
	}

	for (i = 0; i < nummtypes; i++) {
		if (IsSkel(Monsters[i].mtype)) {
			themeVar1 = i;
			return TFit_Obj5(tidx);
		}
	}

	return FALSE;
}

BOOL TFit_GoatShrine(int tidx)
{
	int i;

	for (i = 0; i < nummtypes; i++) {
		if (IsGoat(Monsters[i].mtype)) {
			themeVar1 = i;
			return TFit_Obj5(tidx);
		}
	}

	return FALSE;
}

BOOL CheckThemeObj3(int xp, int yp, int tidx, int rndfrq)
{
	int i, xx, yy;
	const char tv = themes[tidx].ttval;

	for (i = 0; i < 9; i++) {
		xx = xp + trm3x[i];
		yy = yp + trm3y[i];
		if (xx < 0 || yy < 0)
			return FALSE;
		if ((nSolidTable[dPiece[xx][yy]] | dObject[xx][yy]) != 0)
			return FALSE;
		if (dTransVal[xx][yy] != tv)
			return FALSE;
		if (rndfrq != -1 && random_(0, rndfrq) == 0)
			return FALSE;
	}

	return TRUE;
}

BOOL TFit_Obj3(int tidx)
{
	int xp, yp;
	const char objrnds[4] = { 4, 4, 3, 5 };
	const char objrnd = objrnds[leveltype - 1];

	for (yp = 1; yp < MAXDUNY - 1; yp++) {
		for (xp = 1; xp < MAXDUNX - 1; xp++) {
			if (CheckThemeObj3(xp, yp, tidx, objrnd)) {
				themex = xp;
				themey = yp;
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CheckThemeReqs(int theme)
{
	BOOL rv;

	rv = TRUE;
	switch (theme) {
	case THEME_SHRINE:
	case THEME_SKELROOM:
	case THEME_LIBRARY:
		if (leveltype == DTYPE_CAVES || leveltype == DTYPE_HELL) {
			rv = FALSE;
		}
		break;
	case THEME_BLOODFOUNTAIN:
		if (!bFountainFlag) {
			rv = FALSE;
		}
		break;
	case THEME_PURIFYINGFOUNTAIN:
		if (!pFountainFlag) {
			rv = FALSE;
		}
		break;
	case THEME_ARMORSTAND:
		if (leveltype == DTYPE_CATHEDRAL) {
			rv = FALSE;
		}
		break;
	case THEME_CAULDRON:
		if (leveltype != DTYPE_HELL || !cauldronFlag) {
			rv = FALSE;
		}
		break;
	case THEME_MURKYFOUNTAIN:
		if (!mFountainFlag) {
			rv = FALSE;
		}
		break;
	case THEME_TEARFOUNTAIN:
		if (!tFountainFlag) {
			rv = FALSE;
		}
		break;
	case THEME_WEAPONRACK:
		if (leveltype == DTYPE_CATHEDRAL) {
			rv = FALSE;
		}
		break;
	}

	return rv;
}

BOOL SpecialThemeFit(int tidx, int theme)
{
	BOOL rv;

	rv = CheckThemeReqs(theme);
	switch (theme) {
	case THEME_SHRINE:
	case THEME_LIBRARY:
		if (rv) {
			rv = TFit_Shrine(tidx);
		}
		break;
	case THEME_SKELROOM:
		if (rv) {
			rv = TFit_SkelRoom(tidx);
		}
		break;
	case THEME_BLOODFOUNTAIN:
		if (rv) {
			rv = TFit_Obj5(tidx);
		}
		if (rv) {
			bFountainFlag = FALSE;
		}
		break;
	case THEME_PURIFYINGFOUNTAIN:
		if (rv) {
			rv = TFit_Obj5(tidx);
		}
		if (rv) {
			pFountainFlag = FALSE;
		}
		break;
	case THEME_MURKYFOUNTAIN:
		if (rv) {
			rv = TFit_Obj5(tidx);
		}
		if (rv) {
			mFountainFlag = FALSE;
		}
		break;
	case THEME_TEARFOUNTAIN:
		if (rv) {
			rv = TFit_Obj5(tidx);
		}
		if (rv) {
			tFountainFlag = FALSE;
		}
		break;
	case THEME_CAULDRON:
		if (rv) {
			rv = TFit_Obj5(tidx);
		}
		if (rv) {
			cauldronFlag = FALSE;
		}
		break;
	case THEME_GOATSHRINE:
		if (rv) {
			rv = TFit_GoatShrine(tidx);
		}
		break;
	case THEME_TORTURE:
	case THEME_DECAPITATED:
	case THEME_ARMORSTAND:
	case THEME_BRNCROSS:
	case THEME_WEAPONRACK:
		if (rv) {
			rv = TFit_Obj3(tidx);
		}
		break;
	case THEME_TREASURE:
		rv = treasureFlag;
		if (rv) {
			treasureFlag = FALSE;
		}
		break;
	}

	return rv;
}

BOOL CheckThemeRoom(int tv)
{
	int i, j, tarea;

	for (i = 0; i < numtrigs; i++) {
		if (dTransVal[trigs[i]._tx][trigs[i]._ty] == tv)
			return FALSE;
	}

	tarea = 0;
	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (dTransVal[i][j] != tv)
				continue;
			if (dFlags[i][j] & BFLAG_POPULATED)
				return FALSE;

			tarea++;
		}
	}

	if (leveltype == DTYPE_CATHEDRAL && (tarea < 9 || tarea > 100))
		return FALSE;

	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (dTransVal[i][j] != tv || nSolidTable[dPiece[i][j]])
				continue;
			if (dTransVal[i - 1][j] != tv && !nSolidTable[dPiece[i - 1][j]])
				return FALSE;
			if (dTransVal[i + 1][j] != tv && !nSolidTable[dPiece[i + 1][j]])
				return FALSE;
			if (dTransVal[i][j - 1] != tv && !nSolidTable[dPiece[i][j - 1]])
				return FALSE;
			if (dTransVal[i][j + 1] != tv && !nSolidTable[dPiece[i][j + 1]])
				return FALSE;
		}
	}

	return TRUE;
}

void InitThemes()
{
	int i, j;

	zharlib = -1;
	numthemes = 0;
	armorFlag = TRUE;
	bFountainFlag = TRUE;
	cauldronFlag = TRUE;
	mFountainFlag = TRUE;
	pFountainFlag = TRUE;
	tFountainFlag = TRUE;
	treasureFlag = TRUE;
	bCrossFlag = FALSE;
	weaponFlag = TRUE;

	if (currlevel == 16)
		return;

	if (leveltype == DTYPE_CATHEDRAL) {
		for (i = 0; i < sizeof(ThemeGoodIn) / sizeof(ThemeGoodIn[0]); i++)
			ThemeGoodIn[i] = FALSE;

		for (i = 0; i < 256 && numthemes < MAXTHEMES; i++) {
			if (CheckThemeRoom(i)) {
				themes[numthemes].ttval = i;
				j = ThemeGood[random_(0, 4)];
				while (!SpecialThemeFit(numthemes, j))
					j = random_(0, 17);
				themes[numthemes].ttype = j;
				numthemes++;
			}
		}
	}
	if (leveltype == DTYPE_CATACOMBS || leveltype == DTYPE_CAVES || leveltype == DTYPE_HELL) {
		for (i = 0; i < themeCount; i++)
			themes[i].ttype = THEME_NONE;
		if (QuestStatus(Q_ZHAR)) {
			for (i = 0; i < themeCount; i++) {
				themes[i].ttval = themeLoc[i].ttval;
				if (SpecialThemeFit(i, THEME_LIBRARY)) {
					themes[i].ttype = THEME_LIBRARY;
					zharlib = i;
					break;
				}
			}
		}
		for (i = 0; i < themeCount; i++) {
			if (themes[i].ttype == THEME_NONE) {
				themes[i].ttval = themeLoc[i].ttval;
				j = ThemeGood[random_(0, 4)];
				while (!SpecialThemeFit(i, j))
					j = random_(0, 17);
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
	int i, x, y;
	char v;

	if (currlevel != 16) {
		if (leveltype == DTYPE_CATHEDRAL) {
			for (i = 0; i < numthemes; i++) {
				v = themes[i].ttval;
				for (y = 0; y < MAXDUNY; y++) {
					for (x = 0; x < MAXDUNX; x++) {
						if (dTransVal[x][y] == v) {
							dFlags[x][y] |= BFLAG_POPULATED;
						}
					}
				}
			}
		} else {
			DRLG_HoldThemeRooms();
		}
	}
}

/**
 * PlaceThemeMonsts places theme monsters with the specified frequency.
 *
 * @param tidx theme number (index into themes array).
 * @param rndfrq frequency (1/f likelihood of adding monster).
 */
void PlaceThemeMonsts(int tidx, int rndfrq)
{
	int xp, yp;
#ifdef HELLFIRE
	int scattertypes[138];
#else
	int scattertypes[111];
#endif
	int numscattypes, mtype, i;
	char tv;

	numscattypes = 0;
	for (i = 0; i < nummtypes; i++) {
		if (Monsters[i].mPlaceFlags & 1) {
			scattertypes[numscattypes] = i;
			numscattypes++;
		}
	}
	mtype = scattertypes[random_(0, numscattypes)];
	tv = themes[tidx].ttval;
	for (yp = 0; yp < MAXDUNY; yp++) {
		for (xp = 0; xp < MAXDUNX; xp++) {
			if (dTransVal[xp][yp] == tv && (nSolidTable[dPiece[xp][yp]] | dItem[xp][yp] | dObject[xp][yp]) == 0) {
				if (random_(0, rndfrq) == 0) {
					AddMonster(xp, yp, random_(0, 8), mtype, TRUE);
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
void Theme_Barrel(int tidx)
{
	int xp, yp, r;
	const char barrnds[4] = { 2, 6, 4, 8 };
	const char monstrnds[4] = { 5, 7, 3, 9 };
	const char barrnd = barrnds[leveltype - 1];
	const char monstrnd = monstrnds[leveltype - 1];
	const char tv = themes[tidx].ttval;

	for (yp = 0; yp < MAXDUNY; yp++) {
		for (xp = 0; xp < MAXDUNX; xp++) {
			if (dTransVal[xp][yp] == tv && !nSolidTable[dPiece[xp][yp]]) {
				if (random_(0, barrnd) == 0) {
					if (random_(0, barrnd) == 0) {
						r = OBJ_BARREL;
					} else {
						r = OBJ_BARRELEX;
					}
					AddObject(r, xp, yp);
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
void Theme_Shrine(int tidx)
{
	const char monstrnds[4] = { 6, 6, 3, 9 };

	TFit_Shrine(tidx);
	if (themeVar1 == 1) {
		AddObject(OBJ_CANDLE2, themex - 1, themey);
		AddObject(OBJ_SHRINER, themex, themey);
		AddObject(OBJ_CANDLE2, themex + 1, themey);
	} else {
		AddObject(OBJ_CANDLE2, themex, themey - 1);
		AddObject(OBJ_SHRINEL, themex, themey);
		AddObject(OBJ_CANDLE2, themex, themey + 1);
	}
	PlaceThemeMonsts(tidx, monstrnds[leveltype - 1]);
}

/**
 * Theme_MonstPit initializes the monster pit theme.
 *
 * @param tidx theme number (index into themes array).
 */
void Theme_MonstPit(int tidx)
{
	int r;
	int ixp, iyp;
	const char monstrnds[4] = { 6, 7, 3, 9 };
	const char tv = themes[tidx].ttval;

	r = random_(0, 100) + 1;
	ixp = 0;
	iyp = 0;
	while (TRUE) {
		if (dTransVal[ixp][iyp] == tv && !nSolidTable[dPiece[ixp][iyp]]) {
			--r;
			if (r <= 0)
				break;
		}
		ixp++;
		if (ixp == MAXDUNX) {
			ixp = 0;
			iyp++;
			if (iyp == MAXDUNY) {
				iyp = 0;
			}
		}
	}
	CreateRndItem(ixp, iyp, TRUE, FALSE, TRUE);
	ItemNoFlippy();
	PlaceThemeMonsts(tidx, monstrnds[leveltype - 1]);
}

/**
 * Theme_SkelRoom initializes the skeleton room theme.
 *
 * @param tidx theme number (index into themes array).
 */
void Theme_SkelRoom(int tidx)
{
	int xp, yp, i;
	const char monstrnds[4] = { 6, 7, 3, 9 };
	const char monstrnd = monstrnds[leveltype - 1];

	TFit_SkelRoom(tidx);

	xp = themex;
	yp = themey;

	AddObject(OBJ_SKFIRE, xp, yp);

	if (random_(0, monstrnd) != 0) {
		i = PreSpawnSkeleton();
		SpawnSkeleton(i, xp - 1, yp - 1);
	} else {
		AddObject(OBJ_BANNERL, xp - 1, yp - 1);
	}

	i = PreSpawnSkeleton();
	SpawnSkeleton(i, xp, yp - 1);

	if (random_(0, monstrnd) != 0) {
		i = PreSpawnSkeleton();
		SpawnSkeleton(i, xp + 1, yp - 1);
	} else {
		AddObject(OBJ_BANNERR, xp + 1, yp - 1);
	}
	if (random_(0, monstrnd) != 0) {
		i = PreSpawnSkeleton();
		SpawnSkeleton(i, xp - 1, yp);
	} else {
		AddObject(OBJ_BANNERM, xp - 1, yp);
	}
	if (random_(0, monstrnd) != 0) {
		i = PreSpawnSkeleton();
		SpawnSkeleton(i, xp + 1, yp);
	} else {
		AddObject(OBJ_BANNERM, xp + 1, yp);
	}
	if (random_(0, monstrnd) != 0) {
		i = PreSpawnSkeleton();
		SpawnSkeleton(i, xp - 1, yp + 1);
	} else {
		AddObject(OBJ_BANNERR, xp - 1, yp + 1);
	}

	i = PreSpawnSkeleton();
	SpawnSkeleton(i, xp, yp + 1);

	if (random_(0, monstrnd) != 0) {
		i = PreSpawnSkeleton();
		SpawnSkeleton(i, xp + 1, yp + 1);
	} else {
		AddObject(OBJ_BANNERL, xp + 1, yp + 1);
	}

	if (dObject[xp][yp - 3] == 0) {
		AddObject(OBJ_SKELBOOK, xp, yp - 2);
	}
	if (dObject[xp][yp + 3] == 0) {
		AddObject(OBJ_SKELBOOK, xp, yp + 2);
	}
}

/**
 * Theme_Treasure initializes the treasure theme.
 *
 * @param tidx theme number (index into themes array).
 */
void Theme_Treasure(int tidx)
{
	int xp, yp, i;
	const char treasrnds[4] = { 4, 9, 7, 10 };
	const char monstrnds[4] = { 6, 8, 3, 7 };
	const char treasrnd = treasrnds[leveltype - 1];
	const char monstrnd = monstrnds[leveltype - 1];
	const char tv = themes[tidx].ttval;

	GetRndSeed();
	for (yp = 0; yp < MAXDUNY; yp++) {
		for (xp = 0; xp < MAXDUNX; xp++) {
			if (dTransVal[xp][yp] == tv && !nSolidTable[dPiece[xp][yp]]) {
				int rv = random_(0, treasrnd);
				// BUGFIX: the `2*` in `2*random_(0, treasrnd...) == 0` has no effect, should probably be `random_(0, 2*treasrnd...) == 0`
				if ((2 * random_(0, treasrnd)) == 0) {
					CreateTypeItem(xp, yp, FALSE, ITYPE_GOLD, IMISC_NONE, FALSE, TRUE);
					ItemNoFlippy();
				}
				if (rv == 0) {
					CreateRndItem(xp, yp, FALSE, FALSE, TRUE);
					ItemNoFlippy();
				}
				if (rv == 0 || rv >= treasrnd - 2) {
					i = ItemNoFlippy();
					if (rv >= treasrnd - 2 && leveltype != DTYPE_CATHEDRAL) {
						item[i]._ivalue >>= 1;
					}
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
void Theme_Library(int tidx)
{
	int xp, yp, oi;
	const char librnds[4] = { 1, 2, 2, 5 };
	const char monstrnds[4] = { 5, 7, 3, 9 };
	const char librnd = librnds[leveltype - 1];
	const char monstrnd = monstrnds[leveltype];  /// BUGFIX: `leveltype - 1`

	TFit_Shrine(tidx);

	if (themeVar1 == 1) {
		AddObject(OBJ_BOOKCANDLE, themex - 1, themey);
		AddObject(OBJ_BOOKCASER, themex, themey);
		AddObject(OBJ_BOOKCANDLE, themex + 1, themey);
	} else {
		AddObject(OBJ_BOOKCANDLE, themex, themey - 1);
		AddObject(OBJ_BOOKCASEL, themex, themey);
		AddObject(OBJ_BOOKCANDLE, themex, themey + 1);
	}

	for (yp = 1; yp < MAXDUNY - 1; yp++) {
		for (xp = 1; xp < MAXDUNX - 1; xp++) {
			if (CheckThemeObj3(xp, yp, tidx, -1) && dMonster[xp][yp] == 0 && random_(0, librnd) == 0) {
				AddObject(OBJ_BOOKSTAND, xp, yp);
				if (random_(0, 2 * librnd) != 0 && dObject[xp][yp]) { /// BUGFIX: check dObject[xp][yp] was populated by AddObject (fixed)
					oi = dObject[xp][yp] - 1;
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
void Theme_Torture(int tidx)
{
	int xp, yp;
	const char tortrnds[4] = { 6, 8, 3, 8 };
	const char monstrnds[4] = { 6, 8, 3, 9 };
	const char tortrnd = tortrnds[leveltype - 1];
	const char monstrnd = monstrnds[leveltype - 1];
	const char tv = themes[tidx].ttval;

	for (yp = 1; yp < MAXDUNY - 1; yp++) {
		for (xp = 1; xp < MAXDUNX - 1; xp++) {
			if (dTransVal[xp][yp] == tv && !nSolidTable[dPiece[xp][yp]]) {
				if (CheckThemeObj3(xp, yp, tidx, -1)) {
					if (random_(0, tortrnd) == 0) {
						AddObject(OBJ_TNUDEM2, xp, yp);
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
void Theme_BloodFountain(int tidx)
{
	const char monstrnds[4] = { 6, 8, 3, 9 };

	TFit_Obj5(tidx);
	AddObject(OBJ_BLOODFTN, themex, themey);
	PlaceThemeMonsts(tidx, monstrnds[leveltype - 1]);
}

/**
 * Theme_Decap initializes the decapitated theme.
 *
 * @param tidx theme number (index into themes array).
 */
void Theme_Decap(int tidx)
{
	int xp, yp;
	const char decaprnds[4] = { 6, 8, 3, 8 };
	const char monstrnds[4] = { 6, 8, 3, 9 };
	const char decaprnd = decaprnds[leveltype - 1];
	const char monstrnd = monstrnds[leveltype - 1];
	const char tv = themes[tidx].ttval;

	for (yp = 1; yp < MAXDUNY - 1; yp++) {
		for (xp = 1; xp < MAXDUNX - 1; xp++) {
			if (dTransVal[xp][yp] == tv && !nSolidTable[dPiece[xp][yp]]) {
				if (CheckThemeObj3(xp, yp, tidx, -1)) {
					if (random_(0, decaprnd) == 0) {
						AddObject(OBJ_DECAP, xp, yp);
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
void Theme_PurifyingFountain(int tidx)
{
	const char monstrnds[4] = { 6, 7, 3, 9 };

	TFit_Obj5(tidx);
	AddObject(OBJ_PURIFYINGFTN, themex, themey);
	PlaceThemeMonsts(tidx, monstrnds[leveltype - 1]);
}

/**
 * Theme_ArmorStand initializes the armor stand theme.
 *
 * @param tidx theme number (index into themes array).
 */
void Theme_ArmorStand(int tidx)
{
	int xp, yp;
	const char armorrnds[4] = { 6, 8, 3, 8 };
	const char monstrnds[4] = { 6, 7, 3, 9 };
	const char armorrnd = armorrnds[leveltype - 1];
	const char monstrnd = monstrnds[leveltype - 1];
	const char tv = themes[tidx].ttval;

	if (armorFlag) {
		TFit_Obj3(tidx);
		AddObject(OBJ_ARMORSTAND, themex, themey);
	}
	for (yp = 0; yp < MAXDUNY; yp++) {
		for (xp = 0; xp < MAXDUNX; xp++) {
			if (dTransVal[xp][yp] == tv && !nSolidTable[dPiece[xp][yp]]) {
				if (CheckThemeObj3(xp, yp, tidx, -1)) {
					if (random_(0, armorrnd) == 0) {
						AddObject(OBJ_ARMORSTANDN, xp, yp);
					}
				}
			}
		}
	}
	PlaceThemeMonsts(tidx, monstrnd);
	armorFlag = FALSE;
}

/**
 * Theme_GoatShrine initializes the goat shrine theme.
 *
 * @param tidx theme number (index into themes array).
 */
void Theme_GoatShrine(int tidx)
{
	int xx, yy;
	const char tv = themes[tidx].ttval;

	TFit_GoatShrine(tidx);
	AddObject(OBJ_GOATSHRINE, themex, themey);
	for (yy = themey - 1; yy <= themey + 1; yy++) {
		for (xx = themex - 1; xx <= themex + 1; xx++) {
			if (dTransVal[xx][yy] == tv && !nSolidTable[dPiece[xx][yy]] && (xx != themex || yy != themey)) {
				AddMonster(xx, yy, DIR_SW, themeVar1, TRUE);
			}
		}
	}
}

/**
 * Theme_Cauldron initializes the cauldron theme.
 *
 * @param tidx theme number (index into themes array).
 */
void Theme_Cauldron(int tidx)
{
	const char monstrnds[4] = { 6, 7, 3, 9 };

	TFit_Obj5(tidx);
	AddObject(OBJ_CAULDRON, themex, themey);
	PlaceThemeMonsts(tidx, monstrnds[leveltype - 1]);
}

/**
 * Theme_MurkyFountain initializes the murky fountain theme.
 *
 * @param tidx theme number (index into themes array).
 */
void Theme_MurkyFountain(int tidx)
{
	const char monstrnds[4] = { 6, 7, 3, 9 };

	TFit_Obj5(tidx);
	AddObject(OBJ_MURKYFTN, themex, themey);
	PlaceThemeMonsts(tidx, monstrnds[leveltype - 1]);
}

/**
 * Theme_TearFountain initializes the tear fountain theme.
 *
 * @param tidx theme number (index into themes array).
 */
void Theme_TearFountain(int tidx)
{
	const char monstrnds[4] = { 6, 7, 3, 9 };

	TFit_Obj5(tidx);
	AddObject(OBJ_TEARFTN, themex, themey);
	PlaceThemeMonsts(tidx, monstrnds[leveltype - 1]);
}

/**
 * Theme_BrnCross initializes the burning cross theme.
 *
 * @param tidx theme number (index into themes array).
 */
void Theme_BrnCross(int tidx)
{
	int xp, yp;
	const char monstrnds[4] = { 6, 8, 3, 9 };
	const char bcrossrnds[4] = { 5, 7, 3, 8 };
	const char bcrossrnd = bcrossrnds[leveltype - 1];
	const char monstrnd = monstrnds[leveltype - 1];
	const char tv = themes[tidx].ttval;

	for (yp = 0; yp < MAXDUNY; yp++) {
		for (xp = 0; xp < MAXDUNX; xp++) {
			if (dTransVal[xp][yp] == tv && !nSolidTable[dPiece[xp][yp]]) {
				if (CheckThemeObj3(xp, yp, tidx, -1)) {
					if (random_(0, bcrossrnd) == 0) {
						AddObject(OBJ_TBCROSS, xp, yp);
					}
				}
			}
		}
	}
	PlaceThemeMonsts(tidx, monstrnd);
	bCrossFlag = TRUE;
}

/**
 * Theme_WeaponRack initializes the weapon rack theme.
 *
 * @param tidx theme number (index into themes array).
 */
void Theme_WeaponRack(int tidx)
{
	int xp, yp;
	const char weaponrnds[4] = { 6, 8, 5, 8 };
	const char monstrnds[4] = { 6, 7, 3, 9 };
	const char weaponrnd = weaponrnds[leveltype - 1];
	const char monstrnd = monstrnds[leveltype - 1];
	const char tv = themes[tidx].ttval;

	if (weaponFlag) {
		TFit_Obj3(tidx);
		AddObject(OBJ_WEAPONRACK, themex, themey);
	}
	for (yp = 0; yp < MAXDUNY; yp++) {
		for (xp = 0; xp < MAXDUNX; xp++) {
			if (dTransVal[xp][yp] == tv && !nSolidTable[dPiece[xp][yp]]) {
				if (CheckThemeObj3(xp, yp, tidx, -1)) {
					if (random_(0, weaponrnd) == 0) {
						AddObject(OBJ_WEAPONRACKN, xp, yp);
					}
				}
			}
		}
	}
	PlaceThemeMonsts(tidx, monstrnd);
	weaponFlag = FALSE;
}

/**
 * UpdateL4Trans sets each value of the transparency map to 1.
 */
void UpdateL4Trans()
{
	int i, j;

	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (dTransVal[i][j] != 0) {
				dTransVal[i][j] = 1;
			}
		}
	}
}

/**
 * CreateThemeRooms adds thematic elements to rooms.
 */
void CreateThemeRooms()
{
	int i;

	if (currlevel == 16) {
		return;
	}
	InitObjFlag = TRUE;
	for (i = 0; i < numthemes; i++) {
		themex = 0;
		themey = 0;
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
		}
	}
	InitObjFlag = FALSE;
	if (leveltype == DTYPE_HELL && themeCount > 0) {
		UpdateL4Trans();
	}
}

DEVILUTION_END_NAMESPACE
