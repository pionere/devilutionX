/**
 * @file drlg_l3.cpp
 *
 * Implementation of the caves level generation algorithms.
 *
 * dflags matrix is used as a BOOLEAN matrix to protect the quest room.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** Starting position of the megatiles. */
#define BASE_MEGATILE_L3 (8 - 1)
/** The required number of lava pools.*/
#define MIN_LAVA_POOL 3
/** Helper variable to check if sufficient number of lava pools have been generated */
unsigned _guLavapools;
unsigned _guLockoutcnt;
bool _gabLockout[DMAXX][DMAXY];

/**
 * A lookup table for the 16 possible patterns of a 2x2 area,
 * where each cell either contains a SW wall or it doesn't.
 */
const BYTE L3ConvTbl[16] = { 8, 11, 3, 10, 1, 9, 12, 12, 6, 13, 4, 13, 2, 14, 5, 7 };
/** Miniset: Stairs up. */
const BYTE L3USTAIRS[] = {
	// clang-format off
	3, 3, // width, height

	 8,  8, 0, // search
	10, 10, 0,
	 7,  7, 0,

	51, 50, 0, // replace
	48, 49, 0,
	 0,  0, 0,
/*  181,182    178,179,     0,  0,	// MegaTiles
	183, 31    180, 31,     0,  0,

	170,171    174,175,     0,  0,
	172,173    176,177,     0,  0,

	  0,  0,     0,  0,     0,  0,
	  0,  0,     0,  0,     0,  0, */
	// clang-format on
};
#ifdef HELLFIRE
const BYTE L6USTAIRS[] = {
	// clang-format off
	3, 3, // width, height

	 8,  8, 0, // search
	10, 10, 0,
	 7,  7, 0,

	20, 19, 0, // replace
	17, 18, 0,
	 0,  0, 0,
	// clang-format on
};
#endif
/** Miniset: Stairs down. */
const BYTE L3DSTAIRS[] = {
	// clang-format off
	3, 3, // width, height

	8, 9, 7, // search
	8, 9, 7,
	0, 0, 0,

	0, 47, 0, // replace
	0, 46, 0,
	0,  0, 0,
	/*0,  0,   166,167,     0,  0,	// MegaTiles
	  0,  0,   168,169,     0,  0,

	  0,  0,   162,163,     0,  0,
	  0,  0,   164,165,     0,  0,

	  0,  0,     0,  0,     0,  0,
	  0,  0,     0,  0,     0,  0, */
	// clang-format on
};
#ifdef HELLFIRE
const BYTE L6DSTAIRS[] = {
	// clang-format off
	3, 3, // width, height

	8, 9, 7, // search
	8, 9, 7,
	0, 0, 0,

	0, 16, 0, // replace
	0, 15, 0,
	0,  0, 0,
	// clang-format on
};
#endif
/** Miniset: Stairs up to town. */
const BYTE L3TWARP[] = {
	// clang-format off
	3, 3, // width, height

	 8,  8, 0, // search
	10, 10, 0,
	 7,  7, 0,

	125, 125, 0, // replace
	125, 125, 0,
	  0,   0, 0,
/*  559,182    556,557,     0,  0,	// MegaTiles
	560, 31    558, 31,     0,  0,

	548,549    552,553,     0,  0,
	550,551    554,555,     0,  0,

	  0,  0,     0,  0,     0,  0,
	  0,  0,     0,  0,     0,  0, */
	// clang-format on
};
#ifdef HELLFIRE
const BYTE L6TWARP[] = {
	// clang-format off
	3, 3, // width, height

	 8,  8, 0, // search
	10, 10, 0,
	 7,  7, 0,

	24, 23, 0, // replace
	21, 22, 0,
	 0,  0, 0,
	// clang-format on
};
#endif
/** Miniset: Stalagmite white stalactite 1. */
const BYTE L3TITE1[] = {
	// clang-format off
	4, 4, // width, height

	7, 7, 7, 7, // search
	7, 7, 7, 7,
	7, 7, 7, 7,
	7, 7, 7, 7,

	0,  0,  0, 0, // replace
	0, 57, 58, 0,
	0, 56, 55, 0,
	0,  0,  0, 0,
	// clang-format on
};
/** Miniset: Stalagmite white stalactite 2. */
const BYTE L3TITE2[] = {
	// clang-format off
	4, 4, // width, height

	7, 7, 7, 7, // search
	7, 7, 7, 7,
	7, 7, 7, 7,
	7, 7, 7, 7,

	0,  0,  0, 0, // replace
	0, 61, 62, 0,
	0, 60, 59, 0,
	0,  0,  0, 0,
	// clang-format on
};
/** Miniset: Stalagmite white stalactite 3. */
const BYTE L3TITE3[] = {
	// clang-format off
	4, 4, // width, height

	7, 7, 7, 7, // search
	7, 7, 7, 7,
	7, 7, 7, 7,
	7, 7, 7, 7,

	0,  0,  0, 0, // replace
	0, 65, 66, 0,
	0, 64, 63, 0,
	0,  0,  0, 0,
	// clang-format on
};
/** Miniset: Stalagmite white stalactite horizontal. */
const BYTE L3TITE6[] = {
	// clang-format off
	5, 4, // width, height

	7, 7, 7, 7, 7, // search
	7, 7, 7, 0, 7,
	7, 7, 7, 7, 7,
	7, 7, 7, 7, 7,

	0,  0,  0,  0, 0, // replace
	0, 77, 78,  0, 0,
	0, 76, 74, 75, 0,
	0,  0,  0,  0, 0,
	// clang-format on
};
/** Miniset: Stalagmite white stalactite vertical. */
const BYTE L3TITE7[] = {
	// clang-format off
	4, 5, // width, height

	7, 7, 7, 7, // search
	7, 7, 0, 7,
	7, 7, 7, 7,
	7, 7, 7, 7,
	7, 7, 7, 7,

	0,  0,  0, 0, // replace
	0, 83,  0, 0,
	0, 82, 80, 0,
	0, 81, 79, 0,
	0,  0,  0, 0,
	// clang-format on
};
/** Miniset: Stalagmite 1. */
const BYTE L3TITE8[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	0,  0, 0, // replace
	0, 52, 0,
	0,  0, 0,
	// clang-format on
};
/** Miniset: Stalagmite 2. */
const BYTE L3TITE9[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	0,  0, 0, // replace
	0, 53, 0,
	0,  0, 0,
	// clang-format on
};
/** Miniset: Stalagmite 3. */
const BYTE L3TITE10[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	0,  0, 0, // replace
	0, 54, 0,
	0,  0, 0,
	// clang-format on
};
/** Miniset: Stalagmite 4. */
const BYTE L3TITE11[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	0,  0, 0, // replace
	0, 67, 0,
	0,  0, 0,
	// clang-format on
};
/** Miniset: Stalagmite on vertical wall. */
const BYTE L3TITE12[] = {
	// clang-format off
	2, 1, // width, height

	9, 7, // search

	68, 0, // replace
	// clang-format on
};
/** Miniset: Stalagmite on horizontal wall. */
const BYTE L3TITE13[] = {
	// clang-format off
	1, 2, // width, height

	10, // search
	 7,

	69, // replace
	 0,
	// clang-format on
};
/** Miniset: Cracked vertical wall 1. */
const BYTE L3CREV1[] = {
	// clang-format off
	2, 1, // width, height

	8, 7, // search

	84, 85, // replace
	// clang-format on
};
/** Miniset: Cracked vertical wall - north corner. */
const BYTE L3CREV2[] = {
	// clang-format off
	2, 1, // width, height

	8, 11, // search

	86, 87, // replace
	// clang-format on
};
/** Miniset: Cracked horizontal wall 1. */
const BYTE L3CREV3[] = {
	// clang-format off
	1, 2, // width, height

	 8, // search
	10,

	89, // replace
	88,
	// clang-format on
};
/** Miniset: Cracked vertical wall 2. */
const BYTE L3CREV4[] = {
	// clang-format off
	2, 1, // width, height

	8, 7, // search

	90, 91, // replace
	// clang-format on
};
/** Miniset: Cracked horizontal wall - north corner. */
const BYTE L3CREV5[] = {
	// clang-format off
	1, 2, // width, height

	 8, // search
	11,

	92, // replace
	93,
	// clang-format on
};
/** Miniset: Cracked horizontal wall 2. */
const BYTE L3CREV6[] = {
	// clang-format off
	1, 2, // width, height

	 8, // search
	10,

	95, // replace
	94,
	// clang-format on
};
/** Miniset: Cracked vertical wall - west corner. */
const BYTE L3CREV7[] = {
	// clang-format off
	2, 1, // width, height

	8, 7, // search

	96, 101, // replace
	// clang-format on
};
/** Miniset: Cracked horizontal wall - north. */
const BYTE L3CREV8[] = {
	// clang-format off
	1, 2, // width, height

	2, // search
	8,

	102, // replace
	 97,
	// clang-format on
};
/** Miniset: Cracked vertical wall - east corner. */
const BYTE L3CREV9[] = {
	// clang-format off
	2, 1, // width, height

	3, 8, // search

	103, 98, // replace
	// clang-format on
};
/** Miniset: Cracked vertical wall - west. */
const BYTE L3CREV10[] = {
	// clang-format off
	2, 1, // width, height

	4, 8, // search

	104, 99, // replace
	// clang-format on
};
/** Miniset: Cracked horizontal wall - south corner. */
const BYTE L3CREV11[] = {
	// clang-format off
	1, 2, // width, height

	6, // search
	8,

	105, // replace
	100,
	// clang-format on
};
/** Miniset: Replace broken wall with floor 1. */
const BYTE L3ISLE1[] = {
	// clang-format off
	2, 3, // width, height

	5, 14, // search
	4,  9,
	13, 12,

	7, 7, // replace
	7, 7,
	7, 7,
	// clang-format on
};
/** Miniset: Replace small wall with floor 2. */
const BYTE L3ISLE2[] = {
	// clang-format off
	3, 2, // width, height

	 5,  2, 14, // search
	13, 10, 12,

	7, 7, 7, // replace
	7, 7, 7,
	// clang-format on
};
/** Miniset: Replace small wall with lava 1. */
const BYTE L3ISLE3[] = {
	// clang-format off
	2, 3, // width, height

	 5, 14, // search
	 4,  9,
	13, 12,

	29, 30, // replace
	25, 28,
	31, 32,
	// clang-format on
};
/** Miniset: Replace small wall with lava 2. */
const BYTE L3ISLE4[] = {
	// clang-format off
	3, 2, // width, height

	 5,  2, 14, // search
	13, 10, 12,

	29, 26, 30, // replace
	31, 27, 32,
	// clang-format on
};
/** Miniset: Replace small wall with floor 3. */
const BYTE L3ISLE5[] = {
	// clang-format off
	2, 2, // width, height

	 5, 14, // search
	13, 12,

	7, 7, // replace
	7, 7,
	// clang-format on
};
#ifdef HELLFIRE
/** Miniset: Small acid patch 1. */
const BYTE L6PATCH1[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	0,   0, 0, // replace
	0, 126, 0,
	0,   0, 0,
	// clang-format on
};
/** Miniset: Small acid patch 2. */
const BYTE L6PATCH2[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	0,   0, 0, // replace
	0, 124, 0,
	0,   0, 0,
	// clang-format on
};
/** Miniset: Two floor tiles with a vertical wall in the middle 1. */
const BYTE L6MITE1[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	67,  0, 0, // replace
	66, 51, 0,
	 0,  0, 0,
	// clang-format on
};
/** Miniset: Two floor tiles with a vertical wall in the middle 2. */
const BYTE L6MITE2[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	69,  0, 0, // replace
	68, 52, 0,
	 0,  0, 0,
	// clang-format on
};
/** Miniset: Two floor tiles with a vertical wall in the middle 3. */
const BYTE L6MITE3[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	70,  0, 0, // replace
	71, 53, 0,
	 0,  0, 0,
	// clang-format on
};
/** Miniset: Two floor tiles with a vertical wall in the middle 4. */
const BYTE L6MITE4[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	73,  0, 0, // replace
	72, 54, 0,
	 0,  0, 0,
	// clang-format on
};
/** Miniset: Two floor tiles with a vertical wall in the middle 5. */
const BYTE L6MITE5[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	75,  0, 0, // replace
	74, 55, 0,
	 0,  0, 0,
	// clang-format on
};
/** Miniset: Two floor tiles with a vertical wall in the middle 6. */
const BYTE L6MITE6[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	77,  0, 0, // replace
	76, 56, 0,
	 0,  0, 0,
	// clang-format on
};
/** Miniset: Two floor tiles with a vertical wall in the middle 7. */
const BYTE L6MITE7[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	79,  0, 0, // replace
	78, 57, 0,
	 0,  0, 0,
	// clang-format on
};
/** Miniset: Two floor tiles with a vertical wall in the middle 8. */
const BYTE L6MITE8[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	81,  0, 0, // replace
	80, 58, 0,
	 0,  0, 0,
	// clang-format on
};
/** Miniset: Two floor tiles with a vertical wall in the middle 9. */
const BYTE L6MITE9[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	83,  0, 0, // replace
	82, 59, 0,
	 0,  0, 0,
	// clang-format on
};
const BYTE L6MITE10[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	84,  0, 0, // replace
	85, 60, 0,
	 0,  0, 0,
	// clang-format on
};
const BYTE L6ISLE3[] = {
	// clang-format off
	2, 3, // width, height

	 5, 14, // search
	 4,  9,
	13, 12,

	107, 115, // replace
	119, 122,
	131, 123,
	// clang-format on
};
const BYTE L6ISLE4[] = {
	// clang-format off
	3, 2, // width, height

	 5,  2, 14, // search
	13, 10, 12,

	107, 120, 115, // replace
	131, 121, 123,
	// clang-format on
};
const BYTE L6SPOOL1[] = {
	// clang-format off
	4, 4, // width, height

	7, 7, 7, 7, // search
	7, 7, 7, 7,
	7, 7, 7, 7,
	7, 7, 7, 7,

	7,   7,   7, 7, // replace
	7, 107, 115, 7,
	7, 131, 123, 7,
	7,   7,   7, 7,
	// clang-format on
};
const BYTE L6SPOOL2[] = {
	// clang-format off
	4, 4, // width, height

	7, 7, 7, 7, // search
	7, 7, 7, 7,
	7, 7, 7, 7,
	7, 7, 7, 7,

	7,   7,   7, 7, // replace
	7,   7, 108, 7,
	7, 109, 112, 7,
	7,   7,   7, 7,
	// clang-format on
};
const BYTE L6LPOOL1[] = {
	// clang-format off
	4, 5, // width, height

	7, 7, 7, 7, // search
	7, 7, 7, 7,
	7, 7, 7, 7,
	7, 7, 7, 7,
	7, 7, 7, 7,

	7,   7,   7, 7, // replace
	7, 107, 115, 7,
	7, 119, 122, 7,
	7, 131, 123, 7,
	7,   7,   7, 7,
	// clang-format on
};
const BYTE L6LPOOL2[] = {
	// clang-format off
	4, 5, // width, height

	7, 7, 7, 7, // search
	7, 7, 7, 7,
	7, 7, 7, 7,
	7, 7, 7, 7,
	7, 7, 7, 7,

	7,   7,   7, 7, // replace
	7, 126, 108, 7,
	7,   7, 117, 7,
	7, 109, 112, 7,
	7,   7,   7, 7,
	// clang-format on
};
#endif

