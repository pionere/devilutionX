/**
 * @file drlg_l1.cpp
 *
 * Implementation of the cathedral level generation algorithms.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** Starting position of the megatiles. */
#define BASE_MEGATILE_L1 (22 - 1)

/** Specifies whether to generate a horizontal room at position 1 in the Cathedral. */
BOOL HR1;
/** Specifies whether to generate a horizontal room at position 2 in the Cathedral. */
BOOL HR2;
/** Specifies whether to generate a horizontal room at position 3 in the Cathedral. */
BOOL HR3;
/** Specifies whether to generate a vertical room at position 1 in the Cathedral. */
BOOL VR1;
/** Specifies whether to generate a vertical room at position 2 in the Cathedral. */
BOOL VR2;
/** Specifies whether to generate a vertical room at position 3 in the Cathedral. */
BOOL VR3;

/** Contains shadows for 2x2 blocks of base tile IDs in the Cathedral. */
const ShadowStruct L1SPATS[] = {
	// clang-format off
	// strig, s1, s2, s3, nv1, nv2, nv3
	{      7, 13,  0, 13, 144,   0, 142 },
	{     16, 13,  0, 13, 144,   0, 142 },
	{     15, 13,  0, 13, 145,   0, 142 },
	{      5, 13, 13, 13, 152, 140, 139 },
	{      5, 13,  1, 13, 143, 146, 139 },
	{      5, 13, 13,  2, 143, 140, 148 },
	{      5,  0,  1,  2,   0, 146, 148 },
	{      5, 13, 11, 13, 143, 147, 139 },
	{      5, 13, 13, 12, 143, 140, 149 },
	{      5, 13, 11, 12, 150, 147, 149 },
	{      5, 13,  1, 12, 143, 146, 149 },
	{      5, 13, 11,  2, 143, 147, 148 },
	{      9, 13, 13, 13, 144, 140, 142 },
	{      9, 13,  1, 13, 144, 146, 142 },
	{      9, 13, 11, 13, 151, 147, 142 },
	{      8, 13,  0, 13, 144,   0, 139 },
	{      8, 13,  0, 12, 143,   0, 149 },
	{      8,  0,  0,  2,   0,   0, 148 },
	{     11,  0,  0, 13,   0,   0, 139 },
	{     11, 13,  0, 13, 139,   0, 139 },
	{     11,  2,  0, 13, 148,   0, 139 },
	{     11, 12,  0, 13, 149,   0, 139 },
	{     11, 13, 11, 12, 139,   0, 149 },
	{     14,  0,  0, 13,   0,   0, 139 },
	{     14, 13,  0, 13, 139,   0, 139 },
	{     14,  2,  0, 13, 148,   0, 139 },
	{     14, 12,  0, 13, 149,   0, 139 },
	{     14, 13, 11, 12, 139,   0, 149 },
	{     10,  0, 13,  0,   0, 140,   0 },
	{     10, 13, 13,  0, 140, 140,   0 },
	{     10,  0,  1,  0,   0, 146,   0 },
	{     10, 13, 11,  0, 140, 147,   0 },
	{     12,  0, 13,  0,   0, 140,   0 },
	{     12, 13, 13,  0, 140, 140,   0 },
	{     12,  0,  1,  0,   0, 146,   0 },
	{     12, 13, 11,  0, 140, 147,   0 },
	{      3, 13, 11, 12, 150,   0,   0 },
	{      0,  0,  0,  0,   0,   0,   0 }
	// clang-format on
};

// BUGFIX: This array should contain an additional 0 (207 elements).
/** Maps tile IDs to their corresponding base tile ID. */
const BYTE BSTYPES[] = {
	// clang-format off
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 0, 0,
	0, 0, 0, 0, 0, 1, 2, 10, 4, 5,
	6, 7, 8, 9, 10, 11, 12, 14, 5, 14,
	10, 4, 14, 4, 5, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	2, 3, 4, 1, 6, 7, 16, 17, 2, 1,
	1, 2, 2, 1, 1, 2, 2, 2, 2, 2,
	1, 1, 11, 1, 13, 13, 13, 1, 2, 1,
	2, 1, 2, 1, 2, 2, 2, 2, 12, 0,
	0, 11, 1, 11, 1, 13, 0, 0, 0, 0,
	0, 0, 0, 13, 13, 13, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13, 1, 11, 2, 12,
	13, 13, 13, 12, 2, 1, 2, 2, 4, 14,
	4, 10, 13, 13, 4, 4, 1, 1, 4, 2,
	2, 13, 13, 13, 13, 25, 26, 28, 30, 31,
	41, 43, 40, 41, 42, 43, 25, 41, 43, 28,
	28, 1, 2, 25, 26, 22, 22, 25, 26, 0,
	0, 0, 0, 0, 0, 0, 0
	// clang-format on
};

// BUGFIX: This array should contain an additional 0 (207 elements) (fixed).
/** Maps tile IDs to their corresponding undecorated tile ID. */
const BYTE L1BTYPES[] = {
	// clang-format off
	0, 1, 2, 0/*3*/, 0/*4*/, 0/*5*/, 0/*6*/, 0/*7*/, 0/*8*/, 0/*9*/,
	0/*10*/, 11, 12, 13, 0/*14*/, 0/*15*/, 0/*16*/, 0/*17*/, 0, 0,
	0, 0, 0, 0, 0, 25, 0/*26*/, 0, 0/*28*/, 0,
	0/*30*/, 0/*31*/, 0, 0, 0, 0, 0, 0, 0, 0,
	0/*40*/, 0/*41*/, 0/*42*/, 0/*43*/, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 79,
	80, 0, 0/*82*/, 0, 0, 0, 0, 0, 0, 79,
	0, 80, 0, 0, 79, 80, 0, 2, 2, 2,
	1, 1, 11, 25, 13, 13, 13, 1, 2, 1,
	2, 1, 2, 1, 2, 2, 2, 2, 12, 0,
	0, 11, 1, 11, 1, 13, 0, 0, 0, 0,
	0, 0, 0, 13, 13, 13, 13, 13, 13, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0
	// clang-format on
};
/** Miniset: stairs up on a corner wall. */
//const BYTE STAIRSUP[] = {
//	// clang-format off
//	4, 4, // width, height
//
//	13, 13, 13, 13, // search
//	 2,  2,  2,  2,
//	13, 13, 13, 13,
//	13, 13, 13, 13,
//
//	 0, 66,  6,  0, // replace
//	63, 64, 65,  0,
//	 0, 67, 68,  0,
//	 0,  0,  0,  0,
/*    0,  0,   137,138,    13,  2,     0,  0,	// MegaTiles
	  0,  0,   139,140,     3,  4,     0,  0,

	  5,127,   129,130,   133,134,     0,  0,
	  7,128,   131,132,   135,136,     0,  0,

	  0,  0,   141,142,   143,  2,     0,  0,
	  0,  0,     7,  4,     7,  4,     0,  0,

	  0,  0,     0,  0,     0,  0,     0,  0,
	  0,  0,     0,  0,     0,  0,     0,  0, */
/*    0,  0,   192,193,    21, 22,     0,  0,	// MegaTiles - HELLFIRE
	  0,  0,   194,195,    23, 24,     0,  0,

	180,181,   184,185,   188,189,     0,  0,
	182,183,   186,187,   190,191,     0,  0,

	  0,  0,   196,197,   200,201,     0,  0,
	  0,  0,   198,199,   202,203,     0,  0,

	  0,  0,     0,  0,     0,  0,     0,  0,
	  0,  0,     0,  0,     0,  0,     0,  0, */
	// clang-format on
//};
#ifdef HELLFIRE
const BYTE L5USTAIRS[] = {
	// clang-format off
	4, 5, // width, height

	22, 22, 22, 22, // search
	22, 22, 22, 22,
	 2,  2,  2,  2,
	13, 13, 13, 13,
	13, 13, 13, 13,

	 0, 54, 23,  0, // replace
	 0, 53, 18,  0,
	55, 56, 57,  0,
	58, 59, 60,  0,
	 0,  0,  0,  0
/*    0,  0   151,152,    73, 64,     0,  0,	// MegaTiles
	  0,  0   153,130,    65, 66,     0,  0,

	  0,  0   149,130,    63, 64,     0,  0,
	  0,  0   150,130,    65, 66,     0,  0,

	154,155,  158,159,   162,163,     0,  0,
	156,157,  160,161,   164,165,     0,  0,

	166,167,  168,169,   170,171,     0,  0,
	 47, 48,   47, 48,    47, 48,     0,  0,

	  0,  0,    0,  0,     0,  0,     0,  0,
	  0,  0,    0,  0,     0,  0,     0,  0, */
	// clang-format on
};
#endif
/** Miniset: stairs up. */
const BYTE L1USTAIRS[] = {
	// clang-format off
	4, 4, // width, height

	22, 22, 22, 22, // search
	 2,  2,  2,  2,
	13, 13, 13, 13,
	13, 13, 13, 13,

	 0, 66, 23,  0, // replace
	63, 64, 65,  0,
	 0, 67, 68,  0,
	 0,  0,  0,  0,
/*    0,  0,   137,138,    41, 31,     0,  0,	// MegaTiles
	  0,  0,   139,140,    39, 33,     0,  0,

	  5,127,   129,130,   133,134,     0,  0,
	  7,128,   131,132,   135,136,     0,  0,

	  0,  0,   141,142,   143,  2,     0,  0,
	  0,  0,     7,  4,     7,  4,     0,  0,

	  0,  0,     0,  0,     0,  0,     0,  0,
	  0,  0,     0,  0,     0,  0,     0,  0, */
	// clang-format on
};
/*
 * Miniset: stairs down.
 * Added an extra line to the top to prevent placing it too close to other entities (eg. upstair)
 */
