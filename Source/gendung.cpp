/**
 * @file gendung.cpp
 *
 * Implementation of general dungeon generation code.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** Contains the tile IDs of the map. */
BYTE dungeon[DMAXX][DMAXY];
/** Contains a backup of the tile IDs of the map. */
BYTE pdungeon[DMAXX][DMAXY];
/** Represents a tile ID map of twice the size, repeating each tile of the original map in blocks of 4. */
BYTE drlgFlags[DMAXX][DMAXY];
/** Specifies the active set level X-coordinate of the map. */
int setpc_x;
/** Specifies the active set level Y-coordinate of the map. */
int setpc_y;
/** Specifies the width of the active set level of the map. */
int setpc_w;
/** Specifies the height of the active set level of the map. */
int setpc_h;
/** Contains the contents of the single player quest DUN file. */
BYTE *pSetPiece = NULL;
BYTE *pSpecialCels;
/** Specifies the tile definitions of the active dungeon type; (e.g. levels/l1data/l1.til). */
BYTE *pMegaTiles;
uint16_t *pLevelPieces;
BYTE *pDungeonCels;
/**
 * The original flags of the dPieces
 */
BYTE pieceFlags[MAXTILES + 1];
/**
 * List of light blocking dPieces
 */
bool nBlockTable[MAXTILES + 1];
/**
 * List of path blocking dPieces
 */
bool nSolidTable[MAXTILES + 1];
/**
 * List of transparent dPieces
 */
bool nTransTable[MAXTILES + 1];
/**
 * List of missile blocking dPieces
 */
bool nMissileTable[MAXTILES + 1];
int gnDifficulty;
/** Contains the data of the active dungeon level. */
LevelStruct currLvl;
int MicroTileLen;
/** Specifies the number of transparency blocks on the map. */
BYTE numtrans;
/** Specifies the active transparency indices. */
bool TransList[256];
/** Contains the piece IDs of each tile on the map. */
int dPiece[MAXDUNX][MAXDUNY];
/** Specifies the dungeon piece information for a given coordinate and block number. */
MICROS dpiece_defs_map_2[MAXDUNX][MAXDUNY];
/** Specifies the transparency index at each coordinate of the map. */
BYTE dTransVal[MAXDUNX][MAXDUNY];
char dLight[MAXDUNX][MAXDUNY];
char dPreLight[MAXDUNX][MAXDUNY];
BYTE dFlags[MAXDUNX][MAXDUNY];
/**
 * Contains the player numbers (players array indices) of the map.
 *   pnum + 1 : the player is on spot
 * -(pnum + 1): reserved for a moving player
 */
char dPlayer[MAXDUNX][MAXDUNY];
/**
 * Contains the NPC numbers of the map. The NPC number represents a
 * towner number (towners array index) in Tristram and a monster number
 * (monsters array index) in the dungeon.
 *   mnum + 1 : the NPC is on spot
 * -(mnum + 1): reserved for a moving NPC
 */
int dMonster[MAXDUNX][MAXDUNY];
/**
 * Contains the dead numbers (deads array indices) and dead direction of
 * the map, encoded as specified by the pseudo-code below.
 * dDead[x][y] & 0x1F - index of dead
 * dDead[x][y] >> 0x5 - direction
 */
BYTE dDead[MAXDUNX][MAXDUNY];
/**
 * Contains the object numbers (objects array indices) of the map.
 *   oi + 1 : the object is on the given location
 * -(oi + 1): a large object protrudes from its base location
 */
char dObject[MAXDUNX][MAXDUNY];
/**
 * Contains the item numbers (items array indices) of the map.
 *   ii + 1 : the item is on the floor on the given location.
 */
BYTE dItem[MAXDUNX][MAXDUNY];
/**
 * Contains the missile numbers (missiles array indices) of the map.
 *   mi + 1 : the missile is on the given location.
 *     -1   : more than one missile on the given location.
 */
char dMissile[MAXDUNX][MAXDUNY];
/**
 * Contains the arch frame numbers of the map from the special tileset
 * (e.g. "levels/l1data/l1s.cel"). Note, the special tileset of Tristram (i.e.
 * "levels/towndata/towns.cel") contains trees rather than arches.
 */
BYTE dSpecial[MAXDUNX][MAXDUNY];
/** Specifies the number of themes generated in the dungeon. */
int themeCount;
THEME_LOC themeLoc[MAXTHEMES];