static void InitL3Dungeon()
{
	memset(dungeon, 0, sizeof(dungeon));
	memset(dflags, 0, sizeof(dflags));
}

static void DRLG_LoadL3SP()
{
	assert(pSetPiece == NULL);
	if (QuestStatus(Q_ANVIL)) {
		pSetPiece = LoadFileInMem("Levels\\L3Data\\Anvil.DUN");
	}
}

static void DRLG_FreeL3SP()
{
	MemFreeDbg(pSetPiece);
}

static bool DRLG_L3FillRoom(int x1, int y1, int x2, int y2)
{
	int i, j, v;

	if (x1 <= 1 || x2 >= DMAXX - 2 || y1 <= 1 || y2 >= DMAXY - 2) {
		return false;
	}

	v = 0;
	for (j = y1; j <= y2; j++) {
		for (i = x1; i <= x2; i++) {
			v += dungeon[i][j];
		}
	}

	if (v != 0) {
		return false;
	}

	for (j = y1 + 1; j < y2; j++) {
		for (i = x1 + 1; i < x2; i++) {
			dungeon[i][j] = 1;
		}
	}
	for (j = y1; j <= y2; j++) {
		if (random_(0, 2) != 0) {
			dungeon[x1][j] = 1;
		}
		if (random_(0, 2) != 0) {
			dungeon[x2][j] = 1;
		}
	}
	for (i = x1; i <= x2; i++) {
		if (random_(0, 2) != 0) {
			dungeon[i][y1] = 1;
		}
		if (random_(0, 2) != 0) {
			dungeon[i][y2] = 1;
		}
	}

	return true;
}

