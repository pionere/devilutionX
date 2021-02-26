/**
 * @file drlg_l3.cpp
 *
 * Implementation of the caves level generation algorithms.
 */

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** Starting position of the megatiles. */
#define BASE_MEGATILE_L3 (8 - 1)
/** This will be true if a lava pool has been generated for the level */
BOOLEAN lavapool;
int lockoutcnt;
BOOLEAN lockout[DMAXX][DMAXY];

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
	7, 7, 7, 0, 7,
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
/** Miniset: Use random floor tile 1. */
const BYTE L3XTRA1[] = {
	// clang-format off
	1, 1, // width, height

	7, // search

	106, // replace
	// clang-format on
};
/** Miniset: Use random floor tile 2. */
const BYTE L3XTRA2[] = {
	// clang-format off
	1, 1, // width, height

	7, // search

	107, // replace
	// clang-format on
};
/** Miniset: Use random floor tile 3. */
const BYTE L3XTRA3[] = {
	// clang-format off
	1, 1, // width, height

	7, // search

	108, // replace
	// clang-format on
};
/** Miniset: Use random horizontal wall tile. */
const BYTE L3XTRA4[] = {
	// clang-format off
	1, 1, // width, height

	9, // search

	109, // replace
	// clang-format on
};
/** Miniset: Use random vertical wall tile. */
const BYTE L3XTRA5[] = {
	// clang-format off
	1, 1, // width, height

	10, // search

	110, // replace
	// clang-format on
};

/** Miniset: Anvil of Fury island. */
const BYTE L3ANVIL[] = {
	// clang-format off
	11, 11, // width, height

	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, // search
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,

	0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, // replace
	0,  0, 29, 26, 26, 26, 26, 26, 30,  0, 0,
	0, 29, 34, 33, 33, 37, 36, 33, 35, 30, 0,
	0, 25, 33, 37, 27, 32, 31, 36, 33, 28, 0,
	0, 25, 37, 32,  7,  7,  7, 31, 27, 32, 0,
	0, 25, 28,  7,  7,  7,  7,  2,  2,  2, 0,
	0, 25, 35, 30,  7,  7,  7, 29, 26, 30, 0,
	0, 25, 33, 35, 26, 30, 29, 34, 33, 28, 0,
	0, 31, 36, 33, 33, 35, 34, 33, 37, 32, 0,
	0,  0, 31, 27, 27, 27, 27, 27, 32,  0, 0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0,
	// clang-format on
};

