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
/*
 * The micros of the dPieces
 */
MICROS pMicroPieces[MAXTILES + 1];
/*
 * Micro images CEL
 */
BYTE* pMicroCels;
/**
 * Flags to control the drawing of dPieces
 */
BYTE microFlags[MAXTILES + 1];
/**
 * List of light blocking dPieces
 */
bool nBlockTable[MAXTILES + 1];
/**
 * List of path blocking dPieces
 */
bool nSolidTable[MAXTILES + 1];
/**
 * List of trap-source dPieces
 */
bool nTrapTable[MAXTILES + 1];
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
/** Specifies the transparency index at each coordinate of the map. */
BYTE dTransVal[MAXDUNX][MAXDUNY];
/** Specifies the base darkness levels of each tile on the map. */
BYTE dPreLight[MAXDUNX][MAXDUNY];
/** Specifies the current darkness levels of each tile on the map. */
BYTE dLight[MAXDUNX][MAXDUNY];
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
 * MIS_MULTI: more than one missile on the given location.
 */
static_assert(MAXMISSILES < UCHAR_MAX, "Index of a missile might not fit to dMissile.");
static_assert((BYTE)(MAXMISSILES + 1) < (BYTE)MIS_MULTI, "Multi-missile in dMissile reserves one entry.");
BYTE dMissile[MAXDUNX][MAXDUNY];
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
	BYTE c;

	memset(dFlags, 0, sizeof(dFlags));
	memset(dPlayer, 0, sizeof(dPlayer));
	memset(dMonster, 0, sizeof(dMonster));
	memset(dDead, 0, sizeof(dDead));
	memset(dObject, 0, sizeof(dObject));
	memset(dItem, 0, sizeof(dItem));
	memset(dMissile, 0, sizeof(dMissile));
	memset(dSpecial, 0, sizeof(dSpecial));
	c = MAXDARKNESS;
#if DEBUG_MODE
	if (lightflag)
		c = 0;
#endif
	memset(dLight, c, sizeof(dLight));
}