void DRLG_Init_Globals()
{
	char c;

	memset(dFlags, 0, sizeof(dFlags));
	memset(dPlayer, 0, sizeof(dPlayer));
	memset(dMonster, 0, sizeof(dMonster));
	memset(dDead, 0, sizeof(dDead));
	memset(dObject, 0, sizeof(dObject));
	memset(dItem, 0, sizeof(dItem));
	memset(dMissile, 0, sizeof(dMissile));
	memset(dSpecial, 0, sizeof(dSpecial));
	c = LIGHTMAX;
#ifdef _DEBUG
	if (lightflag)
		c = 0;
#endif
	memset(dLight, c, sizeof(dLight));
}

void InitLvlDungeon()
{
	BYTE bv;
	size_t i, dwTiles;
	BYTE *pSBFile, *pTmp;

	const LevelData *lds;
	assert(pDungeonCels == NULL);
	lds = &AllLevels[currLvl._dLevelIdx];

	pDungeonCels = LoadFileInMem(lds->dDunCels);
	pMegaTiles = LoadFileInMem(lds->dMegaTiles);
	pLevelPieces = (uint16_t *)LoadFileInMem(lds->dLvlPieces);
	pSpecialCels = LoadFileInMem(lds->dSpecCels);

#ifdef _DEBUG
	static_assert(false == 0, "InitLvlDungeon fills tables with 0 instead of false values.");
	memset(nBlockTable, 0, sizeof(nBlockTable));
	memset(nSolidTable, 0, sizeof(nSolidTable));
	memset(nTransTable, 0, sizeof(nTransTable));
	memset(nMissileTable, 0, sizeof(nMissileTable));
#endif
	pSBFile = LoadFileInMem(lds->dSolidTable, &dwTiles);

	assert(dwTiles <= MAXTILES);
	pTmp = pSBFile;

	// dpiece 0 is always black/void -> make it non-passable to reduce the necessary checks
	// no longer necessary, because dPiece is never zero
	//nSolidTable[0] = true;

	for (i = 1; i <= dwTiles; i++) {
		bv = *pTmp++;
		pieceFlags[i] = bv;
		nSolidTable[i] = (bv & PFLAG_BLOCK_PATH) != 0;
		nBlockTable[i] = (bv & PFLAG_BLOCK_LIGHT) != 0;
		nMissileTable[i] = (bv & PFLAG_BLOCK_MISSILE) != 0;
		nTransTable[i] = (bv & PFLAG_TRANSPARENT) != 0;
	}

	mem_free_dbg(pSBFile);

	// patch dSolidTable - L4.SOL
	if (currLvl._dType == DTYPE_HELL) {
		nMissileTable[141] = false;
		nSolidTable[130] = true;
		nSolidTable[132] = true;
	}
}

void FreeLvlDungeon()
{
	MemFreeDbg(pDungeonCels);
	MemFreeDbg(pMegaTiles);
	MemFreeDbg(pLevelPieces);
	MemFreeDbg(pSpecialCels);
}

void SetDungeonMicros(int x1, int y1, int x2, int y2)
{
	int i, x, y, pn, blocks;
	uint16_t *pPiece;
	MICROS *pMap;

	MicroTileLen = AllLevels[currLvl._dLevelIdx].dMicroTileLen;
	blocks = AllLevels[currLvl._dLevelIdx].dBlocks;

	for (y = y1; y < y2; y++) {
		for (x = x1; x < x2; x++) {
			pn = dPiece[x][y];
			pMap = &dpiece_defs_map_2[x][y];
			assert(pn != 0 && (unsigned)pn < MAXTILES);
			//if (pn != 0) {
				pn--;
				pPiece = &pLevelPieces[blocks * pn];
				for (i = 0; i < blocks; i++)
					pMap->mt[i] = SwapLE16(pPiece[(i & 1) + blocks - 2 - (i & 0xE)]);
			//} else {
			//	for (i = 0; i < blocks; i++)
			//		pMap->mt[i] = 0;
			//}
		}
	}
}

void DRLG_PlaceRndTile(BYTE search, BYTE replace, BYTE rndper)
{
	int i, j, rv; // rv could be BYTE, but in that case VC generates a pointless movzx...

	rv = rndper * 128 / 100; // make the life of random_ easier

	for (i = 0; i < DMAXX; i++) {
		for (j = 0; j < DMAXY; j++) {
			if (dungeon[i][j] == search && drlgFlags[i][j] == 0 && random_(0, 128) < rv) {
				dungeon[i][j] = replace;
			}
		}
	}
}

