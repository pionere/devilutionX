/**
 * @file gendung.cpp
 *
 * Implementation of general dungeon generation code.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** Contains the mega tile IDs of the (mega-)map. */
BYTE dungeon[DMAXX][DMAXY];
/** Contains a backup of the mega tile IDs of the (mega-)map. */
BYTE pdungeon[DMAXX][DMAXY];
/** Flags of mega tiles during dungeon generation. */
BYTE drlgFlags[DMAXX][DMAXY];
/** Container for large, temporary entities during dungeon generation. */
DrlgMem drlg;
/**
 * Contains information about the set pieces of the map.
 * pData contains the content of the .DUN file.
 *  - First the post version, at the end of CreateLevel the pre version is loaded.
 *  - this is not available after the player enters the level.
 */
SetPieceStruct pSetPieces[4];
/** List of the warp-points on the current level */
WarpStruct pWarps[NUM_DWARP];
/** Specifies the tiles (groups of four subtiles). */
static uint16_t pTiles[MAXTILES + 1][4];
/*
 * The micros of the subtiles
 */
uint16_t pSubtiles[MAXSUBTILES + 1][16 * ASSET_MPL * ASSET_MPL];
/** Images of the subtile-micros. */
BYTE* pMicrosCel;
/** Images of the special subtiles. */
BYTE* pSpecialsCel;
/**
 * Flags to control the drawing of a subtile (piece_micro_flag)
 */
BYTE microFlags[MAXSUBTILES + 1];
/**
 * List of light blocking dPieces
 */
bool nBlockTable[MAXSUBTILES + 1];
/**
 * List of path blocking dPieces
 */
bool nSolidTable[MAXSUBTILES + 1];
/**
 * Flags of subtiles to specify trap-sources (_piece_trap_type) and special cel-frames
 */
BYTE nSpecTrapTable[MAXSUBTILES + 1];
/**
 * List of missile blocking dPieces
 */
bool nMissileTable[MAXSUBTILES + 1];
/** The difficuly level of the current game (_difficulty) */
int gnDifficulty;
/** Contains the data of the active dungeon level. */
LevelStruct currLvl;
int MicroTileLen;
/** Specifies the number of transparency blocks on the map. */
BYTE numtrans;
/* Specifies whether the transvals should be re-processed. */
static bool gbDoTransVals;
/** Specifies the active transparency indices. */
bool TransList[256];
/** Contains the subtile IDs of each square on the map. */
int dPiece[MAXDUNX][MAXDUNY];
/** Specifies the transparency index of each square on the map. */
BYTE dTransVal[MAXDUNX][MAXDUNY];
/** Specifies the base darkness levels of each square on the map. */
BYTE dPreLight[MAXDUNX][MAXDUNY];
/** Specifies the current darkness levels of each square on the map. */
BYTE dLight[MAXDUNX][MAXDUNY];
/** Specifies the (runtime) flags of each square on the map (dflag) */
BYTE dFlags[MAXDUNX][MAXDUNY];
/**
 * Contains the player numbers (players array indices) of the map.
 *   pnum + 1 : the player is on the given location.
 * -(pnum + 1): reserved for a moving player
 */
int8_t dPlayer[MAXDUNX][MAXDUNY];
static_assert(MAX_PLRS <= CHAR_MAX, "Index of a player might not fit to dPlayer.");
/**
 * Contains the NPC numbers of the map. The NPC number represents a
 * towner number (towners array index) in Tristram and a monster number
 * (monsters array index) in the dungeon.
 *   mnum + 1 : the NPC is on the given location.
 * -(mnum + 1): reserved for a moving NPC
 */
int dMonster[MAXDUNX][MAXDUNY];
/**
 * Contains the dead NPC numbers of the map (only monsters at the moment).
 *   mnum + 1 : the NPC corpse is on the given location.
 *  DEAD_MULTI: more than one corpse on the given location.
 */
BYTE dDead[MAXDUNX][MAXDUNY];
static_assert(MAXMONSTERS <= UCHAR_MAX, "Index of a monster might not fit to dDead.");
static_assert((BYTE)(MAXMONSTERS + 1) < (BYTE)DEAD_MULTI, "Multi-dead in dDead reserves one entry.");
/**
 * Contains the object numbers (objects array indices) of the map.
 *   oi + 1 : the object is on the given location.
 * -(oi + 1): a large object protrudes from its base location.
 */
int8_t dObject[MAXDUNX][MAXDUNY];
static_assert(MAXOBJECTS <= CHAR_MAX, "Index of an object might not fit to dObject.");
/**
 * Contains the item numbers (items array indices) of the map.
 *   ii + 1 : the item is on the floor on the given location.
 */
BYTE dItem[MAXDUNX][MAXDUNY];
static_assert(MAXITEMS <= UCHAR_MAX, "Index of an item might not fit to dItem.");
/**
 * Contains the missile numbers (missiles array indices) of the map.
 *   mi + 1 : the missile is on the given location.
 * MIS_MULTI: more than one missile on the given location.
 */