static void DRLG_L3CreateBlock(int x, int y, int obs, int dir)
{
	int blksizex, blksizey, x1, y1, x2, y2;

	blksizex = RandRange(3, 4);
	blksizey = RandRange(3, 4);

	switch (dir) {
	case 0: // block to the north
		y2 = y - 1;
		y1 = y2 - blksizey;
		x1 = x;
		if (blksizex < obs)
			x1 += random_(0, blksizex);
		else if (blksizex > obs)
			x1 -= random_(0, blksizex);
		x2 = blksizex + x1;
		break;
	case 1: // block to the east
		x1 = x + 1;
		x2 = x1 + blksizex;
		y1 = y;
		if (blksizey < obs)
			y1 += random_(0, blksizey);
		else if (blksizey > obs)
			y1 -= random_(0, blksizey);
		y2 = y1 + blksizey;
		break;
	case 2: // block to the south
		y1 = y + 1;
		y2 = y1 + blksizey;
		x1 = x;
		if (blksizex < obs)
			x1 += random_(0, blksizex);
		else if (blksizex > obs)
			x1 -= random_(0, blksizex);
		x2 = blksizex + x1;
		break;
	case 3: // block to the west
		x2 = x - 1;
		x1 = x2 - blksizex;
		y1 = y;
		if (blksizey < obs)
			y1 += random_(0, blksizey);
		else if (blksizey > obs)
			y1 -= random_(0, blksizey);
		y2 = y1 + blksizey;
		break;
	case 4: // the central block
		x1 = x;
		y1 = y;
		x2 = x1 + blksizex;
		y2 = y1 + blksizey;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	if (DRLG_L3FillRoom(x1, y1, x2, y2) && (random_(0, 4) != 0 || dir == 4)) {
		if (dir != 2) {
			DRLG_L3CreateBlock(x1, y1, blksizey, 0);
		}
		if (dir != 3) {
			DRLG_L3CreateBlock(x2, y1, blksizex, 1);
		}
		if (dir != 0) {
			DRLG_L3CreateBlock(x1, y2, blksizey, 2);
		}
		if (dir != 1) {
			DRLG_L3CreateBlock(x1, y1, blksizex, 3);
		}
	}
}

static void DRLG_L3FloorArea(int x1, int y1, int x2, int y2)
{
	int i, j;

	for (j = y1; j <= y2; j++) {
		for (i = x1; i <= x2; i++) {
			dungeon[i][j] = 1;
		}
	}
}

static void DRLG_L3FillDiags()
{
	int i, j, v;

	for (j = 0; j < DMAXY - 1; j++) {
		for (i = 0; i < DMAXX - 1; i++) {
			// assert(dungeon[i][j] <= 1);
			v = dungeon[i + 1][j + 1]
			 | (dungeon[i][j + 1] << 1)
			 | (dungeon[i + 1][j] << 2)
			 | (dungeon[i][j] << 3);
			if (v == 6) {
				if (random_(0, 2) == 0) {
					dungeon[i][j] = 1;
				} else {
					dungeon[i + 1][j + 1] = 1;
				}
			}
			if (v == 9) {
				if (random_(0, 2) == 0) {
					dungeon[i + 1][j] = 1;
				} else {
					dungeon[i][j + 1] = 1;
				}
			}
		}
	}
}

static void DRLG_L3FillSingles()
{
	int i, j;

	for (j = 1; j < DMAXY - 1; j++) {
		for (i = 1; i < DMAXX - 1; i++) {
			// assert(dungeon[i][j] <= 1);
			if (dungeon[i][j] == 0
			 && (dungeon[i][j - 1] & dungeon[i - 1][j - 1] & dungeon[i + 1][j - 1])
			 && (dungeon[i + 1][j] & dungeon[i - 1][j])
			 && (dungeon[i][j + 1] & dungeon[i - 1][j + 1] & dungeon[i + 1][j + 1])) {
				dungeon[i][j] = 1;
			}
		}
	}
}

static void DRLG_L3FillStraights()
{
	int i, j, sxy;

	for (j = 0; j < DMAXY - 1; j++) {
		sxy = 0;
		for (i = 0; i < DMAXX - 3; i++) {
			if (dungeon[i][j] == 0 && dungeon[i][j + 1] == 1) {
				sxy++;
			} else {
				if (sxy > 3 && random_(0, 2) != 0) {
					for (sxy = i - sxy; sxy < i; sxy++) {
						dungeon[sxy][j] = random_(0, 2);
					}
				}
				sxy = 0;
			}
		}
	}
	for (j = 0; j < DMAXY - 1; j++) {
		sxy = 0;
		for (i = 0; i < DMAXX - 3; i++) {
			if (dungeon[i][j] == 1 && dungeon[i][j + 1] == 0) {
				sxy++;
			} else {
				if (sxy > 3 && random_(0, 2) != 0) {
					for (sxy = i - sxy; sxy < i; sxy++) {
						dungeon[sxy][j + 1] = random_(0, 2);
					}
				}
				sxy = 0;
			}
		}
	}
	for (i = 0; i < DMAXX - 1; i++) {
		sxy = 0;
		for (j = 0; j < DMAXY - 3; j++) {
			if (dungeon[i][j] == 0 && dungeon[i + 1][j] == 1) {
				sxy++;
			} else {
				if (sxy > 3 && random_(0, 2) != 0) {
					for (sxy = j - sxy; sxy < j; sxy++) {
						dungeon[i][sxy] = random_(0, 2);
					}
				}
				sxy = 0;
			}
		}
	}
	for (i = 0; i < DMAXX - 1; i++) {
		sxy = 0;
		for (j = 0; j < DMAXY - 3; j++) {
			if (dungeon[i][j] == 1 && dungeon[i + 1][j] == 0) {
				sxy++;
			} else {
				if (sxy > 3 && random_(0, 2) != 0) {
					for (sxy = j - sxy; sxy < j; sxy++) {
						dungeon[i + 1][sxy] = random_(0, 2);
					}
				}
				sxy = 0;
			}
		}
	}
}

static void DRLG_L3Edges()
{
	int i, j;

	for (j = 0; j < DMAXY; j++) {
		dungeon[DMAXX - 1][j] = 0;
	}
	for (i = 0; i < DMAXX; i++) {
		dungeon[i][DMAXY - 1] = 0;
	}
}

static int DRLG_L3GetFloorArea()
{
	int i, rv;
	BYTE *pTmp;

	rv = 0;
	static_assert(sizeof(dungeon) == DMAXX * DMAXY, "Linear traverse of dungeon does not work in DRLG_L3GetFloorArea.");
	pTmp = &dungeon[0][0];
	for (i = 0; i < DMAXX * DMAXY; i++, pTmp++) {
		assert(*pTmp <= 1);
		rv += *pTmp;
	}

	return rv;
}

static void DRLG_L3MakeMegas()
{
	int i, j;
	BYTE v;

	for (j = 0; j < DMAXY - 1; j++) {
		for (i = 0; i < DMAXX - 1; i++) {
			// assert(dungeon[i][j] <= 1);
			v = dungeon[i + 1][j + 1]
			 | (dungeon[i][j + 1] << 1)
			 | (dungeon[i + 1][j] << 2)
			 | (dungeon[i][j] << 3);
			if (v == 6) {
				if (random_(0, 2) == 0) {
					v = 12;
				} else {
					v = 5;
				}
			}
			if (v == 9) {
				if (random_(0, 2) == 0) {
					v = 13;
				} else {
					v = 14;
				}
			}
			dungeon[i][j] = L3ConvTbl[v];
		}
	}
	for (j = 0; j < DMAXY; j++)
		dungeon[DMAXX - 1][j] = 8;
	for (i = 0; i < DMAXX - 1; i++)
		dungeon[i][DMAXY - 1] = 8;
}

static void DRLG_L3FloodTVal()
{
	//const POS32 offs[12] = { { 0, -1}, { 1, -1 },
	//	{ -1, 0 }, { 0, 0 }, { 1, 0 }, { 2, 0 },
	//	{ -1, 1 }, { 0, 1 }, { 1, 1 }, { 2, 1 },
	//	{ 0, 2 }, { 1, 2 } };
	//const BYTE L3ConvTbl[16] =     { 8, 11,  3, 10,  1,  9, 12, 12,  6, 13,  4, 13,  2, 14,  5,  7 };
	//                                 0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
	//const BYTE L3BackConvTbl[16] = { 0, 10,  3, 12,  5, 15,  1, 15,  0, 10, 12,  8, 15, 15, 15,  0 };
	//int i, j, k;
	int i, j;

	for (i = 0; i < DMAXX; i++) {
		for (j = 0; j < DMAXY; j++) {
			/*if (dungeon[i][j] == 7) {
				for (k = 0; k < lengthof(offs); k++)
					dTransVal[DBORDERX + i * 2 + offs[k].x][DBORDERY + j * 2 + offs[k].y] = 1;
			}*/
			if (dungeon[i][j] != 8) {
				dTransVal[DBORDERX + i * 2 + 0][DBORDERY + j * 2 + 0] = 1;
				dTransVal[DBORDERX + i * 2 + 1][DBORDERY + j * 2 + 0] = 1;
				dTransVal[DBORDERX + i * 2 + 0][DBORDERY + j * 2 + 1] = 1;
				dTransVal[DBORDERX + i * 2 + 1][DBORDERY + j * 2 + 1] = 1;
			}
		}
	}

	assert(TransVal == 1);
	TransVal = 2;
}

static void DRLG_L3River()
{
	int rx, ry, px, py, dir, pdir, nodir, nodir2, dircheck;
	int river[3][100];
	int rivercnt, riveramt;
	int i, tries, found, bridge, lpcnt;
	bool done;

	rivercnt = 0;
	tries = 0;

	while (tries < 200 && rivercnt < 4) {
		done = false;
		while (!done && tries < 200) {
			tries++;
			lpcnt = 0;
			do {
				rx = random_(0, DMAXX);
				ry = random_(0, DMAXY);
				while (dungeon[rx][ry] < 25 || dungeon[rx][ry] > 28) {
					if (++rx == DMAXX) {
						rx = 0;
						if (++ry == DMAXY)
							break;
					}
				}
			} while (ry == DMAXY && ++lpcnt < 100);
			if (lpcnt == 100)
				continue;

			switch (dungeon[rx][ry]) {
			case 25:
				dir = 3;
				nodir = 2;
				river[2][0] = 40;
				break;
			case 26:
				dir = 0;
				nodir = 1;
				river[2][0] = 38;
				break;
			case 27:
				dir = 1;
				nodir = 0;
				river[2][0] = 41;
				break;
			case 28:
				dir = 2;
				nodir = 3;
				river[2][0] = 39;
				break;
			default:
				ASSUME_UNREACHABLE
			}
			river[0][0] = rx;
			river[1][0] = ry;
			/// BUGFIX: pdir is uninitialized, add code `pdir = -1;`(fixed)
			pdir = -1;
			riveramt = 1;
			nodir2 = 4;
			dircheck = 0;
			while (dircheck < 4 && riveramt < 100) {
				px = rx;
				py = ry;
				if (dircheck == 0) {
					dir = random_(0, 4);
				} else {
					dir = (dir + 1) & 3;
				}
				dircheck++;
				while (dir == nodir || dir == nodir2) {
					dir = (dir + 1) & 3;
					dircheck++;
				}
				switch (dir) {
				case 0:
					if (ry > 0)
						ry--;
					break;
				case 1:
					if (ry < DMAXY - 1)
						ry++;
					break;
				case 2:
					if (rx < DMAXX - 1)
						rx++;
					break;
				case 3:
					if (rx > 0)
						rx--;
					break;
				default:
					ASSUME_UNREACHABLE
				}
				if (dungeon[rx][ry] == 7) {
					dircheck = 0;
					river[0][riveramt] = rx;
					river[1][riveramt] = ry;
					river[2][riveramt] = RandRange(15, 16) + (1 - (dir >> 1)) * 2;
					riveramt++;
					if ((dir == 0 && pdir == 2) || (dir == 3 && pdir == 1)) {
						if (riveramt > 2) {
							river[2][riveramt - 2] = 22;
						}
						nodir2 = dir == 0 ? 1 : 2;
					}
					if ((dir == 0 && pdir == 3) || (dir == 2 && pdir == 1)) {
						if (riveramt > 2) {
							river[2][riveramt - 2] = 21;
						}
						nodir2 = dir + 1; // dir == 0 ? 1 : 3;
					}
					if ((dir == 1 && pdir == 2) || (dir == 3 && pdir == 0)) {
						if (riveramt > 2) {
							river[2][riveramt - 2] = 20;
						}
						nodir2 = dir - 1; // dir == 1 ? 0 : 2;
					}
					if ((dir == 1 && pdir == 3) || (dir == 2 && pdir == 0)) {
						if (riveramt > 2) {
							river[2][riveramt - 2] = 19;
						}
						nodir2 = dir == 1 ? 0 : 3;
					}
					pdir = dir;
				} else {
					rx = px;
					ry = py;
				}
			}
			switch (dir) {
			case 0:
				// BUGFIX: Check `ry >= 2` (fixed)
				if (ry >= 2 && dungeon[rx][ry - 1] == 10 && dungeon[rx][ry - 2] == 8) {
					river[0][riveramt] = rx;
					river[1][riveramt] = ry - 1;
					river[2][riveramt] = 24;
					if (pdir == 2) {
						river[2][riveramt - 1] = 22;
					} else if (pdir == 3) {
						river[2][riveramt - 1] = 21;
					}
					done = true;
				}
				break;
			case 1:
				// BUGFIX: Check `ry + 2 < DMAXY` (fixed)
				if (ry + 2 < DMAXY && dungeon[rx][ry + 1] == 2 && dungeon[rx][ry + 2] == 8) {
					river[0][riveramt] = rx;
					river[1][riveramt] = ry + 1;
					river[2][riveramt] = 42;
					if (pdir == 2) {
						river[2][riveramt - 1] = 20;
					} else if (pdir == 3) {
						river[2][riveramt - 1] = 19;
					}
					done = true;
				}
				break;
			case 2:
				// BUGFIX: Check `rx + 2 < DMAXX` (fixed)
				if (rx + 2 < DMAXX && dungeon[rx + 1][ry] == 4 && dungeon[rx + 2][ry] == 8) {
					river[0][riveramt] = rx + 1;
					river[1][riveramt] = ry;
					river[2][riveramt] = 43;
					if (pdir == 0) {
						river[2][riveramt - 1] = 19;
					} else if (pdir == 1) {
						river[2][riveramt - 1] = 21;
					}
					done = true;
				}
				break;
			case 3:
				// BUGFIX: Check `rx >= 2` (fixed)
				if (rx >= 2 && dungeon[rx - 1][ry] == 9 && dungeon[rx - 2][ry] == 8) {
					river[0][riveramt] = rx - 1;
					river[1][riveramt] = ry;
					river[2][riveramt] = 23;
					if (pdir == 0) {
						river[2][riveramt - 1] = 20;
					} else if (pdir == 1) {
						river[2][riveramt - 1] = 22;
					}
					done = true;
				}
				break;
			default:
				ASSUME_UNREACHABLE
			}
		}
		if (!done || riveramt < 7)
			continue;
		found = 0;
		lpcnt = 0;
		while (found == 0 && lpcnt < 30) {
			lpcnt++;
			bridge = random_(0, riveramt);
			if ((river[2][bridge] == 15 || river[2][bridge] == 16)
			 && dungeon[river[0][bridge]][river[1][bridge] - 1] == 7
			 && dungeon[river[0][bridge]][river[1][bridge] + 1] == 7) {
				found = 44;
				for (i = 0; i < riveramt && found != 0; i++) {
					if ((river[1][bridge] - 1 == river[1][i] || river[1][bridge] + 1 == river[1][i])
					 && river[0][bridge] == river[0][i]) {
						found = 0;
					}
				}
			} else if ((river[2][bridge] == 17 || river[2][bridge] == 18)
			 && dungeon[river[0][bridge] - 1][river[1][bridge]] == 7
			 && dungeon[river[0][bridge] + 1][river[1][bridge]] == 7) {
				found = 45;
				for (i = 0; i < riveramt && found != 0; i++) {
					if ((river[0][bridge] - 1 == river[0][i] || river[0][bridge] + 1 == river[0][i])
					 && river[1][bridge] == river[1][i]) {
						found = 0;
					}
				}
			}
		}
		if (found != 0) {
			river[2][bridge] = found;
			rivercnt++;
			for (i = 0; i <= riveramt; i++) {
				dungeon[river[0][i]][river[1][i]] = river[2][i];
			}
		}
	}
}

static int lavaarea;
static bool DRLG_L3SpawnLava(int x, int y, int dir)
{
	BYTE i; //                        0     1     2     3     4    ?5    ?6     7     8     9    10   ?11    12    13    14
	static BYTE spawntable[15] = { 0x00, 0x0A, 0x08, 0x05, 0x01, 0x00, 0x00, 0xFF, 0x00, 0x02, 0x04, 0x00, 0x06, 0x05, 0x0A };

	if (x < 0 || x >= DMAXX || y < 0 || y >= DMAXY) {
		return true;
	}
	i = dungeon[x][y];
	if (i & 0x80) {
		return false;
	}
	if (i > 15) {
		return true;
	}

	i = spawntable[i];
	/*switch (dir) {
	case 3: // DIR_S
		if (i & 8)
			return false;
		break;
	case 2: // DIR_N
		if (i & 4)
			return false;
		break;
	case 0: // DIR_E
		if (i & 1)
			return false;
		break;
	case 1: // DIR_W
		if (i & 2)
			return false;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}*/
	if (i & (1 << dir))
		return false;

	dungeon[x][y] |= 0x80;
	lavaarea += 1;
	if (lavaarea > 40) {
		return true;
	}

	if (DRLG_L3SpawnLava(x + 1, y, 0)) {
		return true;
	}
	if (DRLG_L3SpawnLava(x - 1, y, 1)) {
		return true;
	}
	if (DRLG_L3SpawnLava(x, y + 1, 3)) {
		return true;
	}
	if (DRLG_L3SpawnLava(x, y - 1, 2)) {
		return true;
	}

	return false;
}

static void DRLG_L3DrawLava(int x, int y)
{
	BYTE i;                 //     0     1     2     3     4     5     6     7     8     9    10    11    12    13    14 
	static BYTE poolsub[15] = { 0x00, 0x23, 0x1A, 0x24, 0x19, 0x1D, 0x22, 0x07, 0x21, 0x1C, 0x1B, 0x25, 0x20, 0x1F, 0x1E };

	if (x < 0 || x >= DMAXX || y < 0 || y >= DMAXY) {
		return;
	}

	i = dungeon[x][y];
	if (!(i & 0x80)) {
		return;
	}

	i &= ~0x80;
	if (lavaarea != 0) {
		i = poolsub[i];
	}

	dungeon[x][y] = i;

	DRLG_L3DrawLava(x + 1, y);
	DRLG_L3DrawLava(x - 1, y);
	DRLG_L3DrawLava(x, y + 1);
	DRLG_L3DrawLava(x, y - 1);
}

/**
 * Flood fills dirt and wall tiles looking for
 * an area of at most 40 tiles and disconnected from the map edge.
 * If it finds one, converts it to lava tiles and sets lavapool to TRUE.
 */
static void DRLG_L3Pool()
{
	int i, j;
	bool badPos;

	for (i = 3; i < DMAXY - 3; i++) {
		for (j = 3; j < DMAXY - 3; j++) {
			if (dungeon[i][j] != 8 || random_(0, 2) != 0) {
				continue;
			}
			lavaarea = 0;
			badPos = DRLG_L3SpawnLava(i, j, 0);
			if (badPos || lavaarea < 4)
				lavaarea = 0;
			else
				_guLavapools = MIN_LAVA_POOL;
			DRLG_L3DrawLava(i, j);
		}
	}
}

static void DRLG_L3PlaceRndSet(const BYTE *miniset, BYTE rndper)
{
	int sx, sy, sw, sh, xx, yy, ii;
	bool found;

	sw = miniset[0];
	sh = miniset[1];

	for (sy = 0; sy < DMAXX - sh; sy++) {
		for (sx = 0; sx < DMAXY - sw; sx++) {
			found = true;
			ii = 2;
			for (yy = sy; yy < sy + sh && found; yy++) {
				for (xx = sx; xx < sx + sw && found; xx++) {
					if (miniset[ii] != 0 && dungeon[xx][yy] != miniset[ii]) {
						found = false;
					}
					if (dflags[xx][yy]) {
						found = false;
					}
					ii++;
				}
			}
			if (!found)
				continue;
			// assert(ii == sw * sh + 2);
			if (miniset[ii] >= 84 && miniset[ii] <= 100) {
				// BUGFIX: accesses to dungeon can go out of bounds (fixed)
				// BUGFIX: Comparisons vs 100 should use same tile as comparisons vs 84. (fixed)
				if ((sx > 0 && dungeon[sx - 1][sy] >= 84 && dungeon[sx - 1][sy] <= 100)
				 || (dungeon[sx + 1][sy] >= 84 && dungeon[sx + 1][sy] <= 100)
				 || (dungeon[sx][sy + 1] >= 84 && dungeon[sx][sy + 1] <= 100)
				 || (sy > 0 && dungeon[sx][sy - 1] >= 84 && dungeon[sx][sy - 1] <= 100)) {
					continue;
				}
			}
			if (random_(0, 100) < rndper) {
				for (yy = sy; yy < sy + sh; yy++) {
					for (xx = sx; xx < sx + sw; xx++) {
						if (miniset[ii] != 0) {
							dungeon[xx][yy] = miniset[ii];
						}
						ii++;
					}
				}
			}
		}
	}
}

#ifdef HELLFIRE
static void DRLG_L6PlaceRndPool(const BYTE *miniset, BYTE rndper)
{
	int sx, sy, sw, sh, xx, yy, ii;
	bool found, placed;

	placed = false;
	sw = miniset[0];
	sh = miniset[1];

	for (sy = 0; sy < DMAXX - sh; sy++) {
		for (sx = 0; sx < DMAXY - sw; sx++) {
			found = true;
			ii = 2;
			for (yy = sy; yy < sy + sh && found; yy++) {
				for (xx = sx; xx < sx + sw && found; xx++) {
					if (/*miniset[ii] != 0 &&*/ dungeon[xx][yy] != miniset[ii]) {
						found = false;
					}
					if (dflags[xx][yy]) {
						found = false;
					}
					ii++;
				}
			}
			if (!found)
				continue;
			assert(ii == sw * sh + 2);
			/*if (miniset[ii] >= 84 && miniset[ii] <= 100) {
				// BUGFIX: Comparisons vs 100 should use same tile as comparisons vs 84.
				if ((dungeon[sx - 1][sy] >= 84 && dungeon[sx - 1][sy] <= 100)
				 || (dungeon[sx + 1][sy] >= 84 && dungeon[sx + 1][sy] <= 100)
				 || (dungeon[sx][sy + 1] >= 84 && dungeon[sx][sy + 1] <= 100)
				 || (dungeon[sx][sy - 1] >= 84 && dungeon[sx][sy - 1] <= 100)) {
					continue;
				}
			}*/
			if (random_(0, 100) < rndper) {
				placed = true;
				for (yy = sy; yy < sy + sh; yy++) {
					for (xx = sx; xx < sx + sw; xx++) {
						//if (miniset[ii] != 0) {
							dungeon[xx][yy] = miniset[ii];
						//}
						ii++;
					}
				}
			}
		}
	}

	_guLavapools += placed ? 1 : 0;
}
#endif

static bool WoodVertU(int i, int y)
{
	BYTE bv;

	bv = dungeon[i + 1][y];
	if (bv >= 130 && bv <= 152)
		return false;
	bv = dungeon[i - 1][y];
	if (bv >= 130 && bv <= 152)
		return false;
	bv = dungeon[i][y];
	return bv == 7 || bv == 10 || bv == 126 || bv == 129 || bv == 134 || bv == 136;
}

static bool WoodVertD(int i, int y)
{
	BYTE bv;

	bv = dungeon[i + 1][y];
	if (bv >= 130 && bv <= 152)
		return false;
	bv = dungeon[i - 1][y];
	if (bv >= 130 && bv <= 152)
		return false;
	bv = dungeon[i][y];
	return bv == 7 || bv == 2 || bv == 134 || bv == 136;
}

static bool WoodHorizL(int x, int j)
{
	BYTE bv;

	bv = dungeon[x][j + 1];
	if (bv >= 130 && bv <= 152)
		return false;
	bv = dungeon[x][j - 1];
	if (bv >= 130 && bv <= 152)
		return false;
	bv = dungeon[x][j];
	return bv == 7 || bv == 9 || bv == 121 || bv == 124 || bv == 135 || bv == 137;
}

static bool WoodHorizR(int x, int j)
{
	BYTE bv;

	bv = dungeon[x][j + 1];
	if (bv >= 130 && bv <= 152)
		return false;
	bv = dungeon[x][j - 1];
	if (bv >= 130 && bv <= 152)
		return false;
	bv = dungeon[x][j];
	return bv == 7 || bv == 4 || bv == 135 || bv == 137;
}

static void AddFenceDoors()
{
	int i, j;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (dungeon[i][j] == 7) {
				if (dungeon[i - 1][j] <= 152 && dungeon[i - 1][j] >= 130
				    && dungeon[i + 1][j] <= 152 && dungeon[i + 1][j] >= 130) {
					dungeon[i][j] = 146;
					continue;
				}
				if (dungeon[i][j - 1] <= 152 && dungeon[i][j - 1] >= 130
				    && dungeon[i][j + 1] <= 152 && dungeon[i][j + 1] >= 130) {
					dungeon[i][j] = 147;
					continue;
				}
			}
		}
	}
}