const BYTE L1DSTAIRS[] = {
	// clang-format off
	4, 4, // width, height

	13, 13, 13, 13, // search
	13, 13, 13, 13,
	13, 13, 13, 13,
	13, 13, 13, 13,

	 0,  0,  0,  0,
	62, 57, 58,  0, // replace
	61, 59, 60,  0,
	 0,  0,  0,  0,
/*	  0,  0,     0,  0,     0,  0,     0,  0,
	  0,  0,     0,  0,     0,  0,     0,  0, 

	124,  2,   106,107,   110,111,     0,  0,	// MegaTiles
	125,126,   108,109,   112,113,     0,  0,

	122,123,   114,115,   118,119,     0,  0,
	  7,  4,   116,117,   120,121,     0,  0,

	  0,  0,     0,  0,     0,  0,     0,  0,
	  0,  0,     0,  0,     0,  0,     0,  0, */
	// clang-format on
};
#ifdef HELLFIRE
const BYTE L5DSTAIRS[] = {
	// clang-format off
	4, 5, // width, height

	13, 13, 13, 13, // search
	13, 13, 13, 13,
	13, 13, 13, 13,
	13, 13, 13, 13,
	13, 13, 13, 13,

	 0,  0, 52,  0, // replace
	 0, 48, 51,  0,
	 0, 47, 50,  0,
	45, 46, 49,  0,
	 0,  0,  0,  0,
	// clang-format on
};
const BYTE L5TWARP[] = {
	// clang-format off
	4, 5, // width, height

	22, 22, 22, 22, // search
	22, 22, 22, 22,
	 2,  2,  2,  2,
	13, 13, 13, 13,
	13, 13, 13, 13,

	 0, 62, 23,  0, // replace
	 0, 61, 18,  0,
	63, 64, 65,  0,
	66, 67, 68,  0,
	 0,  0,  0,  0,
	// clang-format on
};
#endif
/** Miniset: candlestick. */
const BYTE LAMPS[] = {
	// clang-format off
	2, 2, // width, height

	13,  0, // search
	13, 13,

	129,   0, // replace
	130, 128,
	// clang-format on
};
/** Miniset: Poisoned Water Supply entrance. */
const BYTE PWATERIN[] = {
	// clang-format off
	6, 6, // width, height

	13, 13, 13, 13, 13, 13, // search
	13, 13, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13,

	 0,   0,   0,   0,   0, 0, // replace
	 0, 202, 200, 200,  84, 0,
	 0, 199, 203, 203,  83, 0,
	 0,  85, 206,  80,  81, 0,
	 0,   0, 134, 135,   0, 0,
	 0,   0,   0,   0,   0, 0,
	// clang-format on
};
#ifdef HELLFIRE
/** Miniset: Column on the northern side of a vertical wall 1. */
const BYTE L5VERTCOL1[4] = { 1, 1, 11, 95 };
/** Miniset: Column on the northern side of a horizontal wall 1. */
const BYTE L5HORIZCOL1[4] = { 1, 1, 12, 96 };
/** Miniset: Prepare random large vertical wall tile */
const BYTE L5PREVERTWALL[8] = {
	// clang-format off
	1, 3, // width, height

	1, // search
	1,
	1,

	91, // replace
	90,
	89,
	// clang-format on
};
/** Miniset: Prepare random large horizontal wall tile */
const BYTE L5PREHORIZWALL[8] = {
	// clang-format off
	3, 1, // width, height

	 2,  2,  2, // search

	94, 93, 92, // replace
	// clang-format on
};
/** Miniset: Use random floor tile 1. */
const BYTE L5RNDFLOOR1[4] = { 1, 1, 13, 97 };
/** Miniset: Use random floor tile 2. */
const BYTE L5RNDFLOOR2[4] = { 1, 1, 13, 98 };
/** Miniset: Use random floor tile 3. */
const BYTE L5RNDFLOOR3[4] = { 1, 1, 13, 99 };
/** Miniset: Use random floor tile 4. */
const BYTE L5RNDFLOOR4[4] = { 1, 1, 13, 100 };
/** Miniset: Use random large floor tile. */
const BYTE L5RNDLFLOOR1[20] = {
	// clang-format off
	3, 3, // width, height

	13, 13, 13, // search
	13, 13, 13,
	13, 13, 13,

	0,   0, 0, // replace
	0, 101, 0,
	0,   0, 0,
	// clang-format on
};
/** Miniset: Replace random floor tile 2 with another one 1. */
const BYTE L5RNDFLOOR2_0[4] = { 1, 1, 98, 189 };
/** Miniset: Replace random floor tile 2 with another one 2. */
const BYTE L5RNDFLOOR2_1[4] = { 1, 1, 98, 190 };
/** Miniset: Replace random floor tile 1 with another one. */
const BYTE L5RNDFLOOR1_0[4] = { 1, 1, 97, 191 };
/** Miniset: Replace random floor tile 3 with another one 1. */
const BYTE L5RNDFLOOR3_0[4] = { 1, 1, 99, 193 };
/** Miniset: Replace random floor tile 3 with another one 2. */
const BYTE L5RNDFLOOR3_1[4] = { 1, 1, 99, 194 };
/** Miniset: Replace random floor tile 4 with another one. */
const BYTE L5RNDFLOOR4_0[4] = { 1, 1, 100, 195 };
/** Miniset: Lava fountain bowl. */
const BYTE L5RNDLFLOOR2[20] = {
	// clang-format off
	3, 3, // width, height

	13, 13, 13, // search
	13, 13, 13,
	13, 13, 13,

	0,   0, 0, // replace
	0, 167, 0,
	0,   0, 0,
	// clang-format on
};
/** Miniset: Mooring on a stone. */
const BYTE L5RNDLFLOOR3[20] = {
	// clang-format off
	3, 3, // width, height

	13, 13, 13, // search
	13, 13, 13,
	13, 13, 13,

	0,   0, 0, // replace
	0, 168, 0,
	0,   0, 0,
	// clang-format on
};
/** Miniset: A stone coffin(?). */
const BYTE L5RNDLFLOOR4[20] = {
	// clang-format off
	3, 3, // width, height

	13, 13, 13, // search
	13, 13, 13,
	13, 13, 13,

	0,   0, 0, // replace
	0, 169, 0,
	0,   0, 0,
};
/** Miniset: A stone. TODO: bad collision check on the northern side */
const BYTE L5RNDLFLOOR5[20] = {
	// clang-format off
	3, 3, // width, height

	13, 13, 13, // search
	13, 13, 13,
	13, 13, 13,

	0,   0, 0, // replace
	0, 170, 0,
	0,   0, 0,
	// clang-format on
};
/** Miniset: A broken head of a stone-column. */
const BYTE L5RNDLFLOOR6[20] = {
	// clang-format off
	3, 3, // width, height

	13, 13, 13, // search
	13, 13, 13,
	13, 13, 13,

	0,   0, 0, // replace
	0, 171, 0,
	0,   0, 0,
	// clang-format on
};
/** Miniset: A leaf on the floor. */
const BYTE L5RNDLFLOOR7[20] = {
	// clang-format off
	3, 3, // width, height

	13, 13, 13, // search
	13, 13, 13,
	13, 13, 13,

	 0,   0, 0, // replace
	 0, 172, 0,
	 0,   0, 0,
	// clang-format on
};
#endif

/**
 * A lookup table for the 16 possible patterns of a 2x2 area,
 * where each cell either contains a SW wall or it doesn't.
 */
const BYTE L1ConvTbl[16] = { 22, 13, 1, 13, 2, 13, 13, 13, 4, 13, 1, 13, 2, 13, 16, 13 };

#ifdef HELLFIRE
static void DRLG_InitL5Vals()
{
	int i, *dp;
	BYTE pc, *dsp;

	static_assert(sizeof(dPiece) == MAXDUNX * MAXDUNY * sizeof(int), "Linear traverse of dPiece does not work in DRLG_InitL5Vals.");
	static_assert(sizeof(dSpecial) == MAXDUNX * MAXDUNY, "Linear traverse of dSpecial does not work in DRLG_InitL5Vals.");
	dsp = &dSpecial[0][0];
	dp = &dPiece[0][0];
	for (i = 0; i < MAXDUNX * MAXDUNY; i++, dsp++, dp++) {
		if (*dp == 77)
			pc = 1;
		else if (*dp == 80)
			pc = 2;
		else
			continue;
		*dsp = pc;
	}
}
#endif

static void DRLG_L1PlaceDoor(int x, int y)
{
	BYTE df = dflags[x][y];

	if (!(df & DLRG_PROTECTED)) {
		BYTE c = dungeon[x][y];

		if (df == DLRG_HDOOR) {
			if (y != 1 && c == 2)
				dungeon[x][y] = 26;
			if (y != 1 && c == 7)
				dungeon[x][y] = 31;
			if (y != 1 && c == 14)
				dungeon[x][y] = 42;
			if (y != 1 && c == 4)
				dungeon[x][y] = 43;
			if (x != 1 && c == 1)
				dungeon[x][y] = 25;
			if (x != 1 && c == 10)
				dungeon[x][y] = 40;
			if (x != 1 && c == 6)
				dungeon[x][y] = 30;
		} else if (df == DLRG_VDOOR) {
			if (x != 1 && c == 1)
				dungeon[x][y] = 25;
			if (x != 1 && c == 6)
				dungeon[x][y] = 30;
			if (x != 1 && c == 10)
				dungeon[x][y] = 40;
			if (x != 1 && c == 4)
				dungeon[x][y] = 41;
			if (y != 1 && c == 2)
				dungeon[x][y] = 26;
			if (y != 1 && c == 14)
				dungeon[x][y] = 42;
			if (y != 1 && c == 7)
				dungeon[x][y] = 31;
		} else if (df == (DLRG_HDOOR | DLRG_VDOOR)) {
			if (x != 1 && y != 1 && c == 4)
				dungeon[x][y] = 28;
			if (x != 1 && c == 10)
				dungeon[x][y] = 40;
			if (y != 1 && c == 14)
				dungeon[x][y] = 42;
			if (y != 1 && c == 2)
				dungeon[x][y] = 26;
			if (x != 1 && c == 1)
				dungeon[x][y] = 25;
			if (y != 1 && c == 7)
				dungeon[x][y] = 31;
			if (x != 1 && c == 6)
				dungeon[x][y] = 30;
		}
	}

	dflags[x][y] = DLRG_PROTECTED;
}

