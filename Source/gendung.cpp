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
/** Specifies the set level/piece X-coordinate of the (mega-)map. */
int setpc_x;
/** Specifies the set level/piece Y-coordinate of the (mega-)map. */
int setpc_y;
/** Specifies the width of the set level/piece of the (mega-)map. */
int setpc_w;
/** Specifies the height of the set level/piece of the (mega-)map. */
int setpc_h;
/** Specifies the type of the set level/piece of the (mega-)map (_setpiece_type). */
int setpc_type;
/** Contains the contents of the set piece (DUN file). */
BYTE* pSetPiece = NULL;
/** Specifies the mega tiles (groups of four tiles). */
uint16_t* pMegaTiles;
/*
 * The micros of the dPieces
 */
uint16_t pMicroPieces[MAXTILES + 1][16 * ASSET_MPL * ASSET_MPL];
/** Images of the micros of normal tiles. */
BYTE* pMicroCels;
/** Images of the special tiles. */
BYTE* pSpecialCels;
/**
 * Flags to control the drawing of dPieces (piece_micro_flag)
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
 * List of trap-source dPieces (_piece_trap_type)
 */
BYTE nTrapTable[MAXTILES + 1];
/**
 * List of missile blocking dPieces
 */
bool nMissileTable[MAXTILES + 1];
/** The difficuly level of the current game (_difficulty) */
int gnDifficulty;
/** Contains the data of the active dungeon level. */
LevelStruct currLvl;
int MicroTileLen;
/** Specifies the number of transparency blocks on the map. */
BYTE numtrans;
/** Specifies the active transparency indices. */
bool TransList[256];
/** Contains the tile IDs of each square on the map. */
int dPiece[MAXDUNX][MAXDUNY];
/** Specifies the transparency index at each coordinate of the map. */
BYTE dTransVal[MAXDUNX][MAXDUNY];
/** Specifies the base darkness levels of each tile on the map. */
BYTE dPreLight[MAXDUNX][MAXDUNY];
/** Specifies the current darkness levels of each tile on the map. */
BYTE dLight[MAXDUNX][MAXDUNY];
/** Specifies the (runtime) flags of each tile on the map (dflag) */
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
/**
 * Contains the arch frame numbers of the map from the special tileset
 * (e.g. "levels/l1data/l1s.cel"). Note, the special tileset of Tristram (i.e.
 * "levels/towndata/towns.cel") contains trees rather than arches.
 */
BYTE dSpecial[MAXDUNX][MAXDUNY];
/** Specifies the number of themes generated in the dungeon (valid entries in themeLoc). */
int themeCount;
/** Themes on the (mega-)map. */
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
	uint16_t bv;
	size_t i, dwTiles;
	BYTE *pSBFile, *pTmp;
#if ASSET_MPL == 1
	uint16_t blocks, *pLPFile, *pPiece, *pPTmp;
#endif
	const LevelData* lds;
	assert(pMicroCels == NULL);
	lds = &AllLevels[currLvl._dLevelIdx];

	pMicroCels = LoadFileInMem(lds->dMicroCels);
	assert(pMegaTiles == NULL);
	pMegaTiles = (uint16_t*)LoadFileInMem(lds->dMegaTiles);
	assert(pSpecialCels == NULL);
	if (currLvl._dLevelIdx != DLV_TOWN)
		pSpecialCels = LoadFileInMem(lds->dSpecCels);
	else
		pSpecialCels = (BYTE*)CelLoadImage(lds->dSpecCels, TILE_WIDTH);
	MicroTileLen = lds->dMicroTileLen * ASSET_MPL * ASSET_MPL;
	LoadFileWithMem(lds->dMicroFlags, microFlags);