static void FenceDoorFix()
{
	int i, j;
	BYTE bv0, bv1;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (dungeon[i][j] == 146) {
				bv0 = dungeon[i + 1][j];
				bv1 = dungeon[i - 1][j];
				if (bv0 > 152 || bv0 < 130 || bv1 > 152 || bv1 < 130) {
					dungeon[i][j] = 7;
					continue;
				}

				if (bv0 != 130 && bv1 != 130
				 && bv0 != 132 && bv1 != 132
				 && bv0 != 133 && bv1 != 133
				 && bv0 != 134 && bv1 != 134
				 && bv0 != 136 && bv1 != 136
				 && bv0 != 138 && bv1 != 138
				 && bv0 != 140 && bv1 != 140) {
					dungeon[i][j] = 7;
					continue;
				}
			} else if (dungeon[i][j] == 147) {
				bv0 = dungeon[i][j + 1];
				bv1 = dungeon[i][j - 1];
				if (bv0 > 152 || bv0 < 130 || bv1 > 152 || bv1 < 130) {
					dungeon[i][j] = 7;
					continue;
				}

				if (bv0 != 131 && bv1 != 131
				 && bv0 != 132 && bv1 != 132
				 && bv0 != 133 && bv1 != 133
				 && bv0 != 135 && bv1 != 135
				 && bv0 != 137 && bv1 != 137
				 && bv0 != 138 && bv1 != 138
				 && bv0 != 139 && bv1 != 139) {
					dungeon[i][j] = 7;
					continue;
				}
			}
		}
	}
}