BYTE dMissile[MAXDUNX][MAXDUNY];
static_assert(MAXMISSILES <= UCHAR_MAX, "Index of a missile might not fit to dMissile.");
static_assert((BYTE)(MAXMISSILES + 1) < (BYTE)MIS_MULTI, "Multi-missile in dMissile reserves one entry.");

void InitLvlDungeon()
{
	uint16_t bv;
	size_t dwSubtiles;
	BYTE *solFile, *pTmp;
#if ASSET_MPL == 1
	uint16_t blocks, *minFile, *pSubtile, *pPTmp;
#endif
	const LevelData* lds = &AllLevels[currLvl._dLevelIdx];
	const LevelFileData* lfd = &levelfiledata[lds->dfindex];

	static_assert((int)WRPT_NONE == 0, "InitLvlDungeon fills pWarps with 0 instead of WRPT_NONE values.");
	memset(pWarps, 0, sizeof(pWarps));
	static_assert((int)SPT_NONE == 0, "InitLvlDungeon fills pSetPieces with 0 instead of SPT_NONE values.");
	memset(pSetPieces, 0, sizeof(pSetPieces));

	assert(pMicrosCel == NULL);
	pMicrosCel = LoadFileInMem(lfd->dMicroCels); // .CEL
	if (lfd->dMegaTiles != NULL) { 
		LoadFileWithMem(lfd->dMegaTiles, (BYTE*)&pTiles[1][0]); // .TIL
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		for (int i = 1; i < lengthof(pTiles); i++) {
			for (bv = 0; bv < lengthof(pTiles[0]); bv++) {
				pTiles[i][bv] = SwapLE16(pTiles[i][bv]);
			}
		}
#endif
		for (int i = 1; i < lengthof(pTiles); i++) {
			for (bv = 0; bv < lengthof(pTiles[0]); bv++) {
				pTiles[i][bv] = pTiles[i][bv] + 1;
			}
		}
	}
	assert(pSpecialsCel == NULL);
	if (lfd->dSpecCels != NULL) {
		pSpecialsCel = LoadFileInMem(lfd->dSpecCels); // s.CEL
	}
	MicroTileLen = lds->dMicroTileLen * ASSET_MPL * ASSET_MPL;
	LoadFileWithMem(lfd->dMicroFlags, microFlags); // .TMI
#if ASSET_MPL == 1
	minFile = (uint16_t*)LoadFileInMem(lfd->dMiniTiles, &dwSubtiles); // .MIN

	blocks = lds->dBlocks;
	dwSubtiles /= (2 * blocks);
	assert(dwSubtiles <= MAXSUBTILES);

	for (unsigned i = 1; i <= dwSubtiles; i++) {
		pPTmp = &pSubtiles[i][0];
		pSubtile = &minFile[blocks * i];
		for (bv = 0; bv < blocks; bv += 2) {
			pSubtile -= 2;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			pPTmp[0] = SwapLE16(pSubtile[0]);
			pPTmp[1] = SwapLE16(pSubtile[1]);
#else
			*((uint32_t*)pPTmp) = *((uint32_t*)pSubtile);
#endif
			pPTmp += 2;
		}
	}

	mem_free_dbg(minFile);
#else
	LoadFileWithMem(lfd->dMiniTiles, (BYTE*)&pSubtiles[1][0]);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	for (int i = 1; i < lengthof(pSubtiles); i++) {
		for (bv = 0; bv < lengthof(pSubtiles[0]); bv++) {
			pSubtiles[i][bv] = SwapLE16(pSubtiles[i][bv]);
		}
	}
#endif
#endif /* ASSET_MPL == 1 */
	LoadFileWithMem(lfd->dSpecFlags, &nSpecTrapTable[0]); // .SPT
#if DEBUG_MODE
	static_assert(false == 0, "InitLvlDungeon fills tables with 0 instead of false values.");
	memset(nBlockTable, 0, sizeof(nBlockTable));
	memset(nSolidTable, 0, sizeof(nSolidTable));
	memset(nMissileTable, 0, sizeof(nMissileTable));
#endif
	solFile = LoadFileInMem(lfd->dSolidTable, &dwSubtiles); // .SOL

	assert(dwSubtiles <= MAXSUBTILES);
	pTmp = solFile;

	// dpiece 0 is always black/void -> make it non-passable to reduce the necessary checks
	// no longer necessary, because dPiece is never zero
	//nSolidTable[0] = true;

	for (unsigned i = 1; i <= dwSubtiles; i++) {
		bv = *pTmp++;
		nSolidTable[i] = (bv & PFLAG_BLOCK_PATH) != 0;
		nBlockTable[i] = (bv & PFLAG_BLOCK_LIGHT) != 0;
		nMissileTable[i] = (bv & PFLAG_BLOCK_MISSILE) != 0;
	}

	mem_free_dbg(solFile);

	switch (currLvl._dType) {
	case DTYPE_TOWN:
#if !USE_PATCH
		// patch dSolidTable - Town.SOL
		// nSolidTable[553] = false; // allow walking on the left side of the pot at Adria
		// nSolidTable[761] = true;  // make the tile of the southern window of the church non-walkable
		// nSolidTable[945] = true;  // make the eastern side of Griswold's house consistent (non-walkable)
#if ASSET_MPL == 1
		// patch dMicroCels - TOWN.CEL
		// - overwrite subtile 237 with subtile 402 to make the inner tile of Griswold's house non-walkable
		pSubtiles[237][0] = pSubtiles[402][0];
		pSubtiles[237][1] = pSubtiles[402][1];
#endif
#endif
		break;
	case DTYPE_CATHEDRAL:
#if !USE_PATCH
		// patch dSolidTable - L1.SOL
		nMissileTable[8] = false; // the only column which was blocking missiles
#endif
		break;
	case DTYPE_CATACOMBS:
#if !USE_PATCH
		// patch dMegaTiles, dMiniTiles and dSolidTable - L2.TIL, L2.MIN, L2.SOL
		// reuse subtiles
		assert(pTiles[41][1] == 139 || pTiles[41][1] == 135);
		pTiles[41][1] = 135;
		// add separate tiles and subtiles for the arches
		// - floor tile(3) with vertical arch
		pTiles[161][0] = 560;
		pTiles[161][1] = 10;
		pTiles[161][2] = 561;
		pTiles[161][3] = 12;
		// - floor tile(3) with horizontal arch
		pTiles[162][0] = 562;
		pTiles[162][1] = 563;
		pTiles[162][2] = 11;
		pTiles[162][3] = 12;
		// - floor tile with shadow(49) with vertical arch
		pTiles[163][0] = 564; // - 159
		pTiles[163][1] = 160;
		pTiles[163][2] = 565; // - 161
		pTiles[163][3] = 162;
		// - floor tile with shadow(51) with horizontal arch
		pTiles[164][0] = 566; // - 166
		pTiles[164][1] = 567; // - 167
		pTiles[164][2] = 168;
		pTiles[164][3] = 169;
		pSubtiles[560][0] = pSubtiles[9][0];
		pSubtiles[560][1] = pSubtiles[9][1];
		pSubtiles[561][0] = pSubtiles[11][0];
		pSubtiles[561][1] = pSubtiles[11][1];
		pSubtiles[562][0] = pSubtiles[9][0];
		pSubtiles[562][1] = pSubtiles[9][1];
		pSubtiles[563][0] = pSubtiles[10][0];
		pSubtiles[563][1] = pSubtiles[10][1];
		pSubtiles[564][0] = pSubtiles[159][0];
		pSubtiles[564][1] = pSubtiles[159][1];
		pSubtiles[565][0] = pSubtiles[161][0];
		pSubtiles[565][1] = pSubtiles[161][1];
		pSubtiles[566][0] = pSubtiles[166][0];
		pSubtiles[566][1] = pSubtiles[166][1];
		pSubtiles[567][0] = pSubtiles[167][0];
		pSubtiles[567][1] = pSubtiles[167][1];
		// - reset flags of the 'new' floor tiles with arches
		for (int ii = 0; ii < 8; ii++) {
			nSolidTable[559 + ii] = false;
			nBlockTable[559 + ii] = false;
			nMissileTable[559 + ii] = false;
		}
		// make the back of the stairs non-walkable
		pTiles[72][1] = 56;
		nSolidTable[252] = true;
		nBlockTable[252] = true;
		nMissileTable[252] = true;
#endif // !USE_PATCH
		break;
	case DTYPE_CAVES:
#if !USE_PATCH
		// patch dSolidTable - L3.SOL
		nSolidTable[249] = false; // sync tile 68 and 69 by making subtile 249 of tile 68 walkable.
#endif
		break;
	case DTYPE_HELL:
#if !USE_PATCH
		// patch dSolidTable - L4.SOL
		nMissileTable[141] = false; // fix missile-blocking tile of down-stairs.
		nMissileTable[137] = false; // fix missile-blocking tile of down-stairs.
		nSolidTable[137] = false;   // fix non-walkable tile of down-stairs. - causes a graphic glitch, but keep in sync with patch users
		nSolidTable[130] = true;    // make the inner tiles of the down-stairs non-walkable I.
		nSolidTable[132] = true;    // make the inner tiles of the down-stairs non-walkable II.
		nSolidTable[131] = true;    // make the inner tiles of the down-stairs non-walkable III.
		// fix all-blocking tile on the diablo-level
		nSolidTable[211] = false;
		nMissileTable[211] = false;
		nBlockTable[211] = false;
#endif
		break;
#ifdef HELLFIRE
	case DTYPE_NEST:
#if !USE_PATCH
		// patch dSolidTable - L6.SOL
		nSolidTable[390] = false; // make a pool tile walkable I.
		nSolidTable[413] = false; // make a pool tile walkable II.
		nSolidTable[416] = false; // make a pool tile walkable III.
#endif
		break;
	case DTYPE_CRYPT:
#if !USE_PATCH
		// patch dSolidTable - L5.SOL
		nSolidTable[143] = false; // make right side of down-stairs consistent (walkable)
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
#endif
		break;
#endif /* HELLFIRE */
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

void FreeSetPieces()
{
	for (int i = 0; i < lengthof(pSetPieces); i++) {
		MemFreeDbg(pSetPieces[i]._spData);
	}
}

void FreeLvlDungeon()
{
	MemFreeDbg(pMicrosCel);
	MemFreeDbg(pSpecialsCel);
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

POS32 DRLG_PlaceMiniSet(const BYTE* miniset)
{
	int sx, sy, sw, sh, xx, yy, ii, tries;
	bool done;

	sw = miniset[0];
	sh = miniset[1];
	// assert(sw < DMAXX && sh < DMAXY);
	tries = (DMAXX * DMAXY) & ~0xFF;
	while (TRUE) {
		if ((tries & 0xFF) == 0) {
			sx = random_low(0, DMAXX - sw);
			sy = random_low(0, DMAXY - sh);
		}
		if (--tries == 0)
			return { -1, 0 };
		ii = 2;
		done = true;
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

void DRLG_PlaceMegaTiles(int mt)
{
	int i, j, xx, yy;
	int v1, v2, v3, v4;
	uint16_t* pTile;

	/*int cursor = 0;
	char tmpstr[1024];
	long lvs[] = { 22, 56, 57, 58, 59, 60, 61 };
	for (i = 0; i < lengthof(lvs); i++) {
		lv = lvs[i];
		pTile = &pTiles[mt][0];
		v1 = pTile[0];
		v2 = pTile[1];
		v3 = pTile[2];
		v4 = pTile[3];
		cat_str(tmpstr, cursor, "- %d: %d, %d, %d, %d", lv, v1, v2, v3, v4);
	}
	app_fatal(tmpstr);*/

	pTile = &pTiles[mt][0];
	v1 = pTile[0];
	v2 = pTile[1];
	v3 = pTile[2];
	v4 = pTile[3];

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
			mt = dungeon[i][j];
			assert(mt > 0);
			pTile = &pTiles[mt][0];
			v1 = pTile[0];
			v2 = pTile[1];
			v3 = pTile[2];
			v4 = pTile[3];
			dPiece[xx][yy] = v1;
			dPiece[xx + 1][yy] = v2;
			dPiece[xx][yy + 1] = v3;
			dPiece[xx + 1][yy + 1] = v4;
			xx += 2;
		}
		yy += 2;
	}
}

void DRLG_DrawMiniSet(const BYTE* miniset, int sx, int sy)
{
	int xx, yy, sh, sw, ii;

	sw = miniset[0];
	sh = miniset[1];

	ii = sw * sh + 2;

	for (yy = sy; yy < sy + sh; yy++) {
		for (xx = sx; xx < sx + sw; xx++) {
			if (miniset[ii] != 0) {
				dungeon[xx][yy] = miniset[ii];
			}
			ii++;
		}
	}
}

void DRLG_DrawMap(int idx)
{
	int x, y, rw, rh, i, j;
	BYTE* pMap;
	BYTE* sp;

	pMap = pSetPieces[idx]._spData;
	rw = SwapLE16(*(uint16_t*)&pMap[0]);
	rh = SwapLE16(*(uint16_t*)&pMap[2]);

	sp = &pMap[4];
	x = pSetPieces[idx]._spx;
	y = pSetPieces[idx]._spy;
	rw += x;
	rh += y;
	for (j = y; j < rh; j++) {
		for (i = x; i < rw; i++) {
			// dungeon[i][j] = *sp != 0 ? *sp : bv;
			if (*sp != 0) {
				dungeon[i][j] = *sp;
			}
			sp += 2;
		}
	}
}

void DRLG_InitTrans()
{
	memset(dTransVal, 0, sizeof(dTransVal));
	//memset(TransList, 0, sizeof(TransList)); - LoadGame() needs this preserved
	numtrans = 1;
	gbDoTransVals = false;
}

/*void DRLG_MRectTrans(int x1, int y1, int x2, int y2, int tv)
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
}*/

/*void DRLG_RectTrans(int x1, int y1, int x2, int y2)
{
	int i, j;

	for (i = x1; i <= x2; i++) {
		for (j = y1; j <= y2; j++) {
			dTransVal[i][j] = numtrans;
		}
	}
	numtrans++;
}*/

/*void DRLG_ListTrans(int num, const BYTE* List)
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

void DRLG_AreaTrans(int num, const BYTE* List)
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
}*/

static void DRLG_FTVR(unsigned offset)
{
	BYTE *tvp = &dTransVal[0][0];
	if (tvp[offset] != 0) {
		return;
	}
	tvp[offset] = numtrans;

	BYTE *tdp = &drlg.transDirMap[0][0];
	if (tdp[offset] & (1 << 0)) { // DIR_SE
		DRLG_FTVR(offset + 1);
	}
	if (tdp[offset] & (1 << 1)) { // DIR_NW
		DRLG_FTVR(offset - 1);
	}
	if (tdp[offset] & (1 << 2)) { // DIR_N
		DRLG_FTVR(offset - 1 - DSIZEY);
	}
	if (tdp[offset] & (1 << 3)) { // DIR_NE
		DRLG_FTVR(offset - DSIZEY);
	}
	if (tdp[offset] & (1 << 4)) { // DIR_E
		DRLG_FTVR(offset + 1 - DSIZEY);
	}
	if (tdp[offset] & (1 << 5)) { // DIR_W
		DRLG_FTVR(offset - 1 + DSIZEY);
	}
	if (tdp[offset] & (1 << 6)) { // DIR_SW
		DRLG_FTVR(offset + DSIZEY);
	}
	if (tdp[offset] & (1 << 7)) { // DIR_S
		DRLG_FTVR(offset + DSIZEY + 1);
	}
}

void DRLG_FloodTVal(const BYTE *floorTypes)
{
	int i, j;
	BYTE *tdp = &drlg.transDirMap[0][0]; // Overlaps with transvalMap!
	BYTE *tvp = &dTransVal[0][0];

	DRLG_InitTrans();

	// prepare the propagation-directions
	for (i = DMAXX - 1; i >= 0; i--) {
		for (j = DMAXY - 1; j >= 0; j--) {
			BYTE tvm = floorTypes[drlg.transvalMap[i][j]];
			BYTE tpm;
			// 1. subtile
			if (tvm & (1 << 0)) {
				tpm = (1 << 1) | (1 << 2) | (1 << 3); // DIR_NW, DIR_N, DIR_NE
				if (tvm & (1 << 2)) // 3. subtile
					tpm |= (1 << 0); // DIR_SE
				if (tvm & (1 << 1)) // 2. subtile
					tpm |= (1 << 6); // DIR_SW
			} else {
				tpm = 0;
			}
			drlg.transDirMap[2 * i + 0][2 * j + 0] = tpm;
			// 3. subtile
			if (tvm & (1 << 2)) {
				tpm = (1 << 3) | (1 << 4) | (1 << 0); // DIR_NE, DIR_E, DIR_SE
				if (tvm & (1 << 0)) // 1. subtile
					tpm |= (1 << 1); // DIR_NW
				if (tvm & (1 << 3)) // 4. subtile
					tpm |= (1 << 6); // DIR_SW
			} else {
				tpm = 0;
			}
			drlg.transDirMap[2 * i + 0][2 * j + 1] = tpm;
			// 2. subtile
			if (tvm & (1 << 1)) {
				tpm = (1 << 6) | (1 << 5) | (1 << 1); // DIR_SW, DIR_W, DIR_NW
				if (tvm & (1 << 0)) // 1. subtile
					tpm |= (1 << 3); // DIR_NE
				if (tvm & (1 << 3)) // 4. subtile
					tpm |= (1 << 0); // DIR_SE
			} else {
				tpm = 0;
			}
			drlg.transDirMap[2 * i + 1][2 * j + 0] = tpm;
			// 4. subtile
			if (tvm & (1 << 3)) {
				tpm = (1 << 0) | (1 << 7) | (1 << 6); // DIR_SE, DIR_S, DIR_SW
				if (tvm & (1 << 2)) // 3. subtile
					tpm |= (1 << 3); // DIR_NE
				if (tvm & (1 << 1)) // 2. subtile
					tpm |= (1 << 1); // DIR_NW
			} else {
				tpm = 0;
			}
			drlg.transDirMap[2 * i + 1][2 * j + 1] = tpm;
		}
	}
	// create the rooms
	for (i = 0; i < DSIZEX * DSIZEY; i++) {
		if (tvp[i] != 0)
			continue;
		if (tdp[i] == 0)
			continue;
		DRLG_FTVR(i);
		numtrans++;
	}
	// move the values into position (add borders)
	static_assert(DBORDERY + DBORDERX * MAXDUNY > DSIZEY, "DRLG_FloodTVal requires large enough border(x) to use memcpy instead of memmove.");
	for (i = DSIZEX - 1; i >= 0; i--) {
		BYTE *tvpSrc = tvp + i * DSIZEY;
		BYTE *tvpDst = tvp + (i + DBORDERX) * MAXDUNY + DBORDERY;
		memcpy(tvpDst, tvpSrc, DSIZEY);
	}
	// clear the borders
	memset(tvp, 0, MAXDUNY * DBORDERX + DBORDERY);
	tvp += MAXDUNY * DBORDERX + DBORDERY + DSIZEY;
	while (tvp < (BYTE*)&dTransVal[0][0] + DSIZEX * DSIZEY) {
		static_assert(DBORDERX != 0, "DRLG_FloodTVal requires large enough border(x) to use merged memset.");
		memset(tvp, 0, 2 * DBORDERY);
		tvp += 2 * DBORDERY + DSIZEY;
	}
}

void DRLG_LoadSP(int idx, BYTE bv)
{
	int rx1, ry1, rw, rh, i, j;
	BYTE* sp;
	SetPieceStruct* pSetPiece = &pSetPieces[idx];

	rx1 = pSetPiece->_spx;
	ry1 = pSetPiece->_spy;
	rw = SwapLE16(*(uint16_t*)&pSetPiece->_spData[0]);
	rh = SwapLE16(*(uint16_t*)&pSetPiece->_spData[2]);
	sp = &pSetPiece->_spData[4];
	// load tiles
	for (j = ry1; j < ry1 + rh; j++) {
		for (i = rx1; i < rx1 + rw; i++) {
			dungeon[i][j] = *sp != 0 ? *sp : bv;
			sp += 2;
		}
	}
	// load flags
	for (j = ry1; j < ry1 + rh; j++) {
		for (i = rx1; i < rx1 + rw; i++) {
			static_assert((int)DRLG_PROTECTED == 1 << 6, "DRLG_LoadSP sets the protection flags with a simple bit-shift I.");
			static_assert((int)DRLG_FROZEN == 1 << 7, "DRLG_LoadSP sets the protection flags with a simple bit-shift II.");
			drlgFlags[i][j] |= (*sp & 3) << 6;
			sp += 2;
		}
	}
}

static void DRLG_InitFlags()
{
	BYTE c;

	c = currLvl._dType == DTYPE_TOWN ? BFLAG_VISIBLE : 0;
	memset(dFlags, c, sizeof(dFlags));

	if (!currLvl._dSetLvl) {
		for (int i = lengthof(pWarps) - 1; i >= 0; i--) {
			int tx = pWarps[i]._wx;
			int ty = pWarps[i]._wy;

			if (tx == 0) {
				continue;
			}
			int r = pWarps[i]._wtype == WRPT_L4_PENTA ? 4 : 2;
			tx -= r;
			ty -= r;
			r = 2 * r + 1;
			for (int xx = 0; xx < r; xx++) {
				for (int yy = 0; yy < r; yy++) {
					dFlags[tx + xx][ty + yy] |= BFLAG_MON_PROTECT | BFLAG_OBJ_PROTECT;
				}
			}
		}
	}

	for (int n = lengthof(pSetPieces) - 1; n >= 0; n--) {
		if (pSetPieces[n]._spData != NULL) { // pSetPieces[n]._sptype != SPT_NONE
			int x = pSetPieces[n]._spx;
			int y = pSetPieces[n]._spy;
			int w = SwapLE16(*(uint16_t*)&pSetPieces[n]._spData[0]);
			int h = SwapLE16(*(uint16_t*)&pSetPieces[n]._spData[2]);

			x = 2 * x + DBORDERX;
			y = 2 * y + DBORDERY;

			BYTE* sp = &pSetPieces[n]._spData[4];
			sp += 2 * w * h; // skip tiles

			sp++;
			for (int j = 0; j < h; j++) {
				for (int i = 0; i < w; i++) {
					BYTE flags = *sp;
					static_assert((1 << (BFLAG_MON_PROTECT_SHL + 1)) == (int)BFLAG_OBJ_PROTECT, "DRLG_SetPC uses bitshift to populate dFlags");
					dFlags[x + 2 * i][y + 2 * j] |= (flags & 3) << BFLAG_MON_PROTECT_SHL;
					flags >>= 2;
					dFlags[x + 2 * i + 1][y + 2 * j] |= (flags & 3) << BFLAG_MON_PROTECT_SHL;
					flags >>= 2;
					dFlags[x + 2 * i][y + 2 * j  + 1] |= (flags & 3) << BFLAG_MON_PROTECT_SHL;
					flags >>= 2;
					dFlags[x + 2 * i + 1][y + 2 * j + 1] |= (flags & 3) << BFLAG_MON_PROTECT_SHL;
					sp += 2;
				}
			}
		}
	}
}

static void DRLG_LightSubtiles()
{
	BYTE c;
	int i, j, pn;

	c = currLvl._dType == DTYPE_TOWN ? 0 : MAXDARKNESS;
#if DEBUG_MODE
	if (lightflag)
		c = 0;
#endif
	memset(dLight, c, sizeof(dLight));

	assert(LightList[MAXLIGHTS]._lxoff == 0);
	assert(LightList[MAXLIGHTS]._lyoff == 0);
	if (currLvl._dType == DTYPE_CAVES) {
		LightList[MAXLIGHTS]._lradius = 7;
		for (i = 0; i < MAXDUNX; i++) {
			for (j = 0; j < MAXDUNY; j++) {
				pn = dPiece[i][j];
				if (pn >= 56 && pn <= 161
				 && (pn <= 147 || pn >= 154 || pn == 150 || pn == 152)) {
					LightList[MAXLIGHTS]._lx = i;
					LightList[MAXLIGHTS]._ly = j;
					DoLighting(MAXLIGHTS);
				}
			}
		}
#ifdef HELLFIRE
	} else if (currLvl._dType == DTYPE_NEST) {
		LightList[MAXLIGHTS]._lradius = 6; // 9
		for (i = 0; i < MAXDUNX; i++) {
			for (j = 0; j < MAXDUNY; j++) {
				pn = dPiece[i][j];
				if ((pn >= 386 && pn <= 496) || (pn >= 534 && pn <= 537)) {
					LightList[MAXLIGHTS]._lx = i;
					LightList[MAXLIGHTS]._ly = j;
					DoLighting(MAXLIGHTS);
				}
			}
		}
#endif
	}
}

void InitLvlMap()
{
	DRLG_LightSubtiles();
	DRLG_InitFlags();

	memset(dPlayer, 0, sizeof(dPlayer));
	memset(dMonster, 0, sizeof(dMonster));
	memset(dDead, 0, sizeof(dDead));
	memset(dObject, 0, sizeof(dObject));
	memset(dItem, 0, sizeof(dItem));
	memset(dMissile, 0, sizeof(dMissile));
}

/**
 * Find the largest available room (rectangle) starting from (x;y) using floor.
 * 
 * @param floor the id of the floor tile in dungeon
 * @param x the x-coordinate of the starting position
 * @param y the y-coordinate of the starting position
 * @param minSize the minimum size of the room (must be less than 20)
 * @param maxSize the maximum size of the room (must be less than 20)
 * @return the size of the room
 */
static POS32 DRLG_FitThemeRoom(BYTE floor, int x, int y, int minSize, int maxSize)
{
	int xmax, ymax, i, j, smallest;
	int xArray[20], yArray[20];
	int size, bestSize, w, h;

	// assert(maxSize < 20);

	xmax = std::min(maxSize, DMAXX - x);
	ymax = std::min(maxSize, DMAXY - y);
	// BUGFIX: change '&&' to '||' (fixed)
	if (xmax < minSize || ymax < minSize)
		return { 0, 0 };

	memset(xArray, 0, sizeof(xArray));
	memset(yArray, 0, sizeof(yArray));

	// find horizontal(x) limits
	smallest = xmax;
	for (i = 0; i < ymax; ) {
		for (j = 0; j < smallest; j++) {
			if (dungeon[x + j][y + i] != floor || drlgFlags[x + j][y + i]) {
				smallest = j;
				break;
			}
		}
		if (smallest < minSize)
			break;
		xArray[++i] = smallest;
	}
	if (i < minSize)
		return { 0, 0 };

	// find vertical(y) limits
	smallest = ymax;
	for (i = 0; i < xmax; ) {
		for (j = 0; j < smallest; j++) {
			if (dungeon[x + i][y + j] != floor || drlgFlags[x + i][y + j]) {
				smallest = j;
				break;
			}
		}
		if (smallest < minSize)
			break;
		yArray[++i] = smallest;
	}
	if (i < minSize)
		return { 0, 0 };

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
	return { w - 2, h - 2 };
}

static void DRLG_CreateThemeRoom(int themeIndex, const BYTE (&themeTiles)[NUM_DRT_TYPES])
{
	int xx, yy;
	const int x1 = themes[themeIndex]._tsx1;
	const int y1 = themes[themeIndex]._tsy1;
	const int x2 = themes[themeIndex]._tsx2;
	const int y2 = themes[themeIndex]._tsy2;
	BYTE v;

	// left/right side
	v = themeTiles[DRT_WALL_VERT];
	for (yy = y1; yy <= y2; yy++) {
		dungeon[x1][yy] = v;
		dungeon[x2][yy] = v;
	}
	// top/bottom line
	v = themeTiles[DRT_WALL_HORIZ];
	for (xx = x1 + 1; xx < x2; xx++) {
		dungeon[xx][y1] = v;
		dungeon[xx][y2] = v;
	}
	// inner tiles
	v = themeTiles[DRT_FLOOR];
	for (xx = x1 + 1; xx < x2; xx++) {
		for (yy = y1 + 1; yy < y2; yy++) {
			dungeon[xx][yy] = v;
		}
	}
	// corners
	dungeon[x1][y1] = themeTiles[DRT_TOP_LEFT];
	dungeon[x2][y1] = themeTiles[DRT_TOP_RIGHT];
	dungeon[x1][y2] = themeTiles[DRT_BOTTOM_LEFT];
	dungeon[x2][y2] = themeTiles[DRT_BOTTOM_RIGHT];

	// exits
	if (random_(0, 2) == 0) {
		dungeon[x2][(y1 + y2 + 1) / 2] = themeTiles[DRT_DOOR_VERT];
	} else {
		dungeon[(x1 + x2 + 1) / 2][y2] = themeTiles[DRT_DOOR_HORIZ];
	}
}

void DRLG_PlaceThemeRooms(int minSize, int maxSize, const BYTE (&themeTiles)[NUM_DRT_TYPES], int rndSkip, bool rndSize)
{
	int i, j;
	int min;

	for (i = 0; i < DMAXX; i++) {
		for (j = 0; j < DMAXY; j++) {
			// always start from a floor tile
			if (dungeon[i][j] != themeTiles[DRT_FLOOR]) {
				continue;
			}
			if (random_(0, 128) < rndSkip) {
				continue;
			}
			// check if there is enough space
			POS32 tArea = DRLG_FitThemeRoom(themeTiles[DRT_FLOOR], i, j, minSize, maxSize);
			if (tArea.x <= 0) {
				continue;
			}
			// randomize the size
			if (rndSize) {
				// assert(minSize > 2);
				min = minSize - 2;
				static_assert(DMAXX /* - minSize */ + 2 < 0x7FFF, "DRLG_PlaceThemeRooms uses RandRangeLow to set themeW.");
				static_assert(DMAXY /* - minSize */ + 2 < 0x7FFF, "DRLG_PlaceThemeRooms uses RandRangeLow to set themeH.");
				tArea.x = RandRangeLow(min, tArea.x);
				tArea.y = RandRangeLow(min, tArea.y);
			}
			// ensure there is no overlapping with previous themes
			if (!InThemeRoom(i + 1, j + 1)) {
				// create the room
				themes[numthemes]._tsx1 = i + 1;
				themes[numthemes]._tsy1 = j + 1;
				themes[numthemes]._tsx2 = i + 1 + tArea.x - 1;
				themes[numthemes]._tsy2 = j + 1 + tArea.y - 1;
				DRLG_CreateThemeRoom(numthemes, themeTiles);
				numthemes++;
				if (numthemes == lengthof(themes))
					return;
			}

			j += tArea.x + 2;
		}
	}
}

bool InThemeRoom(int x, int y)
{
	int i;

	for (i = numthemes - 1; i >= 0; i--) {
		if (x > themes[i]._tsx1 && x < themes[i]._tsx2
		 && y > themes[i]._tsy1 && y < themes[i]._tsy2)
			return true;
	}

	return false;
}

static void SetMini(int x, int y, int mt)
{
	int xx, yy;
	long v1, v2, v3, v4;
	uint16_t* pTile;

	xx = 2 * x + DBORDERX;
	yy = 2 * y + DBORDERY;

	pTile = &pTiles[mt][0];
	v1 = pTile[0];
	v2 = pTile[1];
	v3 = pTile[2];
	v4 = pTile[3];

	dPiece[xx][yy] = v1;
	dPiece[xx + 1][yy] = v2;
	dPiece[xx][yy + 1] = v3;
	dPiece[xx + 1][yy + 1] = v4;
}

void DRLG_ChangeMap(int x1, int y1, int x2, int y2/*, bool hasNewObjPiece*/)
{
	int i, j;

	for (i = x1; i <= x2; i++) {
		for (j = y1; j <= y2; j++) {
			dungeon[i][j] = pdungeon[i][j];
			SetMini(i, j, pdungeon[i][j]);
		}
	}
	x1 = 2 * x1 + DBORDERX;
	y1 = 2 * y1 + DBORDERY;
	x2 = 2 * x2 + DBORDERX + 1;
	y2 = 2 * y2 + DBORDERY + 1;
	// TODO: LoadPreLighting, DRLG_LightSubtiles?
	ObjChangeMap(x1, y1, x2, y2 /*, bool hasNewObjPiece*/);
	// activate monsters
	MonChangeMap();
	gbDoTransVals = true;
	if (!deltaload) {
		DRLG_RedoTrans();
	}
	// RedoLightAndVision();
	// TODO: SavePreLighting?
}

void DRLG_RedoTrans()
{
	if (!gbDoTransVals) {
		return;
	}
	switch (currLvl._dDunType) {
	case DGT_CATHEDRAL:
		DRLG_L1InitTransVals();
		break;
	case DGT_CATACOMBS:
		DRLG_L2InitTransVals();
		break;
	case DGT_CAVES:
		DRLG_L3InitTransVals();
		break;
	case DGT_HELL:
		DRLG_L4InitTransVals();
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	// assert(!gbDoTransVals);
	RedoLightAndVision();
}

DEVILUTION_END_NAMESPACE