POS32 DRLG_PlaceMiniSet(const BYTE *miniset)
{
	int sx, sy, sw, sh, xx, yy, ii, tries;
	bool done;

	sw = miniset[0];
	sh = miniset[1];

	tries = 0;
	while (TRUE) {
		done = true;
		if ((tries & 0xFF) == 0) {
			sx = random_(0, DMAXX - sw);
			sy = random_(0, DMAXY - sh);
		}
		if (++tries == DMAXX * DMAXY)
			return { DMAXX, DMAXY };
		ii = 2;
		for (yy = sy; yy < sy + sh && done; yy++) {
			for (xx = sx; xx < sx + sw && done; xx++) {
				if (miniset[ii] != 0 && dungeon[xx][yy] != miniset[ii]) {
					done = false;
				}
				if (drlgFlags[xx][yy]) {
					done = false;
				}
				ii++;
			}
		}
		if (done)
			break;
		if (++sx == DMAXX - sw) {
			sx = 0;
			if (++sy == DMAXY - sh) {
				sy = 0;
			}
		}
	}

	//assert(ii == sw * sh + 2);
	for (yy = sy; yy < sy + sh; yy++) {
		for (xx = sx; xx < sx + sw; xx++) {
			if (miniset[ii] != 0) {
				dungeon[xx][yy] = miniset[ii];
			}
			ii++;
		}
	}

	return { sx, sy };
}

void DRLG_PlaceMegaTiles(int lv)
{
	int i, j, xx, yy;
	int v1, v2, v3, v4;
	uint16_t *MegaTiles;

	/*int cursor = 0;
	char tmpstr[1024];
	long lvs[] = { 22, 56, 57, 58, 59, 60, 61 };
	for (i = 0; i < lengthof(lvs); i++) {
		lv = lvs[i];
		MegaTiles = (uint16_t *)&pMegaTiles[lv * 8];
		v1 = SwapLE16(*(MegaTiles + 0)) + 1;
		v2 = SwapLE16(*(MegaTiles + 1)) + 1;
		v3 = SwapLE16(*(MegaTiles + 2)) + 1;
		v4 = SwapLE16(*(MegaTiles + 3)) + 1;
		cat_str(tmpstr, cursor, "- %d: %d, %d, %d, %d", lv, v1, v2, v3, v4);
	}
	app_fatal(tmpstr);*/

	MegaTiles = (uint16_t *)&pMegaTiles[lv * 8];
	v1 = SwapLE16(*(MegaTiles + 0)) + 1;
	v2 = SwapLE16(*(MegaTiles + 1)) + 1;
	v3 = SwapLE16(*(MegaTiles + 2)) + 1;
	v4 = SwapLE16(*(MegaTiles + 3)) + 1;

	for (j = 0; j < MAXDUNY; j += 2) {
		for (i = 0; i < MAXDUNX; i += 2) {
			dPiece[i][j] = v1;
			dPiece[i + 1][j] = v2;
			dPiece[i][j + 1] = v3;
			dPiece[i + 1][j + 1] = v4;
		}
	}

	yy = DBORDERY;
	for (j = 0; j < DMAXY; j++) {
		xx = DBORDERX;
		for (i = 0; i < DMAXX; i++) {
			lv = dungeon[i][j] - 1;
			assert(lv >= 0);
			MegaTiles = (uint16_t *)&pMegaTiles[lv * 8];
			v1 = SwapLE16(*(MegaTiles + 0)) + 1;
			v2 = SwapLE16(*(MegaTiles + 1)) + 1;
			v3 = SwapLE16(*(MegaTiles + 2)) + 1;
			v4 = SwapLE16(*(MegaTiles + 3)) + 1;
			dPiece[xx][yy] = v1;
			dPiece[xx + 1][yy] = v2;
			dPiece[xx][yy + 1] = v3;
			dPiece[xx + 1][yy + 1] = v4;
			xx += 2;
		}
		yy += 2;
	}
}

void DRLG_InitTrans()
{
	memset(dTransVal, 0, sizeof(dTransVal));
	//memset(TransList, 0, sizeof(TransList));
	numtrans = 1;
}

void DRLG_MRectTrans(int x1, int y1, int x2, int y2, int tv)
{
	int i, j;

	x1 = 2 * x1 + DBORDERX + 1;
	y1 = 2 * y1 + DBORDERY + 1;
	x2 = 2 * x2 + DBORDERX;
	y2 = 2 * y2 + DBORDERY;

	for (i = x1; i <= x2; i++) {
		for (j = y1; j <= y2; j++) {
			dTransVal[i][j] = tv;
		}
	}
}