static void DRLG_L3Wood()
{
#if (_MSC_VER >= 1920)
	volatile int j;// visual studio 2019 throws internal compiler error without it, see #708
#else
	int j;
#endif
	int i, x, y, xx, yy, rp, x1, y1, x2, y2;
	BYTE bv;

	for (j = 1; j < DMAXY - 1; j++) {     // BUGFIX: Change '0' to '1' (fixed)
		for (i = 1; i < DMAXX - 1; i++) { // BUGFIX: Change '0' to '1' (fixed)
			if (dungeon[i][j] == 10 && random_(0, 2) != 0) {
				x = i;
				do {
					x++;
				} while (dungeon[x][j] == 10);
				x--;
				if (x - i > 0) {
					dungeon[i][j] = 127;
					for (xx = i + 1; xx < x; xx++) {
						if (random_(0, 2) != 0) {
							dungeon[xx][j] = 126;
						} else {
							dungeon[xx][j] = 129;
						}
					}
					dungeon[x][j] = 128;
				}
			}
			if (dungeon[i][j] == 9 && random_(0, 2) != 0) {
				y = j;
				do {
					y++;
				} while (dungeon[i][y] == 9);
				y--;
				if (y - j > 0) {
					dungeon[i][j] = 123;
					for (yy = j + 1; yy < y; yy++) {
						if (random_(0, 2) != 0) {
							dungeon[i][yy] = 121;
						} else {
							dungeon[i][yy] = 124;
						}
					}
					dungeon[i][y] = 122;
				}
			}
			if (dungeon[i][j] == 11 && dungeon[i + 1][j] == 10 && dungeon[i][j + 1] == 9 && random_(0, 2) != 0) {
				dungeon[i][j] = 125;
				x = i + 1;
				do {
					x++;
				} while (dungeon[x][j] == 10);
				x--;
				for (xx = i + 1; xx < x; xx++) {
					if (random_(0, 2) != 0) {
						dungeon[xx][j] = 126;
					} else {
						dungeon[xx][j] = 129;
					}
				}
				dungeon[x][j] = 128;
				y = j + 1;
				do {
					y++;
				} while (dungeon[i][y] == 9);
				y--;
				for (yy = j + 1; yy < y; yy++) {
					if (random_(0, 2) != 0) {
						dungeon[i][yy] = 121;
					} else {
						dungeon[i][yy] = 124;
					}
				}
				dungeon[i][y] = 122;
			}
		}
	}

	for (j = 1; j < DMAXY; j++) {     // BUGFIX: Change '0' to '1' (fixed)
		for (i = 1; i < DMAXX; i++) { // BUGFIX: Change '0' to '1' (fixed)
			if (dungeon[i][j] == 7 && random_(0, 1) == 0 && SkipThemeRoom(i, j)) {
				if (random_(0, 2) == 0) {
					y1 = j;
					// BUGFIX: Check `y1 >= 0` first (fixed)
					while (y1 >= 0 && WoodVertU(i, y1)) {
						y1--;
					}
					y1++;
					y2 = j;
					// BUGFIX: Check `y2 < DMAXY` first (fixed)
					while (y2 < DMAXY && WoodVertD(i, y2)) {
						y2++;
					}
					y2--;
					if (y2 - y1 > 1 && dungeon[i][y1] != 7 && dungeon[i][y2] != 7) {
						rp = RandRange(y1 + 1, y2 - 1);
						for (y = y1; y <= y2; y++) {
							if (y == rp) {
								continue;
							}
							bv = dungeon[i][y];
							if (bv == 7) {
								if (random_(0, 2) != 0) {
									dungeon[i][y] = 135;
								} else {
									dungeon[i][y] = 137;
								}
							}
							if (bv == 10) {
								dungeon[i][y] = 131;
							}
							if (bv == 126) {
								dungeon[i][y] = 133;
							}
							if (bv == 129) {
								dungeon[i][y] = 133;
							}
							if (bv == 2) {
								dungeon[i][y] = 139;
							}
							if (bv == 134) {
								dungeon[i][y] = 138;
							}
							if (bv == 136) {
								dungeon[i][y] = 138;
							}
						}
					}
				} else {
					x1 = i;
					// BUGFIX: Check `x1 >= 0` first (fixed)
					while (x1 >= 0 && WoodHorizL(x1, j)) {
						x1--;
					}
					x1++;
					x2 = i;
					// BUGFIX: Check `x2 < DMAXX` first (fixed)
					while (x2 < DMAXX && WoodHorizR(x2, j)) {
						x2++;
					}
					x2--;
					if (x2 - x1 > 1 && dungeon[x1][j] != 7 && dungeon[x2][j] != 7) {
						rp = RandRange(x1 + 1, x2 - 1);
						for (x = x1; x <= x2; x++) {
							if (x == rp) {
								continue;
							}
							bv = dungeon[x][j];
							if (bv == 7) {
								if (random_(0, 2) != 0) {
									dungeon[x][j] = 134;
								} else {
									dungeon[x][j] = 136;
								}
							}
							if (bv == 9) {
								dungeon[x][j] = 130;
							}
							if (bv == 121) {
								dungeon[x][j] = 132;
							}
							if (bv == 124) {
								dungeon[x][j] = 132;
							}
							if (bv == 4) {
								dungeon[x][j] = 140;
							}
							if (bv == 135) {
								dungeon[x][j] = 138;
							}
							if (bv == 137) {
								dungeon[x][j] = 138;
							}
						}
					}
				}
			}
		}
	}

	AddFenceDoors();
	FenceDoorFix();
}