#ifdef HELLFIRE
static void DRLG_L5Crypt_lavafloor()
{
	int i, j;

	for (j = 1; j < DMAXY; j++) {
		for (i = 1; i < DMAXX; i++) {
			switch (dungeon[i][j]) {
			case 5:
			case 116:
			case 133:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 203;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 204;
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 205;
				break;
			case 7:
			case 15:
			case 17:
			case 118:
			case 126:
			case 128:
			case 135:
			case 152:
			case 160:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 206;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 207;
				break;
			case 8:
			case 11:
			case 14:
			case 95:
			case 119:
			case 125:
			case 136:
			case 142:
			case 153:
			case 156:
			case 159:
			case 185:
			case 186:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 203;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 204;
				break;
			case 9:
			case 120:
			case 154:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 206;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 207;
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 205;
				break;
			case 10:
			case 12:
			case 121:
			case 123:
			case 138:
			case 155:
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 205;
				break;
			case 96:
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 208;
				break;
			case 122:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 211;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 212;
				break;
			case 137:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 213;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 214;
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 205;
				break;
			case 139:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 215;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 216;
				break;
			case 140:
			case 157:
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 217;
				break;
			case 143:
			case 145:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 213;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 214;
				break;
			case 150:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 203;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 204;
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 217;
				break;
			case 162:
			case 167:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 209;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 210;
				break;
			}
		}
	}
}
#endif

static void DRLG_L1Shadows()
{
	const ShadowStruct *ss;
	int x, y;
	BYTE sd00, sd10, sd01, sd11;
	BYTE tnv3;

	for (y = 1; y < DMAXY; y++) {
		for (x = 1; x < DMAXX; x++) {
			sd00 = BSTYPES[dungeon[x][y]];
			sd10 = BSTYPES[dungeon[x - 1][y]];
			sd01 = BSTYPES[dungeon[x][y - 1]];
			sd11 = BSTYPES[dungeon[x - 1][y - 1]];
			for (ss = L1SPATS; ss->strig != 0; ss++) {
				if (ss->strig == sd00) {
					if ((ss->s1 != 0 && ss->s1 != sd11)
					 || (ss->s2 != 0 && ss->s2 != sd01)
					 || (ss->s3 != 0 && ss->s3 != sd10))
						continue;
					if (ss->nv1 != 0 && dflags[x - 1][y - 1] == 0)
						dungeon[x - 1][y - 1] = ss->nv1;
					if (ss->nv2 != 0 && dflags[x][y - 1] == 0)
						dungeon[x][y - 1] = ss->nv2;
					if (ss->nv3 != 0 && dflags[x - 1][y] == 0)
						dungeon[x - 1][y] = ss->nv3;
				}
			}
		}
	}

	for (y = 1; y < DMAXY; y++) {
		for (x = 1; x < DMAXX; x++) {
			if (dungeon[x - 1][y] == 139 && dflags[x - 1][y] == 0) {
				tnv3 = dungeon[x][y];
				if (tnv3 == 29 || tnv3 == 32 || tnv3 == 35 || tnv3 == 37 || tnv3 == 38 || tnv3 == 39)
					tnv3 = 141;
				else
					tnv3 = 139;
				dungeon[x - 1][y] = tnv3;
			}
			if (dungeon[x - 1][y] == 149 && dflags[x - 1][y] == 0) {
				tnv3 = dungeon[x][y];
				if (tnv3 == 29 || tnv3 == 32 || tnv3 == 35 || tnv3 == 37 || tnv3 == 38 || tnv3 == 39)
					tnv3 = 153;
				else
					tnv3 = 149;
				dungeon[x - 1][y] = tnv3;
			}
			if (dungeon[x - 1][y] == 148 && dflags[x - 1][y] == 0) {
				tnv3 = dungeon[x][y];
				if (tnv3 == 29 || tnv3 == 32 || tnv3 == 35 || tnv3 == 37 || tnv3 == 38 || tnv3 == 39)
					tnv3 = 154;
				else
					tnv3 = 148;
				dungeon[x - 1][y] = tnv3;
			}
		}
	}
}

static bool DRLG_L1PlaceMiniSet(const BYTE *miniset, bool setview)
{
	POS32 result;

	result = DRLG_PlaceMiniSet(miniset);
	if (result.x == DMAXX)
		return false;

	if (setview) {
		ViewX = 2 * result.x + DBORDERX + 3;
		ViewY = 2 * result.y + DBORDERY + 4;
	}
	return true;
}

static void DRLG_L1Floor()
{
	DRLG_PlaceRndTile(13, 162, 33);
	DRLG_PlaceRndTile(13, 163, 50);
}

static void DRLG_LoadL1SP()
{
	assert(pSetPiece == NULL);
	if (QuestStatus(Q_LTBANNER)) {
		pSetPiece = LoadFileInMem("Levels\\L1Data\\Banner2.DUN");
	} else if (QuestStatus(Q_SKELKING)) {
		pSetPiece = LoadFileInMem("Levels\\L1Data\\SKngDO.DUN");
	} else if (QuestStatus(Q_BUTCHER)) {
		pSetPiece = LoadFileInMem("Levels\\L1Data\\Butcher.DUN");
#ifdef HELLFIRE
	} else if (QuestStatus(Q_NAKRUL)) {
		pSetPiece = LoadFileInMem("NLevels\\L5Data\\Nakrul1.DUN");
#endif
	}
}

static void DRLG_FreeL1SP()
{
	MemFreeDbg(pSetPiece);
}

static void DRLG_InitL1Vals()
{
	int i, *dp;
	BYTE pc, *dsp;
	static_assert(sizeof(dPiece) == MAXDUNX * MAXDUNY * sizeof(int), "Linear traverse of dPiece does not work in DRLG_InitL1Vals.");
	static_assert(sizeof(dSpecial) == MAXDUNX * MAXDUNY, "Linear traverse of dSpecial does not work in DRLG_InitL1Vals.");
	dsp = &dSpecial[0][0];
	dp = &dPiece[0][0];
	for (i = 0; i < MAXDUNX * MAXDUNY; i++, dsp++, dp++) {
		if (*dp == 12)
			pc = 1;
		else if (*dp == 11)
			pc = 2;
		else if (*dp == 71)
			pc = 1;
		else if (*dp == 253)
			pc = 3;
		else if (*dp == 267)
			pc = 6;
		else if (*dp == 259)
			pc = 5;
		else if (*dp == 249)
			pc = 2;
		else if (*dp == 325)
			pc = 2;
		else if (*dp == 321)
			pc = 1;
		else if (*dp == 255)
			pc = 4;
		else if (*dp == 211)
			pc = 1;
		else if (*dp == 344)
			pc = 2;
		else if (*dp == 341)
			pc = 1;
		else if (*dp == 331)
			pc = 2;
		else if (*dp == 418)
			pc = 1;
		else if (*dp == 421)
			pc = 2;
		else
			continue;
		*dsp = pc;
	}
}

static BYTE *LoadL1DungeonData(const char *sFileName)
{
	int i, j;
	BYTE *pMap;
	uint16_t rw, rh, *lm;

	//DRLG_InitTrans();
	pMap = LoadFileInMem(sFileName);

	memset(dflags, 0, sizeof(dflags));
	static_assert(sizeof(dungeon) == DMAXX * DMAXY, "Linear traverse of dungeon does not work in LoadL1DungeonData.");
	memset(dungeon, 22, sizeof(dungeon));

	lm = (uint16_t *)pMap;
	rw = SwapLE16(*lm);
	lm++;
	rh = SwapLE16(*lm);
	lm++;

	for (j = 0; j < rh; j++) {
		for (i = 0; i < rw; i++) {
			if (*lm != 0) {
				dungeon[i][j] = SwapLE16(*lm);
				dflags[i][j] |= DLRG_PROTECTED;
			} else {
				dungeon[i][j] = 13;
			}
			lm++;
		}
	}

	DRLG_L1Floor();
	return pMap;
}

void LoadL1Dungeon(const char *sFileName, int vx, int vy)
{
	BYTE *pMap;

	ViewX = vx;
	ViewY = vy;

	pMap = LoadL1DungeonData(sFileName);

	DRLG_PlaceMegaTiles(BASE_MEGATILE_L1);

	DRLG_InitTrans();
	DRLG_Init_Globals();
	// assert(currLvl._dType == DTYPE_CATHEDRAL);
	DRLG_InitL1Vals();

	SetMapMonsters(pMap, 0, 0);
	SetMapObjects(pMap);
	mem_free_dbg(pMap);
}

void LoadPreL1Dungeon(const char *sFileName)
{
	BYTE *pMap = LoadL1DungeonData(sFileName);

	memcpy(pdungeon, dungeon, sizeof(pdungeon));

	mem_free_dbg(pMap);
}

