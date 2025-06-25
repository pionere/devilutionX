/**
 * @file drlg_l4.cpp
 *
 * Implementation of the hell level generation algorithms.
 *
 * drlgFlags matrix is used as a BOOLEAN matrix to protect the quest room and the quads in HELL4.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** The default external tile. */
#define BASE_MEGATILE_L4 30
/** The default floor tile. */
#define DEFAULT_MEGATILE_L4 6

static_assert(DQUAD_ROOM_SIZE <= L4BLOCKX, "Rooms of diablo-quads must fit to the dungeon blocks of DRLG_L4 I.");
static_assert(DQUAD_ROOM_SIZE <= L4BLOCKY, "Rooms of diablo-quads must fit to the dungeon blocks of DRLG_L4 II.");

/* Tiles to build the theme rooms. */
const BYTE themeTiles[NUM_DRT_TYPES] = { DEFAULT_MEGATILE_L4, 1, 2, 1, 2, 9, 16, 15, 12 };

/**
 * A lookup table for the 16 possible patterns of a 2x2 area,
 * where each cell either contains a SW wall or it doesn't.
 */
const BYTE L4ConvTbl[16] = { BASE_MEGATILE_L4, 6, 1, 6, 2, 6, 6, 6, 9, 6, 1, 6, 2, 6, 3, 6 };

/** Miniset: Entry point of the dynamic maps. */
const BYTE L4DYNENTRY1[] = {
	// clang-format off
	2, 2, // width, height

	2, 2, // search
	6, 6,

	64, 0, // replace
	0, 0,
	// clang-format on
};
const BYTE L4DYNENTRY2[] = {
	// clang-format off
	2, 2, // width, height

	2, 6, // search
	2, 6,

	63, 0, // replace
	0, 0,
	// clang-format on
};
/** Miniset: Stairs up. */
const BYTE L4USTAIRS[] = {
	// clang-format off
	5, 5, // width, height

	 6, 6, 6, 6, 6, // search
	 6, 6, 6, 6, 6,
	 6, 6, 6, 6, 6,
	 6, 6, 6, 6, 6,
	 6, 6, 6, 6, 6,

	 0,  0,  0,  0,  0, // replace
	 0, 38, 35,  0,  0,
	37, 34, 33, 32,  0,
	 0,  0, 31,  0,  0,
	 0,  0,  0,  0,  0,
	// clang-format on
};
/** Miniset: Stairs up to town. */
/*const BYTE L4TWARP[] = {
	// clang-format off
	4, 5, // width, height

	6, 6, 6, 6, // search
	6, 6, 6, 6,
	6, 6, 6, 6,
	6, 6, 6, 6,
	6, 6, 6, 6,

	  0,   0,   0,   0, // replace
	134, 136, 133,   0,
	135, 132, 131, 130,
	  0,   0, 129,   0,
	  0,   0,   0,   0,
	// clang-format on
};*/
/** Miniset: Stairs down. */
const BYTE L4DSTAIRS[] = {
	// clang-format off
	5, 5, // width, height

	6, 6, 6, 6, 6, // search
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,

	0,  0,  0,  0, 0, // replace
	0,136, 45, 41, 0,
	0, 44, 43, 40, 0,
	0, 46, 42, 39, 0,
	0,  0,  0,  0, 0,
	// clang-format on
};
/** Miniset: Pentagram. */
const BYTE L4PENTA[] = {
	// clang-format off
	5, 5, // width, height

	6, 6, 6, 6, 6, // search
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,

	0,   0,   0,   0, 0, // replace
	0,  98, 100, 103, 0,
	0,  99, 102, 105, 0,
	0, 101, 104, 106, 0,
	0,   0,   0,   0, 0,
	// clang-format on
};
/** Miniset: Pentagram portal. */
const BYTE L4PENTA2[] = {
	// clang-format off
	5, 5, // width, height

	6, 6, 6, 6, 6, // search
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,

	0,   0,   0,   0, 0, // replace
	0, 107, 109, 112, 0,
	0, 108, 111, 114, 0,
	0, 110, 113, 115, 0,
	0,   0,   0,   0, 0,
	// clang-format on
};
/*
 * Maps tile IDs to their corresponding undecorated tile type.
 */
const BYTE L4BTYPES[138] = {
	// clang-format off
	 0,  1,  2,  0,  3,  4,  5,  6,  0,  7,
	 0,  0,  8,  0,  0,  9, 10,  0,  0,  0, // 10..
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 20..
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 30..
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  5, // 40..
	 5,  5,  0,  0,  0,  0,  0,  0,  0,  0, // 50..
	 0,  0,  0,  1,  2,  1,  2,  1,  1,  2, // 60..
	 2,  0,  0,  0,  0,  0,  0,  9, 10,  7, // 70..
	 8,  3,  4,  6,  0,  0,  0,  0,  0,  0, // 80..
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 90..
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //100..
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //110..
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //120..
	 0,  0,  0,  0,  0,  0,  0,  0          //130..
	// clang-format on
};
/*
 * Miniset replacement to add shadows.
 * New dungeon values: 47 48   54 55   58 59 60   61 62 76 129 130 131 132 133 134 135
 */
static void DRLG_L4Shadows()
{
	int x, y;
	BYTE bv;

	for (x = 1; x < DMAXY; x++) {
		for (y = 1; y < DMAXY; y++) {
			bv = dungeon[x][y];
			if (nTrnShadowTable[bv] & TIF_L4_PILLAR) {
				// 6, 0,
				// 6, 3/4/8/15,  search

				//48, 0, replace
				//47, 0,
				BYTE replaceB = dungeon[x - 1][y];
				if (replaceB == 6) {
					dungeon[x - 1][y] = 47;
				} else {
					continue;
				}
				switch (dungeon[x - 1][y - 1]) {
				case 2:  replaceB = 61;  break;
				case 6:  replaceB = 48;  break;
				case 8:  replaceB = 132; break;
				case 14: replaceB = 134; break;
				case 15: replaceB = 76;  break;
				case 56: replaceB = 130; break;
				default:
					dungeon[x - 1][y] = replaceB; // restore original value
					continue;
				}
				dungeon[x - 1][y - 1] = replaceB;
			} else if (nTrnShadowTable[bv] & TIF_L4_WEST_ARCH) {
				// 6, 0,
				// 6, 53,  search

				//55, 0, replace
				//54, 0,
				BYTE replaceB = dungeon[x - 1][y];
				if (replaceB == 6) {
					dungeon[x - 1][y] = 54;
				} else {
					continue;
				}
				switch (dungeon[x - 1][y - 1]) {
				case 2:  replaceB = 62;  break;
				case 6:  replaceB = 55;  break;
				case 8:  replaceB = 133; break;
				case 14: replaceB = 135; break;
				case 15: replaceB = 129; break;
				case 56: replaceB = 131; break;
				default:
					dungeon[x - 1][y] = replaceB; // restore original value
					continue;
				}
				dungeon[x - 1][y - 1] = replaceB;
			} else if (nTrnShadowTable[bv] & TIF_L4_EAST_ARCH) {
				// 6, 6, 0,
				// 0, 50, 56,  search

				//58, 59, 0, replace
				// 0, 60, 0,
				if (dungeon[x - 1][y] == 50 && dungeon[x - 1][y - 1] == 6 && dungeon[x - 2][y - 1] == 6) {
					dungeon[x - 1][y] = 60;
					dungeon[x - 1][y - 1] = 59;
					dungeon[x - 2][y - 1] = 58;
				}
			}
		}
	}
}