static void DRLG_L3SetRoom(int rx1, int ry1)
{
	int rw, rh, i, j;
	BYTE *sp;

	rw = pSetPiece[0];
	rh = pSetPiece[2];

	// assert(setpc_x == rx1);
	// assert(setpc_y == ry1);
	assert(setpc_w == rw);
	assert(setpc_h == rh);

	sp = &pSetPiece[4];

	rw += rx1;
	rh += ry1;
	for (j = ry1; j < rh; j++) {
		for (i = rx1; i < rw; i++) {
			dungeon[i][j] = *sp != 0 ? *sp : 7;
			dflags[i][j] = TRUE; // |= DLRG_PROTECTED;
			sp += 2;
		}
	}
}

static void FixL3Warp()
{
	int i, j;

	if (currLvl._dLevelIdx != DLV_CAVES1)
		return;

	for (j = 0; j < DMAXY - 1; j++) {
		for (i = 0; i < DMAXX - 1; i++) {
			if (dungeon[i][j] == 125 && dungeon[i + 1][j] == 125 && dungeon[i][j + 1] == 125 && dungeon[i + 1][j + 1] == 125) {
				dungeon[i][j] = 156;
				dungeon[i + 1][j] = 155;
				dungeon[i][j + 1] = 153;
				dungeon[i + 1][j + 1] = 154;
				return;
			}
		}
	}
}