void DRLG_RectTrans(int x1, int y1, int x2, int y2)
{
	int i, j;

	for (j = y1; j <= y2; j++) {
		for (i = x1; i <= x2; i++) {
			dTransVal[i][j] = numtrans;
		}
	}
	numtrans++;
}

void DRLG_ListTrans(int num, const BYTE *List)
{
	int i;
	BYTE x1, y1, x2, y2;

	for (i = 0; i < num; i++) {
		x1 = *List++;
		y1 = *List++;
		x2 = *List++;
		y2 = *List++;
		DRLG_RectTrans(x1, y1, x2, y2);
	}
}

void DRLG_AreaTrans(int num, const BYTE *List)
{
	int i;
	BYTE x1, y1, x2, y2;

	for (i = 0; i < num; i++) {
		x1 = *List++;
		y1 = *List++;
		x2 = *List++;
		y2 = *List++;
		DRLG_RectTrans(x1, y1, x2, y2);
		numtrans--;
	}
	numtrans++;
}

#if defined(__3DS__)
#pragma GCC push_options
#pragma GCC optimize("O0")
#endif
static BYTE TVfloor;
static void DRLG_FTVR(int i, int j, int x, int y, int dir)
{
	if (dungeon[i][j] != TVfloor) {
		switch (dir) {
		case 1:
			dTransVal[x][y] = numtrans;
			dTransVal[x][y + 1] = numtrans;
			break;
		case 2:
			dTransVal[x + 1][y] = numtrans;
			dTransVal[x + 1][y + 1] = numtrans;
			break;
		case 3:
			dTransVal[x][y] = numtrans;
			dTransVal[x + 1][y] = numtrans;
			break;
		case 4:
			dTransVal[x][y + 1] = numtrans;
			dTransVal[x + 1][y + 1] = numtrans;
			break;
		case 5:
			dTransVal[x + 1][y + 1] = numtrans;
			break;
		case 6:
			dTransVal[x][y + 1] = numtrans;
			break;
		case 7:
			dTransVal[x + 1][y] = numtrans;
			break;
		case 8:
			dTransVal[x][y] = numtrans;
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
	} else {
		if (dTransVal[x][y] != 0) {
			// assert(dTransVal[x][y] == TransVal);
			return;
		}
		dTransVal[x][y] = numtrans;
		dTransVal[x + 1][y] = numtrans;
		dTransVal[x][y + 1] = numtrans;
		dTransVal[x + 1][y + 1] = numtrans;
		DRLG_FTVR(i + 1, j, x + 2, y, 1);
		DRLG_FTVR(i - 1, j, x - 2, y, 2);
		DRLG_FTVR(i, j + 1, x, y + 2, 3);
		DRLG_FTVR(i, j - 1, x, y - 2, 4);
		DRLG_FTVR(i - 1, j - 1, x - 2, y - 2, 5);
		DRLG_FTVR(i + 1, j - 1, x + 2, y - 2, 6);
		DRLG_FTVR(i - 1, j + 1, x - 2, y + 2, 7);
		DRLG_FTVR(i + 1, j + 1, x + 2, y + 2, 8);
	}
}

void DRLG_FloodTVal(BYTE floor)
{
	int xx, yy, i, j;

	TVfloor = floor;

	yy = DBORDERY;

	for (j = 0; j < DMAXY; j++) {
		xx = DBORDERX;

		for (i = 0; i < DMAXX; i++) {
			if (dungeon[i][j] == TVfloor && dTransVal[xx][yy] == 0) {
				DRLG_FTVR(i, j, xx, yy, 0);
				numtrans++;
			}
			xx += 2;
		}
		yy += 2;
	}
}
#if defined(__3DS__)
#pragma GCC pop_options
#endif

void DRLG_InitSetPC()
{
	setpc_x = 0;
	setpc_y = 0;
	setpc_w = 0;
	setpc_h = 0;
}

void DRLG_SetPC()
{
	int i, j, x0, y0, x1, y1;

	x0 = 2 * setpc_x + DBORDERX;
	y0 = 2 * setpc_y + DBORDERY;
	x1 = 2 * setpc_w + x0;
	y1 = 2 * setpc_h + y0;

	for (j = y0; j < y1; j++) {
		for (i = x0; i < x1; i++) {
			dFlags[i][j] |= BFLAG_POPULATED;
		}
	}
}

void Make_SetPC(int x, int y, int w, int h)
{
	int i, j, x0, x1, y0, y1;

	x0 = 2 * x + DBORDERX;
	y0 = 2 * y + DBORDERY;
	x1 = 2 * w + x0;
	y1 = 2 * h + y0;

	for (j = y0; j < y1; j++) {
		for (i = x0; i < x1; i++) {
			dFlags[i][j] |= BFLAG_POPULATED;
		}
	}
}

/**
 * Find the largest available room (rectangle) starting from (x;y) using floor.
 * 
 * @param floor the id of the floor tile in dungeon
 * @param x the x-coordinate of the starting position
 * @param y the y-coordinate of the starting position
 * @param minSize the minimum size of the room (must be less than 20)
 * @param maxSize the maximum size of the room (must be less than 20)
 * @param width the width of the room if found
 * @param height the height of the room if found
 * @return true if a room is found
 */
static bool DRLG_WillThemeRoomFit(int floor, int x, int y, int minSize, int maxSize, int *width, int *height)
{
	int xmax, ymax, i, j, smallest;
	int xArray[20], yArray[20];
	int size, bestSize, w, h;

	// assert(maxSize < 20);

	xmax = std::min(maxSize, DMAXX - x);
	ymax = std::min(maxSize, DMAXY - y);
	// BUGFIX: change '&&' to '||' (fixed)
	if (xmax < minSize || ymax < minSize) {
		return false;
	}
	//if (!SkipThemeRoom(x, y)) {
	//	return false;
	//}

	memset(xArray, 0, sizeof(xArray));
	memset(yArray, 0, sizeof(yArray));

	// find horizontal(x) limits
	smallest = xmax;
	for (i = 0; i < ymax; ) {
		for (j = 0; j < smallest; j++) {
			if (dungeon[x + j][y + i] != floor) {
				smallest = j;
				break;
			}
		}
		if (smallest < minSize)
			break;
		xArray[++i] = smallest;
	}
	if (i < minSize)
		return false;

	// find vertical(y) limits
	smallest = ymax;
	for (i = 0; i < xmax; ) {
		for (j = 0; j < smallest; j++) {
			if (dungeon[x + i][y + j] != floor) {
				smallest = j;
				break;
			}
		}
		if (smallest < minSize)
			break;
		yArray[++i] = smallest;
	}
	if (i < minSize)
		return false;

	// select the best option
	xmax = std::max(xmax, ymax);
	bestSize = 0;
	for (i = minSize; i <= xmax; i++) {
		size = xArray[i] * i;
		if (size > bestSize) {
			bestSize = size;
			w = xArray[i];
			h = i;
		}
		size = yArray[i] * i;
		if (size > bestSize) {
			bestSize = size;
			w = i;
			h = yArray[i];
		}
	}
	assert(bestSize != 0);
	*width = w - 2;
	*height = h - 2;
	return true;
}

static void DRLG_CreateThemeRoom(int themeIndex)
{
	int xx, yy;
	const int lx = themeLoc[themeIndex].x;
	const int ly = themeLoc[themeIndex].y;
	const int hx = lx + themeLoc[themeIndex].width;
	const int hy = ly + themeLoc[themeIndex].height;
	BYTE v;

	// left/right side
	v = currLvl._dDunType == DTYPE_CAVES ? 137 : 1;
	for (yy = ly; yy < hy; yy++) {
		dungeon[lx][yy] = v;
		dungeon[hx - 1][yy] = v;
	}
	// top/bottom line
	v = currLvl._dDunType == DTYPE_CAVES ? 134 : 2;
	for (xx = lx; xx < hx; xx++) {
		dungeon[xx][ly] = v;
		dungeon[xx][hy - 1] = v;
	}
	// inner tiles
	v = currLvl._dDunType == DTYPE_CATACOMBS ? 3 : (currLvl._dDunType == DTYPE_CAVES ? 7 : 6);
	for (yy = ly + 1; yy < hy - 1; yy++) {
		for (xx = lx + 1; xx < hx - 1; xx++) {
			dungeon[xx][yy] = v;
		}
	}
	// corners
	if (currLvl._dDunType == DTYPE_CATACOMBS) {
		dungeon[lx][ly] = 8;
		dungeon[hx - 1][ly] = 7;
		dungeon[lx][hy - 1] = 9;
		dungeon[hx - 1][hy - 1] = 6;
	}
	if (currLvl._dDunType == DTYPE_CAVES) {
		dungeon[lx][ly] = 150;
		dungeon[hx - 1][ly] = 151;
		dungeon[lx][hy - 1] = 152;
		dungeon[hx - 1][hy - 1] = 138;
	}
	if (currLvl._dDunType == DTYPE_HELL) {
		dungeon[lx][ly] = 9;
		dungeon[hx - 1][ly] = 16;
		dungeon[lx][hy - 1] = 15;
		dungeon[hx - 1][hy - 1] = 12;
	}

	// exits
	if (currLvl._dDunType == DTYPE_CATACOMBS) {
		if (random_(0, 2) == 0) {
			dungeon[hx - 1][(ly + hy) / 2] = 4;
		} else {
			dungeon[(lx + hx) / 2][hy - 1] = 5;
		}
	}
	if (currLvl._dDunType == DTYPE_CAVES) {
		if (random_(0, 2) == 0) {
			dungeon[hx - 1][(ly + hy) / 2] = 147;
		} else {
			dungeon[(lx + hx) / 2][hy - 1] = 146;
		}
	}
	if (currLvl._dDunType == DTYPE_HELL) {
		if (random_(0, 2) == 0) {
			yy = (ly + hy) / 2;
			dungeon[hx - 1][yy - 1] = 53;
			dungeon[hx - 1][yy] = 6;
			dungeon[hx - 1][yy + 1] = 52;
			dungeon[hx - 2][yy - 1] = 54;
		} else {
			xx = (lx + hx) / 2;
			dungeon[xx - 1][hy - 1] = 57;
			dungeon[xx][hy - 1] = 6;
			dungeon[xx + 1][hy - 1] = 56;
			dungeon[xx][hy - 2] = 59;
			dungeon[xx - 1][hy - 2] = 58;
		}
	}
}

void DRLG_PlaceThemeRooms(int minSize, int maxSize, int floor, int freq, bool rndSize)
{
	int i, j;
	int themeW, themeH;
	int min;

	themeCount = 0;
	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (dungeon[i][j] == floor && random_(0, freq) == 0 && DRLG_WillThemeRoomFit(floor, i, j, minSize, maxSize, &themeW, &themeH)) {
				if (rndSize) {
					min = minSize - 2;
					themeW = RandRange(min, themeW);
					themeH = RandRange(min, themeH);
				}
				themeLoc[themeCount].x = i + 1;
				themeLoc[themeCount].y = j + 1;
				themeLoc[themeCount].width = themeW;
				themeLoc[themeCount].height = themeH;
				themeLoc[themeCount].ttval = numtrans;
				int x1 = 2 * i + DBORDERX + 3;
				int y1 = 2 * j + DBORDERY + 3;
				int x2 = 2 * (i + themeW) + DBORDERX;
				int y2 = 2 * (j + themeH) + DBORDERY;
				if (currLvl._dDunType == DTYPE_CAVES) {
					x1++;
					y1++;
					x2--;
					y2--;
				}
				DRLG_RectTrans(x1, y1, x2, y2);
				DRLG_CreateThemeRoom(themeCount);
				themeCount++;
			}
		}
	}
}

void DRLG_HoldThemeRooms()
{
	int i, x, y, xx, yy;

	for (i = 0; i < themeCount; i++) {
		for (y = themeLoc[i].y; y < themeLoc[i].y + themeLoc[i].height; y++) {
			for (x = themeLoc[i].x; x < themeLoc[i].x + themeLoc[i].width; x++) {
				xx = 2 * x + DBORDERX;
				yy = 2 * y + DBORDERY;
				dFlags[xx][yy] |= BFLAG_POPULATED;
				dFlags[xx + 1][yy] |= BFLAG_POPULATED;
				dFlags[xx][yy + 1] |= BFLAG_POPULATED;
				dFlags[xx + 1][yy + 1] |= BFLAG_POPULATED;
			}
		}
	}
}

bool SkipThemeRoom(int x, int y)
{
	int i;

	for (i = 0; i < themeCount; i++) {
		if (x >= themeLoc[i].x - 2 && x < themeLoc[i].x + themeLoc[i].width + 2
		    && y >= themeLoc[i].y - 2 && y < themeLoc[i].y + themeLoc[i].height + 2)
			return false;
	}

	return true;
}

DEVILUTION_END_NAMESPACE