static void DRLG_LoadL4SP()
{
	// assert(pSetPieces[0]._spData == NULL && ...);
	if (currLvl._dLevelIdx == DLV_HELL4) {
		pSetPieces[0]._sptype = SPT_DIAB_QUAD_1;
		pSetPieces[0]._spData = LoadFileInMem(setpiecedata[pSetPieces[0]._sptype]._spdDunFile);
		pSetPieces[1]._sptype = SPT_DIAB_QUAD_2;
		pSetPieces[1]._spData = LoadFileInMem(setpiecedata[pSetPieces[1]._sptype]._spdDunFile);
		pSetPieces[2]._sptype = SPT_DIAB_QUAD_3;
		pSetPieces[2]._spData = LoadFileInMem(setpiecedata[pSetPieces[2]._sptype]._spdDunFile);
		pSetPieces[3]._sptype = SPT_DIAB_QUAD_4;
		pSetPieces[3]._spData = LoadFileInMem(setpiecedata[pSetPieces[3]._sptype]._spdDunFile);
#if !USE_PATCH
		{
			// patch set-piece - Diab1.DUN
			uint16_t* lm = (uint16_t*)pSetPieces[0]._spData;
			// - fix shadow of the left corner
			lm[2 + 0 + 4 * 6] = SwapLE16(75);
			lm[2 + 0 + 5 * 6] = SwapLE16(74);
			// - fix shadow of the right corner
			lm[2 + 4 + 1 * 6] = SwapLE16(131);
			// protect tiles with monsters/objects from spawning additional monsters/objects
			lm[2 + 6 * 6 + 1 + 1 * 6] = SwapLE16((3 << 14));
			lm[2 + 6 * 6 + 1 + 4 * 6] = SwapLE16((3 << 14));
			lm[2 + 6 * 6 + 2 + 3 * 6] = SwapLE16((3 << 12));
			lm[2 + 6 * 6 + 3 + 3 * 6] = SwapLE16((3 << 14));
			lm[2 + 6 * 6 + 3 + 2 * 6] = SwapLE16((3 << 10));
			lm[2 + 6 * 6 + 4 + 1 * 6] = SwapLE16((3 << 14));
			lm[2 + 6 * 6 + 4 + 4 * 6] = SwapLE16((3 << 14));
			lm[2 + 6 * 6 + 2 + 2 * 6] = SwapLE16((3 << 14));
			// protect tiles with monsters/objects from decoration
			lm[2 + 6 * 6 + 1 + 4 * 6] |= SwapLE16(3);
			lm[2 + 6 * 6 + 2 + 3 * 6] |= SwapLE16(3);
			lm[2 + 6 * 6 + 1 + 1 * 6] |= SwapLE16(3);
			lm[2 + 6 * 6 + 3 + 2 * 6] |= SwapLE16(3);
			lm[2 + 6 * 6 + 3 + 3 * 6] |= SwapLE16(3);
			lm[2 + 6 * 6 + 4 + 4 * 6] |= SwapLE16(3);
			lm[2 + 6 * 6 + 4 + 1 * 6] |= SwapLE16(3);
			lm[2 + 6 * 6 + 2 + 2 * 6] |= SwapLE16(3);
		}
		{
			// patch set-piece - Diab2b.DUN
			uint16_t* lm = (uint16_t*)pSetPieces[1]._spData;
			// external tiles
			for (int y = 0; y < 12; y++) {
				for (int x = 0; x < 11; x++) {
					uint16_t wv = SwapLE16(lm[2 + x + y * 11]);
					if (wv >= 116 && wv <= 128) {
						// if (wv == 118) {
						//	wv = 128;
						// }
						lm[2 + x + y * 11] = SwapLE16(wv - 98);
					}
				}
			}
			// use base tiles and let the engine decorate the walls
			for (int y = 0; y < 12; y++) {
				for (int x = 0; x < 11; x++) {
					if (y == 1 && (x == 8 || x == 9)) {
						continue; // skip protected tiles
					}
					uint16_t wv = SwapLE16(lm[2 + x + y * 11]);
					if (wv >= 63 && wv <= 70) {
						if (wv == 63 || wv == 65 || wv == 67 || wv == 68) {
							wv = 1;
						} else  {
							wv = 2;
						}
						lm[2 + x + y * 11] = SwapLE16(wv);
					}
				}
			}
			// remove shadow to enable optional connection
			lm[2 + 0 + 10 * 11] = 0;
			lm[2 + 0 + 11 * 11] = 0;
			// ensure the changing tiles are reserved
			lm[2 + 11 * 12 + 9 + 1 * 11] = SwapLE16(3);
			lm[2 + 11 * 12 + 9 + 2 * 11] = SwapLE16(3);
			lm[2 + 11 * 12 + 9 + 3 * 11] = SwapLE16(3);
			lm[2 + 11 * 12 + 10 + 1 * 11] = SwapLE16(3);
			lm[2 + 11 * 12 + 10 + 2 * 11] = SwapLE16(3);
			lm[2 + 11 * 12 + 10 + 3 * 11] = SwapLE16(3);
			// protect tiles with monsters/objects from decoration
			lm[2 + 11 * 12 + 2 + 1 * 11] = SwapLE16(3);
			lm[2 + 11 * 12 + 2 + 9 * 11] = SwapLE16(3);
			lm[2 + 11 * 12 + 5 + 4 * 11] = SwapLE16(3);
			lm[2 + 11 * 12 + 5 + 6 * 11] = SwapLE16(3);
			lm[2 + 11 * 12 + 6 + 6 * 11] = SwapLE16(3);
			lm[2 + 11 * 12 + 7 + 5 * 11] = SwapLE16(3);
			lm[2 + 11 * 12 + 8 + 4 * 11] = SwapLE16(3);
			lm[2 + 11 * 12 + 8 + 6 * 11] = SwapLE16(3);
			lm[2 + 11 * 12 + 7 + 8 * 11] = SwapLE16(3);
			lm[2 + 11 * 12 + 8 + 1 * 11] = SwapLE16(3);
			lm[2 + 11 * 12 + 8 + 2 * 11] = SwapLE16(3);
			lm[2 + 11 * 12 + 6 + 5 * 11] = SwapLE16(3);
		}
		{
			// patch set-piece - Diab3b.DUN
			uint16_t* lm = (uint16_t*)pSetPieces[2]._spData;
			// external tiles
			lm[2 + 4 + 4 * 11] = SwapLE16(21);
			lm[2 + 4 + 5 * 11] = SwapLE16(18);
			lm[2 + 5 + 4 * 11] = SwapLE16(19);
			lm[2 + 5 + 5 * 11] = SwapLE16(30);
			// remove partial shadow
			lm[2 + 5 + 0 * 11] = SwapLE16(50);
			// remove shadow to enable optional connection
			lm[2 + 1 + 9 * 11] = 0;
			lm[2 + 1 + 10 * 11] = 0;
			// use base tiles and let the engine decorate the walls
			lm[2 + 3 + 10 * 11] = SwapLE16(2);
			lm[2 + 9 + 8 * 11] = SwapLE16(2);
			lm[2 + 8 + 9 * 11] = SwapLE16(1);
			lm[2 + 6 + 5 * 11] = SwapLE16(1);
			lm[2 + 5 + 6 * 11] = SwapLE16(2);
			lm[2 + 10 + 7 * 11] = SwapLE16(1);
			lm[2 + 2 + 1 * 11] = SwapLE16(1);
			lm[2 + 1 + 2 * 11] = SwapLE16(2);
			lm[2 + 0 + 3 * 11] = SwapLE16(1);
			lm[2 + 10 + 3 * 11] = SwapLE16(1);
			// ensure the changing tiles are reserved
			// - SW-wall
			lm[2 + 11 * 11 + 4 + 10 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 5 + 10 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 6 + 10 * 11] = SwapLE16(3);
			// - NE-wall
			lm[2 + 11 * 11 + 4 + 0 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 5 + 0 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 6 + 0 * 11] = SwapLE16(3);
			// - NW-wall
			lm[2 + 11 * 11 + 0 + 4 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 0 + 5 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 0 + 6 * 11] = SwapLE16(3);
			// - SE-wall
			lm[2 + 11 * 11 + 10 + 4 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 10 + 5 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 10 + 6 * 11] = SwapLE16(3);
			// protect tiles with monsters/objects from decoration
			lm[2 + 11 * 11 + 0 + 2 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 0 + 7 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 2 + 0 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 2 + 4 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 2 + 6 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 2 + 9 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 3 + 3 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 3 + 6 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 4 + 1 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 4 + 2 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 4 + 7 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 4 + 7 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 6 + 2 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 6 + 7 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 6 + 3 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 6 + 6 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 7 + 0 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 7 + 4 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 7 + 6 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 7 + 9 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 9 + 2 * 11] = SwapLE16(3);
			lm[2 + 11 * 11 + 9 + 7 * 11] = SwapLE16(3);
		}
		{
			// patch set-piece - Diab4b.DUN
			uint16_t* lm = (uint16_t*)pSetPieces[3]._spData;
			// external tiles
			for (int y = 0; y < 9; y++) {
				for (int x = 0; x < 9; x++) {
					uint16_t wv = SwapLE16(lm[2 + x + y * 9]);
					if (wv >= 116 && wv <= 128) {
						// if (wv == 118) {
						//	wv = 128;
						// }
						lm[2 + x + y * 9] = SwapLE16(wv - 98);
					}
				}
			}
			// ensure the box is not connected to the rest of the dungeon and the changing tiles are reserved + protect inner tiles from decoration
			for (int y = 0; y < 9; y++) {
				for (int x = 0; x < 9; x++) {
					lm[2 + 9 * 9 + x + y * 9] = SwapLE16(3);
				}
			}
		}
#endif
	} else if (QuestStatus(Q_BETRAYER)) {
		pSetPieces[0]._sptype = IsMultiGame ? SPT_BETRAY_M : SPT_BETRAY_S;
		pSetPieces[0]._spData = LoadFileInMem(setpiecedata[pSetPieces[0]._sptype]._spdDunFile);
#if !USE_PATCH
		if (IsMultiGame) {
			// patch set-piece to add monsters - Vile1.DUN
			uint16_t* lm = (uint16_t*)pSetPieces[0]._spData;
			// fix corner
			lm[2 + 5 + 0 * 7] = SwapLE16(16);
			lm[2 + 6 + 1 * 7] = SwapLE16(16);
			lm[2 + 5 + 1 * 7] = SwapLE16(15);
			// use base tiles and decorate the walls randomly
			lm[2 + 0 + 0 * 7] = SwapLE16(9);
			lm[2 + 0 + 6 * 7] = SwapLE16(15);
			lm[2 + 1 + 0 * 7] = SwapLE16(2);
			lm[2 + 2 + 0 * 7] = SwapLE16(2);
			lm[2 + 3 + 0 * 7] = SwapLE16(2);
			lm[2 + 4 + 0 * 7] = SwapLE16(2);
			lm[2 + 1 + 6 * 7] = SwapLE16(2);
			lm[2 + 2 + 6 * 7] = SwapLE16(2);
			lm[2 + 4 + 6 * 7] = SwapLE16(2);
			// lm[2 + 6 + 3 * 7] = SwapLE16(50);
			// add unique monsters
			lm[2 + 7 * 7 + 7 * 7 * 2 * 2 + 3 + 6 * 7 * 2] = SwapLE16((UMT_LAZARUS + 1) | (1 << 15));
			lm[2 + 7 * 7 + 7 * 7 * 2 * 2 + 5 + 3 * 7 * 2] = SwapLE16((UMT_RED_VEX + 1) | (1 << 15));
			lm[2 + 7 * 7 + 7 * 7 * 2 * 2 + 5 + 9 * 7 * 2] = SwapLE16((UMT_BLACKJADE + 1) | (1 << 15));
			// protect inner tiles from spawning additional monsters/objects
			for (int y = 0; y <= 5; y++) {
				for (int x = 0; x <= 5; x++) {
					lm[2 + 7 * 7 + x + y * 7] |= SwapLE16((3 << 8) | (3 << 10) | (3 << 12) | (3 << 14));
				}
			}
			// ensure the box is not connected to the rest of the dungeon + protect inner tiles from decoration
			for (int y = 0; y <= 6; y++) {
				for (int x = 0; x <= 6; x++) {
					if (x == 6 && (y == 0 || y == 6)) {
						continue;
					}
					if (x == 0 || y == 0 || x == 6 || y == 6) {
						lm[2 + 7 * 7 + x + y * 7] |= SwapLE16(1);
					} else {
						lm[2 + 7 * 7 + x + y * 7] |= SwapLE16(3);
					}
				}
			}
		}
#endif
	} else if (QuestStatus(Q_WARLORD)) {
		pSetPieces[0]._sptype = SPT_WARLORD;
		pSetPieces[0]._spData = LoadFileInMem(setpiecedata[pSetPieces[0]._sptype]._spdDunFile);
#if !USE_PATCH
		// patch set-piece - Warlord.DUN
		uint16_t* lm = (uint16_t*)pSetPieces[0]._spData;
		// fix corner
		lm[2 + 6 + 1 * 8] = SwapLE16(10);
		lm[2 + 6 + 5 * 8] = SwapLE16(10);
		// separate subtiles for the automap
		lm[2 + 1 + 2 * 8] = SwapLE16(136);
		// use base tiles and decorate the walls randomly
		lm[2 + 0 + 0 * 8] = SwapLE16(9);
		lm[2 + 0 + 6 * 8] = SwapLE16(15);
		lm[2 + 1 + 0 * 8] = SwapLE16(2);
		lm[2 + 2 + 0 * 8] = SwapLE16(2);
		lm[2 + 3 + 0 * 8] = SwapLE16(2);
		lm[2 + 4 + 0 * 8] = SwapLE16(2);
		lm[2 + 5 + 0 * 8] = SwapLE16(2);
		lm[2 + 1 + 6 * 8] = SwapLE16(2);
		lm[2 + 2 + 6 * 8] = SwapLE16(2);
		lm[2 + 3 + 6 * 8] = SwapLE16(2);
		lm[2 + 4 + 6 * 8] = SwapLE16(2);
		lm[2 + 5 + 6 * 8] = SwapLE16(2);
		// lm[2 + 6 + 3 * 8] = SwapLE16(50);
		// ensure the changing tiles are protected + protect inner tiles from decoration
		for (int y = 1; y <= 5; y++) {
			for (int x = 1; x <= 7; x++) {
				lm[2 + 8 * 7 + x + y * 8] = SwapLE16(3);
			}
		}
#endif
	}
}

static void DRLG_L4SetRoom(int idx)
{
	DRLG_LoadSP(idx, DEFAULT_MEGATILE_L4);
}

/*
 * Transform dungeon by replacing values using 2x2 block patterns defined in L4ConvTbl
 * New dungeon values: 1 2 3 6 9 30
 */
static void DRLG_L4MakeMegas()
{
	int i, j;
	BYTE v;

	for (j = 0; j < DMAXY - 1; j++) {
		for (i = 0; i < DMAXX - 1; i++) {
			// assert(dungeon[i][j] <= 1 && dungeon[i + 1][j] == 1 && dungeon[i][j + 1] == 1 && dungeon[i + 1][j + 1] == 1);
			v = dungeon[i][j]
			 | (dungeon[i + 1][j] << 1)
			 | (dungeon[i][j + 1] << 2)
			 | (dungeon[i + 1][j + 1] << 3);
			assert(v != 6 && v != 9);
			dungeon[i][j] = L4ConvTbl[v];
		}
	}
	for (j = 0; j < DMAXY; j++)
		dungeon[DMAXX - 1][j] = BASE_MEGATILE_L4;
	for (i = 0; i < DMAXX - 1; i++)
		dungeon[i][DMAXY - 1] = BASE_MEGATILE_L4;
}

/*
 * Add horizontal wall.
 *
 * @param i: the x coordinate of the walls first tile
 * @param j: the y coordinate of the walls first tile
 * @param dx: the length of the wall
 * @return true if after the change a check for vertical wall is necessary.
 */