static void FixL3HallofHeroes()
{
	int i, j;

	for (j = 0; j < DMAXY - 1; j++) {
		for (i = 0; i < DMAXX - 1; i++) {
			if (dungeon[i][j] == 5 && dungeon[i + 1][j + 1] == 7) {
				dungeon[i][j] = 7;
			}
		}
	}
	for (j = 0; j < DMAXY - 1; j++) {
		for (i = 0; i < DMAXX - 1; i++) {
			if (dungeon[i][j] == 5 && dungeon[i + 1][j + 1] == 12) {
				if (dungeon[i + 1][j] == 7) {
					dungeon[i][j] = 7;
					dungeon[i][j + 1] = 7;
					dungeon[i + 1][j + 1] = 7;
				} else if (dungeon[i][j + 1] == 7) {
					dungeon[i][j] = 7;
					dungeon[i + 1][j] = 7;
					dungeon[i + 1][j + 1] = 7;
				}
			}
		}
	}
}

static void DRLG_L3LockRec(int x, int y)
{
	if (!_gabLockout[x][y]) {
		return;
	}

	_gabLockout[x][y] = false;
	_guLockoutcnt++;
	DRLG_L3LockRec(x, y - 1);
	DRLG_L3LockRec(x, y + 1);
	DRLG_L3LockRec(x - 1, y);
	DRLG_L3LockRec(x + 1, y);
}

static bool DRLG_L3Lockout()
{
	int i, j, fx, fy;
	unsigned t;

	t = 0;
	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (dungeon[i][j] != 0) {
				_gabLockout[i][j] = true;
				fx = i;
				fy = j;
				t++;
			} else {
				_gabLockout[i][j] = false;
			}
		}
	}

	_guLockoutcnt = 0;
	DRLG_L3LockRec(fx, fy);

	return t == _guLockoutcnt;
}

struct mini_set {
	const BYTE* data;
	bool setview;
};
static bool DRLG_L3PlaceMiniSets(mini_set* minisets, int n)
{
	int i;
	POS32 mpos;

	for (i = 0; i < n; i++) {
		if (minisets[i].data == NULL)
			continue;
		mpos = DRLG_PlaceMiniSet(minisets[i].data);
		if (mpos.x == DMAXX)
			return false;
		if (minisets[i].setview) {
			ViewX = 2 * mpos.x + DBORDERX + 1;
			ViewY = 2 * mpos.y + DBORDERY + 3;
		}
	}
	return true;
}