#ifdef HELLFIRE
const BYTE byte_48A76C[] = { 1, 1, 8, 25 };
const BYTE byte_48A770[] = { 1, 1, 8, 26 };
const BYTE byte_48A774[] = { 1, 1, 8, 27 };
const BYTE byte_48A778[] = { 1, 1, 8, 28 };
const BYTE byte_48A77C[] = { 1, 1, 7, 29 };
const BYTE byte_48A780[] = { 1, 1, 7, 30 };
const BYTE byte_48A784[] = { 1, 1, 7, 31 };
const BYTE byte_48A788[] = { 1, 1, 7, 32 };
const BYTE byte_48A790[] = {
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
const BYTE byte_48A7A8[] = {
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
const BYTE byte_48A7BC[] = { 1, 1, 9, 33 };
const BYTE byte_48A7C0[] = { 1, 1, 9, 34 };
const BYTE byte_48A7C4[] = { 1, 1, 9, 35 };
const BYTE byte_48A7C8[] = { 1, 1, 9, 36 };
const BYTE byte_48A7CC[] = { 1, 1, 9, 37 };
const BYTE byte_48A7D0[] = { 1, 1, 11, 38 };
const BYTE byte_48A7D4[] = { 1, 1, 10, 39 };
const BYTE byte_48A7D8[] = { 1, 1, 10, 40 };
const BYTE byte_48A7DC[] = { 1, 1, 10, 41 };
const BYTE byte_48A7E0[] = { 1, 1, 10, 42 };
const BYTE byte_48A7E4[] = { 1, 1, 10, 43 };
const BYTE byte_48A7E8[] = { 1, 1, 11, 44 };
const BYTE byte_48A7EC[] = { 1, 1, 9, 45 };
const BYTE byte_48A7F0[] = { 1, 1, 9, 46 };
const BYTE byte_48A7F4[] = { 1, 1, 10, 47 };
const BYTE byte_48A7F8[] = { 1, 1, 10, 48 };
const BYTE byte_48A7FC[] = { 1, 1, 11, 49 };
const BYTE byte_48A800[] = { 1, 1, 11, 50 };
const BYTE byte_48A808[] = {
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
const BYTE byte_48A820[] = {
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
const BYTE byte_48A838[] = {
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
const BYTE byte_48A850[] = {
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
const BYTE byte_48A868[] = {
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
const BYTE byte_48A880[] = {
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
const BYTE byte_48A898[] = {
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
const BYTE byte_48A8B0[] = {
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
const BYTE byte_48A8C8[] = {
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
const BYTE byte_48A8E0[] = {
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
const BYTE L6ISLE1[] = {
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
const BYTE L6ISLE2[] = {
	// clang-format off
	3, 2, // width, height

	 5,  2, 14, // search
	13, 10, 12,

	7, 7, 7, // replace
	7, 7, 7,
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
const BYTE L6ISLE5[] = {
	// clang-format off
	2, 2, // width, height

	 5, 14, // search
	13, 12,

	7, 7, // replace
	7, 7,
	// clang-format on
};
const BYTE byte_48A948[] = {
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
const BYTE byte_48A970[] = {
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
const BYTE byte_48A998[] = {
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
const BYTE byte_48A9C8[] = {
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

static BOOL DRLG_L3FillRoom(int x1, int y1, int x2, int y2)
{
	int i, j, v;

	if (x1 <= 1 || x2 >= DMAXX - 6 || y1 <= 1 || y2 >= DMAXY - 2) {
		return FALSE;
	}

	v = 0;
	for (j = y1; j <= y2; j++) {
		for (i = x1; i <= x2; i++) {
			v += dungeon[i][j];
		}
	}

	if (v != 0) {
		return FALSE;
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

	return TRUE;
}

static void DRLG_L3CreateBlock(int x, int y, int obs, int dir)
{
	int blksizex, blksizey, x1, y1, x2, y2;

	blksizex = RandRange(3, 4);
	blksizey = RandRange(3, 4);

	switch (dir) {
	case 0:
		y2 = y - 1;
		y1 = y2 - blksizey;
		x1 = x;
		if (blksizex < obs)
			x1 += random_(0, blksizex);
		else if (blksizex > obs)
			x1 -= random_(0, blksizex);
		x2 = blksizex + x1;
		break;
	case 1:
		x1 = x + 1;
		x2 = x1 + blksizex;
		y1 = y;
		if (blksizey < obs)
			y1 += random_(0, blksizey);
		else if (blksizey > obs)
			y1 -= random_(0, blksizey);
		y2 = y1 + blksizey;
		break;
	case 2:
		y1 = y + 1;
		y2 = y1 + blksizey;
		x1 = x;
		if (blksizex < obs)
			x1 += random_(0, blksizex);
		else if (blksizex > obs)
			x1 -= random_(0, blksizex);
		x2 = blksizex + x1;
		break;
	case 3:
		x2 = x - 1;
		x1 = x2 - blksizex;
		y1 = y;
		if (blksizey < obs)
			y1 += random_(0, blksizey);
		else if (blksizey > obs)
			y1 -= random_(0, blksizey);
		y2 = y1 + blksizey;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	if (DRLG_L3FillRoom(x1, y1, x2, y2) && random_(0, 4) != 0) {
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
			assert(dungeon[i][j] <= 1);
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
			assert(dungeon[i][j] <= 1);
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
			assert(dungeon[i][j] <= 1);
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

static void DRLG_L3River()
{
	int rx, ry, px, py, dir, pdir, nodir, nodir2, dircheck;
	int river[3][100];
	int rivercnt, riveramt;
	int i, tries, found, bridge, lpcnt;
	BOOL done;

	rivercnt = 0;
	tries = 0;

	while (tries < 200 && rivercnt < 4) {
		done = FALSE;
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
					if (dir == 0 && pdir == 2 || dir == 3 && pdir == 1) {
						if (riveramt > 2) {
							river[2][riveramt - 2] = 22;
						}
						nodir2 = dir == 0 ? 1 : 2;
					}
					if (dir == 0 && pdir == 3 || dir == 2 && pdir == 1) {
						if (riveramt > 2) {
							river[2][riveramt - 2] = 21;
						}
						nodir2 = dir + 1; // dir == 0 ? 1 : 3;
					}
					if (dir == 1 && pdir == 2 || dir == 3 && pdir == 0) {
						if (riveramt > 2) {
							river[2][riveramt - 2] = 20;
						}
						nodir2 = dir - 1; // dir == 1 ? 0 : 2;
					}
					if (dir == 1 && pdir == 3 || dir == 2 && pdir == 0) {
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
					done = TRUE;
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
					done = TRUE;
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
					done = TRUE;
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
					done = TRUE;
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

static BOOL DRLG_L3Spawn(int x, int y, int *totarea);

static BOOL DRLG_L3SpawnEdge(int x, int y, int *totarea)
{
	BYTE i;
	static BYTE spawntable[15] = { 0x00, 0x0A, 0x43, 0x05, 0x2C, 0x06, 0x09, 0x00, 0x00, 0x1C, 0x83, 0x06, 0x09, 0x0A, 0x05 };

	if (*totarea > 40) {
		return TRUE;
	}
	if (x < 0 || y < 0 || x >= DMAXX || y >= DMAXY) {
		return TRUE;
	}
	if (dungeon[x][y] & 0x80) {
		return FALSE;
	}
	if (dungeon[x][y] > 15) {
		return TRUE;
	}

	i = spawntable[dungeon[x][y]];
	dungeon[x][y] |= 0x80;
	*totarea += 1;

	if (i & 8 && DRLG_L3SpawnEdge(x, y - 1, totarea)) {
		return TRUE;
	}
	if (i & 4 && DRLG_L3SpawnEdge(x, y + 1, totarea)) {
		return TRUE;
	}
	if (i & 2 && DRLG_L3SpawnEdge(x + 1, y, totarea)) {
		return TRUE;
	}
	if (i & 1 && DRLG_L3SpawnEdge(x - 1, y, totarea)) {
		return TRUE;
	}
	if (i & 0x80 && DRLG_L3Spawn(x, y - 1, totarea)) {
		return TRUE;
	}
	if (i & 0x40 && DRLG_L3Spawn(x, y + 1, totarea)) {
		return TRUE;
	}
	if (i & 0x20 && DRLG_L3Spawn(x + 1, y, totarea)) {
		return TRUE;
	}
	if (i & 0x10 && DRLG_L3Spawn(x - 1, y, totarea)) {
		return TRUE;
	}

	return FALSE;
}

static BOOL DRLG_L3Spawn(int x, int y, int *totarea)
{
	BYTE i;
	static BYTE spawntable[15] = { 0x00, 0x0A, 0x03, 0x05, 0x0C, 0x06, 0x09, 0x00, 0x00, 0x0C, 0x03, 0x06, 0x09, 0x0A, 0x05 };

	if (*totarea > 40) {
		return TRUE;
	}
	if (x < 0 || y < 0 || x >= DMAXX || y >= DMAXY) {
		return TRUE;
	}
	if (dungeon[x][y] & 0x80) {
		return FALSE;
	}
	if (dungeon[x][y] > 15) {
		return TRUE;
	}

	i = dungeon[x][y];
	dungeon[x][y] |= 0x80;
	*totarea += 1;

	if (i != 8) {
		i = spawntable[i];
		if (i & 8 && DRLG_L3SpawnEdge(x, y - 1, totarea)) {
			return TRUE;
		}
		if (i & 4 && DRLG_L3SpawnEdge(x, y + 1, totarea)) {
			return TRUE;
		}
		if (i & 2 && DRLG_L3SpawnEdge(x + 1, y, totarea)) {
			return TRUE;
		}
		if (i & 1 && DRLG_L3SpawnEdge(x - 1, y, totarea)) {
			return TRUE;
		}
	} else {
		if (DRLG_L3Spawn(x + 1, y, totarea)) {
			return TRUE;
		}
		if (DRLG_L3Spawn(x - 1, y, totarea)) {
			return TRUE;
		}
		if (DRLG_L3Spawn(x, y + 1, totarea)) {
			return TRUE;
		}
		if (DRLG_L3Spawn(x, y - 1, totarea)) {
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * Flood fills dirt and wall tiles looking for
 * an area of at most 40 tiles and disconnected from the map edge.
 * If it finds one, converts it to lava tiles and sets lavapool to TRUE.
 */
static void DRLG_L3Pool()
{
	int i, j, dunx, duny, totarea;
	BOOL notFound, addpool;
	BYTE k;
	static BYTE poolsub[15] = { 0, 35, 26, 36, 25, 29, 34, 7, 33, 28, 27, 37, 32, 31, 30 };

	for (duny = 1; duny < DMAXY - 1; duny++) {
		for (dunx = 1; dunx < DMAXY - 1; dunx++) {
			if (dungeon[dunx][duny] != 8) {
				continue;
			}
			dungeon[dunx][duny] |= 0x80;
			totarea = 1;
			notFound = !DRLG_L3Spawn(dunx + 1, duny, &totarea)
				&& !DRLG_L3Spawn(dunx - 1, duny, &totarea)
				&& !DRLG_L3Spawn(dunx, duny + 1, &totarea)
				&& !DRLG_L3Spawn(dunx, duny - 1, &totarea);
			addpool = random_(0, 100) < 25 && totarea > 4 && notFound;
			for (j = std::max(duny - totarea, 0); j < std::min(duny + totarea, DMAXY); j++) {
				for (i = std::max(dunx - totarea, 0); i < std::min(dunx + totarea, DMAXX); i++) {
					// BUGFIX: In the following swap the order to first do the
					// index checks and only then access dungeon[i][j] (fixed)
					if (dungeon[i][j] & 0x80) {
						dungeon[i][j] &= ~0x80;
						if (addpool) {
							k = poolsub[dungeon[i][j]];
							if (k != 0 && k <= 37) {
								dungeon[i][j] = k;
							}
							lavapool = TRUE;
						}
					}
				}
			}
		}
	}
}

static void DRLG_L3PoolFix()
{
	int dunx, duny;

	for (duny = 1; duny < DMAXY - 1; duny++) {     // BUGFIX: Change '0' to '1' and 'DMAXY' to 'DMAXY - 1' (fixed)
		for (dunx = 1; dunx < DMAXX - 1; dunx++) { // BUGFIX: Change '0' to '1' and 'DMAXX' to 'DMAXX - 1' (fixed)
			if (dungeon[dunx][duny] == 8) {
				if (dungeon[dunx - 1][duny - 1] >= 25 && dungeon[dunx - 1][duny - 1] <= 41
				    && dungeon[dunx - 1][duny] >= 25 && dungeon[dunx - 1][duny] <= 41
				    && dungeon[dunx - 1][duny + 1] >= 25 && dungeon[dunx - 1][duny + 1] <= 41
				    && dungeon[dunx][duny - 1] >= 25 && dungeon[dunx][duny - 1] <= 41
				    && dungeon[dunx][duny + 1] >= 25 && dungeon[dunx][duny + 1] <= 41
				    && dungeon[dunx + 1][duny - 1] >= 25 && dungeon[dunx + 1][duny - 1] <= 41
				    && dungeon[dunx + 1][duny] >= 25 && dungeon[dunx + 1][duny] <= 41
				    && dungeon[dunx + 1][duny + 1] >= 25 && dungeon[dunx + 1][duny + 1] <= 41) {
					dungeon[dunx][duny] = 33;
				}
			}
		}
	}
}

static BOOL DRLG_L3PlaceMiniSet(const BYTE *miniset, BOOL setview)
{
	int sx, sy, sw, sh, xx, yy, ii, tries;
	BOOL done;

	sw = miniset[0];
	sh = miniset[1];

	sx = random_(0, DMAXX - sw);
	sy = random_(0, DMAXY - sh);

	tries = 0;
	while (TRUE) {
		done = TRUE;
		ii = 2;
		for (yy = sy; yy < sy + sh && done; yy++) {
			for (xx = sx; xx < sx + sw && done; xx++) {
				if (miniset[ii] != 0 && dungeon[xx][yy] != miniset[ii]) {
					done = FALSE;
				}
				if (dflags[xx][yy] != 0) {
					done = FALSE;
				}
				ii++;
			}
		}
		tries++;
		if (done || tries == 200)
			break;
		if (++sx == DMAXX - sw) {
			sx = 0;
			if (++sy == DMAXY - sh) {
				sy = 0;
			}
		}
	}
	if (tries == 200)
		return FALSE;

	ii = sw * sh + 2;
	for (yy = sy; yy < sy + sh; yy++) {
		for (xx = sx; xx < sx + sw; xx++) {
			if (miniset[ii] != 0) {
				dungeon[xx][yy] = miniset[ii];
			}
			ii++;
		}
	}

	if (setview) {
		ViewX = 2 * sx + DBORDERX + 1;
		ViewY = 2 * sy + DBORDERY + 3;
	}

	return TRUE;
}

static void DRLG_L3PlaceRndSet(const BYTE *miniset, int rndper)
{
	int sx, sy, sw, sh, xx, yy, ii, kk;
	BOOL found;

	sw = miniset[0];
	sh = miniset[1];

	for (sy = 0; sy < DMAXX - sh; sy++) {
		for (sx = 0; sx < DMAXY - sw; sx++) {
			found = TRUE;
			ii = 2;
			for (yy = sy; yy < sy + sh && found; yy++) {
				for (xx = sx; xx < sx + sw && found; xx++) {
					if (miniset[ii] != 0 && dungeon[xx][yy] != miniset[ii]) {
						found = FALSE;
					}
					if (dflags[xx][yy] != 0) {
						found = FALSE;
					}
					ii++;
				}
			}
			kk = sw * sh + 2;
			if (miniset[kk] >= 84 && miniset[kk] <= 100 && found) {
				// BUGFIX: accesses to dungeon can go out of bounds (fixed)
				// BUGFIX: Comparisons vs 100 should use same tile as comparisons vs 84.
				if (sx - 1 >= 0 && dungeon[sx - 1][sy] >= 84 && dungeon[sx - 1][sy] <= 100) {
					found = FALSE;
				}
				if (sx + 1 < 40 && sx - 1 >= 0 && dungeon[sx + 1][sy] >= 84 && dungeon[sx - 1][sy] <= 100) {
					found = FALSE;
				}
				if (sy + 1 < 40 && sx - 1 >= 0 && dungeon[sx][sy + 1] >= 84 && dungeon[sx - 1][sy] <= 100) {
					found = FALSE;
				}
				if (sy - 1 >= 0 && sx - 1 >= 0 && dungeon[sx][sy - 1] >= 84 && dungeon[sx - 1][sy] <= 100) {
					found = FALSE;
				}
			}
			if (found && random_(0, 100) < rndper) {
				for (yy = sy; yy < sy + sh; yy++) {
					for (xx = sx; xx < sx + sw; xx++) {
						if (miniset[kk] != 0) {
							dungeon[xx][yy] = miniset[kk];
						}
						kk++;
					}
				}
			}
		}
	}
}

#ifdef HELLFIRE
static BOOLEAN DRLG_L6Hive_rnd_piece(const BYTE *miniset, int rndper)
{
	int sx, sy, sw, sh, xx, yy, ii, kk;
	BOOL found;
	BOOLEAN placed;

	placed = FALSE;
	sw = miniset[0];
	sh = miniset[1];

	for (sy = 0; sy < DMAXX - sh; sy++) {
		for (sx = 0; sx < DMAXY - sw; sx++) {
			found = TRUE;
			ii = 2;
			for (yy = sy; yy < sy + sh && found; yy++) {
				for (xx = sx; xx < sx + sw && found; xx++) {
					if (miniset[ii] != 0 && dungeon[xx][yy] != miniset[ii]) {
						found = FALSE;
					}
					if (dflags[xx][yy] != 0) {
						found = FALSE;
					}
					ii++;
				}
			}
			kk = sw * sh + 2;
			if (miniset[kk] >= 84 && miniset[kk] <= 100 && found) {
				// BUGFIX: accesses to dungeon can go out of bounds
				// BUGFIX: Comparisons vs 100 should use same tile as comparisons vs 84.
				if (dungeon[sx - 1][sy] >= 84 && dungeon[sx - 1][sy] <= 100) {
					found = FALSE;
				}
				if (dungeon[sx + 1][sy] >= 84 && dungeon[sx - 1][sy] <= 100) {
					found = FALSE;
				}
				if (dungeon[sx][sy + 1] >= 84 && dungeon[sx - 1][sy] <= 100) {
					found = FALSE;
				}
				if (dungeon[sx][sy - 1] >= 84 && dungeon[sx - 1][sy] <= 100) {
					found = FALSE;
				}
			}
			if (found && random_(0, 100) < rndper) {
				placed = TRUE;
				for (yy = sy; yy < sy + sh; yy++) {
					for (xx = sx; xx < sx + sw; xx++) {
						if (miniset[kk] != 0) {
							dungeon[xx][yy] = miniset[kk];
						}
						kk++;
					}
				}
			}
		}
	}

	return placed;
}
#endif

static BOOL WoodVertU(int i, int y)
{
	BYTE bv;

	bv = dungeon[i + 1][y];
	if (bv >= 130 && bv <= 152)
		return FALSE;
	bv = dungeon[i - 1][y];
	if (bv >= 130 && bv <= 152)
		return FALSE;
	bv = dungeon[i][y];
	return bv == 7 || bv == 10 || bv == 126 || bv == 129 || bv == 134 || bv == 136;
}

static BOOL WoodVertD(int i, int y)
{
	BYTE bv;

	bv = dungeon[i + 1][y];
	if (bv >= 130 && bv <= 152)
		return FALSE;
	bv = dungeon[i - 1][y];
	if (bv >= 130 && bv <= 152)
		return FALSE;
	bv = dungeon[i][y];
	return bv == 7 || bv == 2 || bv == 134 || bv == 136;
}

static BOOL WoodHorizL(int x, int j)
{
	BYTE bv;

	bv = dungeon[x][j + 1];
	if (bv >= 130 && bv <= 152)
		return FALSE;
	bv = dungeon[x][j - 1];
	if (bv >= 130 && bv <= 152)
		return FALSE;
	bv = dungeon[x][j];
	return bv == 7 || bv == 9 || bv == 121 || bv == 124 || bv == 135 || bv == 137;
}

static BOOL WoodHorizR(int x, int j)
{
	BYTE bv;

	bv = dungeon[x][j + 1];
	if (bv >= 130 && bv <= 152)
		return FALSE;
	bv = dungeon[x][j - 1];
	if (bv >= 130 && bv <= 152)
		return FALSE;
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

static BOOL DRLG_L3Anvil()
{
	int sx, sy, sw, sh, xx, yy, ii, tries;
	BOOL done;

	sw = L3ANVIL[0];
	sh = L3ANVIL[1];
	sx = random_(0, DMAXX - sw);
	sy = random_(0, DMAXY - sh);

	tries = 0;
	while (TRUE) {
		done = TRUE;
		ii = 2;
		for (yy = sy; yy < sy + sh && done; yy++) {
			for (xx = sx; xx < sx + sw  && done; xx++) {
				if (L3ANVIL[ii] != 0 && dungeon[xx][yy] != L3ANVIL[ii]) {
					done = FALSE;
				}
				if (dflags[xx][yy] != 0) {
					done = FALSE;
				}
				ii++;
			}
		}
		tries++;
		if (done || tries == 200)
			break;
		if (++sx == DMAXX - sw) {
			sx = 0;
			if (++sy == DMAXY - sh) {
				sy = 0;
			}
		}
	}
	if (tries == 200)
		return FALSE;

	ii = sw * sh + 2;
	for (yy = sy; yy < sy + sh; yy++) {
		for (xx = sx; xx < sx + sw; xx++) {
			if (L3ANVIL[ii] != 0) {
				dungeon[xx][yy] = L3ANVIL[ii];
			}
			dflags[xx][yy] |= DLRG_PROTECTED;
			ii++;
		}
	}

	setpc_x = sx;
	setpc_y = sy;
	setpc_w = sw;
	setpc_h = sh;

	return TRUE;
}

static void FixL3Warp()
{
	int i, j;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (dungeon[i][j] == 125 && dungeon[i + 1][j] == 125 && dungeon[i][j + 1] == 125 && dungeon[i + 1][j + 1] == 125) {
				dungeon[i][j] = 156;
				dungeon[i + 1][j] = 155;
				dungeon[i][j + 1] = 153;
				dungeon[i + 1][j + 1] = 154;
				return;
			}
			if (dungeon[i][j] == 5 && dungeon[i + 1][j + 1] == 7) {
				dungeon[i][j] = 7;
			}
		}
	}
}

static void FixL3HallofHeroes()
{
	int i, j;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
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
	if (!lockout[x][y]) {
		return;
	}

	lockout[x][y] = FALSE;
	lockoutcnt++;
	DRLG_L3LockRec(x, y - 1);
	DRLG_L3LockRec(x, y + 1);
	DRLG_L3LockRec(x - 1, y);
	DRLG_L3LockRec(x + 1, y);
}

static BOOL DRLG_L3Lockout()
{
	int i, j, t, fx, fy;

	t = 0;
	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (dungeon[i][j] != 0) {
				lockout[i][j] = TRUE;
				fx = i;
				fy = j;
				t++;
			} else {
				lockout[i][j] = FALSE;
			}
		}
	}

	lockoutcnt = 0;
	DRLG_L3LockRec(fx, fy);

	return t == lockoutcnt;
}

struct mini_set {
	const BYTE* data;
	BOOL setview;
};
static BOOL DRLG_L3PlaceMiniSets(mini_set* minisets, int n)
{
	int i;

	for (i = 0; i < n; i++) {
		if (minisets[i].data != NULL && !DRLG_L3PlaceMiniSet(minisets[i].data, minisets[i].setview)) {
			return FALSE;
		}
	}
	return TRUE;
}

static void DRLG_L3(int entry)
{
	int x1, y1, x2, y2;
	BOOL doneflag;

	lavapool = FALSE;
	do {
		do {
			do {
				InitL3Dungeon();
				x1 = RandRange(10, 29);
				y1 = RandRange(10, 29);
				x2 = x1 + 2;
				y2 = y1 + 2;
				DRLG_L3FillRoom(x1, y1, x2, y2);
				DRLG_L3CreateBlock(x1, y1, 2, 0);
				DRLG_L3CreateBlock(x2, y1, 2, 1);
				DRLG_L3CreateBlock(x1, y2, 2, 2);
				DRLG_L3CreateBlock(x1, y1, 2, 3);
				if (QuestStatus(Q_ANVIL)) {
					x1 = RandRange(10, 19);
					y1 = RandRange(10, 19);
					x2 = x1 + 12;
					y2 = y1 + 12;
					DRLG_L3FloorArea(x1, y1, x2, y2);
				}
				DRLG_L3FillDiags();
				DRLG_L3FillSingles();
				DRLG_L3FillStraights();
				DRLG_L3FillDiags();
				DRLG_L3Edges();
			} while (DRLG_L3GetFloorArea() < 600 || !DRLG_L3Lockout());
			DRLG_L3MakeMegas();
#ifdef HELLFIRE
			if (currlevel >= 17) {
				mini_set stairs[2] = {
					{ currlevel != 17 ? L6USTAIRS : L6TWARP, entry != ENTRY_PREV },
					{ currlevel != 20 ? L6DSTAIRS : NULL, entry == ENTRY_PREV }
				};
				doneflag = DRLG_L3PlaceMiniSets(stairs, 2);
				if (entry == ENTRY_PREV) {
					ViewX += 2;
					ViewY -= 2;
				}
			} else
#endif
			{
				mini_set stairs[3] = {
					{ L3USTAIRS,  entry == ENTRY_MAIN },
					{ L3DSTAIRS, entry == ENTRY_PREV },
					{ currlevel != 9 ? NULL : L3TWARP, entry != ENTRY_MAIN  && entry != ENTRY_PREV },
				};
				doneflag = DRLG_L3PlaceMiniSets(stairs, 3);
				if (entry == ENTRY_PREV) {
					ViewX += 2;
					ViewY -= 2;
				}
			}
			if (doneflag && QuestStatus(Q_ANVIL)) {
				doneflag = DRLG_L3Anvil();
			}
		} while (!doneflag);
#ifdef HELLFIRE
		if (currlevel >= 17) {
			lavapool += DRLG_L6Hive_rnd_piece(byte_48A998, 30);
			lavapool += DRLG_L6Hive_rnd_piece(byte_48A9C8, 40);
			lavapool += DRLG_L6Hive_rnd_piece(byte_48A948, 50);
			lavapool += DRLG_L6Hive_rnd_piece(byte_48A970, 60);
			if (lavapool < 3)
				lavapool = FALSE;
		} else
#endif
			DRLG_L3Pool();
	} while (!lavapool);

#ifdef HELLFIRE
	if (currlevel >= 17) {
		DRLG_L3PlaceRndSet(L6ISLE1, 70);
		DRLG_L3PlaceRndSet(L6ISLE2, 70);
		DRLG_L3PlaceRndSet(L6ISLE3, 30);
		DRLG_L3PlaceRndSet(L6ISLE4, 30);
		DRLG_L3PlaceRndSet(L6ISLE1, 100);
		DRLG_L3PlaceRndSet(L6ISLE2, 100);
		DRLG_L3PlaceRndSet(L6ISLE5, 90);
	} else
#endif
	{
		DRLG_L3PoolFix();
		FixL3Warp();

		DRLG_L3PlaceRndSet(L3ISLE1, 70);
		DRLG_L3PlaceRndSet(L3ISLE2, 70);
		DRLG_L3PlaceRndSet(L3ISLE3, 30);
		DRLG_L3PlaceRndSet(L3ISLE4, 30);
		DRLG_L3PlaceRndSet(L3ISLE1, 100);
		DRLG_L3PlaceRndSet(L3ISLE2, 100);
		DRLG_L3PlaceRndSet(L3ISLE5, 90);

		FixL3HallofHeroes();
		DRLG_L3River();
	}

	if (QuestStatus(Q_ANVIL)) {
		dungeon[setpc_x + 7][setpc_y + 5] = 7;
		dungeon[setpc_x + 8][setpc_y + 5] = 7;
		dungeon[setpc_x + 9][setpc_y + 5] = 7;
		if (dungeon[setpc_x + 10][setpc_y + 5] == 17 || dungeon[setpc_x + 10][setpc_y + 5] == 18) {
			dungeon[setpc_x + 10][setpc_y + 5] = 45;
		}
	}

#ifdef HELLFIRE
	if (currlevel >= 17) {
		DRLG_L3PlaceRndSet(byte_48A76C, 20);
		DRLG_L3PlaceRndSet(byte_48A770, 20);
		DRLG_L3PlaceRndSet(byte_48A774, 20);
		DRLG_L3PlaceRndSet(byte_48A778, 20);
		DRLG_L3PlaceRndSet(byte_48A808, 10);
		DRLG_L3PlaceRndSet(byte_48A820, 15);
		DRLG_L3PlaceRndSet(byte_48A838, 20);
		DRLG_L3PlaceRndSet(byte_48A850, 25);
		DRLG_L3PlaceRndSet(byte_48A868, 30);
		DRLG_L3PlaceRndSet(byte_48A880, 35);
		DRLG_L3PlaceRndSet(byte_48A898, 40);
		DRLG_L3PlaceRndSet(byte_48A8B0, 45);
		DRLG_L3PlaceRndSet(byte_48A8C8, 50);
		DRLG_L3PlaceRndSet(byte_48A8E0, 55);
		DRLG_L3PlaceRndSet(byte_48A8E0, 10);
		DRLG_L3PlaceRndSet(byte_48A8C8, 15);
		DRLG_L3PlaceRndSet(byte_48A8B0, 20);
		DRLG_L3PlaceRndSet(byte_48A898, 25);
		DRLG_L3PlaceRndSet(byte_48A880, 30);
		DRLG_L3PlaceRndSet(byte_48A868, 35);
		DRLG_L3PlaceRndSet(byte_48A850, 40);
		DRLG_L3PlaceRndSet(byte_48A838, 45);
		DRLG_L3PlaceRndSet(byte_48A820, 50);
		DRLG_L3PlaceRndSet(byte_48A808, 55);
		DRLG_L3PlaceRndSet(byte_48A790, 40);
		DRLG_L3PlaceRndSet(byte_48A7A8, 45);
		DRLG_L3PlaceRndSet(byte_48A77C, 25);
		DRLG_L3PlaceRndSet(byte_48A780, 25);
		DRLG_L3PlaceRndSet(byte_48A784, 25);
		DRLG_L3PlaceRndSet(byte_48A788, 25);
		DRLG_L3PlaceRndSet(byte_48A7BC, 25);
		DRLG_L3PlaceRndSet(byte_48A7C0, 25);
		DRLG_L3PlaceRndSet(byte_48A7C4, 25);
		DRLG_L3PlaceRndSet(byte_48A7C8, 25);
		DRLG_L3PlaceRndSet(byte_48A7CC, 25);
		DRLG_L3PlaceRndSet(byte_48A7D4, 25);
		DRLG_L3PlaceRndSet(byte_48A7D8, 25);
		DRLG_L3PlaceRndSet(byte_48A7DC, 25);
		DRLG_L3PlaceRndSet(byte_48A7E0, 25);
		DRLG_L3PlaceRndSet(byte_48A7E4, 25);
		DRLG_L3PlaceRndSet(byte_48A7EC, 25);
		DRLG_L3PlaceRndSet(byte_48A7F0, 25);
		DRLG_L3PlaceRndSet(byte_48A7F4, 25);
		DRLG_L3PlaceRndSet(byte_48A7F8, 25);
		DRLG_L3PlaceRndSet(byte_48A7D0, 25);
		DRLG_L3PlaceRndSet(byte_48A7E8, 25);
		DRLG_L3PlaceRndSet(byte_48A7FC, 25);
		DRLG_L3PlaceRndSet(byte_48A800, 25);
	} else
#endif
	{
		DRLG_PlaceThemeRooms(5, 10, 7, 0, FALSE);

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
		DRLG_L3PlaceRndSet(L3XTRA1, 25);
		DRLG_L3PlaceRndSet(L3XTRA2, 25);
		DRLG_L3PlaceRndSet(L3XTRA3, 25);
		DRLG_L3PlaceRndSet(L3XTRA4, 25);
		DRLG_L3PlaceRndSet(L3XTRA5, 25);
	}

	memcpy(pdungeon, dungeon, sizeof(pdungeon));

	DRLG_Init_Globals();
}

void CreateL3Dungeon(DWORD rseed, int entry)
{
	int i, j, pn;

	SetRndSeed(rseed);

	DRLG_InitTrans();
	DRLG_InitSetPC();
	DRLG_L3(entry);
	DRLG_PlaceMegaTiles(BASE_MEGATILE_L3);

#ifdef HELLFIRE
	if (currlevel >= 17) {
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

	DRLG_SetPC();
}

static BYTE *LoadL3DungeonData(const char *sFileName)
{
	int i, j, rw, rh;
	BYTE *pLevelMap, *lm, *pTmp;

	InitL3Dungeon();

	DRLG_InitTrans();
	pLevelMap = LoadFileInMem(sFileName, NULL);

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
	int i, j;
	BYTE *pLevelMap = LoadL3DungeonData(sFileName);

	DRLG_PlaceMegaTiles(BASE_MEGATILE_L3);
	DRLG_Init_Globals();
	ViewX = vx;
	ViewY = vy;
	SetMapMonsters(pLevelMap, 0, 0);
	SetMapObjects(pLevelMap, 0, 0);

	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (dPiece[i][j] >= 56 && dPiece[i][j] <= 147) {
				DoLighting(i, j, 7, -1);
			} else if (dPiece[i][j] >= 154 && dPiece[i][j] <= 161) {
				DoLighting(i, j, 7, -1);
			} else if (dPiece[i][j] == 150) {
				DoLighting(i, j, 7, -1);
			} else if (dPiece[i][j] == 152) {
				DoLighting(i, j, 7, -1);
			}
		}
	}

	mem_free_dbg(pLevelMap);
}

void LoadPreL3Dungeon(const char *sFileName)
{
	BYTE *pLevelMap = LoadL3DungeonData(sFileName);

	memcpy(pdungeon, dungeon, sizeof(pdungeon));
	mem_free_dbg(pLevelMap);
}

DEVILUTION_END_NAMESPACE