void InitLvlDungeon()
{
	BYTE bv, blocks;
	size_t i, dwTiles;
	BYTE *pSBFile, *pTmp;
	uint16_t *pLPFile, *pPiece, *pPTmp;

	const LevelData *lds;
	assert(pMicroCels == NULL);
	lds = &AllLevels[currLvl._dLevelIdx];

	pMicroCels = LoadFileInMem(lds->dMicroCels);
	assert(pMegaTiles == NULL);
	pMegaTiles = LoadFileInMem(lds->dMegaTiles);
	assert(pSpecialCels == NULL);
	pSpecialCels = LoadFileInMem(lds->dSpecCels);
	MicroTileLen = lds->dMicroTileLen;
	LoadFileWithMem(lds->dMicroFlags, microFlags);

	pLPFile = (uint16_t *)LoadFileInMem(lds->dMiniTiles, &dwTiles);

	blocks = lds->dBlocks;
	dwTiles /= (2 * blocks);
	assert(dwTiles <= MAXTILES);

	for (i = 1; i <= dwTiles; i++) {
		pPTmp = pMicroPieces[i].mt;
		pPiece = &pLPFile[blocks * i];
		for (bv = 0; bv < blocks; bv += 2) {
			pPiece -= 2;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			pPTmp[0] = SwapLE16(pPiece[0]);
			pPTmp[1] = SwapLE16(pPiece[1]);
#else
			*((uint32_t*)pPTmp) = *((uint32_t*)pPiece);
#endif			
			pPTmp += 2;
		}
	}

	mem_free_dbg(pLPFile);

#if DEBUG_MODE
	static_assert(false == 0, "InitLvlDungeon fills tables with 0 instead of false values.");
	memset(nBlockTable, 0, sizeof(nBlockTable));
	memset(nSolidTable, 0, sizeof(nSolidTable));
	memset(nTrapTable, 0, sizeof(nTrapTable));
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
		nSolidTable[i] = (bv & PFLAG_BLOCK_PATH) != 0;
		nBlockTable[i] = (bv & PFLAG_BLOCK_LIGHT) != 0;
		nMissileTable[i] = (bv & PFLAG_BLOCK_MISSILE) != 0;
		nTrapTable[i] = (bv & PFLAG_TRAP_SOURCE) != 0;
	}

	mem_free_dbg(pSBFile);

	switch (currLvl._dType) {
	case DTYPE_TOWN:
		// patch dSolidTable - Town.SOL
		nSolidTable[553] = false; // allow walking on the left side of the pot at Adria
		nSolidTable[761] = true; // make the tile of the southern window of the church non-walkable
		nSolidTable[945] = true; // make the eastern side of Griswold's house consistent (non-walkable)

		// patch dMiniTiles - Town.MIN
		// pointless tree micros (re-drawn by dSpecial)
		pMicroPieces[117].mt[3] = 0;
		pMicroPieces[117].mt[5] = 0;
		pMicroPieces[128].mt[2] = 0;
		pMicroPieces[128].mt[4] = 0;
		pMicroPieces[128].mt[6] = 0;
		pMicroPieces[129].mt[3] = 0;
		pMicroPieces[129].mt[5] = 0;
		pMicroPieces[129].mt[7] = 0;
		pMicroPieces[130].mt[2] = 0;
		pMicroPieces[130].mt[4] = 0;
		pMicroPieces[130].mt[6] = 0;
		pMicroPieces[156].mt[2] = 0;
		pMicroPieces[156].mt[3] = 0;
		pMicroPieces[156].mt[4] = 0;
		pMicroPieces[156].mt[5] = 0;
		pMicroPieces[156].mt[6] = 0;
		pMicroPieces[156].mt[7] = 0;
		pMicroPieces[156].mt[8] = 0;
		pMicroPieces[156].mt[9] = 0;
		pMicroPieces[156].mt[10] = 0;
		pMicroPieces[156].mt[11] = 0;
		pMicroPieces[157].mt[3] = 0;
		pMicroPieces[157].mt[5] = 0;
		pMicroPieces[157].mt[7] = 0;
		pMicroPieces[157].mt[9] = 0;
		pMicroPieces[157].mt[11] = 0;
		pMicroPieces[158].mt[2] = 0;
		pMicroPieces[158].mt[4] = 0;
		pMicroPieces[160].mt[2] = 0;
		pMicroPieces[160].mt[3] = 0;
		pMicroPieces[160].mt[4] = 0;
		pMicroPieces[160].mt[5] = 0;
		pMicroPieces[160].mt[6] = 0;
		pMicroPieces[160].mt[7] = 0;
		pMicroPieces[160].mt[8] = 0;
		pMicroPieces[160].mt[9] = 0;
		pMicroPieces[162].mt[2] = 0;
		pMicroPieces[162].mt[4] = 0;
		pMicroPieces[162].mt[6] = 0;
		pMicroPieces[162].mt[8] = 0;
		pMicroPieces[162].mt[10] = 0;
		pMicroPieces[212].mt[3] = 0;
		pMicroPieces[212].mt[4] = 0;
		pMicroPieces[212].mt[5] = 0;
		pMicroPieces[212].mt[6] = 0;
		pMicroPieces[212].mt[7] = 0;
		pMicroPieces[212].mt[8] = 0;
		pMicroPieces[212].mt[9] = 0;
		pMicroPieces[212].mt[10] = 0;
		pMicroPieces[212].mt[11] = 0;
		//pMicroPieces[214].mt[4] = 0;
		//pMicroPieces[214].mt[6] = 0;
		pMicroPieces[216].mt[2] = 0;
		pMicroPieces[216].mt[4] = 0;
		pMicroPieces[216].mt[6] = 0;
		//pMicroPieces[217].mt[4] = 0;
		//pMicroPieces[217].mt[6] = 0;
		//pMicroPieces[217].mt[8] = 0;
		//pMicroPieces[358].mt[4] = 0;
		//pMicroPieces[358].mt[5] = 0;
		//pMicroPieces[358].mt[6] = 0;
		//pMicroPieces[358].mt[7] = 0;
		//pMicroPieces[358].mt[8] = 0;
		//pMicroPieces[358].mt[9] = 0;
		//pMicroPieces[358].mt[10] = 0;
		//pMicroPieces[358].mt[11] = 0;
		//pMicroPieces[358].mt[12] = 0;
		//pMicroPieces[358].mt[13] = 0;
		//pMicroPieces[360].mt[4] = 0;
		//pMicroPieces[360].mt[6] = 0;
		//pMicroPieces[360].mt[8] = 0;
		//pMicroPieces[360].mt[10] = 0;
		// fix bad artifact
		pMicroPieces[233].mt[6] = 0;
		// useless black micros
		pMicroPieces[426].mt[1] = 0;
		pMicroPieces[427].mt[0] = 0;
		pMicroPieces[427].mt[1] = 0;
		pMicroPieces[429].mt[1] = 0;
		// fix bad artifacts
		pMicroPieces[828].mt[12] = 0;
		pMicroPieces[828].mt[13] = 0;
		pMicroPieces[1018].mt[2] = 0;
		// useless black micros
		pMicroPieces[1143].mt[0] = 0;
		pMicroPieces[1145].mt[0] = 0;
		pMicroPieces[1145].mt[1] = 0;
		pMicroPieces[1146].mt[0] = 0;
		pMicroPieces[1153].mt[0] = 0;
		pMicroPieces[1155].mt[1] = 0;
		pMicroPieces[1156].mt[0] = 0;
		pMicroPieces[1169].mt[1] = 0;
		pMicroPieces[1170].mt[0] = 0;
		pMicroPieces[1170].mt[1] = 0;
		pMicroPieces[1172].mt[1] = 0;
		pMicroPieces[1176].mt[1] = 0;
		pMicroPieces[1199].mt[1] = 0;
		pMicroPieces[1200].mt[0] = 0;
		pMicroPieces[1200].mt[1] = 0;
		pMicroPieces[1202].mt[1] = 0;
		pMicroPieces[1203].mt[1] = 0;
		pMicroPieces[1205].mt[1] = 0;
		pMicroPieces[1212].mt[0] = 0;
		pMicroPieces[1219].mt[0] = 0;
#ifdef HELLFIRE
		// fix bad artifact
		pMicroPieces[1273].mt[7] = 0;
#endif
		break;
	case DTYPE_CATHEDRAL:
		// patch dSolidTable - L1.SOL
		nMissileTable[8] = false; // the only column which was blocking missiles
		// patch dMiniTiles - L1.MIN
		// useless black micros
		pMicroPieces[107].mt[0] = 0;
		pMicroPieces[107].mt[1] = 0;
		pMicroPieces[109].mt[1] = 0;
		pMicroPieces[137].mt[1] = 0;
		pMicroPieces[138].mt[0] = 0;
		pMicroPieces[138].mt[1] = 0;
		pMicroPieces[140].mt[1] = 0;
		break;
	case DTYPE_CATACOMBS:
		// patch dSolidTable - L2.SOL
		// enable torches on (southern) walls
		// nTrapTable[37] = true;
		// nTrapTable[41] = true;
		nTrapTable[520] = true;
		nTrapTable[522] = true;
		nTrapTable[524] = true;
		nTrapTable[526] = true;
		break;
	case DTYPE_CAVES:
		// patch dMiniTiles - L3.MIN
		// fix bad artifact
		pMicroPieces[82].mt[4] = 0;
		break;
	case DTYPE_HELL:
		// patch dSolidTable - L4.SOL
		nMissileTable[141] = false; // fix missile-blocking tile of down-stairs.
		nSolidTable[130] = true; // make the inner tiles of the down-stairs non-walkable I.
		nSolidTable[132] = true; // make the inner tiles of the down-stairs non-walkable II.
		// fix all-blocking tile on the diablo-level
		nSolidTable[211] = false;
		nMissileTable[211] = false;
		nBlockTable[211] = false;
		break;
#ifdef HELLFIRE
	case DTYPE_NEST:
		// patch dSolidTable - L6.SOL
		nSolidTable[390] = false; // make a pool tile walkable I.
		nSolidTable[413] = false; // make a pool tile walkable II.
		nSolidTable[416] = false; // make a pool tile walkable III.
		// patch dMiniTiles - L6.MIN
		// useless black micros
		pMicroPieces[21].mt[0] = 0;
		pMicroPieces[21].mt[1] = 0;
		// fix bad artifacts
		pMicroPieces[132].mt[7] = 0;
		pMicroPieces[366].mt[1] = 0;
		break;
	case DTYPE_CRYPT:
		// patch dSolidTable - L5.SOL
		nSolidTable[148] = false; // make the back of down-stairs consistent (walkable)
		// make collision-checks more reasonable
		//  - prevent non-crossable floor-tile configurations I.
		nSolidTable[461] = false;
		//  - set top right tile of an arch non-walkable (full of lava)
		//nSolidTable[471] = true;
		//  - set top right tile of a pillar walkable (just a small obstacle)
		nSolidTable[481] = false;
		//  - tile 491 is the same as tile 594 which is not solid
		//  - prevents non-crossable floor-tile configurations
		nSolidTable[491] = false;
		//  - set bottom left tile of a rock non-walkable (rather large obstacle, feet of the hero does not fit)
		//  - prevents non-crossable floor-tile configurations
		nSolidTable[523] = true;
		//  - set the top right tile of a floor mega walkable (similar to 594 which is not solid)
		nSolidTable[570] = false;
		//  - prevent non-crossable floor-tile configurations II.
		nSolidTable[598] = false;
		nSolidTable[600] = false;
		// patch dMiniTiles - L5.MIN
		// useless black micros
		pMicroPieces[130].mt[0] = 0;
		pMicroPieces[130].mt[1] = 0;
		pMicroPieces[132].mt[1] = 0;
		pMicroPieces[134].mt[0] = 0;
		pMicroPieces[134].mt[1] = 0;
		pMicroPieces[149].mt[0] = 0;
		pMicroPieces[149].mt[1] = 0;
		pMicroPieces[149].mt[2] = 0;
		pMicroPieces[150].mt[0] = 0;
		pMicroPieces[150].mt[1] = 0;
		pMicroPieces[150].mt[2] = 0;
		pMicroPieces[150].mt[4] = 0;
		pMicroPieces[151].mt[0] = 0;
		pMicroPieces[151].mt[1] = 0;
		pMicroPieces[151].mt[3] = 0;
		pMicroPieces[152].mt[0] = 0;
		pMicroPieces[152].mt[1] = 0;
		pMicroPieces[152].mt[3] = 0;
		pMicroPieces[152].mt[5] = 0;
		pMicroPieces[153].mt[0] = 0;
		pMicroPieces[153].mt[1] = 0;
		// fix bad artifact
		pMicroPieces[156].mt[2] = 0;
		// useless black micros
		pMicroPieces[172].mt[0] = 0;
		pMicroPieces[172].mt[1] = 0;
		pMicroPieces[172].mt[2] = 0;
		pMicroPieces[173].mt[0] = 0;
		pMicroPieces[173].mt[1] = 0;
		pMicroPieces[174].mt[0] = 0;
		pMicroPieces[174].mt[1] = 0;
		pMicroPieces[174].mt[2] = 0;
		pMicroPieces[174].mt[4] = 0;
		pMicroPieces[175].mt[0] = 0;
		pMicroPieces[175].mt[1] = 0;
		pMicroPieces[176].mt[0] = 0;
		pMicroPieces[176].mt[1] = 0;
		pMicroPieces[176].mt[3] = 0;
		pMicroPieces[177].mt[0] = 0;
		pMicroPieces[177].mt[1] = 0;
		pMicroPieces[177].mt[3] = 0;
		pMicroPieces[177].mt[5] = 0;
		pMicroPieces[178].mt[0] = 0;
		pMicroPieces[178].mt[1] = 0;
		pMicroPieces[179].mt[0] = 0;
		pMicroPieces[179].mt[1] = 0;
		break;
#endif
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

void FreeLvlDungeon()
{
	MemFreeDbg(pMicroCels);
	MemFreeDbg(pMegaTiles);
	MemFreeDbg(pSpecialCels);
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
	//if (NearThemeRoom(x, y)) {
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

bool NearThemeRoom(int x, int y)
{
	int i;

	for (i = 0; i < themeCount; i++) {
		if (x >= themeLoc[i].x - 2 && x < themeLoc[i].x + themeLoc[i].width + 2
		    && y >= themeLoc[i].y - 2 && y < themeLoc[i].y + themeLoc[i].height + 2)
			return true;
	}

	return false;
}

DEVILUTION_END_NAMESPACE