static void DRLG_L3(int entry)
{
	bool doneflag;

	do {
		do {
			do {
				InitL3Dungeon();
				DRLG_L3CreateBlock(RandRange(10, 29), RandRange(10, 29), 0, 4);
				if (pSetPiece != NULL) {
					setpc_w = pSetPiece[0];
					setpc_h = pSetPiece[2];
					setpc_x = RandRange(10, DMAXX - setpc_w - 10);
					setpc_y = RandRange(10, DMAXY - setpc_h - 10);
					DRLG_L3FloorArea(setpc_x, setpc_y, setpc_x + setpc_w, setpc_y + setpc_h);
				}
				DRLG_L3FillDiags();
				DRLG_L3FillSingles();
				DRLG_L3FillStraights();
				DRLG_L3FillDiags();
				DRLG_L3Edges();
			} while (DRLG_L3GetFloorArea() < 600 || !DRLG_L3Lockout());
			DRLG_L3MakeMegas();
			if (pSetPiece != NULL) {
				DRLG_L3SetRoom(setpc_x, setpc_y);
			}

#ifdef HELLFIRE
			if (currLvl._dType == DTYPE_NEST) {
				mini_set stairs[2] = {
					{ currLvl._dLevelIdx != DLV_NEST1 ? L6USTAIRS : L6TWARP, entry != ENTRY_PREV },
					{ currLvl._dLevelIdx != DLV_NEST4 ? L6DSTAIRS : NULL, entry == ENTRY_PREV }
				};
				doneflag = DRLG_L3PlaceMiniSets(stairs, 2);
				if (entry == ENTRY_PREV) {
					ViewX += 2;
					ViewY -= 2;
				}
			} else
#endif
			{
				// assert(currLvl._dType == DTYPE_CAVES);
				mini_set stairs[3] = {
					{ L3USTAIRS,  entry == ENTRY_MAIN },
					{ L3DSTAIRS, entry == ENTRY_PREV },
					{ currLvl._dLevelIdx != DLV_CAVES1 ? NULL : L3TWARP, entry != ENTRY_MAIN  && entry != ENTRY_PREV },
				};
				doneflag = DRLG_L3PlaceMiniSets(stairs, 3);
				if (entry == ENTRY_PREV) {
					ViewX += 2;
					ViewY -= 2;
				}
			}
		} while (!doneflag);
		// generate lava pools
		_guLavapools = 0;
#ifdef HELLFIRE
		if (currLvl._dType == DTYPE_NEST) {
			DRLG_L6PlaceRndPool(L6LPOOL1, 30);
			DRLG_L6PlaceRndPool(L6LPOOL2, 40);
			DRLG_L6PlaceRndPool(L6SPOOL1, 50);
			DRLG_L6PlaceRndPool(L6SPOOL2, 60);
		} else
#endif
		{
			// assert(currLvl._dType == DTYPE_CAVES);
			DRLG_L3Pool();
		}
	} while (_guLavapools < MIN_LAVA_POOL);

	DRLG_L3PlaceRndSet(L3ISLE1, 70);
	DRLG_L3PlaceRndSet(L3ISLE2, 70);
#ifdef HELLFIRE
	if (currLvl._dType == DTYPE_NEST) {
		DRLG_L3PlaceRndSet(L6ISLE3, 30);
		DRLG_L3PlaceRndSet(L6ISLE4, 30);
	} else
#endif
	{
		// assert(currLvl._dType == DTYPE_CAVES);
		DRLG_L3PlaceRndSet(L3ISLE3, 30);
		DRLG_L3PlaceRndSet(L3ISLE4, 30);
	}
	DRLG_L3PlaceRndSet(L3ISLE1, 100);
	DRLG_L3PlaceRndSet(L3ISLE2, 100);
	DRLG_L3PlaceRndSet(L3ISLE5, 90);

	DRLG_InitTrans();
	DRLG_L3FloodTVal();
#ifdef HELLFIRE
	if (currLvl._dType == DTYPE_NEST) {
		/** Miniset: Use random external connection 1. */
		DRLG_PlaceRndTile(8, 25, 20);
		/** Miniset: Use random external connection 2. */
		DRLG_PlaceRndTile(8, 26, 20);
		/** Miniset: Use random external connection 3. */
		DRLG_PlaceRndTile(8, 27, 20);
		/** Miniset: Use random external connection 4. */
		DRLG_PlaceRndTile(8, 28, 20);
		DRLG_L3PlaceRndSet(L6MITE1, 10);
		DRLG_L3PlaceRndSet(L6MITE2, 15);
		DRLG_L3PlaceRndSet(L6MITE3, 20);
		DRLG_L3PlaceRndSet(L6MITE4, 25);
		DRLG_L3PlaceRndSet(L6MITE5, 30);
		DRLG_L3PlaceRndSet(L6MITE6, 35);
		DRLG_L3PlaceRndSet(L6MITE7, 40);
		DRLG_L3PlaceRndSet(L6MITE8, 45);
		DRLG_L3PlaceRndSet(L6MITE9, 50);
		DRLG_L3PlaceRndSet(L6MITE10, 60);
		DRLG_L3PlaceRndSet(L6PATCH1, 50);
		DRLG_L3PlaceRndSet(L6PATCH2, 90);
		/** Miniset: Use random floor tile 1. */
		DRLG_PlaceRndTile(7, 29, 25);
		/** Miniset: Use random floor tile 2. */
		DRLG_PlaceRndTile(7, 30, 25);
		/** Miniset: Use random floor tile 3. */
		DRLG_PlaceRndTile(7, 31, 25);
		/** Miniset: Use random floor tile 4. */
		DRLG_PlaceRndTile(7, 32, 25);
		/** Miniset: Use random vertical wall tile 1. */
		DRLG_PlaceRndTile(9, 33, 25);
		/** Miniset: Use random vertical wall tile 2. */
		DRLG_PlaceRndTile(9, 34, 25);
		/** Miniset: Use random vertical wall tile 3. */
		DRLG_PlaceRndTile(9, 35, 25);
		/** Miniset: Use random vertical wall tile 4. */
		DRLG_PlaceRndTile(9, 36, 25);
		/** Miniset: Use random vertical wall tile 5. */
		DRLG_PlaceRndTile(9, 37, 25);
		/** Miniset: Use random horizontal wall tile 1. */
		DRLG_PlaceRndTile(10, 39, 25);
		/** Miniset: Use random horizontal wall tile 2. but 40 is a north-corner... */
		DRLG_PlaceRndTile(10, 40, 25);
		/** Miniset: Use random horizontal wall tile 3. */
		DRLG_PlaceRndTile(10, 41, 25);
		/** Miniset: Use random horizontal wall tile 4. */
		DRLG_PlaceRndTile(10, 42, 25);
		/** Miniset: Use random horizontal wall tile 5. */
		DRLG_PlaceRndTile(10, 43, 25);
		/** Miniset: Use random vertical wall tile 6. but 45 is a north-corner... */
		DRLG_PlaceRndTile(9, 45, 25);
		/** Miniset: Use random vertical wall tile 7. */
		DRLG_PlaceRndTile(9, 46, 25);
		/** Miniset: Use random horizontal wall tile 6. but 47 is blocked */
		DRLG_PlaceRndTile(10, 47, 25);
		/** Miniset: Use random horizontal wall tile 7. */
		DRLG_PlaceRndTile(10, 48, 25);
		/** Miniset: Use random corner wall tile. north 1. */
		DRLG_PlaceRndTile(11, 38, 25);
		/** Miniset: Use random corner wall tile. north 2. */
		DRLG_PlaceRndTile(11, 44, 25);
		/** Miniset: Use random corner wall tile. north 3. */
		DRLG_PlaceRndTile(11, 49, 25);
		/** Miniset: Use random corner wall tile. north 4. */
		DRLG_PlaceRndTile(11, 50, 25);
	} else
#endif
	{
		// assert(currLvl._dType == DTYPE_CAVES);
		FixL3Warp();
		FixL3HallofHeroes();
		DRLG_L3River();
		if (QuestStatus(Q_ANVIL)) {
			dungeon[setpc_x + 7][setpc_y + 5] = 7;
			dungeon[setpc_x + 8][setpc_y + 5] = 7;
			dungeon[setpc_x + 9][setpc_y + 5] = 7;
			if (dungeon[setpc_x + 10][setpc_y + 5] == 17 || dungeon[setpc_x + 10][setpc_y + 5] == 18) {
				dungeon[setpc_x + 10][setpc_y + 5] = 45;
			}
			dTransVal[DBORDERX + (setpc_x + 7) * 2][DBORDERY + (setpc_y + 5) * 2] = 1;
			dTransVal[DBORDERX + (setpc_x + 8) * 2][DBORDERY + (setpc_y + 5) * 2] = 1;
			dTransVal[DBORDERX + (setpc_x + 9) * 2][DBORDERY + (setpc_y + 5) * 2] = 1;
		}
		DRLG_PlaceThemeRooms(5, 10, 7, 0, false);

		DRLG_L3Wood();
		DRLG_L3PlaceRndSet(L3TITE1, 10);
		DRLG_L3PlaceRndSet(L3TITE2, 10);
		DRLG_L3PlaceRndSet(L3TITE3, 10);
		DRLG_L3PlaceRndSet(L3TITE6, 20);
		DRLG_L3PlaceRndSet(L3TITE7, 20);
		DRLG_L3PlaceRndSet(L3TITE8, 20);
		DRLG_L3PlaceRndSet(L3TITE9, 20);
		DRLG_L3PlaceRndSet(L3TITE10, 20);
		DRLG_L3PlaceRndSet(L3TITE11, 30);
		DRLG_L3PlaceRndSet(L3TITE12, 20);
		DRLG_L3PlaceRndSet(L3TITE13, 20);
		DRLG_L3PlaceRndSet(L3CREV1, 30);
		DRLG_L3PlaceRndSet(L3CREV2, 30);
		DRLG_L3PlaceRndSet(L3CREV3, 30);
		DRLG_L3PlaceRndSet(L3CREV4, 30);
		DRLG_L3PlaceRndSet(L3CREV5, 30);
		DRLG_L3PlaceRndSet(L3CREV6, 30);
		DRLG_L3PlaceRndSet(L3CREV7, 30);
		DRLG_L3PlaceRndSet(L3CREV8, 30);
		DRLG_L3PlaceRndSet(L3CREV9, 30);
		DRLG_L3PlaceRndSet(L3CREV10, 30);
		DRLG_L3PlaceRndSet(L3CREV11, 30);
		/** Miniset: Use random floor tile 1. */
		DRLG_PlaceRndTile(7, 106, 25);
		/** Miniset: Use random floor tile 2. */
		DRLG_PlaceRndTile(7, 107, 25);
		/** Miniset: Use random floor tile 3. */
		DRLG_PlaceRndTile(7, 108, 25);
		/** Miniset: Use random horizontal wall tile. */
		DRLG_PlaceRndTile(9, 109, 25);
		/** Miniset: Use random vertical wall tile. */
		DRLG_PlaceRndTile(10, 110, 25);
	}

	memcpy(pdungeon, dungeon, sizeof(pdungeon));

	DRLG_Init_Globals();
}

static void DRLG_L3LightTiles()
{
	int i, j, pn;

#ifdef HELLFIRE
	if (currLvl._dType == DTYPE_NEST) {
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++) {
				if (dPiece[i][j] >= 382 && dPiece[i][j] <= 457) {
					DoLighting(i, j, 9, -1);
				}
			}
		}
	} else
#endif
	{
		// assert(currLvl._dType == DTYPE_CAVES);
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++) {
				pn = dPiece[i][j];
				if (pn >= 56 && pn <= 161
				 && (pn <= 147 || pn >= 154 || pn == 150 || pn == 152)) {
					DoLighting(i, j, 7, -1);
				}
			}
		}
	}
}

void CreateL3Dungeon(int entry)
{
	DRLG_InitSetPC();
	DRLG_LoadL3SP();
	DRLG_L3(entry);
	DRLG_FreeL3SP();
	DRLG_PlaceMegaTiles(BASE_MEGATILE_L3);
	DRLG_L3LightTiles();
	DRLG_SetPC();
}

static BYTE *LoadL3DungeonData(const char *sFileName)
{
	int i, j, rw, rh;
	BYTE *pLevelMap, *lm, *pTmp;

	InitL3Dungeon();

	pLevelMap = LoadFileInMem(sFileName);

	lm = pLevelMap;
	rw = *lm;
	lm += 2;
	rh = *lm;
	lm += 2;

	for (j = 0; j < rh; j++) {
		for (i = 0; i < rw; i++) {
			if (*lm != 0) {
				dungeon[i][j] = *lm;
			} else {
				dungeon[i][j] = 7;
			}
			lm += 2;
		}
	}
	static_assert(sizeof(dungeon) == DMAXX * DMAXY, "Linear traverse of dungeon does not work in LoadL3DungeonData.");
	pTmp = &dungeon[0][0];
	for (i = 0; i < DMAXX * DMAXY; i++, pTmp++)
		if (*pTmp == 0)
			*pTmp = 8;
	return pLevelMap;
}

void LoadL3Dungeon(const char *sFileName, int vx, int vy)
{
	BYTE *pLevelMap;

	ViewX = vx;
	ViewY = vy;

	pLevelMap = LoadL3DungeonData(sFileName);

	DRLG_InitTrans();
	DRLG_L3FloodTVal();
	DRLG_Init_Globals();
	DRLG_PlaceMegaTiles(BASE_MEGATILE_L3);

	SetMapMonsters(pLevelMap, 0, 0);
	SetMapObjects(pLevelMap);

	DRLG_L3LightTiles();

	mem_free_dbg(pLevelMap);
}

void LoadPreL3Dungeon(const char *sFileName)
{
	BYTE *pLevelMap = LoadL3DungeonData(sFileName);

	memcpy(pdungeon, dungeon, sizeof(pdungeon));
	mem_free_dbg(pLevelMap);
}

DEVILUTION_END_NAMESPACE