static void InitL1Dungeon()
{
	memset(dungeon, 0, sizeof(dungeon));
	memset(dflags, 0, sizeof(dflags));
}

static void L1ClearFlags()
{
	int i;
	BYTE *pTmp;

	static_assert(sizeof(dflags) == DMAXX * DMAXY, "Linear traverse of dflags does not work in L1ClearFlags.");
	pTmp = &dflags[0][0];
	for (i = 0; i < DMAXX * DMAXY; i++, pTmp++)
		*pTmp &= ~DLRG_CHAMBER;
}

static void L1drawRoom(int x, int y, int width, int height)
{
	int i, j, x2, y2;

	x2 = x + width;
	y2 = y + height;
	for (j = y; j < y2; j++) {
		for (i = x; i < x2; i++) {
			dungeon[i][j] = 1;
		}
	}
}

static bool L1checkRoom(int x, int y, int width, int height)
{
	int i, j, x2, y2;

	if (x < 0 || y < 0)
		return false;

	x2 = x + width;
	y2 = y + height;
	if (x2 > DMAXX || y2 > DMAXY)
		return false;

	for (j = y; j < y2; j++) {
		for (i = x; i < x2; i++) {
			if (dungeon[i][j] != 0)
				return false;
		}
	}

	return true;
}

static void L1roomGen(int x, int y, int w, int h, int dir)
{
	int dirProb, i, width, height, rx, ry, rxy2;
	bool ran2;

	dirProb = random_(0, 4);

	if (dir == 1 ? dirProb == 0 : dirProb != 0) {
		for (i = 20; i != 0; i--) {
			width = RandRange(2, 6) & ~1;
			height = RandRange(2, 6) & ~1;
			ry = h / 2 + y - height / 2;
			rx = x - width;
			if (L1checkRoom(rx - 1, ry - 1, width + 1, height + 2))  /// BUGFIX: swap args 3 and 4 ("height+2" and "width+1") (fixed)
				break;
		}

		if (i != 0)
			L1drawRoom(rx, ry, width, height);
		rxy2 = x + w;
		ran2 = L1checkRoom(rxy2, ry - 1, width + 1, height + 2);
		if (ran2)
			L1drawRoom(rxy2, ry, width, height);
		if (i != 0)
			L1roomGen(rx, ry, width, height, 1);
		if (ran2)
			L1roomGen(rxy2, ry, width, height, 1);
	} else {
		for (i = 20; i != 0; i--) {
			width = RandRange(2, 6) & ~1;
			height = RandRange(2, 6) & ~1;
			rx = w / 2 + x - width / 2;
			ry = y - height;
			if (L1checkRoom(rx - 1, ry - 1, width + 2, height + 1))
				break;
		}

		if (i != 0)
			L1drawRoom(rx, ry, width, height);
		rxy2 = y + h;
		ran2 = L1checkRoom(rx - 1, rxy2, width + 2, height + 1);
		if (ran2)
			L1drawRoom(rx, rxy2, width, height);
		if (i != 0)
			L1roomGen(rx, ry, width, height, 0);
		if (ran2)
			L1roomGen(rx, rxy2, width, height, 0);
	}
}

static void L1firstRoom()
{
	int ys, ye, y;
	int xs, xe, x;

	if (random_(0, 2) == 0) {
		// place the main rooms vertically
		ys = 11; // was 1
		ye = 29; // was DMAXY - 1;

		VR1 = random_(0, 2);
		VR2 = random_(0, 2);
		VR3 = random_(0, 2);
		// make sure there are at least two set
		if (!(VR1 & VR3))
			VR2 = TRUE;
		// draw the selected rooms
		if (VR1)
			L1drawRoom(15, 1, 10, 10);
		else
			ys = 25; // was 18;

		if (VR2)
			L1drawRoom(15, 15, 10, 10);
		if (VR3)
			L1drawRoom(15, 29, 10, 10);
		else
			ye = 15; // was 22;
		// draw a hallway between the rooms
		for (y = ys; y < ye; y++) {
			dungeon[17][y] = 1;
			dungeon[18][y] = 1;
			dungeon[19][y] = 1;
			dungeon[20][y] = 1;
			dungeon[21][y] = 1;
			dungeon[22][y] = 1;
		}
		// spread additional rooms starting from the main rooms
		if (VR1)
			L1roomGen(15, 1, 10, 10, 0);
		if (VR2)
			L1roomGen(15, 15, 10, 10, 0);
		if (VR3)
			L1roomGen(15, 29, 10, 10, 0);

		HR3 = FALSE;
		HR2 = FALSE;
		HR1 = FALSE;
	} else {
		// place the main rooms horizontally
		xs = 11; // was 1
		xe = 29; // was DMAXX - 1;

		HR1 = random_(0, 2);
		HR2 = random_(0, 2);
		HR3 = random_(0, 2);
		// make sure there are at least two set
		if (!(HR1 & HR3))
			HR2 = TRUE;
		// draw the selected rooms
		if (HR1)
			L1drawRoom(1, 15, 10, 10);
		else
			xs = 25; // was 18;

		if (HR2)
			L1drawRoom(15, 15, 10, 10);
		if (HR3)
			L1drawRoom(29, 15, 10, 10);
		else
			xe = 15; // was 22;
		// draw a hallway between the rooms
		for (x = xs; x < xe; x++) {
			dungeon[x][17] = 1;
			dungeon[x][18] = 1;
			dungeon[x][19] = 1;
			dungeon[x][20] = 1;
			dungeon[x][21] = 1;
			dungeon[x][22] = 1;
		}
		// spread additional rooms starting from the main rooms
		if (HR1)
			L1roomGen(1, 15, 10, 10, 1);
		if (HR2)
			L1roomGen(15, 15, 10, 10, 1);
		if (HR3)
			L1roomGen(29, 15, 10, 10, 1);

		VR3 = FALSE;
		VR2 = FALSE;
		VR1 = FALSE;
	}
}

static int L1GetArea()
{
	int i, rv;
	BYTE *pTmp;

	rv = 0;
	static_assert(sizeof(dungeon) == DMAXX * DMAXY, "Linear traverse of dungeon does not work in L1GetArea.");
	pTmp = &dungeon[0][0];
	for (i = 0; i < DMAXX * DMAXY; i++, pTmp++) {
		assert(*pTmp <= 1);
		rv += *pTmp;
	}

	return rv;
}

static void DRLG_L1MakeMegas()
{
	int i, j;
	BYTE v;

	for (j = 0; j < DMAXY - 1; j++) {
		for (i = 0; i < DMAXX - 1; i++) {
			assert(dungeon[i][j] <= 1);
			v = dungeon[i][j]
			 | (dungeon[i + 1][j] << 1)
			 | (dungeon[i][j + 1] << 2)
			 | (dungeon[i + 1][j + 1] << 3);
			dungeon[i][j] = L1ConvTbl[v];
		}
	}
	for (j = 0; j < DMAXY; j++)
		dungeon[DMAXX - 1][j] = 22;
	for (i = 0; i < DMAXX - 1; i++)
		dungeon[i][DMAXY - 1] = 22;
}

static int L1HWallOk(int i, int j)
{
	int x;
	BYTE bv;

	for (x = 1; dungeon[i + x][j] == 13; x++) {
		if (dungeon[i + x][j - 1] != 13 || dungeon[i + x][j + 1] != 13 || dflags[i + x][j] != 0)
			break;
	}

	if (x != 1) {
		bv = dungeon[i + x][j];
		if ((bv >= 3 && bv <= 7)
		 || (bv >= 16 && bv <= 24 && bv != 22))
			return x;
	}
	return -1;
}

static int L1VWallOk(int i, int j)
{
	int y;
	BYTE bv;

	for (y = 1; dungeon[i][j + y] == 13; y++) {
		if (dungeon[i - 1][j + y] != 13 || dungeon[i + 1][j + y] != 13 || dflags[i][j + y] != 0)
			break;
	}

	if (y != 1) {
		bv = dungeon[i][j + y];
		if ((bv >= 3 && bv <= 7)
		 || (bv >= 16 && bv <= 24 && bv != 22))
			return y;
	}
	return -1;
}

static void L1HorizWall(int i, int j, char pn, int dx)
{
	int xx;
	char wt, dt;

	switch (random_(0, 4)) {
	case 0:
	case 1:
		dt = 2;
		break;
	case 2:
		dt = 12;
		if (pn == 2)
			pn = 12;
		if (pn == 4)
			pn = 10;
		break;
	case 3:
		dt = 36;
		if (pn == 2)
			pn = 36;
		if (pn == 4)
			pn = 27;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	if (random_(0, 6) == 5)
		wt = 12;
	else
		wt = 26;
	if (dt == 12)
		wt = 12;

	dungeon[i][j] = pn;

	for (xx = 1; xx < dx; xx++) {
		dungeon[i + xx][j] = dt;
	}

	xx = RandRange(1, dx - 1);

	if (wt == 12) {
		dungeon[i + xx][j] = 12;
	} else {
		dungeon[i + xx][j] = 2;
		dflags[i + xx][j] |= DLRG_HDOOR;
	}
}

static void L1VertWall(int i, int j, char pn, int dy)
{
	int yy;
	char wt, dt;

	switch (random_(0, 4)) {
	case 0:
	case 1:
		dt = 1;
		break;
	case 2:
		dt = 11;
		if (pn == 1)
			pn = 11;
		if (pn == 4)
			pn = 14;
		break;
	case 3:
		dt = 35;
		if (pn == 1)
			pn = 35;
		if (pn == 4)
			pn = 37;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	if (random_(0, 6) == 5)
		wt = 11;
	else
		wt = 25;
	if (dt == 11)
		wt = 11;

	dungeon[i][j] = pn;

	for (yy = 1; yy < dy; yy++) {
		dungeon[i][j + yy] = dt;
	}

	yy = RandRange(1, dy - 1);

	if (wt == 11) {
		dungeon[i][j + yy] = 11;
	} else {
		dungeon[i][j + yy] = 1;
		dflags[i][j + yy] |= DLRG_VDOOR;
	}
}

static void L1AddWall()
{
	int i, j, x, y;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (dflags[i][j] == 0) {
				if (dungeon[i][j] == 3) {
					x = L1HWallOk(i, j);
					if (x != -1)
						L1HorizWall(i, j, 2, x);
				}
				if (dungeon[i][j] == 3) {
					y = L1VWallOk(i, j);
					if (y != -1)
						L1VertWall(i, j, 1, y);
				}
				if (dungeon[i][j] == 6) {
					x = L1HWallOk(i, j);
					if (x != -1)
						L1HorizWall(i, j, 4, x);
				}
				if (dungeon[i][j] == 7) {
					y = L1VWallOk(i, j);
					if (y != -1)
						L1VertWall(i, j, 4, y);
				}
				if (dungeon[i][j] == 2) {
					x = L1HWallOk(i, j);
					if (x != -1)
						L1HorizWall(i, j, 2, x);
				}
				if (dungeon[i][j] == 1) {
					y = L1VWallOk(i, j);
					if (y != -1)
						L1VertWall(i, j, 1, y);
				}
			}
		}
	}
}