#if ASSET_MPL == 1
	pLPFile = (uint16_t*)LoadFileInMem(lds->dMiniTiles, &dwTiles);

	blocks = lds->dBlocks;
	dwTiles /= (2 * blocks);
	assert(dwTiles <= MAXTILES);

	for (i = 1; i <= dwTiles; i++) {
		pPTmp = &pMicroPieces[i][0];
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
#else
	LoadFileWithMem(lds->dMiniTiles, (BYTE*)&pMicroPieces[1][0]);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	for (i = 1; i < lengthof(pMicroPieces); i++) {
		for (bv = 0; bv < lengthof(pMicroPieces[0]); bv++) {
			pMicroPieces[i][bv] = SwapLE16(pMicroPieces[i][bv]);
		}
	}
#endif
#endif /* ASSET_MPL == 1 */

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
		nTrapTable[i] = (bv & PFLAG_TRAP_SOURCE) != 0 ? PTT_ANY : PTT_NONE;
	}

	mem_free_dbg(pSBFile);

	switch (currLvl._dType) {
	case DTYPE_TOWN:
		// patch dSolidTable - Town.SOL
		nSolidTable[553] = false; // allow walking on the left side of the pot at Adria
		nSolidTable[761] = true;  // make the tile of the southern window of the church non-walkable
		nSolidTable[945] = true;  // make the eastern side of Griswold's house consistent (non-walkable)

		// patch dMiniTiles - Town.MIN
		// pointless tree micros (re-drawn by dSpecial)
#if ASSET_MPL == 1
		pMicroPieces[117][3] = 0;
		pMicroPieces[117][5] = 0;
		pMicroPieces[128][2] = 0;
		pMicroPieces[128][3] = 0;
		pMicroPieces[128][4] = 0;
		pMicroPieces[128][5] = 0;
		pMicroPieces[128][6] = 0;
		pMicroPieces[128][7] = 0;
		pMicroPieces[129][3] = 0;
		pMicroPieces[129][5] = 0;
		pMicroPieces[129][7] = 0;
		pMicroPieces[130][2] = 0;
		pMicroPieces[130][4] = 0;
		pMicroPieces[130][6] = 0;
		pMicroPieces[156][2] = 0;
		pMicroPieces[156][3] = 0;
		pMicroPieces[156][4] = 0;
		pMicroPieces[156][5] = 0;
		pMicroPieces[156][6] = 0;
		pMicroPieces[156][7] = 0;
		pMicroPieces[156][8] = 0;
		pMicroPieces[156][9] = 0;
		pMicroPieces[156][10] = 0;
		pMicroPieces[156][11] = 0;
		pMicroPieces[157][3] = 0;
		pMicroPieces[157][5] = 0;
		pMicroPieces[157][7] = 0;
		pMicroPieces[157][9] = 0;
		pMicroPieces[157][11] = 0;
		pMicroPieces[158][2] = 0;
		pMicroPieces[158][4] = 0;
		pMicroPieces[160][2] = 0;
		pMicroPieces[160][3] = 0;
		pMicroPieces[160][4] = 0;
		pMicroPieces[160][5] = 0;
		pMicroPieces[160][6] = 0;
		pMicroPieces[160][7] = 0;
		pMicroPieces[160][8] = 0;
		pMicroPieces[160][9] = 0;
		pMicroPieces[162][2] = 0;
		pMicroPieces[162][4] = 0;
		pMicroPieces[162][6] = 0;
		pMicroPieces[162][8] = 0;
		pMicroPieces[162][10] = 0;
		pMicroPieces[212][3] = 0;
		pMicroPieces[212][4] = 0;
		pMicroPieces[212][5] = 0;
		pMicroPieces[212][6] = 0;
		pMicroPieces[212][7] = 0;
		pMicroPieces[212][8] = 0;
		pMicroPieces[212][9] = 0;
		pMicroPieces[212][10] = 0;
		pMicroPieces[212][11] = 0;
		//pMicroPieces[214][4] = 0;
		//pMicroPieces[214][6] = 0;
		pMicroPieces[216][2] = 0;
		pMicroPieces[216][4] = 0;
		pMicroPieces[216][6] = 0;
		//pMicroPieces[217][4] = 0;
		//pMicroPieces[217][6] = 0;
		//pMicroPieces[217][8] = 0;
		//pMicroPieces[358][4] = 0;
		//pMicroPieces[358][5] = 0;
		//pMicroPieces[358][6] = 0;
		//pMicroPieces[358][7] = 0;
		//pMicroPieces[358][8] = 0;
		//pMicroPieces[358][9] = 0;
		//pMicroPieces[358][10] = 0;
		//pMicroPieces[358][11] = 0;
		//pMicroPieces[358][12] = 0;
		//pMicroPieces[358][13] = 0;
		//pMicroPieces[360][4] = 0;
		//pMicroPieces[360][6] = 0;
		//pMicroPieces[360][8] = 0;
		//pMicroPieces[360][10] = 0;
		// fix bad artifact
		pMicroPieces[233][6] = 0;
		// useless black micros
		pMicroPieces[426][1] = 0;
		pMicroPieces[427][0] = 0;
		pMicroPieces[427][1] = 0;
		pMicroPieces[429][1] = 0;
		// fix bad artifacts
		pMicroPieces[828][12] = 0;
		pMicroPieces[828][13] = 0;
		pMicroPieces[1018][2] = 0;
		// useless black micros
		pMicroPieces[1143][0] = 0;
		pMicroPieces[1145][0] = 0;
		pMicroPieces[1145][1] = 0;
		pMicroPieces[1146][0] = 0;
		pMicroPieces[1153][0] = 0;
		pMicroPieces[1155][1] = 0;
		pMicroPieces[1156][0] = 0;
		pMicroPieces[1169][1] = 0;
		pMicroPieces[1170][0] = 0;
		pMicroPieces[1170][1] = 0;
		pMicroPieces[1172][1] = 0;
		pMicroPieces[1176][1] = 0;
		pMicroPieces[1199][1] = 0;
		pMicroPieces[1200][0] = 0;
		pMicroPieces[1200][1] = 0;
		pMicroPieces[1202][1] = 0;
		pMicroPieces[1203][1] = 0;
		pMicroPieces[1205][1] = 0;
		pMicroPieces[1212][0] = 0;
		pMicroPieces[1219][0] = 0;
#ifdef HELLFIRE
		// fix bad artifacts
		pMicroPieces[1273][7] = 0;
		//pMicroPieces[1303][7] = 0; - unused
#endif
#endif /* ASSET_MPL == 1 */
		break;
	case DTYPE_CATHEDRAL:
		// patch dSolidTable - L1.SOL
		nMissileTable[8] = false; // the only column which was blocking missiles
#if ASSET_MPL == 1
		// patch dMiniTiles - L1.MIN
		// useless black micros
		pMicroPieces[107][0] = 0;
		pMicroPieces[107][1] = 0;
		pMicroPieces[109][1] = 0;
		pMicroPieces[137][1] = 0;
		pMicroPieces[138][0] = 0;
		pMicroPieces[138][1] = 0;
		pMicroPieces[140][1] = 0;
#endif /* ASSET_MPL == 1 */
		break;
	case DTYPE_CATACOMBS:
		// patch dSolidTable - L2.SOL
		// specify direction for torches
		nTrapTable[1] = PTT_LEFT;
		nTrapTable[3] = PTT_LEFT;
		nTrapTable[5] = PTT_RIGHT;
		nTrapTable[6] = PTT_RIGHT;
		nTrapTable[15] = PTT_LEFT;
		nTrapTable[18] = PTT_RIGHT;
		nTrapTable[27] = PTT_LEFT;
		nTrapTable[30] = PTT_RIGHT;
		nTrapTable[31] = PTT_LEFT;
		nTrapTable[34] = PTT_RIGHT;
		nTrapTable[57] = PTT_LEFT;  // added
		nTrapTable[59] = PTT_RIGHT; // added
		nTrapTable[60] = PTT_LEFT;
		nTrapTable[62] = PTT_LEFT;
		nTrapTable[64] = PTT_LEFT;
		nTrapTable[66] = PTT_LEFT;
		nTrapTable[68] = PTT_RIGHT;
		nTrapTable[69] = PTT_RIGHT;
		nTrapTable[72] = PTT_RIGHT;
		nTrapTable[73] = PTT_RIGHT;
		nTrapTable[78] = PTT_LEFT;
		nTrapTable[82] = PTT_LEFT;
		nTrapTable[85] = PTT_LEFT;
		nTrapTable[88] = PTT_LEFT;
		nTrapTable[92] = PTT_LEFT;
		nTrapTable[94] = PTT_LEFT;
		nTrapTable[96] = PTT_LEFT;
		nTrapTable[99] = PTT_RIGHT;
		nTrapTable[104] = PTT_RIGHT;
		nTrapTable[108] = PTT_RIGHT;
		nTrapTable[111] = PTT_RIGHT; // added
		nTrapTable[112] = PTT_RIGHT;
		nTrapTable[115] = PTT_LEFT; // added
		nTrapTable[117] = PTT_LEFT;
		nTrapTable[119] = PTT_LEFT;
		nTrapTable[120] = PTT_LEFT;
		nTrapTable[121] = PTT_RIGHT; // added
		nTrapTable[122] = PTT_RIGHT;
		nTrapTable[125] = PTT_RIGHT;
		nTrapTable[126] = PTT_RIGHT;
		nTrapTable[128] = PTT_RIGHT;
		nTrapTable[129] = PTT_LEFT;
		nTrapTable[144] = PTT_LEFT;
		//nTrapTable[170] = PTT_LEFT; // added
		//nTrapTable[172] = PTT_LEFT; // added
		//nTrapTable[174] = PTT_LEFT; // added
		//nTrapTable[176] = PTT_LEFT; // added
		//nTrapTable[180] = PTT_LEFT; // added
		//nTrapTable[183] = PTT_RIGHT; // added
		//nTrapTable[186] = PTT_RIGHT; // added
		//nTrapTable[187] = PTT_RIGHT; // added
		//nTrapTable[190] = PTT_RIGHT; // added
		//nTrapTable[191] = PTT_RIGHT; // added
		//nTrapTable[194] = PTT_RIGHT; // added
		//nTrapTable[195] = PTT_RIGHT; // added
		nTrapTable[234] = PTT_LEFT;
		nTrapTable[236] = PTT_LEFT;
		nTrapTable[238] = PTT_LEFT; // added
		nTrapTable[240] = PTT_LEFT;
		nTrapTable[242] = PTT_LEFT; // added
		nTrapTable[244] = PTT_LEFT;
		nTrapTable[253] = PTT_RIGHT; // added
		nTrapTable[254] = PTT_RIGHT;
		nTrapTable[257] = PTT_RIGHT;
		nTrapTable[258] = PTT_RIGHT; // added
		nTrapTable[261] = PTT_RIGHT; // added
		nTrapTable[262] = PTT_RIGHT;
		nTrapTable[277] = PTT_LEFT;
		nTrapTable[281] = PTT_LEFT;
		nTrapTable[285] = PTT_LEFT;
		nTrapTable[292] = PTT_RIGHT;
		nTrapTable[296] = PTT_RIGHT;
		nTrapTable[300] = PTT_RIGHT;
		nTrapTable[304] = PTT_RIGHT;
		nTrapTable[305] = PTT_LEFT;
		nTrapTable[446] = PTT_LEFT;
		nTrapTable[448] = PTT_LEFT;
		nTrapTable[450] = PTT_LEFT;
		nTrapTable[452] = PTT_LEFT;
		nTrapTable[454] = PTT_RIGHT;
		nTrapTable[455] = PTT_RIGHT;
		nTrapTable[458] = PTT_RIGHT;
		nTrapTable[459] = PTT_RIGHT;
		nTrapTable[480] = PTT_LEFT;
		nTrapTable[499] = PTT_RIGHT;
		nTrapTable[510] = PTT_LEFT;
		nTrapTable[512] = PTT_LEFT;
		nTrapTable[514] = PTT_RIGHT;
		nTrapTable[515] = PTT_RIGHT;
		nTrapTable[539] = PTT_LEFT;  // added
		nTrapTable[543] = PTT_LEFT;  // added
		nTrapTable[545] = PTT_LEFT;  // added
		nTrapTable[547] = PTT_RIGHT; // added
		nTrapTable[548] = PTT_RIGHT; // added
		nTrapTable[552] = PTT_LEFT;  // added
		// enable torches on (southern) walls
		// nTrapTable[37] = PTT_LEFT;
		// nTrapTable[39] = PTT_LEFT;
		// nTrapTable[41] = PTT_RIGHT;
		// nTrapTable[42] = PTT_RIGHT;
		// nTrapTable[46] = PTT_RIGHT;
		// nTrapTable[47] = PTT_LEFT;
		// nTrapTable[49] = PTT_LEFT;
		// nTrapTable[51] = PTT_RIGHT;
		nTrapTable[520] = PTT_LEFT;
		nTrapTable[522] = PTT_LEFT;
		nTrapTable[524] = PTT_RIGHT;
		nTrapTable[525] = PTT_RIGHT;
		nTrapTable[529] = PTT_RIGHT;
		nTrapTable[530] = PTT_LEFT;
		nTrapTable[532] = PTT_LEFT;
		nTrapTable[534] = PTT_RIGHT;
		break;
	case DTYPE_CAVES:
#if ASSET_MPL == 1
		// patch dMiniTiles - L3.MIN
		// fix bad artifact
		pMicroPieces[82][4] = 0;
#endif /* ASSET_MPL == 1 */
		break;
	case DTYPE_HELL:
		// patch dSolidTable - L4.SOL
		nMissileTable[141] = false; // fix missile-blocking tile of down-stairs.
		nSolidTable[130] = true;    // make the inner tiles of the down-stairs non-walkable I.
		nSolidTable[132] = true;    // make the inner tiles of the down-stairs non-walkable II.
		// fix all-blocking tile on the diablo-level
		nSolidTable[211] = false;
		nMissileTable[211] = false;
		nBlockTable[211] = false;
		// enable hooked bodies on  walls
		nTrapTable[2] = PTT_LEFT;
		nTrapTable[189] = PTT_LEFT;
		nTrapTable[197] = PTT_LEFT;
		nTrapTable[205] = PTT_LEFT;
		nTrapTable[209] = PTT_LEFT;
		nTrapTable[5] = PTT_RIGHT;
		nTrapTable[192] = PTT_RIGHT;
		nTrapTable[212] = PTT_RIGHT;
		nTrapTable[216] = PTT_RIGHT;
		break;
#ifdef HELLFIRE
	case DTYPE_NEST:
		// patch dSolidTable - L6.SOL
		nSolidTable[390] = false; // make a pool tile walkable I.
		nSolidTable[413] = false; // make a pool tile walkable II.
		nSolidTable[416] = false; // make a pool tile walkable III.
#if ASSET_MPL == 1
		// patch dMiniTiles - L6.MIN
		// useless black micros
		pMicroPieces[21][0] = 0;
		pMicroPieces[21][1] = 0;
		// fix bad artifacts
		pMicroPieces[132][7] = 0;
		pMicroPieces[366][1] = 0;
#endif /* ASSET_MPL == 1 */
		break;
	case DTYPE_CRYPT:
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
#if ASSET_MPL == 1
		// patch dMiniTiles - L5.MIN
		// useless black micros
		pMicroPieces[130][0] = 0;
		pMicroPieces[130][1] = 0;
		pMicroPieces[132][1] = 0;
		pMicroPieces[134][0] = 0;
		pMicroPieces[134][1] = 0;
		pMicroPieces[149][0] = 0;
		pMicroPieces[149][1] = 0;
		pMicroPieces[149][2] = 0;
		pMicroPieces[150][0] = 0;
		pMicroPieces[150][1] = 0;
		pMicroPieces[150][2] = 0;
		pMicroPieces[150][4] = 0;
		pMicroPieces[151][0] = 0;
		pMicroPieces[151][1] = 0;
		pMicroPieces[151][3] = 0;
		pMicroPieces[152][0] = 0;
		pMicroPieces[152][1] = 0;
		pMicroPieces[152][3] = 0;
		pMicroPieces[152][5] = 0;
		pMicroPieces[153][0] = 0;
		pMicroPieces[153][1] = 0;
		// fix bad artifact
		pMicroPieces[156][2] = 0;
		// useless black micros
		pMicroPieces[172][0] = 0;
		pMicroPieces[172][1] = 0;
		pMicroPieces[172][2] = 0;
		pMicroPieces[173][0] = 0;
		pMicroPieces[173][1] = 0;
		pMicroPieces[174][0] = 0;
		pMicroPieces[174][1] = 0;
		pMicroPieces[174][2] = 0;
		pMicroPieces[174][4] = 0;
		pMicroPieces[175][0] = 0;
		pMicroPieces[175][1] = 0;
		pMicroPieces[176][0] = 0;
		pMicroPieces[176][1] = 0;
		pMicroPieces[176][3] = 0;
		pMicroPieces[177][0] = 0;
		pMicroPieces[177][1] = 0;
		pMicroPieces[177][3] = 0;
		pMicroPieces[177][5] = 0;
		pMicroPieces[178][0] = 0;
		pMicroPieces[178][1] = 0;
		pMicroPieces[179][0] = 0;
		pMicroPieces[179][1] = 0;
#endif /* ASSET_MPL == 1 */
		break;
#endif /* HELLFIRE */
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

POS32 DRLG_PlaceMiniSet(const BYTE* miniset)
{
	int sx, sy, sw, sh, xx, yy, ii, tries;
	bool done;

	sw = miniset[0];
	sh = miniset[1];
	// assert(sw < DMAXX && sh < DMAXY);
	tries = 0;
	while (TRUE) {
		done = true;
		if ((tries & 0xFF) == 0) {
			sx = random_low(0, DMAXX - sw);
			sy = random_low(0, DMAXY - sh);
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

void DRLG_PlaceMegaTiles(int mt)
{
	int i, j, xx, yy;
	int v1, v2, v3, v4;
	uint16_t* Tiles;

	/*int cursor = 0;
	char tmpstr[1024];
	long lvs[] = { 22, 56, 57, 58, 59, 60, 61 };
	for (i = 0; i < lengthof(lvs); i++) {
		lv = lvs[i];
		Tiles = &pMegaTiles[mt * 4];
		v1 = SwapLE16(Tiles[0]) + 1;
		v2 = SwapLE16(Tiles[1]) + 1;
		v3 = SwapLE16(Tiles[2]) + 1;
		v4 = SwapLE16(Tiles[3]) + 1;
		cat_str(tmpstr, cursor, "- %d: %d, %d, %d, %d", lv, v1, v2, v3, v4);
	}
	app_fatal(tmpstr);*/

	Tiles = &pMegaTiles[mt * 4];
	v1 = SwapLE16(Tiles[0]) + 1;
	v2 = SwapLE16(Tiles[1]) + 1;
	v3 = SwapLE16(Tiles[2]) + 1;
	v4 = SwapLE16(Tiles[3]) + 1;

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
			mt = dungeon[i][j] - 1;
			assert(mt >= 0);
			Tiles = &pMegaTiles[mt * 4];
			v1 = SwapLE16(Tiles[0]) + 1;
			v2 = SwapLE16(Tiles[1]) + 1;
			v3 = SwapLE16(Tiles[2]) + 1;
			v4 = SwapLE16(Tiles[3]) + 1;
			dPiece[xx][yy] = v1;
			dPiece[xx + 1][yy] = v2;
			dPiece[xx][yy + 1] = v3;
			dPiece[xx + 1][yy + 1] = v4;
			xx += 2;
		}
		yy += 2;
	}
}

void DRLG_DrawMap(const char* name, int bv)
{
	int x, y, rw, rh, i, j;
	BYTE* pMap;
	BYTE* sp;

	pMap = LoadFileInMem(name);
	rw = pMap[0];
	rh = pMap[2];

	sp = &pMap[4];
	assert(setpc_w == rw);
	assert(setpc_h == rh);
	x = setpc_x;
	y = setpc_y;
	rw += x;
	rh += y;
	for (j = y; j < rh; j++) {
		for (i = x; i < rw; i++) {
			dungeon[i][j] = *sp != 0 ? *sp : bv;
			sp += 2;
		}
	}
	mem_free_dbg(pMap);
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

void DRLG_ListTrans(int num, const BYTE* List)
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
		case 0:
			dTransVal[x][y] = numtrans;
			dTransVal[x][y + 1] = numtrans;
			break;
		case 1:
			dTransVal[x + 1][y] = numtrans;
			dTransVal[x + 1][y + 1] = numtrans;
			break;
		case 2:
			dTransVal[x][y] = numtrans;
			dTransVal[x + 1][y] = numtrans;
			break;
		case 3:
			dTransVal[x][y + 1] = numtrans;
			dTransVal[x + 1][y + 1] = numtrans;
			break;
		case 4:
			dTransVal[x + 1][y + 1] = numtrans;
			break;
		case 5:
			dTransVal[x][y + 1] = numtrans;
			break;
		case 6:
			dTransVal[x + 1][y] = numtrans;
			break;
		case 7:
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
		DRLG_FTVR(i + 1, j, x + 2, y, 0);
		DRLG_FTVR(i - 1, j, x - 2, y, 1);
		DRLG_FTVR(i, j + 1, x, y + 2, 2);
		DRLG_FTVR(i, j - 1, x, y - 2, 3);
		DRLG_FTVR(i - 1, j - 1, x - 2, y - 2, 4);
		DRLG_FTVR(i + 1, j - 1, x + 2, y - 2, 5);
		DRLG_FTVR(i - 1, j + 1, x - 2, y + 2, 6);
		DRLG_FTVR(i + 1, j + 1, x + 2, y + 2, 7);
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

void DRLG_SetMapTrans(BYTE* pMap)
{
	int i, j;
	BYTE tv;
	uint16_t rw, rh, *lm;

	lm = (uint16_t*)pMap;
	rw = SwapLE16(*lm);
	lm++;
	rh = SwapLE16(*lm);
	lm++;
	lm += rw * rh; // skip dun
	rw <<= 1;
	rh <<= 1;
	lm += 3 * rw * rh; // skip items?, monsters, objects

	rw += DBORDERX;
	rh += DBORDERY;
	for (j = DBORDERY; j < rh; j++) {
		for (i = DBORDERX; i < rw; i++) {
			tv = SwapLE16(*lm);
			dTransVal[i][j] = tv;
			if (tv >= numtrans)
				numtrans = tv + 1;
			lm++;
		}
	}
}

void DRLG_InitSetPC()
{
	setpc_x = 0;
	setpc_y = 0;
	setpc_w = 0;
	setpc_h = 0;
	setpc_type = SPT_NONE;
}

static void Make_SetPC(int x, int y, int w, int h)
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

void DRLG_SetPC()
{
	Make_SetPC(setpc_x, setpc_y, setpc_w, setpc_h);
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
static bool DRLG_WillThemeRoomFit(int floor, int x, int y, int minSize, int maxSize, int* width, int* height)
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
			if (dungeon[i][j] == floor && (freq == 0 || random_low(0, freq) == 0) && DRLG_WillThemeRoomFit(floor, i, j, minSize, maxSize, &themeW, &themeH)) {
				if (rndSize) {
					// assert(minSize > 2);
					min = minSize - 2;
					static_assert(DMAXX /* - minSize */ + 2 < 0x7FFF, "DRLG_PlaceThemeRooms uses RandRangeLow to set themeW.");
					static_assert(DMAXY /* - minSize */ + 2 < 0x7FFF, "DRLG_PlaceThemeRooms uses RandRangeLow to set themeH.");
					themeW = RandRangeLow(min, themeW);
					themeH = RandRangeLow(min, themeH);
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