static void L4HorizWall(int i, int j, int dx)
{
	int xx;
	BYTE bv;
	// convert the internal tiles
	for (xx = 1; xx < dx; xx++) {
		dungeon[i + xx][j] = 2;
	}
	// convert the last tile
	bv = dungeon[i + dx][j];
	switch (bv) {
	case 7:
		bv = 16;
		break;
	case 9:
		bv = 11;
		break;
	case 10:
		bv = 17;
		break;
	case 15:
		bv = 14;
		break;
	case 21:
		bv = 23;
		break;
	case 22:
		bv = 29;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	dungeon[i + dx][j] = bv;
	// add 'door'
	xx = RandRange(1, dx - 3);
	dungeon[i + xx][j] = 57;
	dungeon[i + xx + 2][j] = 56;
	dungeon[i + xx + 1][j] = 50;

	// convert the first tile
	if (dungeon[i][j] == 13)
		dungeon[i][j] = 17;
	else if (dungeon[i][j] == 7)
		dungeon[i][j] = 9;
	else if (dungeon[i][j] == 16)
		dungeon[i][j] = 11;
	else // if (dungeon[i][j] == 12)
		dungeon[i][j] = 14;
}

static constexpr uint32_t HORIZ_WALL_ENDS =
	  (1 << 7)
	| (1 << 9)
	| (1 << 10)
	| (1 << 15)
	| (1 << 21)
	| (1 << 22);
static void L4AddHWall(int x, int y)
{
	int i;
	BYTE bv;

	i = x;
	while (true) {
		i++;
		bv = dungeon[i][y];
		if (bv != 6)
			break;
		if (drlgFlags[i][y])
			break;
		if (dungeon[i][y - 1] != 6)
			break;
		if (dungeon[i][y + 1] != 6)
			break;
	}

	i -= x;
	if (i > 3
	// && (bv == 7 || bv == 9 || bv == 10 || bv == 15 || bv == 21 || bv == 22)) {
	 && (bv < 23 && (HORIZ_WALL_ENDS & (1 << bv)))) {
		L4HorizWall(x, y, i);
	}
}

/*
 * Add vertical wall.
 *
 * @param i: the x coordinate of the walls first tile
 * @param j: the y coordinate of the walls first tile
 * @param dy: the length of the wall
 */
static void L4VertWall(int i, int j, int dy)
{
	int yy;
	BYTE bv;
	// convert the first tile
	if (dungeon[i][j] == 14)
		dungeon[i][j] = 17;
	else if (dungeon[i][j] == 8)
		dungeon[i][j] = 9;
	else if (dungeon[i][j] == 15)
		dungeon[i][j] = 10;
	else // if (dungeon[i][j] == 12)
		dungeon[i][j] = 13;
	// convert the internal tiles
	for (yy = 1; yy < dy; yy++) {
		dungeon[i][j + yy] = 1;
	}
	// convert the last tile
	bv = dungeon[i][j + dy];
	switch (bv) {
	case 8:
		bv = 15;
		break;
	case 9:
		bv = 10;
		break;
	case 11:
		bv = 17;
		break;
	case 16:
		bv = 13;
		break;
	case 21:
		bv = 22;
		break;
	case 23:
		bv = 29;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	dungeon[i][j + dy] = bv;
	// add 'door'
	yy = RandRange(1, dy - 3);
	dungeon[i][j + yy] = 53;
	dungeon[i][j + yy + 2] = 52;
	dungeon[i][j + yy + 1] = 50;
}

static constexpr uint32_t VERT_WALL_ENDS =
	  (1 << 8)
	| (1 << 9)
	| (1 << 11)
	| (1 << 16)
	| (1 << 21)
	| (1 << 23);
static void L4AddVWall(int x, int y)
{
	int j;
	BYTE bv;

	j = y;
	while (true) {
		j++;
		bv = dungeon[x][j];
		if (bv != 6)
			break;
		if (drlgFlags[x][j])
			break;
		if (dungeon[x - 1][j] != 6)
			break;
		if (dungeon[x + 1][j] != 6)
			break;
	}

	j -= y;
	if (j > 3
	 //&& (bv == 8 || bv == 9 || bv == 11 || bv == 16 || bv == 21 || bv == 23)) {
	 && (bv < 24 && (VERT_WALL_ENDS & (1 << bv)))) {
		L4VertWall(x, y, j);
	}
}

/*
 * Draw walls between pillars.
 * New dungeon values: 2 11 14 17 23 29 56 57 58 59 60
 *                     1 9 10 13 17 22 29 52 53 54 55 60
 */
static void L4AddWall()
{
	int i, j;
	bool checkHoriz, checkVert;

	for (i = 0; i < DMAXX; i++) {
		for (j = 0; j < DMAXY; j++) {
			if (drlgFlags[i][j]) {
				continue;
			}
			checkHoriz = false;
			checkVert = false;
			switch (dungeon[i][j]) {
			case 12:
				checkHoriz = true;
				checkVert = true;
				break;
			case 7:
			case 13:
			case 16:
				checkHoriz = true;
				break;
			case 8:
			case 14:
			case 15:
				checkVert = true;
				break;
			}
			if (checkHoriz) {
				L4AddHWall(i, j);
			}
			if (checkVert) {
				L4AddVWall(i, j);
			}
		}
	}
}

void EnsureSWConnection(int x, int y)
{
	assert(y < DMAXY - 1);
	y++;
	if (dungeon[x][y] == 2) {
		dungeon[x][y] = 14;
	} else if (dungeon[x][y] == 9) {
		dungeon[x][y] = 10;
	} else if (dungeon[x][y] == 11) {
		dungeon[x][y] = 17;
	} else if (dungeon[x][y] == 28) {
		dungeon[x][y] = 25; // new wall type
	} else if (dungeon[x][y] == 30) {
		dungeon[x][y] = 24;
	} /*else {
		assert(dungeon[x][y] == 15 || dungeon[x][y] == 17
			|| dungeon[x][y] == 18 || dungeon[x][y] == 24);
	}*/
}

void EnsureSEConnection(int x, int y)
{
	assert(x < DMAXX - 1);
	x++;
	if (dungeon[x][y] == 1) {
		dungeon[x][y] = 13;
	} else if (dungeon[x][y] == 10) {
		dungeon[x][y] = 17;
	} else if (dungeon[x][y] == 9) {
		dungeon[x][y] = 11;
	} else if (dungeon[x][y] == 27) {
		dungeon[x][y] = 26; // new wall type
	} else if (dungeon[x][y] == 30) {
		dungeon[x][y] = 24;
	} /*else {
		assert(dungeon[x][y] == 16
		 || dungeon[x][y] == 19 || dungeon[x][y] == 24);
	}*/
}

#if DEBUG_MODE
static void ValidateNWConnection(int x, int y, bool full)
{
	assert(x > 0);
	x--;
	if (dungeon[x][y] == 19 || dungeon[x][y] == 21
	 || dungeon[x][y] == 22 || dungeon[x][y] == 23
	 || dungeon[x][y] == 26 || dungeon[x][y] == 27
	 || dungeon[x][y] == 29)
		return;
	assert(full);
	assert(dungeon[x][y] == 2 || dungeon[x][y] == 8
	 || dungeon[x][y] == 9 || dungeon[x][y] == 10
	 || dungeon[x][y] == 11 || dungeon[x][y] == 14
	 || dungeon[x][y] == 15 || dungeon[x][y] == 17);
}

static void ValidateNEConnection(int x, int y, bool full)
{
	assert(y > 0);
	y--;
	if (dungeon[x][y] == 18 || dungeon[x][y] == 21
	 || dungeon[x][y] == 22 || dungeon[x][y] == 23
	 || dungeon[x][y] == 25 || dungeon[x][y] == 28
	 || dungeon[x][y] == 29)
		return;
	assert(full);
	assert(dungeon[x][y] == 1 || dungeon[x][y] == 7
	 || dungeon[x][y] == 9 || dungeon[x][y] == 10
	 || dungeon[x][y] == 11 || dungeon[x][y] == 13
	 || dungeon[x][y] == 16 || dungeon[x][y] == 17);
}

static void ValidateSWConnection(int x, int y)
{
	assert(y < DMAXY - 1);
	y++;
	assert(dungeon[x][y] == 14 || dungeon[x][y] == 10
		 || dungeon[x][y] == 15 || dungeon[x][y] == 17
		 || dungeon[x][y] == 18 || dungeon[x][y] == 24
		 || dungeon[x][y] == 25);
}

static void ValidateSEConnection(int x, int y)
{
	assert(x < DMAXX - 1);
	x++;
	assert(dungeon[x][y] == 11 || dungeon[x][y] == 13
	 || dungeon[x][y] == 16 || dungeon[x][y] == 17
	 || dungeon[x][y] == 19 || dungeon[x][y] == 24
	 || dungeon[x][y] == 26);
}
/* possible configurations:
		[ 1, 6, 9 ]
	[ 2, 6, 30]  	1		[ 6 ]
		[ 1, 2, 3, 6, 9, 30 ]

		[ 1, 6, 30 ]
	[ 2, 6, 9 ]	2		[ 1, 2, 3, 6, 9, 30 ]
		[ 6 ]

		[ 1, 6, 9 ]
	[ 2, 6, 9 ]	3		[ 6 ]
		[ 6 ]

		[ 1, 6, 30 ]
	[ 2, 6, 30 ]	9		[ 2 3 6 ]
		[ 1, 3, 6 ]

		[ 1, 6, 30 ]
	[ 2, 6, 30 ]	30		 [ 1, 9, 30 ]
		[ 2, 9, 30 ]

		[ 1, 2, 3, 6, 9 ]
	[ 1, 2, 3, 6, 9 ] 6 	[ 1, 2, 3, 6, 9, 30 ]
		[ 1, 2, 3, 6, 9, 30 ]
*/
#endif

/*
 * Draw wall around the tiles selected by L4FirstRoom (and L4ConnectBlock).
 * New dungeon values: 4 .. 29 except 20
 */
static void L4TileFix()
{
	int i, j;

	// convert tiles based on the tile above and left from them.
	// Connect, add or cut walls.
	for (i = DMAXX - 1; i > 0; i--) {
		for (j = DMAXY - 1; j > 0; j--) {
			switch (dungeon[i][j]) {
			case 1:
				//		[ 6(L2:16, L6:7, L30: 16) ]
				// [ 2(13) ]  	1
				if (dungeon[i][j - 1] == 6) {
					if (dungeon[i - 1][j] == 2 || dungeon[i - 1][j] == 30) {
						dungeon[i][j] = 16; // connect + cut || new wall (NW) + cut
					} else {
						assert(dungeon[i - 1][j] == 6);
						dungeon[i][j] = 7; // cut
					}
				} else {
					if (dungeon[i - 1][j] == 2) {
						dungeon[i][j] = 13; // connect
					}
				}
				break;
			case 2:
			//													[ 1(14) ]
			// [ 6(T1:15, T6:8, T30:15) ]	2
				if (dungeon[i - 1][j] == 6) {
					if (dungeon[i][j - 1] == 1) {
						dungeon[i][j] = 15; // connect + cut
					} else if (dungeon[i][j - 1] == 6) {
						dungeon[i][j] = 8; // cut
					} else {
						assert(dungeon[i][j - 1] == 30);
						dungeon[i][j] = 15; // new wall (NE) + cut
					}
				} else {
					if (dungeon[i][j - 1] == 1) {
						dungeon[i][j] = 14; // connect
					}
				}
				break;
			case 3:
			case 6:
				//				[ 1(4), 9(4) ]
				// [ 2(5), 9(5) ]	[3, 6 ]
				if (dungeon[i - 1][j] == 2 || dungeon[i - 1][j] == 9) {
					if (dungeon[i][j - 1] == 1 || dungeon[i][j - 1] == 9) {
						dungeon[i][j] = 12; // connect
					} else {
						dungeon[i][j] = 5; // connect
					}
				} else {
					if (dungeon[i][j - 1] == 1 || dungeon[i][j - 1] == 9) {
						dungeon[i][j] = 4; // connect
					}
				}
				break;
			case 9:
				//			[ 1(10) ]
				// [ 2(11) ]	9
				if (dungeon[i - 1][j] == 2) {
					if (dungeon[i][j - 1] == 1) {
						dungeon[i][j] = 17; // connect
					} else {
						dungeon[i][j] = 11; // connect
					}
				} else {
					if (dungeon[i][j - 1] == 1) {
						dungeon[i][j] = 10; // connect
					}
				}
				break;
			case 30:
				//			[ 1(27), 6(19) ]
				// [ 2(28), 6(18) ]	30
				if (dungeon[i - 1][j] == 2) {
					if (dungeon[i][j - 1] == 1) {
						dungeon[i][j] = 29; // connect + new wall (SW, SE)
					} else if (dungeon[i][j - 1] == 6) {
						dungeon[i][j] = 23; // connect + new wall (SW, SE)
					} else {
						assert(dungeon[i][j - 1] == 30);
						dungeon[i][j] = 28; // connect + new wall (SW)
					}
				} else if (dungeon[i - 1][j] == 6) {
					if (dungeon[i][j - 1] == 1) {
						dungeon[i][j] = 22; // connect + new wall (SW, SE)
					} else if (dungeon[i][j - 1] == 6) {
						dungeon[i][j] = 21; // new wall (SW, SE)
					} else {
						assert(dungeon[i][j - 1] == 30);
						dungeon[i][j] = 18; // new wall (SW, NE)
					}
				} else {
					assert(dungeon[i - 1][j] == 30);
					if (dungeon[i][j - 1] == 1) {
						dungeon[i][j] = 27; // connect + new wall (SE)
					} else if (dungeon[i][j - 1] == 6) {
						dungeon[i][j] = 19; // new wall (NW, SE)
					} else {
						assert(dungeon[i][j - 1] == 30);
					}
				}
				break;
			default:
				ASSUME_UNREACHABLE
				break;
			}
		}
	}

	// apply the same logic to the first row/column
	for (i = DMAXX - 1; i > 0; i--) {
		if (dungeon[i][0] == 30) {
			// [ 2(28) ]	30
			if (dungeon[i - 1][0] == 2) {
				dungeon[i][0] = 28; // connect + new wall (SW)
			}
		} else if (dungeon[i][0] == 9) {
			// [ 2(11) ]	9
			if (dungeon[i - 1][0] == 2) {
				dungeon[i][0] = 11; // connect
			}
		}
	}
	for (j = DMAXY - 1; j > 0; j--) {
		if (dungeon[0][j] == 30) {
			// [ 1(27) ]
			//	30
			if (dungeon[0][j - 1] == 1) {
				dungeon[0][j] = 27; // connect + new wall (SE)
			} else {
				assert(dungeon[0][j - 1] == 30);
			}
		} else if (dungeon[0][j] == 9) {
			// [ 1(10) ]
			//	9
			if (dungeon[0][j - 1] == 1) {
				dungeon[0][j] = 10; // connect
			}
		}
	}

	// connect the new walls
	for (i = 0; i < DMAXX; i++) {
		for (j = 0; j < DMAXY; j++) {
			switch (dungeon[i][j]) {
			case 19: // new wall (NW, SE) 30
				//assert(j > 0);
				//assert(dungeon[i][j - 1] == 6);
				// check SE (29-SE)
				EnsureSEConnection(i, j);
				break;
			case 18: // new wall (SW, NE) 30
			case 28: // new wall (SW) 30
			case 25: // new wall type (SW) 30
				// check SW (21-SW)
				EnsureSWConnection(i, j);
				break;
			case 21: // new wall (SW, SE) 30
			case 22: // new wall (SW, SE) 30
			case 23: // new wall (SW, SE) 30
			case 29: // new wall (SW, SE) 30
				// check SW (21-SW)
				EnsureSWConnection(i, j);
				// check SE (29-SE)
				EnsureSEConnection(i, j);
				break;
				break;
			case 26: // new wall type (SE) 30
			case 27: // new wall (SE) 30
				assert(i < DMAXX - 1);
				if (dungeon[i + 1][j] == 9) {
					dungeon[i + 1][j] = 11;
				}
				break;
			}
		}
	}

#if DEBUG_MODE
	for (i = 0; i < DMAXX; i++) {
		for (j = 0; j < DMAXY; j++) {
			switch (dungeon[i][j]) {
			case 28: // new wall (SW) 30
			case 25: // new wall type (SW) 30
				// check SW (21-SW)
				ValidateSWConnection(i, j);
				break;
			case 29: // new wall (SW, SE) 30
			case 23: // new wall (SW, SE) 30
			case 22: // new wall (SW, SE) 30
			case 21: // new wall (SW, SE) 30
				// check SW (21-SW)
				ValidateSWConnection(i, j);
				// check SE (29-SE)
				ValidateSEConnection(i, j);
				break;
			case 18: // new wall (SW, NE) 30
				// check SW (21-SW)
				ValidateSWConnection(i, j);
				// check NE
				ValidateNEConnection(i, j, false);
				break;
			case 26: // new wall type (SE) 30
			case 27: // new wall (SE) 30
				assert(i < DMAXX - 1);
				assert(dungeon[i + 1][j] == 11
				 || dungeon[i + 1][j] == 16 || dungeon[i + 1][j] == 19);
				break;
			case 19: // new wall (NW, SE) 30
				assert(j > 0);
				assert(dungeon[i][j - 1] == 6);
				// check SE (29-SE)
				ValidateSEConnection(i, j);
				// check NW
				ValidateNWConnection(i, j, false);
				break;
			case 15: // new wall (NE) 2
				ValidateNEConnection(i, j, true);
				break;
			case 16: // new wall (NW) 1
				ValidateNWConnection(i, j, true);
				break;
			}
		}
	}
#endif
}

/*
 * Replace undecorated tiles with matching decorated tiles.
 * New dungeon values: 49 50 51 (49..51)   61 62 63 64 65 66 67 68 69 70 (61..70)  77 78 79 80 81 82 83 (77..83)
 */
static void DRLG_L4Subs()
{
	int x, y;
	BYTE c, k;
	int8_t rv;
	const unsigned MAX_MATCH = 8; // 6;
	const unsigned NUM_L4TYPES = 84;
	static_assert(MAX_MATCH <= INT8_MAX, "MAX_MATCH does not fit to rv(int8_t) in DRLG_L4Subs.");
	static_assert(NUM_L4TYPES <= UCHAR_MAX, "NUM_L4TYPES does not fit to i(BYTE) in DRLG_L4Subs.");
#if DEBUG_MODE
	for (int i = sizeof(L4BTYPES) - 1; i >= 0; i--) {
		if (L4BTYPES[i] != 0) {
			if ((unsigned)i >= NUM_L4TYPES)
				app_fatal("Value %d is ignored in L4BTYPES at %d", L4BTYPES[i], i);
			break;
		}
	}

	for (unsigned i = 0; i < sizeof(L4BTYPES); i++) {
		c = L4BTYPES[i];
		if (c == 0)
			continue;
		x = 0;
		for (unsigned j = 0; j < sizeof(L4BTYPES); j++) {
			if (c == L4BTYPES[j])
				x++;
		}
		if ((unsigned)x > MAX_MATCH)
			app_fatal("Too many(%d) matching('%d') values in L4BTYPES", x, c);
	}
#endif
	for (x = 0; x < DMAXX; x++) {
		for (y = 0; y < DMAXY; y++) {
			if (random_(0, 3) == 0) {
				c = L4BTYPES[dungeon[x][y]];
				if (c != 0 && (drlgFlags[x][y] & DRLG_FROZEN) == 0) {
					rv = random_(0, MAX_MATCH);
					k = 0;
					while (true) {
						if (c == L4BTYPES[k] && --rv < 0)
							break;
						if (++k == NUM_L4TYPES)
							k = 0;
					}
					dungeon[x][y] = k;
				}
			}
		}
	}
}

/** Fill dungeon based on the dungBlock matrix. */
static void L4Block2Dungeon()
{
	int i, j;
	static_assert(sizeof(dungeon[0][0]) == 1, "memset on dungeon does not work in L4Block2Dungeon.");
	if (sizeof(dungeon) != L4BLOCKX * L4BLOCKY * 4 || sizeof(dungeon[0]) != L4BLOCKY * 2) {
		memset(dungeon, BASE_MEGATILE_L4, sizeof(dungeon));
	}
	for (j = 0; j < L4BLOCKY; j++) {
		for (i = 0; i < L4BLOCKX; i++) {
			dungeon[i][j] = drlg.dungBlock[i][j];
		}
	}
	for (j = 0; j < L4BLOCKY; j++) {
		for (i = 0; i < L4BLOCKX; i++) {
			dungeon[i][j + L4BLOCKY] = drlg.dungBlock[i][L4BLOCKY - 1 - j];
		}
	}
	for (j = 0; j < L4BLOCKY; j++) {
		for (i = 0; i < L4BLOCKX; i++) {
			dungeon[i + L4BLOCKX][j] = drlg.dungBlock[L4BLOCKX - 1 - i][j];
		}
	}
	for (j = 0; j < L4BLOCKY; j++) {
		for (i = 0; i < L4BLOCKX; i++) {
			dungeon[i + L4BLOCKX][j + L4BLOCKY] = drlg.dungBlock[L4BLOCKX - 1 - i][L4BLOCKY - 1 - j];
		}
	}
}

static int L4SelectPos(const BYTE (&hall)[20])
{
	int i, n, rv;
	BYTE match[20];
	n = 0;
	for (i = 20 - 3; i >= 0; i--) {
		if (hall[i] != 0 && hall[i] == hall[i + 1]) {
			match[n] = i;
			n++;
		}
	}
	// assert(n != 0);
	rv = random_low(0, n);
	return match[rv];
}

/*
 * Create link between the quarters (blocks) of the dungeon.
 */
static void L4ConnectBlock()
{
	int j, i, rv;
	BYTE hallok[std::max(L4BLOCKX, L4BLOCKY)];
	// find the right side of the rooms
	for (j = L4BLOCKY - 1; j >= 0; j--) {
		for (i = L4BLOCKX - 1; i > 0; i--) {
			if (drlg.dungBlock[i][j] == 1) {
				break;
			}
		}
		hallok[j] = i;
	}
	// select a position with matching 2 tiles-wide ending
	rv = L4SelectPos(hallok);
	// connect to the right side of the dungeon
	for (i = L4BLOCKX - 1; i > hallok[rv]; i--) {
		drlg.dungBlock[i][rv] = 1;
		drlg.dungBlock[i][rv + 1] = 1;
	}

	// find the bottom side of the rooms
	for (i = L4BLOCKX - 1; i >= 0; i--) {
		for (j = L4BLOCKY - 1; j > 0; j--) {
			if (drlg.dungBlock[i][j] == 1) {
				break;
			}
		}
		hallok[i] = j;
	}
	// select a position with matching 2 tiles-wide ending
	rv = L4SelectPos(hallok);
	// connect to the bottom side of the dungeon
	for (j = L4BLOCKY - 1; j > hallok[rv]; j--) {
		drlg.dungBlock[rv][j] = 1;
		drlg.dungBlock[rv + 1][j] = 1;
	}
}

static int DRLG_L4GetArea()
{
	int i, rv;
	BYTE* pTmp;

	rv = 0;
	static_assert(sizeof(drlg.dungBlock) == L4BLOCKX * L4BLOCKY, "Linear traverse of dungBlock does not work in DRLG_L4GetArea.");
	pTmp = &drlg.dungBlock[0][0];
	for (i = 0; i < L4BLOCKX * L4BLOCKY; i++, pTmp++) {
		assert(*pTmp <= 1);
		rv += *pTmp;
	}

	return rv;
}

static void L4DrawRoom(int x, int y, int width, int height)
{
	int i, j, x2, y2;

	x2 = x + width;
	y2 = y + height;
	for (j = y; j < y2; j++) {
		for (i = x; i < x2; i++) {
			drlg.dungBlock[i][j] = 1;
		}
	}
}

static bool L4CheckRoom(int x, int y, int width, int height)
{
	int i, j, x2, y2;

	if (x < 0 || y < 0)
		return false;

	x2 = x + width;
	y2 = y + height;
	if (x2 > L4BLOCKX || y2 > L4BLOCKY)
		return false;

	for (j = y; j < y2; j++) {
		for (i = x; i < x2; i++) {
			if (drlg.dungBlock[i][j] != 0) {
				return false;
			}
		}
	}

	return true;
}

/* L4CheckVHall and L4CheckHHall
 *  make sure there is at least one empty block between the rooms
 */
static bool L4CheckVHall(int x, int top, int h)
{
	int j = top, bottom;

	//assert((unsigned)x < L4BLOCKX);
	if (j < 0)
		return false;
	bottom = j + h;
	if (bottom >= L4BLOCKY)
		return false;
	while (j < bottom && drlg.dungBlock[x][j] == 0)
		j++;
	while (j < bottom && drlg.dungBlock[x][j] == 1)
		j++;
	while (j < bottom && drlg.dungBlock[x][j] == 0)
		j++;
	return j == bottom;
}

static bool L4CheckHHall(int y, int left, int w)
{
	int i = left, right;

	//assert((unsigned)y < L4BLOCKY);
	if (i < 0)
		return false;
	right = i + w;
	if (right >= L4BLOCKX)
		return false;
	while (i < right && drlg.dungBlock[i][y] == 0)
		i++;
	while (i < right && drlg.dungBlock[i][y] == 1)
		i++;
	while (i < right && drlg.dungBlock[i][y] == 0)
		i++;
	return i == right;
}

static void L4RoomGen(int x, int y, int w, int h, bool dir)
{
	int dirProb, i, width, height, rx, ry, rxy2;

	dirProb = random_(0, 4);

	if (dir == (dirProb == 0)) {
		// try to place a room to the left
		for (i = 20; i != 0; i--) {
			width = RandRange(2, 6) & ~1;
			height = RandRange(2, 6) & ~1;
			ry = h / 2u + y - height / 2u;
			rx = x - width;
			if (L4CheckVHall(x, ry - 1, height + 2)
			 && L4CheckRoom(rx - 1, ry - 1, width + 1, height + 2)) { /// BUGFIX: swap args 3 and 4 ("ch+2" and "cw+1") (fixed)
				// - add room to the left
				L4DrawRoom(rx, ry, width, height);
				break;
			}
		}
		if (i != 0) {
			// room added to the left -> force similar room on the right side
			i = 1;
		} else {
			// room was not added to the left -> try more options on the right
			rx = -1;
			i = 20;
		}
		// try to place a room to the right
		rxy2 = x + w;
		while (true) {
			if (L4CheckVHall(rxy2 - 1, ry - 1, height + 2)
			 && L4CheckRoom(rxy2, ry - 1, width + 1, height + 2)) {
				// - add room to the right
				L4DrawRoom(rxy2, ry, width, height);
				break;
			}
			if (--i == 0)
				break;
			width = RandRange(2, 6) & ~1;
			height = RandRange(2, 6) & ~1;
			ry = h / 2u + y - height / 2u;
		}
		// proceed with the placed a room on the left
		if (rx >= 0)
			L4RoomGen(rx, ry, width, height, true);
		// proceed with the placed a room on the right
		if (i != 0)
			L4RoomGen(rxy2, ry, width, height, true);
	} else {
		// try to place a room to the top
		for (i = 20; i != 0; i--) {
			width = RandRange(2, 6) & ~1;
			height = RandRange(2, 6) & ~1;
			rx = w / 2u + x - width / 2u;
			ry = y - height;
			if (L4CheckHHall(y, rx - 1, width + 2)
			 && L4CheckRoom(rx - 1, ry - 1, width + 2, height + 1)) {
				// - add room to the top
				L4DrawRoom(rx, ry, width, height);
				break;
			}
		}

		if (i != 0) {
			// room added to the top -> force similar room on the bottom side
			i = 1;
		} else {
			// room was not added to the top -> try more options on the bottom
			ry = -1;
			i = 20;
		}
		// try to place a room to the bottom
		rxy2 = y + h;
		while (true) {
			if (L4CheckHHall(rxy2 - 1, rx - 1, width + 2)
			 && L4CheckRoom(rx - 1, rxy2, width + 2, height + 1)) {
				// - add room to the bottom
				L4DrawRoom(rx, rxy2, width, height);
				break;
			}
			if (--i == 0)
				break;
			width = RandRange(2, 6) & ~1;
			height = RandRange(2, 6) & ~1;
			rx = w / 2u + x - width / 2u;
		}
		// proceed with the placed a room on the top
		if (ry >= 0)
			L4RoomGen(rx, ry, width, height, false);
		// proceed with the placed a room on the bottom
		if (i != 0)
			L4RoomGen(rx, rxy2, width, height, false);
	}
}

/*
 * Create dungeon blueprint, but leave the border of dungBlock empty.
 * New dungeon (dungBlock) values: 1
 */
static void L4FirstRoom()
{
	int x, y, w, h, xmin, xmax, ymin, ymax;

	if (currLvl._dLevelIdx != DLV_HELL4) {
		if (pSetPieces[0]._spData != NULL) { // pSetPieces[0]._sptype != SPT_NONE
			w = SwapLE16(*(uint16_t*)&pSetPieces[0]._spData[0]) + 4; // TODO: add border to the setmaps
			h = SwapLE16(*(uint16_t*)&pSetPieces[0]._spData[2]) + 4;
			if (pSetPieces[0]._sptype == SPT_WARLORD)
				w--;
		} else {
			w = RandRange(2, 6) & ~1;
			h = RandRange(2, 6) & ~1;
		}
	} else {
		w = DQUAD_ROOM_SIZE;
		h = DQUAD_ROOM_SIZE;
	}

	xmax = L4BLOCKX - 1 - w;
	xmin = (xmax + 1) >> 1;
	x = RandRange(xmin, xmax);

	ymax = L4BLOCKY - 1 - h;
	ymin = (ymax + 1) >> 1;
	y = RandRange(ymin, ymax);

	if (currLvl._dLevelIdx != DLV_HELL4) {
		if (pSetPieces[0]._sptype != SPT_NONE) {
			pSetPieces[0]._spx = x + 1;
			pSetPieces[0]._spy = y + 1;
		}
	} else {
		int setpc_x = x + 1;
		int setpc_y = y + 1;
		pSetPieces[0]._spx = DIAB_QUAD_1X;
		pSetPieces[0]._spy = DIAB_QUAD_1Y;
		pSetPieces[1]._spx = DIAB_QUAD_2X;
		pSetPieces[1]._spy = DIAB_QUAD_2Y;
		pSetPieces[2]._spx = DIAB_QUAD_3X;
		pSetPieces[2]._spy = DIAB_QUAD_3Y;
		pSetPieces[3]._spx = DIAB_QUAD_4X;
		pSetPieces[3]._spy = DIAB_QUAD_4Y;
	}

	L4DrawRoom(x, y, w, h);
	static_assert((int)true == 1, "Bool to int conversion in L4FirstRoom.");
	L4RoomGen(x, y, w, h, random_(0, 2));
}

static void DRLG_LoadDiabQuads()
{
	DRLG_L4SetRoom(0);
	DRLG_L4SetRoom(1);
	DRLG_L4SetRoom(2);
	DRLG_L4SetRoom(3);
}

/*
 * Spread transVals further.
 * - spread transVals on corner tiles to make transparency smoother.
 * - spread transVals between 'rooms' where the special 'door'-tiles stop the standard spread.
 */
static void DRLG_L4TransFix()
{
	int i, j, xx, yy;
	// BYTE tv;

	yy = DBORDERY;
	for (j = 0; j < DMAXY; j++) {
		xx = DBORDERX;
		for (i = 0; i < DMAXX; i++) {
			switch (dungeon[i][j]) {
			//* commented out because a simplified version is added below
			/*case 23:
			case 25:
			case 28:
				if (dungeon[i][j - 1] == 18) {
					DRLG_CopyTrans(xx, yy, xx + 1, yy);
					DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				}
				break;
			case 22:
			case 26:
			case 27:
				if (dungeon[i + 1][j] == 19) {
					DRLG_CopyTrans(xx, yy, xx, yy + 1);
					DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				}
				break;
			case 18:
				DRLG_CopyTrans(xx, yy, xx + 1, yy);
				DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;
			case 19:
				DRLG_CopyTrans(xx, yy, xx, yy + 1);
				DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;*/
			// fix transVals of corners
			// case 20:
			// case 118:
			case 24:
			//case 122:
				DRLG_CopyTrans(xx, yy, xx + 1, yy);
				DRLG_CopyTrans(xx, yy, xx, yy + 1);
				//DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;
			case 25:
			//case 123:
				//if (dungeon[i][j - 1] != 116) {
				//	break;
				//}
				/* fall-through */
			case 18:
			//case 116:
				DRLG_CopyTrans(xx, yy, xx + 1, yy);
				//DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;
			case 26:
			//case 124:
				//if (dungeon[i + 1][j] != 117) {
				//	break;
				//}
				/* fall-through */
			case 19:
			//case 117:
				DRLG_CopyTrans(xx, yy, xx, yy + 1);
				//DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;
			/*// fix transVals of 'doors'
			case 52:
				DRLG_CopyTrans(xx + 1, yy + 1, xx, yy);
				break;
			case 53:
				DRLG_CopyTrans(xx + 1, yy, xx, yy);
				DRLG_CopyTrans(xx + 1, yy + 1, xx, yy + 1);
				break;
			case 56:
				DRLG_CopyTrans(xx + 1, yy + 1, xx, yy);
				break;
			case 57:
				DRLG_CopyTrans(xx, yy + 1, xx + 1, yy);
				break;
			case 60:
				DRLG_CopyTrans(xx + 1, yy + 1, xx, yy);
				DRLG_CopyTrans(xx + 1, yy + 1, xx + 1, yy);
				DRLG_CopyTrans(xx + 1, yy + 1, xx, yy + 1);
				break;
			case 54:
				tv = dTransVal[xx][yy];
				if (tv != 0 && dTransVal[xx][yy + 1] == tv) {
					// the tile has a valid tv - use it
					dTransVal[xx + 1][yy] = tv;
					dTransVal[xx + 1][yy + 1] = tv;
					// fix the wall on the south side - in case the tile is in the corner
					if (dTransVal[xx + 1][yy + 2] == 0)
						dTransVal[xx + 1][yy + 2] = tv;
					// fix the wall on the north side - in case 55 is not placed
					if (dTransVal[xx + 1][yy - 1] == 0)
						dTransVal[xx + 1][yy - 1] = tv;
					break;
				}
				tv = dTransVal[xx][yy + 1];
				if (tv != 0) {
					// there is a valid tv on the south side - use it
					dTransVal[xx][yy] = tv;
					dTransVal[xx + 1][yy] = tv;
					// fix the wall on the west side
					if (dTransVal[xx - 1][yy] == 0)
						dTransVal[xx - 1][yy] = tv;
					if (dTransVal[xx - 1][yy + 1] == 0)
						dTransVal[xx - 1][yy + 1] = tv;
					// fix the wall on the east side
					if (dTransVal[xx + 2][yy] == 0)
						dTransVal[xx + 2][yy] = tv;
					if (dTransVal[xx + 2][yy + 1] == 0)
						dTransVal[xx + 2][yy + 1] = tv;
					// fix the wall on the north side - in case 55 is not placed
					if (dTransVal[xx][yy - 1] == 0)
						dTransVal[xx][yy - 1] = tv;
					if (dTransVal[xx + 1][yy - 1] == 0)
						dTransVal[xx + 1][yy - 1] = tv;
					break;
				}
				// no valid tv on the south side -> there must be a one on the north side
				tv = dTransVal[xx][yy - 3];
				assert(tv != 0);
				// use it on the current tile
				dTransVal[xx][yy] = tv;
				dTransVal[xx + 1][yy] = tv;
				dTransVal[xx][yy + 1] = tv;
				dTransVal[xx + 1][yy + 1] = tv;
				// fix the wall on the south side
				assert(dTransVal[xx][yy + 2] == 0);
				dTransVal[xx][yy + 2] = tv;
				assert(dTransVal[xx + 1][yy + 2] == 0);
				dTransVal[xx + 1][yy + 2] = tv;
				// fix the wall on the west side
				if (dTransVal[xx - 1][yy] == 0)
					dTransVal[xx - 1][yy] = tv;
				if (dTransVal[xx - 1][yy + 1] == 0)
					dTransVal[xx - 1][yy + 1] = tv;
				// fix the corner on south-west
				if (dTransVal[xx - 1][yy + 2] == 0)
					dTransVal[xx - 1][yy + 2] = tv;
				// fix the wall on the east side
				if (dTransVal[xx + 2][yy] == 0)
					dTransVal[xx + 2][yy] = tv;
				if (dTransVal[xx + 2][yy + 1] == 0)
					dTransVal[xx + 2][yy + 1] = tv;
				// fix the corner on south-east
				if (dTransVal[xx + 2][yy + 2] == 0)
					dTransVal[xx + 2][yy + 2] = tv;
				break;
			case 55:
				tv = dTransVal[xx][yy + 1];
				if (tv != 0 && dTransVal[xx][yy] == tv) {
					// the tile has a valid tv - use it
					dTransVal[xx + 1][yy] = tv;
					dTransVal[xx + 1][yy + 1] = tv;
					// fix the wall on the north side - in case the tile is in the corner
					if (dTransVal[xx + 1][yy - 1] == 0)
						dTransVal[xx + 1][yy - 1] = tv;
					// fix the wall on the south side - in case 54 is not placed
					if (dTransVal[xx + 1][yy + 2] == 0)
						dTransVal[xx + 1][yy + 2] = tv;
					break;
				}
				tv = dTransVal[xx][yy];
				if (tv != 0) {
					// there is a valid tv on the north side - use it
					dTransVal[xx][yy + 1] = tv;
					dTransVal[xx + 1][yy + 1] = tv;
					// fix the wall on the west side
					if (dTransVal[xx - 1][yy] == 0)
						dTransVal[xx - 1][yy] = tv;
					if (dTransVal[xx - 1][yy + 1] == 0)
						dTransVal[xx - 1][yy + 1] = tv;
					// fix the wall on the east side
					if (dTransVal[xx + 2][yy] == 0)
						dTransVal[xx + 2][yy] = tv;
					if (dTransVal[xx + 2][yy + 1] == 0)
						dTransVal[xx + 2][yy + 1] = tv;
					// fix the wall on the south side - in case 54 is not placed
					if (dTransVal[xx][yy + 2] == 0)
						dTransVal[xx][yy + 2] = tv;
					if (dTransVal[xx + 1][yy + 2] == 0)
						dTransVal[xx + 1][yy + 2] = tv;
					break;
				}
				// no valid tv on the north side -> there must be a one on the south side
				tv = dTransVal[xx][yy + 4];
				assert(tv != 0);
				// use it on the current tile
				dTransVal[xx][yy] = tv;
				dTransVal[xx + 1][yy] = tv;
				dTransVal[xx][yy + 1] = tv;
				dTransVal[xx + 1][yy + 1] = tv;
				// fix the wall on the north side
				assert(dTransVal[xx][yy - 1] == 0);
				dTransVal[xx][yy - 1] = tv;
				assert(dTransVal[xx + 1][yy - 1] == 0);
				dTransVal[xx + 1][yy - 1] = tv;
				// fix the wall on the west side
				if (dTransVal[xx - 1][yy] == 0)
					dTransVal[xx - 1][yy] = tv;
				if (dTransVal[xx - 1][yy + 1] == 0)
					dTransVal[xx - 1][yy + 1] = tv;
				// fix the corner on north-west
				if (dTransVal[xx - 1][yy - 1] == 0)
					dTransVal[xx - 1][yy - 1] = tv;
				// fix the wall on the east side
				if (dTransVal[xx + 2][yy] == 0)
					dTransVal[xx + 2][yy] = tv;
				if (dTransVal[xx + 2][yy + 1] == 0)
					dTransVal[xx + 2][yy + 1] = tv;
				// fix the corner on north-east
				if (dTransVal[xx + 2][yy - 1] == 0)
					dTransVal[xx + 2][yy - 1] = tv;
				break;
			case 58:
				tv = dTransVal[xx + 1][yy];
				if (tv != 0 && dTransVal[xx][yy] == tv) {
					// the tile has a valid tv - use it
					dTransVal[xx][yy + 1] = tv;
					dTransVal[xx + 1][yy + 1] = tv;
					// fix the wall on the west side - in case the tile is in the corner
					if (dTransVal[xx - 1][yy + 1] == 0)
						dTransVal[xx - 1][yy + 1] = tv;
					// fix the wall on the east side - in case 59 is not placed
					if (dTransVal[xx + 2][yy + 1] == 0)
						dTransVal[xx + 2][yy + 1] = tv;
					break;
				}
				tv = dTransVal[xx][yy];
				if (tv != 0) {
					// there is a valid tv on the west side - use it
					dTransVal[xx + 1][yy] = tv;
					dTransVal[xx + 1][yy + 1] = tv;
					// fix the wall on the north side
					if (dTransVal[xx][yy - 1] == 0)
						dTransVal[xx][yy - 1] = tv;
					if (dTransVal[xx + 1][yy - 1] == 0)
						dTransVal[xx + 1][yy - 1] = tv;
					// fix the wall on the south side
					if (dTransVal[xx][yy + 2] == 0)
						dTransVal[xx][yy + 2] = tv;
					if (dTransVal[xx + 1][yy + 2] == 0)
						dTransVal[xx + 1][yy + 2] = tv;
					// fix the wall on the east side - in case 59 is not placed
					if (dTransVal[xx + 2][yy] == 0)
						dTransVal[xx + 2][yy] = tv;
					if (dTransVal[xx + 2][yy + 1] == 0)
						dTransVal[xx + 2][yy + 1] = tv;
					break;
				}
				// no valid tv on the west side -> there must be a one on the east side
				tv = dTransVal[xx + 4][yy];
				assert(tv != 0);
				// use it on the current tile
				dTransVal[xx][yy] = tv;
				dTransVal[xx + 1][yy] = tv;
				dTransVal[xx][yy + 1] = tv;
				dTransVal[xx + 1][yy + 1] = tv;
				// fix the wall on the west side
				assert(dTransVal[xx - 1][yy] == 0);
				dTransVal[xx - 1][yy] = tv;
				assert(dTransVal[xx - 1][yy + 1] == 0);
				dTransVal[xx - 1][yy + 1] = tv;
				// fix the wall on the north side
				if (dTransVal[xx][yy - 1] == 0)
					dTransVal[xx][yy - 1] = tv;
				if (dTransVal[xx + 1][yy - 1] == 0)
					dTransVal[xx + 1][yy - 1] = tv;
				// fix the corner on north-west
				if (dTransVal[xx - 1][yy - 1] == 0)
					dTransVal[xx - 1][yy - 1] = tv;
				// fix the wall on the south side
				if (dTransVal[xx][yy + 2] == 0)
					dTransVal[xx][yy + 2] = tv;
				if (dTransVal[xx + 1][yy + 2] == 0)
					dTransVal[xx + 1][yy + 2] = tv;
				// fix the corner on south-west
				if (dTransVal[xx - 1][yy + 2] == 0)
					dTransVal[xx - 1][yy + 2] = tv;
				break;
			case 59:
				tv = dTransVal[xx][yy];
				if (tv != 0 && dTransVal[xx + 1][yy] == tv) {
					// the tile has a valid tv - use it
					dTransVal[xx][yy + 1] = tv;
					dTransVal[xx + 1][yy + 1] = tv;
					// fix the wall on the east side - in case the tile is in the corner
					if (dTransVal[xx + 2][yy + 1] == 0)
						dTransVal[xx + 2][yy + 1] = tv;
					// fix the wall on the west side - in case 58 is not placed
					if (dTransVal[xx - 1][yy + 1] == 0)
						dTransVal[xx - 1][yy + 1] = tv;
					break;
				}
				tv = dTransVal[xx + 1][yy];
				if (tv != 0) {
					// there is a valid tv on the east side - use it
					dTransVal[xx][yy] = tv;
					dTransVal[xx][yy + 1] = tv;
					// fix the wall on the north side
					if (dTransVal[xx][yy - 1] == 0)
						dTransVal[xx][yy - 1] = tv;
					if (dTransVal[xx + 1][yy - 1] == 0)
						dTransVal[xx + 1][yy - 1] = tv;
					// fix the wall on the south side
					if (dTransVal[xx][yy + 2] == 0)
						dTransVal[xx][yy + 2] = tv;
					if (dTransVal[xx + 1][yy + 2] == 0)
						dTransVal[xx + 1][yy + 2] = tv;
					// fix the wall on the west side - in case 58 is not placed
					if (dTransVal[xx - 1][yy] == 0)
						dTransVal[xx - 1][yy] = tv;
					if (dTransVal[xx - 1][yy + 1] == 0)
						dTransVal[xx - 1][yy + 1] = tv;
					break;
				}
				// no valid tv on the east side -> there must be a one on the west side
				tv = dTransVal[xx - 3][yy];
				assert(tv != 0);
				// use it on the current tile
				dTransVal[xx][yy] = tv;
				dTransVal[xx + 1][yy] = tv;
				dTransVal[xx][yy + 1] = tv;
				dTransVal[xx + 1][yy + 1] = tv;
				// fix the wall on the east side
				assert(dTransVal[xx + 2][yy] == 0);
				dTransVal[xx + 2][yy] = tv;
				assert(dTransVal[xx + 2][yy + 1] == 0);
				dTransVal[xx + 2][yy + 1] = tv;
				// fix the wall on the north side
				if (dTransVal[xx][yy - 1] == 0)
					dTransVal[xx][yy - 1] = tv;
				if (dTransVal[xx + 1][yy - 1] == 0)
					dTransVal[xx + 1][yy - 1] = tv;
				// fix the corner on north-east
				if (dTransVal[xx + 2][yy - 1] == 0)
					dTransVal[xx + 2][yy - 1] = tv;
				// fix the wall on the south side
				if (dTransVal[xx][yy + 2] == 0)
					dTransVal[xx][yy + 2] = tv;
				if (dTransVal[xx + 1][yy + 2] == 0)
					dTransVal[xx + 1][yy + 2] = tv;
				// fix the corner on south-east
				if (dTransVal[xx + 1][yy + 2] == 0)
					dTransVal[xx + 1][yy + 2] = tv;
				break;*/
			// fix transVals around the stairs - necessary only if DRLG_FloodTVal is run after the placement
			// - due to complex cases with the horizontal/vertical 'doors', this is not really feasible
			/*case 36:
			case 134:
				tv = dTransVal[xx][yy];
				dTransVal[xx][yy + 1] = tv;
				dTransVal[xx + 1][yy + 1] = tv;
				// fix the wall on the west side
				//if (dTransVal[xx - 1][yy] == 0)
					dTransVal[xx - 1][yy] = tv;
				//if (dTransVal[xx - 1][yy + 1] == 0)
					dTransVal[xx - 1][yy + 1] = tv;
				break;
			case 37:
			case 135:
				tv = dTransVal[xx][yy + 1];
				dTransVal[xx][yy] = tv;
				dTransVal[xx + 1][yy] = tv;
				// fix the wall on the west side
				//if (dTransVal[xx - 1][yy] == 0)
					dTransVal[xx - 1][yy] = tv;
				//if (dTransVal[xx - 1][yy + 1] == 0)
					dTransVal[xx - 1][yy + 1] = tv;
				break;
			case 38:
			case 136:
				tv = dTransVal[xx][yy];
				dTransVal[xx][yy + 1] = tv;
				dTransVal[xx + 1][yy + 1] = tv;
				break;
			case 32:
			case 130:
				tv = dTransVal[xx][yy];
				// fix the wall on the east side
				//if (dTransVal[xx + 3][yy] == 0)
					dTransVal[xx + 3][yy] = tv;
				//if (dTransVal[xx + 3][yy + 1] == 0)
					dTransVal[xx + 3][yy + 1] = tv;
				break;*/
			}
			xx += 2;
		}
		yy += 2;
	}
}

void DRLG_L4InitTransVals()
{
	int i, j;

	static_assert(sizeof(drlg.transvalMap) == sizeof(dungeon), "transvalMap vs dungeon mismatch.");
	memcpy(drlg.transvalMap, dungeon, sizeof(dungeon));
	// block arches with walls to stop the spread of transVals
	for (i = 0; i < DMAXX - 1; i++) {
		for (j = 0; j < DMAXY; j++) {
			if (drlg.transvalMap[i][j] == 53) {
				drlg.transvalMap[i][j + 1] = 1;
			} else if (drlg.transvalMap[i][j] == 57) {
				drlg.transvalMap[i + 1][j] = 2;
			}
		}
	}

	DRLG_FloodTVal();
	DRLG_L4TransFix();
}

/*
 * Replace tiles with complete ones to hide rendering glitch of transparent corners.
 * New dungeon values: 116..128
 * Obsolete dungeon values: 18..29
 */
/*static void DRLG_L4Corners()
{
	int i;
	BYTE* pTmp;

	static_assert(sizeof(dungeon) == DMAXX * DMAXY, "Linear traverse of dungeon does not work in DRLG_L4Corners.");
	pTmp = &dungeon[0][0];
	for (i = 0; i < DMAXX * DMAXY; i++, pTmp++) {
		if (*pTmp >= 18 && *pTmp <= 29) // && *pTmp != 20 && *pTmp != 24
			*pTmp += 98;
	}
}*/

/*
 * Miniset replacement of corner tiles.
 * New dungeon values: (17)
 * TODO: use DRLG_PlaceMiniSet instead?
 */
/*static void DRLG_L4GeneralFix()
{
	int i, j;

	for (j = 0; j < DMAXY - 1; j++) {
		for (i = 0; i < DMAXX - 1; i++) {
			//24/122, 2,  search
			// 5,     0,

			//17, 0, replace
			// 0, 0,
			if ((dungeon[i][j] == 24 || dungeon[i][j] == 122) && dungeon[i + 1][j] == 2 && dungeon[i][j + 1] == 5) {
				dungeon[i][j] = 17;
			}
		}
	}
}*/

static void DRLG_L4ThemeExitFix()
{
	int i, x1, y1, x2, y2, xx, yy;

	for (i = numthemes - 1; i >= 0; i--) {
		x1 = themes[i]._tsx1;
		y1 = themes[i]._tsy1;
		x2 = themes[i]._tsx2;
		y2 = themes[i]._tsy2;
		switch (random_(0, 4)) {
		case 0:
			yy = (y1 + y2 + 1) / 2u;
			dungeon[x1][yy - 1] = 53;
			dungeon[x1][yy + 0] = 50;
			dungeon[x1][yy + 1] = 52;
			//dungeon[x2 - 1][yy - 1] = 54;
			break;
		case 1:
			yy = (y1 + y2 + 1) / 2u;
			dungeon[x2][yy - 1] = 53;
			dungeon[x2][yy + 0] = 50;
			dungeon[x2][yy + 1] = 52;
			//dungeon[x2 - 1][yy - 1] = 54;
			break;
		case 2:
			xx = (x1 + x2 + 1) / 2u;
			dungeon[xx - 1][y1] = 57;
			dungeon[xx + 0][y1] = 50;
			dungeon[xx + 1][y1] = 56;
			//dungeon[xx + 0][y2 - 1] = 59;
			//dungeon[xx - 1][y2 - 1] = 58;
			break;
		case 3:
			xx = (x1 + x2 + 1) / 2u;
			dungeon[xx - 1][y2] = 57;
			dungeon[xx + 0][y2] = 50;
			dungeon[xx + 1][y2] = 56;
			//dungeon[xx + 0][y2 - 1] = 59;
			//dungeon[xx - 1][y2 - 1] = 58;
			break;
		}
	}
}

static void DRLG_L4()
{
	while (true) {
		do {
			memset(drlg.dungBlock, 0, sizeof(drlg.dungBlock));

			//static_assert(sizeof(dungeon) == DMAXX * DMAXY, "Linear traverse of dungeon does not work in DRLG_L4.");
			//memset(dungeon, 30, sizeof(dungeon));
			L4FirstRoom();
			L4ConnectBlock();
		} while (DRLG_L4GetArea() < 190);

		L4Block2Dungeon();
		DRLG_L4MakeMegas();
		L4TileFix();
		memset(drlgFlags, 0, sizeof(drlgFlags));
		if (currLvl._dLevelIdx == DLV_HELL4) {
			DRLG_LoadDiabQuads();
		} else if (pSetPieces[0]._spData != NULL) { // pSetPieces[0]._sptype != SPT_NONE
			DRLG_L4SetRoom(0);
		}
		L4AddWall();

		if (currLvl._dDynLvl) {
			POS32 warpPos = DRLG_PlaceMiniSet(random_(141, 2) ? L4DYNENTRY1 : L4DYNENTRY2);
			if (warpPos.x < 0) {
				continue;
			}
			pWarps[DWARP_ENTRY]._wx = warpPos.x;
			pWarps[DWARP_ENTRY]._wy = warpPos.y;
			pWarps[DWARP_ENTRY]._wx = 2 * pWarps[DWARP_ENTRY]._wx + DBORDERX + 1;
			pWarps[DWARP_ENTRY]._wy = 2 * pWarps[DWARP_ENTRY]._wy + DBORDERY + 1;
			pWarps[DWARP_ENTRY]._wtype = WRPT_CIRCLE;
			break;
		}
		POS32 warpPos = DRLG_PlaceMiniSet(L4USTAIRS); // L4USTAIRS (5, 6)
		if (warpPos.x < 0) {
			continue;
		}
		pWarps[DWARP_ENTRY]._wx = warpPos.x + 1;
		pWarps[DWARP_ENTRY]._wy = warpPos.y + 2;
		pWarps[DWARP_ENTRY]._wx = 2 * pWarps[DWARP_ENTRY]._wx + DBORDERX + 1;
		pWarps[DWARP_ENTRY]._wy = 2 * pWarps[DWARP_ENTRY]._wy + DBORDERY + 1;
		pWarps[DWARP_ENTRY]._wtype = WRPT_L4_UP;
		if (currLvl._dLevelIdx != DLV_HELL4) {
			if (currLvl._dLevelIdx == DLV_HELL1) {
				warpPos = DRLG_PlaceMiniSet(L4USTAIRS); // L4TWARP (5, 6)
				if (warpPos.x < 0) {
					continue;
				}
				pWarps[DWARP_TOWN]._wx = warpPos.x + 1;
				pWarps[DWARP_TOWN]._wy = warpPos.y + 2;
				pWarps[DWARP_TOWN]._wx = 2 * pWarps[DWARP_TOWN]._wx + DBORDERX + 1;
				pWarps[DWARP_TOWN]._wy = 2 * pWarps[DWARP_TOWN]._wy + DBORDERY + 1;
				pWarps[DWARP_TOWN]._wtype = WRPT_L4_UP;
			}
			if (currLvl._dLevelIdx == DLV_HELL3) {
				warpPos = DRLG_PlaceMiniSet(L4PENTA2); // L4PENTA (5, 6)
				if (warpPos.x < 0) {
					continue;
				}
				pWarps[DWARP_EXIT]._wx = warpPos.x + 2;
				pWarps[DWARP_EXIT]._wy = warpPos.y + 2;
				pWarps[DWARP_EXIT]._wtype = WRPT_L4_PENTA;
			} else if (pSetPieces[0]._sptype == SPT_WARLORD) {
				pWarps[DWARP_EXIT]._wx = pSetPieces[0]._spx + 2; // L4DSTAIRS (4, 4)
				pWarps[DWARP_EXIT]._wy = pSetPieces[0]._spy + 3;
				pWarps[DWARP_EXIT]._wtype = WRPT_L4_DOWN;
			} else {
				warpPos = DRLG_PlaceMiniSet(L4DSTAIRS); // L4DSTAIRS (4, 4)
				if (warpPos.x < 0) {
					continue;
				}
				pWarps[DWARP_EXIT]._wx = warpPos.x + 2;
				pWarps[DWARP_EXIT]._wy = warpPos.y + 2;
				pWarps[DWARP_EXIT]._wtype = WRPT_L4_DOWN;
			}
			pWarps[DWARP_EXIT]._wx = 2 * pWarps[DWARP_EXIT]._wx + DBORDERX + 1;
			pWarps[DWARP_EXIT]._wy = 2 * pWarps[DWARP_EXIT]._wy + DBORDERY + 1;
		}
		break;
	}

	// DRLG_L4GeneralFix(); - commented out, because this is no longer necessary

	if (currLvl._dLevelIdx != DLV_HELL4) {
		DRLG_PlaceThemeRooms(7, 10, themeTiles, 112);
		DRLG_L4ThemeExitFix();
	}

	DRLG_L4Shadows();
	// DRLG_L4Corners(); - commented out, because this is no longer necessary
	DRLG_PlaceRndTile(DEFAULT_MEGATILE_L4, 95, 3);
	DRLG_PlaceRndTile(DEFAULT_MEGATILE_L4, 96, 4);
	DRLG_PlaceRndTile(DEFAULT_MEGATILE_L4, 97, 4);
}
#if !USE_PATCH
static void DRLG_L4FixPreMap(int idx)
{
	uint16_t* lm = (uint16_t*)pSetPieces[idx]._spData;

	if (pSetPieces[idx]._sptype == SPT_DIAB_QUAD_2) {
		// patch set-piece - Diab2a.DUN
		// external tiles
		for (int y = 1; y <= 3; y++) {
			for (int x = 9; x <= 10; x++) {
				uint16_t wv = SwapLE16(lm[2 + x + y * 11]);
				if (wv >= 116 && wv <= 128) {
					// if (wv == 118) {
					//	wv = 128;
					// }
					lm[2 + x + y * 11] = SwapLE16(wv - 98);
				}
			}
		}
		// useless tiles
		for (int y = 0; y < 12; y++) {
			for (int x = 0; x < 11; x++) {
				if (x >= 9 && x <= 10 && y >= 1 && y <= 3) {
					continue;
				}
				lm[2 + x + y * 11] = 0;
			}
		}
		// protect changing tiles from objects
		lm[2 + 11 * 12 + 10 + 1 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 12 + 10 + 1 * 11] |= SwapLE16((3 << 14));
		lm[2 + 11 * 12 + 10 + 2 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 12 + 10 + 2 * 11] |= SwapLE16((3 << 14));
		lm[2 + 11 * 12 + 10 + 3 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 12 + 10 + 3 * 11] |= SwapLE16((3 << 14));
		// protect tiles with monsters/objects from spawning additional monsters/objects
		lm[2 + 11 * 12 + 2 + 1 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 12 + 2 + 9 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 12 + 5 + 4 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 12 + 5 + 6 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 12 + 6 + 6 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 12 + 7 + 5 * 11] = SwapLE16((3 << 8));
		lm[2 + 11 * 12 + 8 + 4 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 12 + 8 + 6 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 12 + 7 + 8 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 12 + 8 + 1 * 11] = SwapLE16((3 << 12));
		lm[2 + 11 * 12 + 8 + 2 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 12 + 6 + 5 * 11] = SwapLE16((3 << 10));
	} else if (pSetPieces[idx]._sptype == SPT_DIAB_QUAD_3) {
		// patch set-piece - Diab3a.DUN
		// useless tiles
		for (int y = 0; y < 11; y++) {
			for (int x = 0; x < 11; x++) {
				if (x >= 4 && x <= 6 && y >= 10 && y <= 10) {
					continue; // SW-wall
				}
				if (x >= 0 && x <= 0 && y >= 4 && y <= 6) {
					continue; // NW-wall
				}
				if (x >= 4 && x <= 6 && y >= 0 && y <= 0) {
					continue; // NE-wall
				}
				if (x >= 10 && x <= 10 && y >= 3 && y <= 6) {
					continue; // SE-wall
				}
				lm[2 + x + y * 11] = 0;
			}
		}
		// protect changing tiles from objects
		// - SW-wall
		lm[2 + 11 * 11 + 4 + 10 * 11] = SwapLE16((3 << 12));
		lm[2 + 11 * 11 + 4 + 10 * 11] |= SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 5 + 10 * 11] = SwapLE16((3 << 12));
		lm[2 + 11 * 11 + 5 + 10 * 11] |= SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 6 + 10 * 11] = SwapLE16((3 << 12));
		lm[2 + 11 * 11 + 6 + 10 * 11] |= SwapLE16((3 << 14));
		// - NE-wall
		lm[2 + 11 * 11 + 4 + 0 * 11] = SwapLE16((3 << 12));
		lm[2 + 11 * 11 + 4 + 0 * 11] |= SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 5 + 0 * 11] = SwapLE16((3 << 12));
		lm[2 + 11 * 11 + 5 + 0 * 11] |= SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 6 + 0 * 11] = SwapLE16((3 << 12));
		lm[2 + 11 * 11 + 6 + 0 * 11] |= SwapLE16((3 << 14));
		// - NW-wall
		lm[2 + 11 * 11 + 0 + 4 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 11 + 0 + 4 * 11] |= SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 0 + 5 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 11 + 0 + 5 * 11] |= SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 0 + 6 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 11 + 0 + 6 * 11] |= SwapLE16((3 << 14));
		// - SE-wall
		lm[2 + 11 * 11 + 10 + 4 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 11 + 10 + 4 * 11] |= SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 10 + 5 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 11 + 10 + 5 * 11] |= SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 10 + 6 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 11 + 10 + 6 * 11] |= SwapLE16((3 << 14));
		// protect tiles with monsters/objects from spawning additional monsters/objects
		lm[2 + 11 * 11 + 0 + 2 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 0 + 7 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 2 + 0 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 2 + 4 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 11 + 2 + 6 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 11 + 2 + 9 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 3 + 3 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 3 + 6 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 4 + 1 * 11] = SwapLE16((3 << 8));
		lm[2 + 11 * 11 + 4 + 2 * 11] = SwapLE16((3 << 12));
		lm[2 + 11 * 11 + 4 + 7 * 11] = SwapLE16((3 << 8));
		lm[2 + 11 * 11 + 4 + 7 * 11] |= SwapLE16((3 << 12));
		lm[2 + 11 * 11 + 6 + 2 * 11] = SwapLE16((3 << 12));
		lm[2 + 11 * 11 + 6 + 7 * 11] = SwapLE16((3 << 12));
		lm[2 + 11 * 11 + 6 + 3 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 6 + 6 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 7 + 0 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 7 + 4 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 11 + 7 + 6 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 11 + 7 + 9 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 9 + 2 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 9 + 7 * 11] = SwapLE16((3 << 14));
	} else if (pSetPieces[idx]._sptype == SPT_DIAB_QUAD_4) {
		// patch set-piece - Diab4a.DUN
		for (int y = 0; y < 9; y++) {
			for (int x = 0; x < 9; x++) {
				// external tiles
				uint16_t wv = SwapLE16(lm[2 + x + y * 9]);
				if (wv >= 116 && wv <= 128) {
					// if (wv == 118) {
					//	wv = 128;
					// }
					lm[2 + x + y * 9] = SwapLE16(wv - 98);
				}
				// useless tiles
				if (x >= 2 && x <= 6 && y >= 7 && y <= 8) {
					continue; // SW-wall
				}
				if (x >= 0 && x <= 1 && y >= 2 && y <= 6) {
					continue; // NW-wall
				}
				if (x >= 2 && x <= 6 && y >= 0 && y <= 1) {
					continue; // NE-wall
				}
				if (x >= 7 && x <= 8 && y >= 2 && y <= 6) {
					continue; // SE-wall
				}
				lm[2 + x + y * 9] = 0;
			}
		}
		// - replace diablo
		lm[2 + 9 * 9 + 9 * 9 * 2 * 2 + 8 + 8 * 9 * 2] = SwapLE16((UMT_DIABLO + 1) | (1 << 15));
		// - replace the only black knight
		lm[2 + 9 * 9 + 9 * 9 * 2 * 2 + 4 + 6 * 9 * 2] = SwapLE16(101);
		// protect changing tiles from objects
		// - SW-wall
		for (int y = 2; y < 7; y++) {
			lm[2 + 9 * 9 + 8 + y * 9] = SwapLE16((3 << 10) | (3 << 14));
		}
		// - NE-wall
		for (int x = 2; x < 7; x++) {
			lm[2 + 9 * 9 + x + 1 * 9] = SwapLE16((3 << 12) | (3 << 14));
		}
		// - NW-wall
		for (int y = 2; y < 7; y++) {
			lm[2 + 9 * 9 + 1 + y * 9] = SwapLE16((3 << 10) | (3 << 14));
		}
		// - SE-wall
		for (int x = 2; x < 7; x++) {
			lm[2 + 9 * 9 + x + 8 * 9] = SwapLE16((3 << 12) | (3 << 14));
		}
		// protect tiles with monsters/objects from spawning additional monsters/objects
		lm[2 + 9 * 9 + 2 + 2 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 2 + 3 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 2 + 4 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 2 + 6 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 3 + 3 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 3 + 4 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 3 + 5 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 3 + 6 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 4 + 2 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 4 + 3 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 4 + 4 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 4 + 5 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 4 + 6 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 5 + 2 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 5 + 3 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 5 + 4 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 5 + 5 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 6 + 2 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 6 + 4 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 6 + 5 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 6 + 6 * 9] = SwapLE16((3 << 8));
	} else if (pSetPieces[idx]._sptype == SPT_WARLORD) {
		// patch set-piece - Warlord2.DUN
		// useless tiles
		for (int y = 0; y < 7; y++) {
			for (int x = 0; x < 8; x++) {
				if (x >= 7 && y >= 1 && x <= 7 && y <= 5) {
					continue;
				}
				lm[2 + x + y * 8] = 0;
			}
		}
		// replace monsters
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 2 + 2 * 8 * 2] = SwapLE16(100);
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 2 + 10 * 8 * 2] = SwapLE16(100);
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 13 + 4 * 8 * 2] = SwapLE16(100);
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 13 + 9 * 8 * 2] = SwapLE16(100);
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 10 + 2 * 8 * 2] = SwapLE16(100);
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 10 + 10 * 8 * 2] = SwapLE16(100);
		// add monsters
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 6 + 2 * 8 * 2] = SwapLE16(100);
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 6 + 10 * 8 * 2] = SwapLE16(100);
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 11 + 2 * 8 * 2] = SwapLE16(100);
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 11 + 10 * 8 * 2] = SwapLE16(100);
		// - add unique
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 6 + 7 * 8 * 2] = SwapLE16((UMT_WARLORD + 1) | (1 << 15));
		// add objects
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 8 * 7 * 2 * 2 + 2 + 3 * 8 * 2] = SwapLE16(108);
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 8 * 7 * 2 * 2 + 2 + 9 * 8 * 2] = SwapLE16(108);
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 8 * 7 * 2 * 2 + 5 + 2 * 8 * 2] = SwapLE16(109);
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 8 * 7 * 2 * 2 + 8 + 2 * 8 * 2] = SwapLE16(109);
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 8 * 7 * 2 * 2 + 5 + 10 * 8 * 2] = SwapLE16(109);
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 8 * 7 * 2 * 2 + 8 + 10 * 8 * 2] = SwapLE16(109);
		// protect inner tiles from spawning additional monsters/objects
		for (int y = 0; y <= 5; y++) {
			for (int x = 0; x <= 6; x++) {
				lm[2 + 8 * 7 + x + y * 8] = SwapLE16((3 << 8) | (3 << 10) | (3 << 12) | (3 << 14));
			}
		}
	}
}
#endif
static void DRLG_L4DrawPreMaps()
{
	for (int i = lengthof(pSetPieces) - 1; i >= 0; i--) {
		if (pSetPieces[i]._sptype == SPT_NONE)
			continue;
		if (setpiecedata[pSetPieces[i]._sptype]._spdPreDunFile != NULL) {
			MemFreeDbg(pSetPieces[i]._spData);
			pSetPieces[i]._spData = LoadFileInMem(setpiecedata[pSetPieces[i]._sptype]._spdPreDunFile);
#if !USE_PATCH
			DRLG_L4FixPreMap(i);
#endif
			DRLG_DrawMap(i);
		}
	}
	// TODO: add setpiece?
	if (currLvl._dLevelIdx == DLV_HELL3) {
		int sx = (pWarps[DWARP_EXIT]._wx - (1 + 4 + DBORDERX)) >> 1;
		int sy = (pWarps[DWARP_EXIT]._wy - (1 + 4 + DBORDERY)) >> 1;
		DRLG_DrawMiniSet(L4PENTA, sx, sy);
	}
}