static void DRLG_L1GChamber(int sx, int sy, BOOL topflag, BOOL bottomflag, BOOL leftflag, BOOL rightflag)
{
	int i, j;

	if (topflag) {
		dungeon[sx + 2][sy] = 12;
		dungeon[sx + 3][sy] = 12;
		dungeon[sx + 4][sy] = 3;
		dungeon[sx + 7][sy] = 9;
		dungeon[sx + 8][sy] = 12;
		dungeon[sx + 9][sy] = 2;
	}
	if (bottomflag) {
		sy += 11;
		dungeon[sx + 2][sy] = 10;
		dungeon[sx + 3][sy] = 12;
		dungeon[sx + 4][sy] = 8;
		dungeon[sx + 7][sy] = 5;
		dungeon[sx + 8][sy] = 12;
		if (dungeon[sx + 9][sy] != 4) {
			dungeon[sx + 9][sy] = 21;
		}
		sy -= 11;
	}
	if (leftflag) {
		dungeon[sx][sy + 2] = 11;
		dungeon[sx][sy + 3] = 11;
		dungeon[sx][sy + 4] = 3;
		dungeon[sx][sy + 7] = 8;
		dungeon[sx][sy + 8] = 11;
		dungeon[sx][sy + 9] = 1;
	}
	if (rightflag) {
		sx += 11;
		dungeon[sx][sy + 2] = 14;
		dungeon[sx][sy + 3] = 11;
		dungeon[sx][sy + 4] = 9;
		dungeon[sx][sy + 7] = 5;
		dungeon[sx][sy + 8] = 11;
		if (dungeon[sx][sy + 9] != 4) {
			dungeon[sx][sy + 9] = 21;
		}
		sx -= 11;
	}

	for (j = 1; j < 11; j++) {
		for (i = 1; i < 11; i++) {
			dungeon[i + sx][j + sy] = 13;
			dflags[i + sx][j + sy] |= DLRG_CHAMBER;
		}
	}

	dungeon[sx + 4][sy + 4] = 15;
	dungeon[sx + 7][sy + 4] = 15;
	dungeon[sx + 4][sy + 7] = 15;
	dungeon[sx + 7][sy + 7] = 15;
}

static void DRLG_L1GHall(int x1, int y1, int x2, int y2)
{
	int i;

	if (y1 == y2) {
		for (i = x1; i < x2; i++) {
			dungeon[i][y1] = 12;
			dungeon[i][y1 + 3] = 12;
		}
	} else {
		for (i = y1; i < y2; i++) {
			dungeon[x1][i] = 11;
			dungeon[x1 + 3][i] = 11;
		}
	}
}

static void L1tileFix()
{
	int i, j;
	BYTE v1, v2;

	// BUGFIX: Bounds checks are required in all loop bodies.
	// See https://github.com/diasurgical/devilutionX/pull/401

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			v1 = dungeon[i][j];
			if (i + 1 < DMAXX) {
				v2 = dungeon[i + 1][j];
				if (v1 == 2 && v2 == 22)
					dungeon[i + 1][j] = 23;
				if (v1 == 13 && v2 == 22)
					dungeon[i + 1][j] = 18;
				if (v1 == 13 && v2 == 2)
					dungeon[i + 1][j] = 7;
				if (v1 == 6 && v2 == 22)
					dungeon[i + 1][j] = 24;
			}
			if (j + 1 < DMAXY) {
				v2 = dungeon[i][j + 1];
				if (v1 == 1 && v2 == 22)
					dungeon[i][j + 1] = 24;
				if (v1 == 13 && v2 == 1)
					dungeon[i][j + 1] = 6;
				if (v1 == 13 && v2 == 22)
					dungeon[i][j + 1] = 19;
			}
		}
	}

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			v1 = dungeon[i][j];
			if (i + 1 < DMAXX) {
				v2 = dungeon[i + 1][j];
				if (v1 == 13 && v2 == 19)
					dungeon[i + 1][j] = 21;
				if (v1 == 13 && v2 == 22)
					dungeon[i + 1][j] = 20;
				if (v1 == 13 && v2 == 24)
					dungeon[i + 1][j] = 21;
				if (v1 == 7 && v2 == 22)
					dungeon[i + 1][j] = 23;
				if (v1 == 7 && v2 == 19)
					dungeon[i + 1][j] = 21;
				if (v1 == 7 && v2 == 1)
					dungeon[i + 1][j] = 6;
				if (v1 == 7 && v2 == 24)
					dungeon[i + 1][j] = 21;
				if (v1 == 7 && v2 == 13)
					dungeon[i + 1][j] = 17;
				if (v1 == 19 && v2 == 22)
					dungeon[i + 1][j] = 20;
				if (v1 == 19 && v2 == 1)
					dungeon[i + 1][j] = 6;
				if (v1 == 2 && v2 == 19)
					dungeon[i + 1][j] = 21;
				if (v1 == 2 && v2 == 1)
					dungeon[i + 1][j] = 6;
				if (v1 == 2 && v2 == 24)
					dungeon[i + 1][j] = 21;
				if (v1 == 2 && v2 == 13)
					dungeon[i + 1][j] = 17;
				if (v1 == 3 && v2 == 22)
					dungeon[i + 1][j] = 24;
				if (v1 == 21 && v2 == 1)
					dungeon[i + 1][j] = 6;
				if (v1 == 4 && v2 == 16)
					dungeon[i + 1][j] = 17;
			}
			if (i != 0) {
				v2 = dungeon[i - 1][j];
				if (v1 == 23 && v2 == 22)
					dungeon[i - 1][j] = 19;
				if (v1 == 19 && v2 == 23)
					dungeon[i - 1][j] = 21;
				if (v1 == 6 && v2 == 22)
					dungeon[i - 1][j] = 24;
				if (v1 == 6 && v2 == 23)
					dungeon[i - 1][j] = 21;
			}
			if (j + 1 < DMAXY) {
				v2 = dungeon[i][j + 1];
				if (v1 == 1 && v2 == 2)
					dungeon[i][j + 1] = 7;
				if (v1 == 1 && v2 == 13)
					dungeon[i][j + 1] = 16;
				if (v1 == 6 && v2 == 18)
					dungeon[i][j + 1] = 21;
				if (v1 == 6 && v2 == 2)
					dungeon[i][j + 1] = 7;
				if (v1 == 6 && v2 == 22)
					dungeon[i][j + 1] = 24;
				if (v1 == 6 && v2 == 13)
					dungeon[i][j + 1] = 16;
				if (v1 == 18 && v2 == 2)
					dungeon[i][j + 1] = 7;
				if (v1 == 21 && v2 == 2)
					dungeon[i][j + 1] = 7;
				if (v1 == 13 && v2 == 16)
					dungeon[i][j + 1] = 17;
			}
			if (j != 0) {
				v2 = dungeon[i][j - 1];
				if (v1 == 6 && v2 == 22)
					dungeon[i][j - 1] = 7;
				if (v1 == 7 && v2 == 24)
					dungeon[i][j - 1] = 21;
				if (v1 == 18 && v2 == 24)
					dungeon[i][j - 1] = 21;
			}
		}
	}

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			v1 = dungeon[i][j];
			if (v1 == 4 && j + 1 < DMAXY && dungeon[i][j + 1] == 2)
				dungeon[i][j + 1] = 7;
			if (v1 == 2 && i + 1 < DMAXX && dungeon[i + 1][j] == 19)
				dungeon[i + 1][j] = 21;
			if (v1 == 18 && j + 1 < DMAXY && dungeon[i][j + 1] == 22)
				dungeon[i][j + 1] = 20;
		}
	}
}

#ifdef HELLFIRE
static void DRLG_L5PlaceRndSet(const BYTE *miniset, BYTE rndper)
{
	int sx, sy, sw, sh, xx, yy, ii;
	bool found;

	sw = miniset[0];
	sh = miniset[1];

	for (sy = 0; sy < DMAXY - sh; sy++) {
		for (sx = 0; sx < DMAXX - sw; sx++) {
			found = true;
			ii = 2;
			for (yy = 0; yy < sh && found; yy++) {
				for (xx = 0; xx < sw && found; xx++) {
					if (miniset[ii] != 0 && dungeon[xx + sx][yy + sy] != miniset[ii]) {
						found = false;
					}
					if (dflags[xx + sx][yy + sy] != 0) {
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
				// BUGFIX: Comparisons vs 100 should use same tile as comparisons vs 84 (fixed)
				if ((sx > 0 && dungeon[sx - 1][sy] >= 84 && dungeon[sx - 1][sy] <= 100)
				 || (dungeon[sx + 1][sy] >= 84 && dungeon[sx + 1][sy] <= 100)
				 || (dungeon[sx][sy + 1] >= 84 && dungeon[sx][sy + 1] <= 100)
				 || (sy > 0 && dungeon[sx][sy - 1] >= 84 && dungeon[sx][sy - 1] <= 100)) {
					continue;
				}
			}
			if (random_(0, 100) < rndper) {
				for (yy = 0; yy < sh; yy++) {
					for (xx = 0; xx < sw; xx++) {
						if (miniset[ii] != 0) {
							dungeon[xx + sx][yy + sy] = miniset[ii];
						}
						ii++;
					}
				}
			}
		}
	}
}
#endif

static void DRLG_L1Subs()
{
	int x, y, rv, i;

	for (y = 0; y < DMAXY; y++) {
		for (x = 0; x < DMAXX; x++) {
			if (random_(0, 4) == 0) {
				BYTE c = L1BTYPES[dungeon[x][y]];

				if (c != 0 && dflags[x][y] == 0) {
					rv = random_(0, 16);
					i = -1;

					while (rv >= 0) {
						if (++i == sizeof(L1BTYPES))
							i = 0;
						if (c == L1BTYPES[i])
							rv--;
					}

					// BUGFIX: Add `&& y > 0` to the if statement. (fixed)
					if (i == 89 && y > 0) {
						if (L1BTYPES[dungeon[x][y - 1]] != 79 || dflags[x][y - 1] != 0)
							i = 79;
						else
							dungeon[x][y - 1] = 90;
					}
					// BUGFIX: Add `&& x + 1 < DMAXX` to the if statement. (fixed)
					if (i == 91 && x + 1 < DMAXX) {
						if (L1BTYPES[dungeon[x + 1][y]] != 80 || dflags[x + 1][y] != 0)
							i = 80;
						else
							dungeon[x + 1][y] = 92;
					}
					dungeon[x][y] = i;
				}
			}
		}
	}
}

static void DRLG_L1SetRoom(int rx1, int ry1)
{
	int rw, rh, i, j;
	BYTE *sp;

	rw = pSetPiece[0];
	rh = pSetPiece[2];

	setpc_x = rx1;
	setpc_y = ry1;
	setpc_w = rw;
	setpc_h = rh;

	sp = &pSetPiece[4];

	rw += rx1;
	rh += ry1;
	for (j = ry1; j < rh; j++) {
		for (i = rx1; i < rw; i++) {
			dungeon[i][j] = *sp != 0 ? *sp : 13;
			dflags[i][j] |= DLRG_PROTECTED;
			sp += 2;
		}
	}
}

static void L1FillChambers()
{
	int c;

	if (HR1)
		DRLG_L1GChamber(0, 14, FALSE, FALSE, FALSE, TRUE);
	if (HR2)
		DRLG_L1GChamber(14, 14, FALSE, FALSE, HR1, HR3);
	if (HR3)
		DRLG_L1GChamber(28, 14, FALSE, FALSE, TRUE, FALSE);

	if (HR1 && HR2)
		DRLG_L1GHall(12, 18, 14, 18);
	if (HR2 && HR3)
		DRLG_L1GHall(26, 18, 28, 18);
	if (HR1 && !HR2 && HR3)
		DRLG_L1GHall(12, 18, 28, 18);

	if (VR1)
		DRLG_L1GChamber(14, 0, FALSE, TRUE, FALSE, FALSE);
	if (VR2)
		DRLG_L1GChamber(14, 14, VR1, VR3, FALSE, FALSE);
	if (VR3)
		DRLG_L1GChamber(14, 28, TRUE, FALSE, FALSE, FALSE);

	if (VR1 && VR2)
		DRLG_L1GHall(18, 12, 18, 14);
	if (VR2 && VR3)
		DRLG_L1GHall(18, 26, 18, 28);
	if (VR1 && !VR2 && VR3)
		DRLG_L1GHall(18, 12, 18, 28);

	if (pSetPiece != NULL) {
		if (VR1 || VR2 || VR3) {
			c = 1;
			if (!VR1 && VR2 && VR3 && random_(0, 2) != 0)
				c = 2;
			if (VR1 && VR2 && !VR3 && random_(0, 2) != 0)
				c = 0;

			if (VR1 && !VR2 && VR3) {
				if (random_(0, 2) != 0)
					c = 0;
				else
					c = 2;
			}

			if (VR1 && VR2 && VR3)
				c = random_(0, 3);

			switch (c) {
			case 0:
				c = 2;
				break;
			case 1:
				c = 16;
				break;
			case 2:
				c = 30;
				break;
			default:
				ASSUME_UNREACHABLE
				break;
			}
			DRLG_L1SetRoom(16, c);
		} else {
			c = 1;
			if (!HR1 && HR2 && HR3 && random_(0, 2) != 0)
				c = 2;
			if (HR1 && HR2 && !HR3 && random_(0, 2) != 0)
				c = 0;

			if (HR1 && !HR2 && HR3) {
				if (random_(0, 2) != 0)
					c = 0;
				else
					c = 2;
			}

			if (HR1 && HR2 && HR3)
				c = random_(0, 3);

			switch (c) {
			case 0:
				c = 2;
				break;
			case 1:
				c = 16;
				break;
			case 2:
				c = 30;
				break;
			default:
				ASSUME_UNREACHABLE
				break;
			}
			DRLG_L1SetRoom(c, 16);
		}
	}
}

#ifdef HELLFIRE
static void DRLG_L5Crypt_pattern2(BYTE rndper)
{
	/** Miniset: Use random vertical wall tile A1. */
	DRLG_PlaceRndTile(1, 112, rndper);
	/** Miniset: Use random horizontal wall tile 1. */
	DRLG_PlaceRndTile(2, 113, rndper);
	/** Miniset: South-Edge column on the northern side 1. */
	DRLG_PlaceRndTile(3, 114, rndper);
	/** Miniset: Use random corner north 1. */
	DRLG_PlaceRndTile(4, 115, rndper);
	/** Miniset: North-Edge column on the northern side 1. */
	DRLG_PlaceRndTile(5, 116, rndper);
	/** Miniset: Use random vertical wall tile B1. */
	DRLG_PlaceRndTile(6, 117, rndper);
	/** Miniset: Use random concave corner east A1. */
	DRLG_PlaceRndTile(7, 118, rndper);
	/** Miniset: East-Edge column on the northern side 1. */
	DRLG_PlaceRndTile(8, 119, rndper);
	/** Miniset: West-Edge column on the northern side 1. */
	DRLG_PlaceRndTile(9, 120, rndper);
	/** Miniset: Use random vertical wall tile C1. */
	DRLG_PlaceRndTile(10, 121, rndper);
	/** Miniset: Column on the northern side of a vertical wall 4. */
	DRLG_PlaceRndTile(11, 122, rndper);
	/** Miniset: Column on the northern side of a horizontal wall 4. */
	DRLG_PlaceRndTile(12, 123, rndper);
	/** Miniset: Use random floor tile 9. */
	DRLG_PlaceRndTile(13, 124, rndper);
	/** Miniset: Use random concave corner east B1. */
	DRLG_PlaceRndTile(14, 125, rndper);
	/** Miniset: Use random column 2. */
	DRLG_PlaceRndTile(15, 126, rndper);
	/** Miniset: Use random concave corner north A1. */
	DRLG_PlaceRndTile(16, 127, rndper);
	/** Miniset: Use random concave corner north B1. */
	DRLG_PlaceRndTile(17, 128, rndper);
}

static void DRLG_L5Crypt_pattern3(BYTE rndper)
{
	/** Miniset: Use random vertical wall tile A2. */
	DRLG_PlaceRndTile(1, 129, rndper);
	/** Miniset: Use random horizontal wall tile 2. */
	DRLG_PlaceRndTile(2, 130, rndper);
	/** Miniset: South-Edge column on the northern side 2. */
	DRLG_PlaceRndTile(3, 131, rndper);
	/** Miniset: Use random corner north 2. */
	DRLG_PlaceRndTile(4, 132, rndper);
	/** Miniset: North-Edge column on the northern side 2. */
	DRLG_PlaceRndTile(5, 133, rndper);
	/** Miniset: Use random vertical wall tile B2. */
	DRLG_PlaceRndTile(6, 134, rndper);
	/** Miniset: Use random concave corner east A2. */
	DRLG_PlaceRndTile(7, 135, rndper);
	/** Miniset: East-Edge column on the northern side 2. */
	DRLG_PlaceRndTile(8, 136, rndper);
	/** Miniset: West-Edge column on the northern side 2. */
	DRLG_PlaceRndTile(9, 137, rndper);
	/** Miniset: Use random vertical wall tile C2. */
	DRLG_PlaceRndTile(10, 138, rndper);
	/** Miniset: Column on the northern side of a vertical wall 5. */
	DRLG_PlaceRndTile(11, 139, rndper);
	/** Miniset: Column on the northern side of a horizontal wall 5. */
	DRLG_PlaceRndTile(12, 140, rndper);
	/** Miniset: Use random floor tile 10. */
	DRLG_PlaceRndTile(13, 141, rndper);
	/** Miniset: Use random concave corner east B2. */
	DRLG_PlaceRndTile(14, 142, rndper);
	/** Miniset: Use random column 3. (cracked column) */
	DRLG_PlaceRndTile(15, 143, rndper);
	/** Miniset: Use random concave corner north A2. */
	DRLG_PlaceRndTile(16, 144, rndper);
	/** Miniset: Use random concave corner north B2. */
	DRLG_PlaceRndTile(17, 145, rndper);
}

static void DRLG_L5Crypt_pattern4(BYTE rndper)
{
	/** Miniset: Use random vertical wall tile A3. */
	DRLG_PlaceRndTile(1, 146, rndper);
	/** Miniset: Use random horizontal wall tile 3. */
	DRLG_PlaceRndTile(2, 147, rndper);
	/** Miniset: South-Edge column on the northern side 3. */
	DRLG_PlaceRndTile(3, 148, rndper);
	/** Miniset: Use random corner north 3. */
	DRLG_PlaceRndTile(4, 149, rndper);
	/** Miniset: North-Edge column on the northern side 3. */
	DRLG_PlaceRndTile(5, 150, rndper);
	/** Miniset: Use random vertical wall tile B3. */
	DRLG_PlaceRndTile(6, 151, rndper);
	/** Miniset: Use random concave corner east A3. */
	DRLG_PlaceRndTile(7, 152, rndper);
	/** Miniset: East-Edge column on the northern side 3. */
	DRLG_PlaceRndTile(8, 153, rndper);
	/** Miniset: West-Edge column on the northern side 3. */
	DRLG_PlaceRndTile(9, 154, rndper);
	/** Miniset: Use random vertical wall tile C3. */
	DRLG_PlaceRndTile(10, 155, rndper);
	/** Miniset: Column on the northern side of a vertical wall 6. */
	DRLG_PlaceRndTile(11, 156, rndper);
	/** Miniset: Column on the northern side of a horizontal wall 6. */
	DRLG_PlaceRndTile(12, 157, rndper);
	/** Miniset: Use random floor tile 11. */
	DRLG_PlaceRndTile(13, 158, rndper);
	/** Miniset: Use random concave corner east B3. */
	DRLG_PlaceRndTile(14, 159, rndper);
	/** Miniset: Use random column 4. (Lava fountain) */
	DRLG_PlaceRndTile(15, 160, rndper);
	/** Miniset: Use random concave corner north A3. */
	DRLG_PlaceRndTile(16, 161, rndper);
	/** Miniset: Use random concave corner north B3. */
	DRLG_PlaceRndTile(17, 162, rndper);
}

static void DRLG_L5Crypt_pattern5(BYTE rndper)
{
	DRLG_L5PlaceRndSet(L5RNDLFLOOR2, rndper);
	DRLG_L5PlaceRndSet(L5RNDLFLOOR3, rndper);
	DRLG_L5PlaceRndSet(L5RNDLFLOOR4, rndper);
	DRLG_L5PlaceRndSet(L5RNDLFLOOR5, rndper);
	DRLG_L5PlaceRndSet(L5RNDLFLOOR6, rndper);
	DRLG_L5PlaceRndSet(L5RNDLFLOOR7, rndper);
	/** Miniset: Use random floor tile 5. */
	DRLG_PlaceRndTile(13, 163, rndper);
	/** Miniset: Use random floor tile 6. */
	DRLG_PlaceRndTile(13, 164, rndper);
	/** Miniset: Use random floor tile 7. */
	DRLG_PlaceRndTile(13, 165, rndper);
	/** Miniset: Use random floor tile 8. */
	DRLG_PlaceRndTile(13, 166, rndper);
}

static void DRLG_L5Crypt_pattern6(BYTE rndper)
{
	/** Miniset: Column on the northern side of a vertical wall 2. pointless? same as 95 */
	DRLG_PlaceRndTile(11, 185, rndper);
	/** Miniset: Column on the northern side of a horizontal wall 2. pointless? same as 96 */
	DRLG_PlaceRndTile(12, 187, rndper);
	/** Miniset: Column on the northern side of a vertical wall 3. pointless? same as 95 */
	DRLG_PlaceRndTile(11, 186, rndper);
	/** Miniset: Column on the northern side of a horizontal wall 3. pointless? same as 96 */
	DRLG_PlaceRndTile(12, 188, rndper);
	/** Miniset: Replace vertical wall tile on the bottom 1. */
	DRLG_PlaceRndTile(89, 173, rndper);
	/** Miniset: Replace vertical wall tile on the bottom 2. */
	DRLG_PlaceRndTile(89, 174, rndper);
	/** Miniset: Replace vertical wall tile in the middle 1. */
	DRLG_PlaceRndTile(90, 175, rndper);
	/** Miniset: Replace vertical wall tile in the middle 2. */
	DRLG_PlaceRndTile(90, 176, rndper);
	/** Miniset: Replace vertical wall tile on the top 1. */
	DRLG_PlaceRndTile(91, 177, rndper);
	/** Miniset: Replace vertical wall tile on the top 2. */
	DRLG_PlaceRndTile(91, 178, rndper);
	/** Miniset: Replace random horizontal wall tile on east 1. but pointless? */
	DRLG_PlaceRndTile(92, 179, rndper);
	/** Miniset: Replace random horizontal wall tile on east 2. */
	DRLG_PlaceRndTile(92, 180, rndper);
	/** Miniset: Replace random horizontal wall tile in the middle 1. */
	DRLG_PlaceRndTile(93, 181, rndper);
	/** Miniset: Replace random horizontal wall tile in the middle 2. */
	DRLG_PlaceRndTile(93, 182, rndper);
	/** Miniset: Replace random horizontal wall tile on west 1. */
	DRLG_PlaceRndTile(94, 183, rndper);
	/** Miniset: Replace random horizontal wall tile on west 2. */
	DRLG_PlaceRndTile(94, 184, rndper);
	DRLG_L5PlaceRndSet(L5RNDFLOOR2_0, rndper);
	DRLG_L5PlaceRndSet(L5RNDFLOOR2_1, rndper);
	DRLG_L5PlaceRndSet(L5RNDFLOOR1_0, rndper);
	/** Miniset: Use random column 1. (Cracked) */
	DRLG_PlaceRndTile(15, 192, rndper);
	DRLG_L5PlaceRndSet(L5RNDFLOOR3_0, rndper);
	DRLG_L5PlaceRndSet(L5RNDFLOOR3_1, rndper);
	DRLG_L5PlaceRndSet(L5RNDFLOOR4_0, rndper);
	/** Miniset: Replace random large floor tile 1. */
	DRLG_PlaceRndTile(101, 196, rndper);
	/** Miniset: Replace random large floor tile 2. */
	DRLG_PlaceRndTile(101, 197, rndper);
	/** Miniset: Replace random large floor tile 3. */
	DRLG_PlaceRndTile(101, 198, rndper);
}

static void DRLG_L5Crypt_pattern7(BYTE rndper)
{
	DRLG_L5PlaceRndSet(L5RNDFLOOR1, rndper);
	DRLG_L5PlaceRndSet(L5RNDFLOOR2, rndper);
	DRLG_L5PlaceRndSet(L5RNDFLOOR3, rndper);
	DRLG_L5PlaceRndSet(L5RNDFLOOR4, rndper);
}
#endif

static void DRLG_L1TransFix()
{
	/* commented out - see the individual entries for more details
	int xx, yy, i, j;
	BYTE tv;

	yy = DBORDERY;

	for (j = 0; j < DMAXY; j++) {
		xx = DBORDERX;

		for (i = 0; i < DMAXX; i++) {
			switch (dungeon[i][j]) {
			/ commented out because DRLG_FloodTVal makes this unnecessary (spreads to more than just the floor tiles)
			case 18:
				DRLG_CopyTrans(xx, yy, xx + 1, yy);
				DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;
			case 19:
				DRLG_CopyTrans(xx, yy, xx, yy + 1);
				DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;
			case 20:
				DRLG_CopyTrans(xx, yy, xx + 1, yy);
				DRLG_CopyTrans(xx, yy, xx, yy + 1);
				DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;
			case 23:
				// BUGFIX: Should check for `j > 0` first. (fixed)
				if (j > 0 && dungeon[i][j - 1] == 18) {
					DRLG_CopyTrans(xx, yy, xx + 1, yy);
					DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				}
				break;
			case 24:
				// BUGFIX: Should check for `i + 1 < DMAXX` first. (fixed)
				if (i < DMAXX - 1 && dungeon[i + 1][j] == 19) {
					DRLG_CopyTrans(xx, yy, xx, yy + 1);
					DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				}
				break;/
			// fix transVals around the stairs - necessary only if DRLG_FloodTVal is run after the placement
			// - due to id conflict between hellfire and diablo, this does not work on tile 63
			case 55:
			case 58:
			case 61:
			case 62:
			case 63:
			case 66:
				tv = dTransVal[xx + 6][yy];
				dTransVal[xx][yy] = tv;
				dTransVal[xx + 1][yy] = tv;
				dTransVal[xx][yy + 1] = tv;
				dTransVal[xx + 1][yy + 1] = tv;
				// fix walls on the west side
				//if (dTransVal[xx - 1][yy] == 0)
					dTransVal[xx - 1][yy] = tv;
				//if (dTransVal[xx - 1][yy + 1] == 0)
					dTransVal[xx - 1][yy + 1] = tv;
				break;
			case 67:
				DRLG_CopyTrans(xx, yy + 1, xx + 1, yy);
				break;
			case 68:
				DRLG_CopyTrans(xx, yy + 1, xx, yy);
				break;
			}
			xx += 2;
		}
		yy += 2;
	}*/
}

static void DRLG_L1DirtFix()
{
	int i, j;
#ifdef HELLFIRE
	BYTE bv;

	if (currLvl._dType == DTYPE_CRYPT) {
		for (j = 0; j < DMAXY; j++) {
			for (i = 0; i < DMAXX; i++) {
				switch (dungeon[i][j]) {
				case 18: bv = 82; break;
				case 19: bv = 83; break;
				case 21: bv = 85; break;
				case 23: bv = 87; break;
				case 24: bv = 88; break;
				default: continue;
				}
				dungeon[i][j] = bv;
			}
		}
		return;
	}
	// assert(currLvl._dType == DTYPE_CATHEDRAL);
#endif
	for (j = 0; j < DMAXY - 1; j++) {
		for (i = 0; i < DMAXX - 1; i++) {
			switch (dungeon[i][j]) {
			case 18:
				if (dungeon[i][j + 1] != 18)
					dungeon[i][j] = 199;
				break;
			case 19:
				if (dungeon[i + 1][j] != 19)
					dungeon[i][j] = 200;
				break;
			case 21:
				if (dungeon[i + 1][j] != 19 || dungeon[i][j + 1] != 18)
					dungeon[i][j] = 202;
				break;
			case 23:
				if (dungeon[i][j + 1] != 18)
					dungeon[i][j] = 204;
				break;
			case 24:
				if (dungeon[i + 1][j] != 19)
					dungeon[i][j] = 205;
				break;
			}
		}
	}
}

static void DRLG_L1CornerFix()
{
	int i, j;

	for (j = 1; j < DMAXY - 1; j++) {
		for (i = 1; i < DMAXX - 1; i++) {
			if (!(dflags[i][j] & DLRG_PROTECTED) && dungeon[i][j] == 17 && dungeon[i - 1][j] == 13 && dungeon[i][j - 1] == 1) {
				dungeon[i][j] = 16;
				dflags[i][j - 1] &= DLRG_PROTECTED;
			}
			if (dungeon[i][j] == 202 && dungeon[i + 1][j] == 13 && dungeon[i][j + 1] == 1) {
				dungeon[i][j] = 8;
			}
		}
	}
}

struct mini_set {
	const BYTE* data;
	bool setview;
};
static bool DRLG_L1PlaceMiniSets(mini_set* minisets, int n)
{
	int i;

	for (i = 0; i < n; i++) {
		if (minisets[i].data != NULL && !DRLG_L1PlaceMiniSet(minisets[i].data, minisets[i].setview)) {
			return false;
		}
	}
	return true;
}

static void DRLG_L1(int entry)
{
	int i, j;
	int minarea;
	bool doneflag;

	switch (currLvl._dLevelIdx) {
	case DLV_CATHEDRAL1:
		minarea = 533;
		break;
	case DLV_CATHEDRAL2:
		minarea = 693;
		break;
	default:
		minarea = 761;
		break;
	}

	do {
		do {
			InitL1Dungeon();
			L1firstRoom();
		} while (L1GetArea() < minarea);

		DRLG_L1MakeMegas();
		L1FillChambers();
		L1tileFix();
		L1AddWall();
		L1ClearFlags();
		DRLG_InitTrans();
		DRLG_FloodTVal(13);

		doneflag = true;

		if (QuestStatus(Q_PWATER)) {
			POS32 mpos = DRLG_PlaceMiniSet(PWATERIN);
			if (mpos.x != DMAXX) {
				// fix transVal of the set-map
				// - uncommented since the pieces are blocked anyway
				//DRLG_MRectTrans(mpos.x + 1, mpos.y + 1, mpos.x + 4, mpos.y + 2, 0);

				quests[Q_PWATER]._qtx = 2 * mpos.x + DBORDERX + 5;
				quests[Q_PWATER]._qty = 2 * mpos.y + DBORDERY + 6;
			} else {
				doneflag = false;
			}
		}
		if (QuestStatus(Q_LTBANNER)) {
			// fix transVal behind the stairs
			// - uncommented since the set-map is 'populated' -> monsters are not spawn there
			//DRLG_MRectTrans(setpc_x, setpc_y + 3, setpc_x, setpc_y + 5,
			//	dTransVal[2 * setpc_x + DBORDERX + 1][2 * setpc_y + DBORDERY + 11]);
			if (entry == ENTRY_PREV) {
				ViewX = 2 * setpc_x + DBORDERX + 4;
				ViewY = 2 * setpc_y + DBORDERY + 12;
			}
			doneflag = DRLG_L1PlaceMiniSet(L1USTAIRS, entry == ENTRY_MAIN); // was STAIRSUP
#ifdef HELLFIRE
		} else if (currLvl._dType == DTYPE_CRYPT) {
			mini_set stairs[2] = {
				{ currLvl._dLevelIdx != DLV_CRYPT1 ? L5USTAIRS : L5TWARP, entry != ENTRY_PREV },
				{ currLvl._dLevelIdx != DLV_CRYPT4 ? L5DSTAIRS : NULL, entry == ENTRY_PREV },
			};
			doneflag = DRLG_L1PlaceMiniSets(stairs, 2);
			if (entry == ENTRY_PREV) {
				ViewY += 3;
			} else {
				ViewY++;
			}
#endif
		} else {
			// assert(currLvl._dType == DTYPE_CATHEDRAL);
			mini_set stairs[2] = {
				{ L1USTAIRS, entry != ENTRY_PREV }, // was STAIRSUP in hellfire
				{ L1DSTAIRS, entry == ENTRY_PREV },
			};
			doneflag &= DRLG_L1PlaceMiniSets(stairs, 2);
			if (entry == ENTRY_PREV) {
				ViewY++;
			}
		}
	} while (!doneflag);

	DRLG_L1TransFix();
	DRLG_L1DirtFix();
	DRLG_L1CornerFix();

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (dflags[i][j] & ~DLRG_PROTECTED)
				DRLG_L1PlaceDoor(i, j);
		}
	}

#ifdef HELLFIRE
	if (currLvl._dType == DTYPE_CRYPT) {
		/** Miniset: Use random vertical wall 4. (Statue). */
		DRLG_PlaceRndTile(1, 199, 10);
		/** Miniset: Use random vertical wall 5. (Succubus statue) */
		DRLG_PlaceRndTile(1, 201, 10);
		/** Miniset: Use random horizontal wall 4. (Statue). */
		DRLG_PlaceRndTile(2, 200, 10);
		/** Miniset: Use random horizontal wall 5. (Succubus statue) */
		DRLG_PlaceRndTile(2, 202, 10);
		DRLG_L5PlaceRndSet(L5VERTCOL1, 95);
		DRLG_L5PlaceRndSet(L5HORIZCOL1, 95);
		DRLG_L5PlaceRndSet(L5PREVERTWALL, 100);
		DRLG_L5PlaceRndSet(L5PREHORIZWALL, 100);
		DRLG_L5PlaceRndSet(L5RNDLFLOOR1, 60);
		DRLG_L5Crypt_lavafloor();
		switch (currLvl._dLevelIdx) {
		case DLV_CRYPT1:
			DRLG_L5Crypt_pattern2(30);
			DRLG_L5Crypt_pattern3(15);
			DRLG_L5Crypt_pattern4(5);
			DRLG_L5Crypt_lavafloor();
			DRLG_L5Crypt_pattern7(10);
			DRLG_L5Crypt_pattern6(5);
			DRLG_L5Crypt_pattern5(20);
			break;
		case DLV_CRYPT2:
			DRLG_L5Crypt_pattern7(10);
			DRLG_L5Crypt_pattern6(10);
			DRLG_L5Crypt_pattern5(20);
			DRLG_L5Crypt_pattern2(30);
			DRLG_L5Crypt_pattern3(20);
			DRLG_L5Crypt_pattern4(10);
			DRLG_L5Crypt_lavafloor();
			break;
		case DLV_CRYPT3:
			DRLG_L5Crypt_pattern7(10);
			DRLG_L5Crypt_pattern6(15);
			DRLG_L5Crypt_pattern5(30);
			DRLG_L5Crypt_pattern2(30);
			DRLG_L5Crypt_pattern3(20);
			DRLG_L5Crypt_pattern4(15);
			DRLG_L5Crypt_lavafloor();
			break;
		case DLV_CRYPT4:
			DRLG_L5Crypt_pattern7(10);
			DRLG_L5Crypt_pattern6(20);
			DRLG_L5Crypt_pattern5(30);
			DRLG_L5Crypt_pattern2(30);
			DRLG_L5Crypt_pattern3(20);
			DRLG_L5Crypt_pattern4(20);
			DRLG_L5Crypt_lavafloor();
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
	} else
#endif
	{
		// assert(currLvl._dType == DTYPE_CATHEDRAL);
		DRLG_L1Subs();
		DRLG_L1Shadows();
		for (i = RandRange(5, 9); i > 0; i--)
			DRLG_L1PlaceMiniSet(LAMPS, false);
		DRLG_L1Floor();
	}

	memcpy(pdungeon, dungeon, sizeof(pdungeon));

	DRLG_Init_Globals();
	DRLG_CheckQuests();
}

void CreateL1Dungeon(int entry)
{
	DRLG_InitSetPC();
	DRLG_LoadL1SP();
	DRLG_L1(entry);
	DRLG_PlaceMegaTiles(BASE_MEGATILE_L1);
	DRLG_FreeL1SP();

#ifdef HELLFIRE
	if (currLvl._dType == DTYPE_CRYPT) {
		DRLG_InitL5Vals();
	} else
#endif
	{
		// assert(currLvl._dType == DTYPE_CATHEDRAL);
		DRLG_InitL1Vals();
	}
	DRLG_SetPC();
}

DEVILUTION_END_NAMESPACE