static void LoadL4Dungeon(const LevelData* lds)
{
	pWarps[DWARP_ENTRY]._wx = lds->dSetLvlDunX;
	pWarps[DWARP_ENTRY]._wy = lds->dSetLvlDunY;
	pWarps[DWARP_ENTRY]._wtype = lds->dSetLvlWarp;

	// load dungeon
	pSetPieces[0]._spx = 0;
	pSetPieces[0]._spy = 0;
	pSetPieces[0]._sptype = lds->dSetLvlPiece;
	pSetPieces[0]._spData = LoadFileInMem(setpiecedata[pSetPieces[0]._sptype]._spdDunFile);

	memset(drlgFlags, 0, sizeof(drlgFlags));
	static_assert(sizeof(dungeon[0][0]) == 1, "memset on dungeon does not work in LoadL4DungeonData.");
	memset(dungeon, BASE_MEGATILE_L4, sizeof(dungeon));

	DRLG_LoadSP(0, DEFAULT_MEGATILE_L4);
}

void CreateL4Dungeon()
{
	const LevelData* lds = &AllLevels[currLvl._dLevelNum];

	if (lds->dSetLvl) {
		LoadL4Dungeon(lds);
	} else {
		DRLG_LoadL4SP();
		DRLG_L4();
	}

	DRLG_L4Subs();

	memcpy(pdungeon, dungeon, sizeof(pdungeon));
	DRLG_L4DrawPreMaps();

	DRLG_L4InitTransVals();
	DRLG_PlaceMegaTiles(BASE_MEGATILE_L4);
}

DEVILUTION_END_NAMESPACE
