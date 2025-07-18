/**
 * @file drlg_l3.cpp
 *
 * Implementation of the caves level generation algorithms.
 *
 * drlgFlags matrix is used as a BOOLEAN matrix to protect the quest room.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** The default external tile. */
#define BASE_MEGATILE_L3 8
/** The default floor tile. */
#define DEFAULT_MEGATILE_L3 7
/** The required number of lava pools.*/
#define MIN_LAVA_POOL 3
/** Helper variable to check if sufficient number of lava pools have been generated */
unsigned _guLavapools;

/* Tiles to build the theme rooms. */
const BYTE themeTiles[NUM_DRT_TYPES] = { DEFAULT_MEGATILE_L3, 135, 134, 147, 146, 150, 144, 145, 138 };
/**
 * A lookup table for the 16 possible patterns of a 2x2 area,
 * where each cell either contains a SW wall or it doesn't.
 */
const BYTE L3ConvTbl[16] = { BASE_MEGATILE_L3, 11, 3, 10, 1, 9, 12, 12, 6, 13, 4, 13, 2, 14, 5, 7 };

/** Miniset: Entry point of the dynamic maps. */
const BYTE L3DYNENTRY[] = {
	// clang-format off
	2, 2, // width, height

	9, 7, // search
	9, 7,

	109, 0, // replace
	0, 0,
	// clang-format on
};
#ifdef HELLFIRE
const BYTE L6DYNENTRY[] = {
	// clang-format off
	2, 2, // width, height

	9, 7, // search
	9, 7,

	15, 0, // replace
	0, 0,
	// clang-format on
};
#endif
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

	111, 47, 0, // replace
	  0, 46, 0,
	  0,  0, 0,
	// clang-format on
};
#ifdef HELLFIRE
const BYTE L6DSTAIRS[] = {
	// clang-format off
	3, 3, // width, height

	8, 9, 7, // search
	8, 9, 7,
	0, 0, 0,

	23, 16, 0, // replace
	 0, 15, 0,
	 0,  0, 0,
	// clang-format on
};
#endif
/** Miniset: Stairs up to town. */
/*const BYTE L3TWARP[] = {
	// clang-format off
	3, 3, // width, height

	 8,  8, 0, // search
	10, 10, 0,
	 7,  7, 0,

	156, 155, 0, // replace
	153, 154, 0,
	  0,   0, 0,
	// clang-format on
};*/
/* same look as L6USTAIRS
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
#endif*/
/*
 * Maps tile IDs to their corresponding undecorated tile type.
 */
const BYTE L3BTYPES[157] = {
	// clang-format off
	0, 0, 0, 0, 0, 0, 0, 1, 0, 2,
	3, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 10..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 20..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 30..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 40..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 50..
	0, 0, 0, 0, 0, 0, 0, 0, 2, 3, // 60..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 70..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 80..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 90..
	0, 0, 0, 0, 0, 0, 1, 1, 1, 2, //100..
	3, 0, 0, 0, 0, 0, 0, 0, 0, 0, //110..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //120..
	0, 0, 0, 0, 4, 5, 4, 5, 0, 0, //130..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //140..
	0, 0, 0, 0, 0, 0, 0           //150..
	// clang-format on
};
#ifdef HELLFIRE
/*
 * Maps tile IDs to their corresponding undecorated tile type.
 */
const BYTE L6BTYPES[167] = {
	// clang-format off
	0, 0, 0, 0, 0, 0, 0, 1, 2, 3,
	4, 5, 0, 0, 0, 0, 0, 0, 0, 0, // 10..
	0, 0, 0, 0, 0, 2, 2, 2, 2, 1, // 20..
	0, 0, 0, 3, 3, 3, 3, 3, 5, 4, // 30..
	4, 4, 4, 4, 5, 3, 3, 4, 4, 5, // 40..
	5, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 50..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 60..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 70..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 80..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 90..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //100..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //110..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //120..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //130..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //140..
	0, 0, 0, 0, 0, 0, 0           //150..
	// clang-format on
};
#endif
/** Miniset: Stalagmite white stalactite 1. */
const BYTE L3TITEBASE[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	57, 58, 0, // replace
	56, 55, 0,
	 0,  0, 0,
	// clang-format on
};
/** Miniset: Stalagmite white stalactite 2. */
const BYTE L3TITE2[] = {
	// clang-format off
	2, 2, // width, height

	57, 58, // search
	56, 55,

	61, 62, // replace
	60, 59,
	// clang-format on
};
/** Miniset: Stalagmite white stalactite 3. */
const BYTE L3TITE3[] = {
	// clang-format off
	2, 2, // width, height

	57, 58, // search
	56, 55,

	65, 66, // replace
	64, 63,
	// clang-format on
};
/** Miniset: Stalagmite 1. */
const BYTE L3TITE8[] = {
	// clang-format off
	2, 2, // width, height

	57, 58, // search
	56, 55,

	7,  7, // replace
	7, 52,
	// clang-format on
};
/** Miniset: Stalagmite 2. */
const BYTE L3TITE9[] = {
	// clang-format off
	2, 2, // width, height

	57, 58, // search
	56, 55,

	7,  7, // replace
	7, 53,
	// clang-format on
};
/** Miniset: Stalagmite 3. */
const BYTE L3TITE10[] = {
	// clang-format off
	2, 2, // width, height

	57, 58, // search
	56, 55,

	7,  7, // replace
	7, 54,
	// clang-format on
};
/** Miniset: Stalagmite 4. */
const BYTE L3TITE11[] = {
	// clang-format off
	2, 2, // width, height

	57, 58, // search
	56, 55,

	7,  7, // replace
	7, 67,
	// clang-format on
};
/** Miniset: Stalagmite white stalactite horizontal. */
/*const BYTE L3LTITE1[] = {
	// clang-format off
	4, 3, // width, height

	7, 7, 0, 7, // search
	7, 7, 7, 7,
	7, 7, 7, 7,

	77, 78,  0, 0, // replace
	76, 74, 75, 0,
	 0,  0,  0, 0,
	// clang-format on
};*/
/** Miniset: Stalagmite white stalactite vertical. */
/*const BYTE L3LTITE2[] = {
	// clang-format off
	3, 4, // width, height

	7, 0, 7, // search
	7, 7, 7,
	7, 7, 7,
	7, 7, 7,

	83,  0, 0, // replace
	82, 80, 0,
	81, 79, 0,
	 0,  0, 0,
	// clang-format on
};*/
/** Miniset: Cracked vertical wall - north-west I. */
const BYTE L3CREV1[] = {
	// clang-format off
	2, 1, // width, height

	8, 9, // search

	84, 85, // replace
	// clang-format on
};
/** Miniset: Cracked vertical wall - north I. */
const BYTE L3CREV2[] = {
	// clang-format off
	2, 1, // width, height

	8, 11, // search

	86, 87, // replace
	// clang-format on
};
/** Miniset: Cracked horizontal wall - north-east I. */
const BYTE L3CREV3[] = {
	// clang-format off
	1, 2, // width, height

	 8, // search
	10,

	89, // replace
	88,
	// clang-format on
};
/** Miniset: Cracked vertical wall - north-west II. */
const BYTE L3CREV4[] = {
	// clang-format off
	2, 1, // width, height

	8, 9, // search

	90, 91, // replace
	// clang-format on
};
/** Miniset: Cracked horizontal wall - north II. */
const BYTE L3CREV5[] = {
	// clang-format off
	1, 2, // width, height

	 8, // search
	11,

	92, // replace
	93,
	// clang-format on
};
/** Miniset: Cracked horizontal wall - north-east II. */
const BYTE L3CREV6[] = {
	// clang-format off
	1, 2, // width, height

	 8, // search
	10,

	95, // replace
	94,
	// clang-format on
};
/** Miniset: Cracked vertical wall - west */
const BYTE L3CREV7[] = {
	// clang-format off
	2, 1, // width, height

	8, 1, // search

	96, 101, // replace
	// clang-format on
};
/** Miniset: Cracked horizontal wall - south-west */
const BYTE L3CREV8[] = {
	// clang-format off
	1, 2, // width, height

	2, // search
	8,

	102, // replace
	 97,
	// clang-format on
};
/** Miniset: Cracked vertical wall - east */
const BYTE L3CREV9[] = {
	// clang-format off
	2, 1, // width, height

	3, 8, // search

	103, 98, // replace
	// clang-format on
};
/** Miniset: Cracked vertical wall - south-east */
const BYTE L3CREV10[] = {
	// clang-format off
	2, 1, // width, height

	4, 8, // search

	104, 99, // replace
	// clang-format on
};
/** Miniset: Cracked horizontal wall - south */
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
const BYTE L3VERTWALLFIX1[] = {
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
const BYTE L3HORZWALLFIX1[] = {
	// clang-format off
	3, 2, // width, height

	 5,  2, 14, // search
	13, 10, 12,

	7, 7, 7, // replace
	7, 7, 7,
	// clang-format on
};
/** Miniset: Replace small wall with lava 1. */
const BYTE L3VERTWALLFIX2[] = {
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
const BYTE L3HORZWALLFIX2[] = {
	// clang-format off
	3, 2, // width, height

	 5,  2, 14, // search
	13, 10, 12,

	29, 26, 30, // replace
	31, 27, 32,
	// clang-format on
};
/** Miniset: Replace small wall with floor 3. */
/*const BYTE L3ISLE5[] = {
	// clang-format off
	2, 2, // width, height

	 5, 14, // search
	13, 12,

	7, 7, // replace
	7, 7,
	// clang-format on
};*/
#ifdef HELLFIRE
/** Miniset: Small acid puddle 1. */
const BYTE L6PUDDLE1[] = {
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
/** Miniset: Small acid puddle 2. */
const BYTE L6PUDDLE2[] = {
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
/** Miniset: Small acid puddle 3. */
/*const BYTE L6PUDDLE3[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	0,   0, 0, // replace
	0, 145, 0,
	0,   0, 0,
	// clang-format on
};*/
/** Miniset: Two floor tiles with a vertical wall in the middle 1. */
const BYTE L6MITEBASE[] = {
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
	2, 2, // width, height

	67,  0, // search
	66, 51,

	 7,  0, // replace
	68, 52,
	// clang-format on
};
/** Miniset: Two floor tiles with a vertical wall in the middle 3. */
const BYTE L6MITE3[] = {
	// clang-format off
	2, 2, // width, height

	67,  0, // search
	66, 51,

	70,  0, // replace
	71, 53,
	// clang-format on
};
/** Miniset: Two floor tiles with a vertical wall in the middle 4. */
const BYTE L6MITE4[] = {
	// clang-format off
	2, 2, // width, height

	67,  0, // search
	66, 51,

	 7,  0, // replace
	72, 54,
	// clang-format on
};
/** Miniset: Two floor tiles with a vertical wall in the middle 5. */
const BYTE L6MITE5[] = {
	// clang-format off
	2, 2, // width, height

	67,  0, // search
	66, 51,

	 7,  0, // replace
	 7, 55,
	// clang-format on
};
/** Miniset: Two floor tiles with a vertical wall in the middle 6. */
const BYTE L6MITE6[] = {
	// clang-format off
	2, 2, // width, height

	67,  0, // search
	66, 51,

	 7,  0, // replace
	76, 56,
	// clang-format on
};
/** Miniset: Two floor tiles with a vertical wall in the middle 7. */
const BYTE L6MITE7[] = {
	// clang-format off
	2, 2, // width, height

	67,  0, // search
	66, 51,

	 7,  0, // replace
	78, 57,
	// clang-format on
};
/** Miniset: Two floor tiles with a vertical wall in the middle 8. */
const BYTE L6MITE8[] = {
	// clang-format off
	2, 2, // width, height

	67,  0, // search
	66, 51,

	 7,  0, // replace
	80, 58,
	// clang-format on
};
/** Miniset: Two floor tiles with a vertical wall in the middle 9. */
const BYTE L6MITE9[] = {
	// clang-format off
	2, 2, // width, height

	67,  0, // search
	66, 51,

	83,  0, // replace
	82, 59,
	// clang-format on
};
const BYTE L6MITE10[] = {
	// clang-format off
	2, 2, // width, height

	67,  0, // search
	66, 51,

	 7,  0, // replace
	85, 60,
	// clang-format on
};
const BYTE L6MITE11[] = {
	// clang-format off
	2, 2, // width, height

	67,  0, // search
	66, 51,

	100,  0, // replace
	99,  89,
	// clang-format on
};
/*const BYTE L6MITE12[] = {
	// clang-format off
	2, 2, // width, height

	67,  0, // search
	66, 51,

	101,  0, // replace
	84/102, 90,
	// clang-format on
};*/
/*const BYTE L6MITE13[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	102,  0, 0, // replace
	101, 91, 0,
	 0,   0, 0,
	// clang-format on
};*/
/*const BYTE L6LMITE1[] = {
	// clang-format off
	4, 3, // width, height

	7, 7, 7, 7, // search
	7, 7, 7, 7,
	7, 7, 7, 7,

	 0,  0,  0, 0, // replace
	 0, 61, 62, 0,
	 0,  0,  0, 0,
	// clang-format on
};*/
/*const BYTE L6LMITE2[] = {
	// clang-format off
	4, 4, // width, height

	7, 7, 7, 7, // search
	7, 7, 7, 7,
	7, 7, 7, 7,
	7, 7, 7, 7,

	 0,  0,  0, 0, // replace
	 0, 64, 65, 0,
	 0, 63, 65, 0,
	 0,  0,  0, 0,
	// clang-format on
};*/
/*const BYTE L6LMITE3[] = {
	// clang-format off
	4, 3, // width, height

	7, 7, 7, 7, // search
	7, 7, 7, 7,
	7, 7, 7, 7,

	 ,  ,   0, 0, // replace
	 , 94, 95, 0,
	 0,  0,  0, 0,
	// clang-format on
};*/
/*const BYTE L6LMITE4[] = {
	// clang-format off
	3, 4, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,
	7, 7, 7,

	  0,  0, 0, // replace
	100, 97, 0,
	 99, 96, 0,
	  0,  0, 0,
	// clang-format on
};*/
const BYTE L6VERTWALLFIX2[] = {
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
const BYTE L6HORZWALLFIX2[] = {
	// clang-format off
	3, 2, // width, height

	 5,  2, 14, // search
	13, 10, 12,

	107, 120, 115, // replace
	131, 121, 123,
	// clang-format on
};
const BYTE L6SPOOLBASE[] = {
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
const BYTE L6SPOOL1[] = {
	// clang-format off
	2, 2, // width, height

	107, 115, // search
	131, 123,

	113, 114, // replace
	111, 112,
	// clang-format on
};
const BYTE L6SPOOL2[] = {
	// clang-format off
	2, 2, // width, height

	107, 115, // search
	131, 123,

	113, 115, // replace
	131, 123,
	// clang-format on
};
const BYTE L6SPOOL3[] = {
	// clang-format off
	2, 2, // width, height

	107, 115, // search
	131, 123,

	  7, 108, // replace
	109, 112,
	// clang-format on
};
const BYTE L6SPOOL4[] = {
	// clang-format off
	2, 2, // width, height

	107, 115, // search
	131, 123,

	110, 126, // replace
	  7, 124,
	// clang-format on
};
const BYTE L6SPOOL5[] = {
	// clang-format off
	2, 2, // width, height

	107, 115, // search
	131, 123,

	110, 124, // replace
	126,   7,
	// clang-format on
};
const BYTE L6VERTLPOOLBASE[] = {
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
const BYTE L6VERTLPOOL1[] = {
	// clang-format off
	2, 3, // width, height

	107, 115, // search
	119, 122,
	131, 123,

	107, 114, // replace
	119, 123,
	111, 110,
	// clang-format on
};
const BYTE L6VERTLPOOL2[] = {
	// clang-format off
	2, 3, // width, height

	107, 115, // search
	119, 122,
	131, 123,

	109, 114, // replace
	113, 112,
	111, 110,
	// clang-format on
};
const BYTE L6VERTLPOOL3[] = {
	// clang-format off
	2, 3, // width, height

	107, 115, // search
	119, 122,
	131, 123,

	126, 108, // replace
	  7, 117,
	109, 112,
	// clang-format on
};
const BYTE L6VERTLPOOL4[] = {
	// clang-format off
	2, 3, // width, height

	107, 115, // search
	119, 122,
	131, 123,

	108,   7, // replace
	111, 115,
	109, 123,
	// clang-format on
};
const BYTE L6VERTLPOOL5[] = {
	// clang-format off
	2, 3, // width, height

	107, 115, // search
	119, 122,
	131, 123,

	108,   7, // replace
	106, 108,
	111, 112,
	// clang-format on
};
const BYTE L6VERTLPOOL6[] = {
	// clang-format off
	2, 3, // width, height

	107, 115, // search
	119, 122,
	131, 123,

	  7, 108, // replace
	113, 122,
	111, 112,
	// clang-format on
};
const BYTE L6VERTLPOOL7[] = {
	// clang-format off
	2, 3, // width, height

	107, 115, // search
	119, 122,
	131, 123,

	113, 114, // replace
	131, 123,
	  7, 124,
	// clang-format on
};
const BYTE L6HORZLPOOLBASE[] = {
	// clang-format off
	5, 4, // width, height

	7, 7, 7, 7, 7, // search
	7, 7, 7, 7, 7,
	7, 7, 7, 7, 7,
	7, 7, 7, 7, 7,

	7,   7,   7,   7, 7, // replace
	7, 107, 120, 115, 7,
	7, 131, 121, 123, 7,
	7,   7,   7,   7, 7,
	// clang-format on
};
const BYTE L6HORZLPOOL1[] = {
	// clang-format off
	3, 2, // width, height

	107, 120, 115, // search
	131, 121, 123,

	107, 104, 115, // replace
	111, 116, 123,
	// clang-format on
};
const BYTE L6HORZLPOOL2[] = {
	// clang-format off
	3, 2, // width, height

	107, 120, 115, // search
	131, 121, 123,

	108, 109, 115, // replace
	111, 116, 112,
	// clang-format on
};
const BYTE L6HORZLPOOL3[] = {
	// clang-format off
	3, 2, // width, height

	107, 120, 115, // search
	131, 121, 123,

	113, 116, 114, // replace
	111, 104, 123,
	// clang-format on
};
const BYTE L6HORZLPOOL4[] = {
	// clang-format off
	3, 2, // width, height

	107, 120, 115, // search
	131, 121, 123,

	113, 104, 114, // replace
	111, 104, 112,
	// clang-format on
};
const BYTE L6HORZLPOOL5[] = {
	// clang-format off
	3, 2, // width, height

	107, 120, 115, // search
	131, 121, 123,

	107, 116, 114, // replace
	131, 104, 112,
	// clang-format on
};
const BYTE L6HORZLPOOL6[] = {
	// clang-format off
	3, 2, // width, height

	107, 120, 115, // search
	131, 121, 123,

	110, 107, 115, // replace
	109, 121, 123,
	// clang-format on
};
const BYTE L6HORZLPOOL7[] = {
	// clang-format off
	3, 2, // width, height

	107, 120, 115, // search
	131, 121, 123,

	113, 114, 126, // replace
	111, 123, 110,
	// clang-format on
};
const BYTE L6WALLSPOOL1[] = {
	// clang-format off
	4, 3, // width, height

	0, 10,  0, 0, // search
	7,  7,  7, 7,
	7,  7,  7, 7,

	0, 136,   0, 0, // replace
	0, 111, 110, 0,
	0,   0,   0, 0,
	// clang-format on
};
const BYTE L6WALLLPOOL1[] = {
	// clang-format off
	4, 4, // width, height

	0, 10,  0, 0, // search
	7,  7,  7, 7,
	7,  7,  7, 7,
	7,  7,  7, 7,

	0, 136,   0, 0, // replace
	0, 105,   0, 0,
	0, 111, 110, 0,
	0,   0,   0, 0,
	// clang-format on
};
const BYTE L6WALLSPOOL2[] = {
	// clang-format off
	3, 4, // width, height

	0, 7, 7, // search
	0, 7, 7,
	9, 7, 7,
	0, 7, 7,

	  0,   0, 0, // replace
	  0, 108, 0,
	135, 112, 0,
	  0,   0, 0,
	// clang-format on
};
const BYTE L6WALLLPOOL2[] = {
	// clang-format off
	4, 4, // width, height

	0, 7, 7, 7, // search
	0, 7, 7, 7,
	9, 7, 7, 7,
	0, 7, 7, 7,

	  0,   0,   0, 0, // replace
	  0, 109, 115, 0,
	135, 104, 123, 0, // 103/104
	  0,   0,   0, 0,
	// clang-format on
};
#endif

static void DRLG_LoadL3SP()
{
	// assert(pSetPieces[0]._spData == NULL);
	if (QuestStatus(Q_ANVIL)) {
		pSetPieces[0]._sptype = SPT_ANVIL;
		pSetPieces[0]._spData = LoadFileInMem(setpiecedata[pSetPieces[0]._sptype]._spdDunFile);
	}
}

static bool DRLG_L3FillRoom(int x1, int y1, int x2, int y2)
{
	int i, j;
	BYTE v;

	if (x1 <= 0 || x2 >= DMAXX - 1 || y1 <= 0 || y2 >= DMAXY - 1) {
		return false;
	}

	v = 0;
	for (j = y1; j <= y2; j++) {
		for (i = x1; i <= x2; i++) {
			v |= dungeon[i][j];
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
		x1 = random_low(0, blksizex);
		x1 = blksizex < obs ? x1 : -x1;
		x1 = x + x1;
		x2 = blksizex + x1;
		break;
	case 1: // block to the east
		x1 = x + 1;
		x2 = x1 + blksizex;
		y1 = random_low(0, blksizey);
		y1 = blksizey < obs ? y1 : -y1;
		y1 = y + y1;
		y2 = y1 + blksizey;
		break;
	case 2: // block to the south
		y1 = y + 1;
		y2 = y1 + blksizey;
		x1 = random_low(0, blksizex);
		x1 = blksizex < obs ? x1 : -x1;
		x1 = x + x1;
		x2 = blksizex + x1;
		break;
	case 3: // block to the west
		x2 = x - 1;
		x1 = x2 - blksizex;
		y1 = random_low(0, blksizey);
		y1 = blksizey < obs ? y1 : -y1;
		y1 = y + y1;
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
			DRLG_L3CreateBlock(x1, y1, blksizex, 0); // to north
		}
		if (dir != 3) {
			DRLG_L3CreateBlock(x2, y1, blksizey, 1); // to east
		}
		if (dir != 0) {
			DRLG_L3CreateBlock(x1, y2, blksizex, 2); // to south
		}
		if (dir != 1) {
			DRLG_L3CreateBlock(x1, y1, blksizey, 3); // to west
		}
	}
}

static void DRLG_L3FloorArea()
{
	int w, h, xr, yb, x1, y1, x2, y2, i, j;

	w = SwapLE16(*(uint16_t*)&pSetPieces[0]._spData[0]);
	h = SwapLE16(*(uint16_t*)&pSetPieces[0]._spData[2]);
	xr = DMAXX - w - 10;
	yb = DMAXY - h - 10;

	assert(xr > 10);
	assert(yb > 10);

	x1 = RandRangeLow(10, xr);
	y1 = RandRangeLow(10, yb);
	pSetPieces[0]._spx = x1;
	pSetPieces[0]._spy = y1;
	x2 = x1 + w;
	y2 = y1 + h;

	for (j = y1; j <= y2; j++) {
		for (i = x1; i <= x2; i++) {
			dungeon[i][j] = 1;
		}
	}
}

/*
 * Prevent diagonal thin walls.
 *            0 1    1 0
 * In case of 1 0 or 0 1, one of the zeros will be filled.
 */
static bool DRLG_L3FillDiags()
{
	int i, j;
	BYTE bv;
	bool result = false;

	for (j = 1; j < DMAXY - 2; j++) {
		for (i = 1; i < DMAXX - 2; i++) {
			// assert(dungeon[i][j] <= 1);
			bv = dungeon[i + 1][j + 1]
			 | (dungeon[i][j + 1] << 1)
			 | (dungeon[i + 1][j] << 2)
			 | (dungeon[i][j] << 3);
			if (bv == 6) {
				result = true;
				if (random_(0, 2) == 0) {
					dungeon[i][j] = 1;
				} else {
					dungeon[i + 1][j + 1] = 1;
				}
			} else if (bv == 9) {
				result = true;
				if (random_(0, 2) == 0) {
					dungeon[i + 1][j] = 1;
				} else {
					dungeon[i][j + 1] = 1;
				}
			}
		}
	}
	return result;
}

/*
 * Fill standalone empty tiles.
 */
static void DRLG_L3FillSingles()
{
	int i, j;

	for (j = 1; j < DMAXY - 1; j++) {
		for (i = 1; i < DMAXX - 1; i++) {
			// assert(dungeon[i][j] <= 1);
			if (dungeon[i][j] == 0
			 && (dungeon[i][j - 1] & dungeon[i][j + 1])
			 && (dungeon[i - 1][j - 1] & dungeon[i - 1][j] & dungeon[i - 1][j + 1])
			 && (dungeon[i + 1][j - 1] & dungeon[i + 1][j] & dungeon[i + 1][j + 1])) {
				dungeon[i][j] = 1;
			}
		}
	}
}

/*
 * Add random protruding tiles to long, straight walls.
 */
static BYTE DRLG_L3FillStraights()
{
	int i, j, sxy;
	BYTE result = 0;

	for (j = 1; j < DMAXY - 2; j++) {
		sxy = 0;
		for (i = 1; i < DMAXX - 2; i++) {
			if (dungeon[i][j] == 0 && dungeon[i][j + 1] == 1) {
				sxy++;
			} else {
				if (sxy > 3 && random_(0, 2) != 0) {
					for (sxy = i - sxy; sxy < i; sxy++) {
						// assert(dungeon[sxy][j] == 0);
						dungeon[sxy][j] = random_(0, 2);
						result |= dungeon[sxy][j];
					}
				}
				sxy = 0;
			}
		}
	}
	for (j = 1; j < DMAXY - 2; j++) {
		sxy = 0;
		for (i = 1; i < DMAXX - 2; i++) {
			if (dungeon[i][j] == 1 && dungeon[i][j + 1] == 0) {
				sxy++;
			} else {
				if (sxy > 3 && random_(0, 2) != 0) {
					for (sxy = i - sxy; sxy < i; sxy++) {
						// assert(dungeon[sxy][j + 1] == 0);
						dungeon[sxy][j + 1] = random_(0, 2);
						result |= dungeon[sxy][j + 1];
					}
				}
				sxy = 0;
			}
		}
	}
	for (i = 1; i < DMAXX - 2; i++) {
		sxy = 0;
		for (j = 1; j < DMAXY - 2; j++) {
			if (dungeon[i][j] == 0 && dungeon[i + 1][j] == 1) {
				sxy++;
			} else {
				if (sxy > 3 && random_(0, 2) != 0) {
					for (sxy = j - sxy; sxy < j; sxy++) {
						// assert(dungeon[i][sxy] == 0);
						dungeon[i][sxy] = random_(0, 2);
						result |= dungeon[i][sxy];
					}
				}
				sxy = 0;
			}
		}
	}
	for (i = 1; i < DMAXX - 2; i++) {
		sxy = 0;
		for (j = 1; j < DMAXY - 2; j++) {
			if (dungeon[i][j] == 1 && dungeon[i + 1][j] == 0) {
				sxy++;
			} else {
				if (sxy > 3 && random_(0, 2) != 0) {
					for (sxy = j - sxy; sxy < j; sxy++) {
						// assert(dungeon[i + 1][sxy] == 0);
						dungeon[i + 1][sxy] = random_(0, 2);
						result |= dungeon[i + 1][sxy];
					}
				}
				sxy = 0;
			}
		}
	}
	return result; // != 0;
}

/*
 * Validate the dungeon to prevent OOB in DRLG_L3LockRec.
 */
/*static void DRLG_L3Edges()
{
	int i, j;

	for (j = 0; j < DMAXY; j++) {
		assert(dungeon[DMAXX - 1][j] == 0);
		assert(dungeon[0][j] == 0);
		//dungeon[DMAXX - 1][j] = 0;
	}
	for (i = 0; i < DMAXX; i++) {
		assert(dungeon[i][DMAXY - 1] == 0);
		assert(dungeon[i][0] == 0);
		//dungeon[i][DMAXY - 1] = 0;
	}
}*/

static int DRLG_L3GetArea()
{
	int i, rv;
	BYTE* pTmp;

	rv = 0;
	static_assert(sizeof(dungeon) == DMAXX * DMAXY, "Linear traverse of dungeon does not work in DRLG_L3GetArea.");
	pTmp = &dungeon[0][0];
	for (i = 0; i < DMAXX * DMAXY; i++, pTmp++) {
		assert(*pTmp <= 1);
		rv += *pTmp;
	}

	return rv;
}

/*
 * Transform dungeon by replacing values using 2x2 block patterns defined in L3ConvTbl
 * New dungeon values: 1..14
 */
static void DRLG_L3MakeMegas()
{
	int i, j;
	BYTE v;

	for (j = 0; j < DMAXY - 1; j++) {
		for (i = 0; i < DMAXX - 1; i++) {
			// assert(dungeon[i][j] <= 1 && dungeon[i + 1][j] <= 1 && dungeon[i][j + 1] <= 1 && dungeon[i + 1][j + 1] <= 1);
			v = dungeon[i + 1][j + 1]
			 | (dungeon[i][j + 1] << 1)
			 | (dungeon[i + 1][j] << 2)
			 | (dungeon[i][j] << 3);
			// assert(v != 6 && v != 9);
			/* Commented out because this is prevented by re-running DRLG_L3FillDiags.
			   The ugly result was attempted to be hidden by miniset replacement, but
			   there are too many cases...
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
			}*/
			dungeon[i][j] = L3ConvTbl[v];
		}
	}
	for (j = 0; j < DMAXY; j++)
		dungeon[DMAXX - 1][j] = BASE_MEGATILE_L3;
	for (i = 0; i < DMAXX - 1; i++)
		dungeon[i][DMAXY - 1] = BASE_MEGATILE_L3;
}

/*static void DRLG_L3FloodTVal()
{
	int i, j;

	for (i = 0; i < DMAXX; i++) {
		for (j = 0; j < DMAXY; j++) {
			if (dungeon[i][j] != 8) {
				dTransVal[DBORDERX + i * 2 + 0][DBORDERY + j * 2 + 0] = 1;
				dTransVal[DBORDERX + i * 2 + 1][DBORDERY + j * 2 + 0] = 1;
				dTransVal[DBORDERX + i * 2 + 0][DBORDERY + j * 2 + 1] = 1;
				dTransVal[DBORDERX + i * 2 + 1][DBORDERY + j * 2 + 1] = 1;
			}
		}
	}

	assert(numtrans == 1);
	numtrans = 2;
}*/

typedef enum DIR4 {
	DR4_UP,
	DR4_DOWN,
	DR4_RIGHT,
	DR4_LEFT,
	NUM_DIR4,
} DIR4;

typedef struct RiverTile {
	int locX;
	int locY;
	int dir;
	int tile;
} RiverTile;
/*
 * Draw lava rivers.
 * Assumes the border of dungeon was empty.
 * New dungeon values: 15 16 17 18 19 20 21 22 23 24 38 39 40 41 42 43 44 45
 */
static void DRLG_L3River()
{
	int rx, ry, px, py, dir, pdir, nodir, nodir2, dircheck;
	constexpr int MIN_RIVER_LEN = 7;
	constexpr int MAX_RIVER_LEN = 100;
	constexpr int MAX_RIVER_COUNT = 4;
	constexpr int diroff_x[NUM_DIR4] = { 0,  0,  1, -1 };
	constexpr int diroff_y[NUM_DIR4] = { -1,  1,  0,  0 };
	constexpr int opposite[NUM_DIR4] = { DR4_DOWN, DR4_UP, DR4_LEFT, DR4_RIGHT };
	RiverTile river[MAX_RIVER_LEN];
	int rivercnt, riverlen;
	int i, tries, bridgeTile, bridgeIndex, lpcnt;
	bool done;

	rivercnt = MAX_RIVER_COUNT;

	for (tries = 0; tries < 200; tries++) {
		// select starting position
		lpcnt = 100;
		do {
			rx = random_(0, DMAXX);
			ry = random_(0, DMAXY);
			while (dungeon[rx][ry] < 25 || dungeon[rx][ry] > 28 || drlgFlags[rx][ry] != 0) {
				if (++rx == DMAXX) {
					rx = 0;
					if (++ry == DMAXY)
						break;
				}
			}
		} while (ry == DMAXY && --lpcnt != 0);
		if (ry == DMAXY)
			continue;
		// walk through normal tiles till it is possible
		switch (dungeon[rx][ry]) {
		case 25:
			dir = DR4_LEFT;
			break;
		case 26:
			dir = DR4_UP;
			break;
		case 27:
			dir = DR4_DOWN;
			break;
		case 28:
			dir = DR4_RIGHT;
			break;
		default:
			ASSUME_UNREACHABLE
		}
		river[0].locX = rx;
		river[0].locY = ry;
		// river[0].dir = dir;
		riverlen = 1;
		nodir = opposite[dir];
		nodir2 = NUM_DIR4;

		while (true) {
			dir = random_(0, NUM_DIR4);
			for (dircheck = NUM_DIR4; dircheck != 0; dircheck--, dir = (dir + 1) & 3) {
				if (dir == nodir || dir == nodir2) {
					continue;
				}
				px = rx + diroff_x[dir];
				py = ry + diroff_y[dir];
				assert(px >= 0 && px < DMAXX);
				assert(py >= 0 && py < DMAXY);
				if (dungeon[px][py] == 7 && drlgFlags[rx][ry] == 0)
					break;
			}

			if (dircheck != 0) {
				nodir2 = opposite[dir];

				rx += diroff_x[dir];
				ry += diroff_y[dir];
				river[riverlen].locX = rx;
				river[riverlen].locY = ry;
				river[riverlen - 1].dir = dir;
				riverlen++;
				if (riverlen != MAX_RIVER_LEN - 1)
					continue;
			}
			break;
		}

		if (riverlen < MIN_RIVER_LEN)
			continue;
		// connect to a wall
		done = false;
		assert(ry > 0);
		assert(ry < DMAXY - 1);
		assert(rx > 0);
		assert(rx < DMAXX - 1);
		if (dungeon[rx][ry - 1] == 10 && (ry < 2 || dungeon[rx][ry - 2] == 8)) {
			river[riverlen].locX = rx;
			river[riverlen].locY = ry - 1;
			river[riverlen - 1].dir = DR4_UP;
			done = true;
		} else if (dungeon[rx][ry + 1] == 2 && (ry >= DMAXY - 2 || dungeon[rx][ry + 2] == 8)) {
			river[riverlen].locX = rx;
			river[riverlen].locY = ry + 1;
			river[riverlen - 1].dir = DR4_DOWN;
			done = true;
		} else if (dungeon[rx + 1][ry] == 4 && (rx >= DMAXX - 2 || dungeon[rx + 2][ry] == 8)) {
			river[riverlen].locX = rx + 1;
			river[riverlen].locY = ry;
			river[riverlen - 1].dir = DR4_RIGHT;
			done = true;
		} else if (dungeon[rx - 1][ry] == 9 && (rx < 2 || dungeon[rx - 2][ry] == 8)) {
			river[riverlen].locX = rx - 1;
			river[riverlen].locY = ry;
			river[riverlen - 1].dir = DR4_LEFT;
			done = true;
		}
		if (!done)
			continue;
		// add a bridge over the river
		bridgeIndex = random_(1, 256);
		bridgeTile = 0;
		for (lpcnt = riverlen - 2; lpcnt != 0 && bridgeTile == 0; lpcnt--) {
			bridgeIndex = 1 + bridgeIndex % (riverlen - 3);
			if (river[bridgeIndex - 1].locY == river[bridgeIndex + 1].locY
				&& dungeon[river[bridgeIndex].locX][river[bridgeIndex + 1].locY - 1] == 7
				&& dungeon[river[bridgeIndex].locX][river[bridgeIndex + 1].locY + 1] == 7) {
				// straight horizontal river
				bridgeTile = 44;
				// if (river[0].dir != DR4_RIGHT && river[0].dir != DR4_LEFT)
				// check if the bridge is blocked by the river
				for (i = 1; i < riverlen && bridgeTile != 0; i++) {
					if ((river[bridgeIndex].locY - 1 == river[i].locY || river[bridgeIndex].locY + 1 == river[i].locY)
						&& river[bridgeIndex].locX == river[i].locX) {
						bridgeTile = 0;
					}
				}
			} else if (river[bridgeIndex - 1].locX == river[bridgeIndex + 1].locX
				&& dungeon[river[bridgeIndex].locX - 1][river[bridgeIndex].locY] == 7
				&& dungeon[river[bridgeIndex].locX + 1][river[bridgeIndex].locY] == 7) {
				// straight vertical river
				bridgeTile = 45;
				// if (river[0].dir != DR4_UP && river[0].dir != DR4_DOWN)
				// check if the bridge is blocked by the river
				for (i = 1; i < riverlen && bridgeTile != 0; i++) {
					if ((river[bridgeIndex].locX - 1 == river[i].locX || river[bridgeIndex].locX + 1 == river[i].locX)
						&& river[bridgeIndex].locY == river[i].locY) {
						bridgeTile = 0;
					}
				}
			}
		}
		if (bridgeTile == 0)
			continue;
		// select appropriate tiles
		// - starting tile
		dir = river[0].dir;
		int rTile;
		switch (dir) {
		case DR4_LEFT:
			rTile = 40;
			break;
		case DR4_UP:
			rTile = 38;
			break;
		case DR4_DOWN:
			rTile = 41;
			break;
		case DR4_RIGHT:
			rTile = 39;
			break;
		default:
			ASSUME_UNREACHABLE
		}
		river[0].tile = rTile;
		// - inner tiles
		for (i = 1; i < riverlen; i++) {
			pdir = dir;
			dir = river[i].dir;
			/*rTile = 0;
			if ((dir == DR4_UP && pdir == DR4_RIGHT) || (dir == DR4_LEFT && pdir == DR4_DOWN)) {
				rTile = 22;
			}
			if ((dir == DR4_UP && pdir == DR4_LEFT) || (dir == DR4_RIGHT && pdir == DR4_DOWN)) {
				rTile = 21;
			}
			if ((dir == DR4_DOWN && pdir == DR4_RIGHT) || (dir == DR4_LEFT && pdir == DR4_UP)) {
				rTile = 20;
			}
			if ((dir == DR4_DOWN && pdir == DR4_LEFT) || (dir == DR4_RIGHT && pdir == DR4_UP)) {
				rTile = 19;
			}
			if (rTile == 0) {
				 rTile = RandRange(15, 16) + (1 - (dir >> 1)) * 2; // (DR4_LEFT || DR4_RIGHT) ? 0 : 2;
			}*/
			const BYTE dirXPdirTiles[NUM_DIR4][NUM_DIR4] = {
			//          UP,DOWN,RIGHT,LEFT
			/*UP*/   {  17,   0,  22,  21 },
			/*DOWN*/ {   0,  17,  20,  19 },
			/*RIGHT*/{  19,  21,  15,   0 },
			/*LEFT*/ {  20,  22,   0,  15 },
			};
			rTile = dirXPdirTiles[dir][pdir];
			assert(rTile != 0);
			// if (rTile == 15 || rTile == 17) {
			if (rTile <= 17) {
				rTile += random_(0, 2);
			}

			river[i].tile = rTile;
		}
		// - last tile
		switch (dir) {
		case DR4_LEFT:
			rTile = 23;
			break;
		case DR4_UP:
			rTile = 24;
			break;
		case DR4_DOWN:
			rTile = 42;
			break;
		case DR4_RIGHT:
			rTile = 43;
			break;
		default:
			ASSUME_UNREACHABLE
		}
		river[riverlen].tile = rTile;
		// - overwrite the bridge tile
		river[bridgeIndex].tile = bridgeTile;
		// add the river
		for (i = 0; i <= riverlen; i++) {
			dungeon[river[i].locX][river[i].locY] = river[i].tile;
		}
		rivercnt--;
		if (rivercnt == 0) {
			break;
		}
	}
}

static int lavaarea;
static bool DRLG_L3SpawnLava(int x, int y, int dir)
{
	BYTE i; //                        0     1     2     3     4    ?5    ?6     7     8     9    10   ?11    12    13    14
	//                                  NW|SW    SW SE|NE    SE               ALL          NW    NE       NW|NE SE|NE NW|SW
	static BYTE blocktable[15] = { 0x00, 0x0A, 0x08, 0x05, 0x01, 0x00, 0x00, 0xFF, 0x00, 0x02, 0x04, 0x00, 0x06, 0x05, 0x0A };

	if (x < 0 || x >= DMAXX || y < 0 || y >= DMAXY) {
		return true;
	}
	i = dungeon[x][y];
	if (i >= 15) {
		return true;
	}

	i = blocktable[i];
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
	if (i & dir)
		return false;

	//if (drlgFlags[x][y] & (DRLG_L3_LAVA | DRLG_PROTECTED)) {
	if (drlgFlags[x][y]) {
		return false;
	}
	drlgFlags[x][y] |= DRLG_L3_LAVA;
	lavaarea += 1;
	if (lavaarea > 40) {
		return true;
	}

	if (DRLG_L3SpawnLava(x + 1, y, (1 << 0))) { // SE
		return true;
	}
	if (DRLG_L3SpawnLava(x - 1, y, (1 << 1))) { // NW
		return true;
	}
	if (DRLG_L3SpawnLava(x, y + 1, (1 << 3))) { // SW
		return true;
	}
	if (DRLG_L3SpawnLava(x, y - 1, (1 << 2))) { // NE
		return true;
	}

	return false;
}

static void DRLG_L3DrawLava(int x, int y)
{
	BYTE i;                 //   0   1   2   3   4   5   6   7   8   9  10  11  12  13  14 
	static BYTE poolsub[15] = {  0, 35, 26, 36, 25, 29, 34,  7, 33, 28, 27, 37, 32, 31, 30 };

	if (x < 0 || x >= DMAXX || y < 0 || y >= DMAXY) {
		return;
	}
	if (!(drlgFlags[x][y] & DRLG_L3_LAVA)) {
		return;
	}
	drlgFlags[x][y] &= ~DRLG_L3_LAVA;

	i = dungeon[x][y];
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
 * New dungeon values: 25 26 27 28 29 30 31 32 33 34 35 36 37
 */
static void DRLG_L3Pool()
{
	int i, j;
	bool badPos;

	for (i = 3; i < DMAXY - 3; i++) {
		for (j = 3; j < DMAXY - 3; j++) {
			if (dungeon[i][j] != 8 || dungeon[i][j + 1] == 8 || random_(0, 2) != 0) {
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

static void DRLG_L3PlaceRndSet(const BYTE* miniset, int rndper)
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
					if (drlgFlags[xx][yy]) {
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

/*
 * Replace undecorated tiles with matching decorated tiles.
 * New dungeon values: 68 69 106 107 108 109 110 112 136 137
 */
static void DRLG_L3Subs()
{
	int x, y; //, i, j;
	BYTE c, k;
	int8_t rv;
	const unsigned MAX_MATCH = 4;
	const unsigned NUM_L3TYPES = 138;
	static_assert(MAX_MATCH <= INT8_MAX, "MAX_MATCH does not fit to rv(int8_t) in DRLG_L3Subs.");
	static_assert(NUM_L3TYPES <= UCHAR_MAX, "NUM_L3TYPES does not fit to i(BYTE) in DRLG_L3Subs.");
#if DEBUG_MODE
	for (int i = sizeof(L3BTYPES) - 1; i >= 0; i--) {
		if (L3BTYPES[i] != 0) {
			if ((unsigned)i >= NUM_L3TYPES)
				app_fatal("Value %d is ignored in L3BTYPES at %d", L3BTYPES[i], i);
			break;
		}
	}

	for (unsigned i = 0; i < sizeof(L3BTYPES); i++) {
		c = L3BTYPES[i];
		if (c == 0)
			continue;
		x = 0;
		for (unsigned j = 0; j < sizeof(L3BTYPES); j++) {
			if (c == L3BTYPES[j])
				x++;
		}
		if ((unsigned)x > MAX_MATCH)
			app_fatal("Too many(%d) matching('%d') values in L3BTYPES", x, c);
	}
#endif
	for (x = 0; x < DMAXX; x++) {
		for (y = 0; y < DMAXY; y++) {
			if (random_(0, 4) == 0) {
				c = L3BTYPES[dungeon[x][y]];
				if (c != 0 && (drlgFlags[x][y] & DRLG_FROZEN) == 0) {
					rv = random_(0, MAX_MATCH);
					k = 0;
					while (true) {
						if (c == L3BTYPES[k] && --rv < 0) {
							break;
						}
						if (++k == NUM_L3TYPES)
							k = 0;
					}
					/*for (j = y - 2; j < y + 2; j++) {
						for (i = x - 2; i < x + 2; i++) {
							if (dungeon[i][j] == k) {
								j = y + 3;
								i = x + 2;
							}
						}
					}
					if (j < y + 3) {*/
						dungeon[x][y] = k;
					//}
				}
			}
		}
	}
}
#ifdef HELLFIRE
/*
 * Replace undecorated tiles with matching decorated tiles.
 * New dungeon values: 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 (25..50)
 */
static void DRLG_L6Subs()
{
	int x, y; //, i, j;
	BYTE c, k;
	int8_t rv;
	const unsigned MAX_MATCH = 8;
	const unsigned NUM_L6TYPES = 51;
	static_assert(MAX_MATCH <= INT8_MAX, "MAX_MATCH does not fit to rv(int8_t) in DRLG_L6Subs.");
	static_assert(NUM_L6TYPES <= UCHAR_MAX, "NUM_L6TYPES does not fit to i(BYTE) in DRLG_L6Subs.");
#if DEBUG_MODE
	for (int i = sizeof(L6BTYPES) - 1; i >= 0; i--) {
		if (L6BTYPES[i] != 0) {
			if ((unsigned)i >= NUM_L6TYPES)
				app_fatal("Value %d is ignored in L6BTYPES at %d", L6BTYPES[i], i);
			break;
		}
	}

	for (unsigned i = 0; i < sizeof(L6BTYPES); i++) {
		c = L6BTYPES[i];
		if (c == 0)
			continue;
		x = 0;
		for (unsigned j = 0; j < sizeof(L6BTYPES); j++) {
			if (c == L6BTYPES[j])
				x++;
		}
		if ((unsigned)x > MAX_MATCH)
			app_fatal("Too many(%d) matching('%d') values in L6BTYPES", x, c);
	}
#endif
	for (x = 0; x < DMAXX; x++) {
		for (y = 0; y < DMAXY; y++) {
			if (drlgFlags[x][y])
				continue;
			if (random_(0, 4) == 0) {
				c = L6BTYPES[dungeon[x][y]];
				if (c != 0) {
					rv = random_(0, MAX_MATCH);
					k = 0;
					while (true) {
						if (c == L6BTYPES[k] && --rv < 0) {
							break;
						}
						if (++k == NUM_L6TYPES)
							k = 0;
					}
					/*for (j = y - 2; j < y + 2; j++) {
						for (i = x - 2; i < x + 2; i++) {
							if (dungeon[i][j] == k) {
								j = y + 3;
								i = x + 2;
							}
						}
					}
					if (j < y + 3) {*/
						dungeon[x][y] = k;
					//}
				}
			}
		}
	}
}

static void DRLG_L6PlaceRndPool(const BYTE* miniset, int rndper)
{
	int sx, sy, sw, sh, xx, yy, ii;
	bool found, placed;

	placed = false;
	sw = miniset[0];
	sh = miniset[1];

	for (sy = 2; sy < DMAXX - 2 - sh; sy++) {
		for (sx = 2; sx < DMAXY - 2 - sw; sx++) {
			found = true;
			ii = 2;
			for (yy = sy; yy < sy + sh && found; yy++) {
				for (xx = sx; xx < sx + sw && found; xx++) {
					if (/*miniset[ii] != 0 &&*/ dungeon[xx][yy] != miniset[ii]) {
						found = false;
					}
					if (drlgFlags[xx][yy]) {
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

	_guLavapools += placed ? 2 : 0;
}
#endif

/*
 * Add fences and planks to the dungeon.
 * New dungeon values: 121, 122, 123, 124, 125, 126, 127, 128, 129, 130,
 *                     131, 132, 133, 134, 135, 139, 140, 142, 143, 146, 147, 151, 152
 */
static void DRLG_L3Wood()
{
	int i, j, x, y, x1, y1;
	BYTE bv;

	// add wooden planks to walls
	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			// horizontal wall
			if (dungeon[i][j] == 10 && random_(0, 2) != 0) {
				x = i;
				do {
					x++;
				} while (dungeon[x][j] == 10);
				x--;
				if (x - i > 0) {
					dungeon[x][j] = 128;
					while (--x > i) {
						dungeon[x][j] = random_(0, 2) != 0 ? 126 : 129;
					}
					dungeon[i][j] = 127;
				}
			// vertical wall
			} else if (dungeon[i][j] == 9 && random_(0, 2) != 0) {
				y = j;
				do {
					y++;
				} while (dungeon[i][y] == 9);
				y--;
				if (y - j > 0) {
					dungeon[i][y] = 122;
					while (--y > j) {
						dungeon[i][y] = random_(0, 2) != 0 ? 121 : 124;
					}
					dungeon[i][j] = 123;
				}
			// NW-corner
			} else if (dungeon[i][j] == 11 && dungeon[i + 1][j] == 10 && dungeon[i][j + 1] == 9 && random_(0, 2) != 0) {
				dungeon[i][j] = 125;
				x = i + 1;
				do {
					x++;
				} while (dungeon[x][j] == 10);
				x--;
				dungeon[x][j] = 128;
				while (--x > i) {
					dungeon[x][j] = random_(0, 2) != 0 ? 126 : 129;
				}
				y = j + 1;
				do {
					y++;
				} while (dungeon[i][y] == 9);
				y--;
				dungeon[i][y] = 122;
				while (--y > j) {
					dungeon[i][y] = random_(0, 2) != 0 ? 121 : 124;
				}
			}
		}
	}
	// add fences between walls
	for (i = 0; i < DMAXX; i++) {
		for (j = 0; j < DMAXY; j++) {
			bv = dungeon[i][j];
			if ((bv == 2 || bv == 134 || bv == 150 || bv == 144) && random_(0, 4) != 0) {
				if (InThemeRoom(i, j - 1))
					continue; // in a theme room -> skip
				y1 = j;
				while (true) {
					y1--;
					bv = dungeon[i][y1];
					if (bv == 10) { // normal wall
						bv = 131;
						break;
					}
					if (bv == 126 || bv == 129) { // wall with fence inlay
						bv = 133;
						break;
					}
					if (bv == 134) { // crossing fence
						bv = 151;
						break;
					}
					if (bv == 138 || bv == 152) { // fence corner
						break;
					}
					if (bv != 7) {
						bv = 7; // mismatching tile -> stop
						break;
					}
					bv = dungeon[i + 1][y1];
					if (bv == 7) {
						bv = dungeon[i - 1][y1];
						if (bv == 7)
							continue;
					}
					bv = 7; // too close to other obstacles -> stop
					break;
				}
				if (bv == 7 || j - y1 <= 1)
					continue;

				// replace last tile
				dungeon[i][y1] = bv;
				// replace inner tiles
				for (y = y1 + 1; y < j; y++) {
					dungeon[i][y] = 135;
				}
				// add door
				dungeon[i][RandRange(y1 + 1, j - 1)] = 147;
				// replace first tile
				if (dungeon[i][j] == 2)
					dungeon[i][j] = 139;
				else if (dungeon[i][j] == 134)
					dungeon[i][j] = 142;
				else if (dungeon[i][j] == 150)
					dungeon[i][j] = 152;
				else if (dungeon[i][j] == 144)
					dungeon[i][j] = 143;
			} else if ((bv == 4 || bv == 135 || bv == 145 || bv == 150) && random_(0, 4) != 0) {
				if (InThemeRoom(i - 1, j))
					continue; // in a theme room -> skip
				x1 = i;
				while (true) {
					x1--;
					bv = dungeon[x1][j];
					if (bv == 9) { // normal wall
						bv = 130;
						break;
					}
					if (bv == 121 || bv == 124) { // wall with fence inlay
						bv = 132;
						break;
					}
					if (bv == 135) { // crossing fence
						bv = 152;
						break;
					}
					if (bv == 150 || bv == 151) { // fence corner
						break;
					}
					if (bv != 7) {
						bv = 7;
						break; // mismatching tile -> stop
					}
					bv = dungeon[x1][j + 1];
					if (bv == 7) {
						bv = dungeon[x1][j - 1];
						if (bv == 7)
							continue;
					}
					bv = 7;
					break; // too close to other obstacles -> stop
				}
				if (bv == 7 || i - x1 <= 1)
					continue;

				// replace last tile
				dungeon[x1][j] = bv;
				// replace inner tiles
				for (x = x1 + 1; x < i; x++) {
					dungeon[x][j] = 134;
				}
				// add door
				dungeon[RandRange(x1 + 1, i - 1)][j] = 146;
				// replace first tile
				if (dungeon[i][j] == 4)
					dungeon[i][j] = 140;
				else if (dungeon[i][j] == 135)
					dungeon[i][j] = 143;
				else if (dungeon[i][j] == 145)
					dungeon[i][j] = 142;
				else if (dungeon[i][j] == 150)
					dungeon[i][j] = 151;
			}
		}
	}
}

static void DRLG_L3SetRoom(int idx)
{
	DRLG_LoadSP(idx, DEFAULT_MEGATILE_L3);
}

static void FixL3HallofHeroes()
{
	/* Commented out because the checked values are impossible to occur at the moment.
	   The tiles can not be placed like that, and none of the minisets can cause this either.
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
	}*/
}

static void DRLG_L3LockRec(unsigned offset)
{
	BYTE* pTmp = &drlg.lockoutMap[0][0];
	if (pTmp[offset] == 0) {
		return;
	}

	pTmp[offset] = 0;
	DRLG_L3LockRec(offset + 1);
	DRLG_L3LockRec(offset - 1);
	DRLG_L3LockRec(offset - DMAXY);
	DRLG_L3LockRec(offset + DMAXY);
}

/*
 * Check if every non-empty tile is reachable from the others
 * using only the four basic directions.
 * Assumes the border of dungeon is empty.
 */
static bool DRLG_L3Lockout()
{
	int i;
	BYTE* pTmp = &drlg.lockoutMap[0][0];

	static_assert(sizeof(dungeon) == sizeof(drlg.lockoutMap), "lockoutMap vs dungeon mismatch.");
	memcpy(drlg.lockoutMap, dungeon, sizeof(dungeon));

	for (i = DMAXX; i < DMAXX * DMAXY; i++) {
		if (pTmp[i] != 0) {
			DRLG_L3LockRec(i);
			break;
		}
	}

	static_assert(sizeof(drlg.lockoutMap) == DMAXX * DMAXY, "Linear traverse of lockoutMap does not work in DRLG_L3Lockout.");
	for (i = 0; i < DMAXX * DMAXY; i++, pTmp++)
		if (*pTmp != 0) {
			return false;
		}

	return true;
}

void DRLG_L3InitTransVals()
{
	static_assert(sizeof(drlg.transvalMap) == sizeof(dungeon), "transvalMap vs dungeon mismatch.");
	memcpy(drlg.transvalMap, dungeon, sizeof(dungeon));

	DRLG_FloodTVal();
}

static void DRLG_L3()
{
	bool doneflag;

	do {
		while (true) {
			do {
				memset(dungeon, 0, sizeof(dungeon));
				DRLG_L3CreateBlock(RandRange(10, 29), RandRange(10, 29), 0, 4);
				if (pSetPieces[0]._spData != NULL) { // pSetPieces[0]._sptype != SPT_NONE
					DRLG_L3FloorArea();
				}
				do {
					doneflag = !DRLG_L3FillDiags();
					doneflag &= !DRLG_L3FillStraights();
				} while (!doneflag);
				DRLG_L3FillSingles();
				// DRLG_L3Edges(); - Commented out because it is no longer necessary
			} while (DRLG_L3GetArea() < 600 || !DRLG_L3Lockout());
			DRLG_L3MakeMegas();
			memset(drlgFlags, 0, sizeof(drlgFlags));
			if (pSetPieces[0]._spData != NULL) { // pSetPieces[0]._sptype != SPT_NONE
				DRLG_L3SetRoom(0);
			}
			if (currLvl._dDynLvl) {
#ifdef HELLFIRE
				POS32 warpPos = DRLG_PlaceMiniSet(currLvl._dType == DTYPE_NEST ? L6DYNENTRY : L3DYNENTRY);
#else
				POS32 warpPos = DRLG_PlaceMiniSet(L3DYNENTRY);
#endif
				if (warpPos.x < 0) {
					continue;
				}
				pWarps[DWARP_ENTRY]._wx = warpPos.x;
				pWarps[DWARP_ENTRY]._wy = warpPos.y;
				pWarps[DWARP_ENTRY]._wx = 2 * pWarps[DWARP_ENTRY]._wx + DBORDERX + 1;
				pWarps[DWARP_ENTRY]._wy = 2 * pWarps[DWARP_ENTRY]._wy + DBORDERY;
				pWarps[DWARP_ENTRY]._wtype = WRPT_CIRCLE;
				break;
			}
#ifdef HELLFIRE
			if (currLvl._dType == DTYPE_NEST) {
				POS32 warpPos = DRLG_PlaceMiniSet(L6USTAIRS); // L6USTAIRS(1, 3)
				if (warpPos.x < 0) {
					continue;
				}
				if (currLvl._dLevelIdx == DLV_NEST1) {
					pWarps[DWARP_TOWN]._wx = warpPos.x + 0;
					pWarps[DWARP_TOWN]._wy = warpPos.y + 1;
					pWarps[DWARP_TOWN]._wx = 2 * pWarps[DWARP_TOWN]._wx + DBORDERX + 1;
					pWarps[DWARP_TOWN]._wy = 2 * pWarps[DWARP_TOWN]._wy + DBORDERY;
					pWarps[DWARP_TOWN]._wtype = WRPT_L3_UP;
				} else {
					pWarps[DWARP_ENTRY]._wx = warpPos.x + 0;
					pWarps[DWARP_ENTRY]._wy = warpPos.y + 1;
					pWarps[DWARP_ENTRY]._wx = 2 * pWarps[DWARP_ENTRY]._wx + DBORDERX + 1;
					pWarps[DWARP_ENTRY]._wy = 2 * pWarps[DWARP_ENTRY]._wy + DBORDERY;
					pWarps[DWARP_ENTRY]._wtype = WRPT_L3_UP;
				}
				if (currLvl._dLevelIdx != DLV_NEST4) {
					warpPos = DRLG_PlaceMiniSet(L6DSTAIRS); // L6DSTAIRS(3, 1)
					if (warpPos.x < 0) {
						continue;
					}
					pWarps[DWARP_EXIT]._wx = warpPos.x + 1;
					pWarps[DWARP_EXIT]._wy = warpPos.y + 0;
					pWarps[DWARP_EXIT]._wx = 2 * pWarps[DWARP_EXIT]._wx + DBORDERX;
					pWarps[DWARP_EXIT]._wy = 2 * pWarps[DWARP_EXIT]._wy + DBORDERY + 1;
					pWarps[DWARP_EXIT]._wtype = WRPT_L3_DOWN;
				}
			} else
#endif
			{
				// assert(currLvl._dType == DTYPE_CAVES);
				POS32 warpPos = DRLG_PlaceMiniSet(L3USTAIRS); // L3USTAIRS(1, 3)
				if (warpPos.x < 0) {
					continue;
				}
				pWarps[DWARP_ENTRY]._wx = warpPos.x + 0;
				pWarps[DWARP_ENTRY]._wy = warpPos.y + 1;
				pWarps[DWARP_ENTRY]._wx = 2 * pWarps[DWARP_ENTRY]._wx + DBORDERX + 1;
				pWarps[DWARP_ENTRY]._wy = 2 * pWarps[DWARP_ENTRY]._wy + DBORDERY;
				pWarps[DWARP_ENTRY]._wtype = WRPT_L3_UP;
				warpPos = DRLG_PlaceMiniSet(L3DSTAIRS); // L3DSTAIRS(3, 1)
				if (warpPos.x < 0) {
					continue;
				}
				pWarps[DWARP_EXIT]._wx = warpPos.x + 1;
				pWarps[DWARP_EXIT]._wy = warpPos.y + 0;
				pWarps[DWARP_EXIT]._wx = 2 * pWarps[DWARP_EXIT]._wx + DBORDERX;
				pWarps[DWARP_EXIT]._wy = 2 * pWarps[DWARP_EXIT]._wy + DBORDERY + 1;
				pWarps[DWARP_EXIT]._wtype = WRPT_L3_DOWN;
				if (currLvl._dLevelIdx == DLV_CAVES1) {
					warpPos = DRLG_PlaceMiniSet(L3USTAIRS); // L3TWARP(1, 3)
					if (warpPos.x < 0) {
						continue;
					}
					pWarps[DWARP_TOWN]._wx = warpPos.x + 0;
					pWarps[DWARP_TOWN]._wy = warpPos.y + 1;
					pWarps[DWARP_TOWN]._wx = 2 * pWarps[DWARP_TOWN]._wx + DBORDERX + 1;
					pWarps[DWARP_TOWN]._wy = 2 * pWarps[DWARP_TOWN]._wy + DBORDERY;
					pWarps[DWARP_TOWN]._wtype = WRPT_L3_UP;
				}
			}
			break;
		}
		// generate lava pools
		_guLavapools = 0;
#ifdef HELLFIRE
		if (currLvl._dType == DTYPE_NEST) {
			DRLG_L6PlaceRndPool(L6VERTLPOOLBASE, 30);
			DRLG_L6PlaceRndPool(L6HORZLPOOLBASE, 40);
			DRLG_L6PlaceRndPool(L6SPOOLBASE, 80);
		} else
#endif
		{
			// assert(currLvl._dType == DTYPE_CAVES);
			DRLG_L3Pool();
		}
	} while (_guLavapools < MIN_LAVA_POOL);

	DRLG_L3PlaceRndSet(L3VERTWALLFIX1, 70);
	DRLG_L3PlaceRndSet(L3HORZWALLFIX1, 70);
#ifdef HELLFIRE
	if (currLvl._dType == DTYPE_NEST) {
		DRLG_L3PlaceRndSet(L6VERTWALLFIX2, 100);
		DRLG_L3PlaceRndSet(L6HORZWALLFIX2, 100);
	} else
#endif
	{
		// assert(currLvl._dType == DTYPE_CAVES);
		DRLG_L3PlaceRndSet(L3VERTWALLFIX2, 100);
		DRLG_L3PlaceRndSet(L3HORZWALLFIX2, 100);
	}
	// not possible because of DRLG_L3FillDiags and DRLG_L3FillSingles
	// DRLG_L3PlaceRndSet(L3ISLE5, 90);

#ifdef HELLFIRE
	if (currLvl._dType == DTYPE_NEST) {
		DRLG_L3PlaceRndSet(L6WALLLPOOL1, 10);
		DRLG_L3PlaceRndSet(L6WALLLPOOL2, 10);
		DRLG_L3PlaceRndSet(L6WALLSPOOL1, 10);
		DRLG_L3PlaceRndSet(L6WALLSPOOL2, 10);
		// DRLG_L3PlaceRndSet(L6LMITE1, 25); - commented out because of a graphical glitch
		// DRLG_L3PlaceRndSet(L6LMITE2, 25);
		// DRLG_L3PlaceRndSet(L6LMITE3, 25);
		// DRLG_L3PlaceRndSet(L6LMITE4, 25);
		DRLG_L3PlaceRndSet(L6MITEBASE, 98);
		DRLG_L3PlaceRndSet(L6MITE2, 8);
		DRLG_L3PlaceRndSet(L6MITE3, 9);
		DRLG_L3PlaceRndSet(L6MITE4, 10);
		DRLG_L3PlaceRndSet(L6MITE5, 11);
		DRLG_L3PlaceRndSet(L6MITE6, 14);
		DRLG_L3PlaceRndSet(L6MITE7, 16);
		DRLG_L3PlaceRndSet(L6MITE8, 20);
		DRLG_L3PlaceRndSet(L6MITE9, 30);
		DRLG_L3PlaceRndSet(L6MITE10, 40);
		DRLG_L3PlaceRndSet(L6MITE11, 50);
		// DRLG_L3PlaceRndSet(L6MITE12, 50); - commented out because of a missing shadow tile
		DRLG_L3PlaceRndSet(L6PUDDLE1, 50);
		DRLG_L3PlaceRndSet(L6PUDDLE2, 90);
		// DRLG_L3PlaceRndSet(L6PUDDLE3, 90); - commented out because it does not make much difference
		DRLG_L3PlaceRndSet(L6VERTLPOOL1, 10);
		DRLG_L3PlaceRndSet(L6VERTLPOOL2, 15);
		DRLG_L3PlaceRndSet(L6VERTLPOOL3, 15);
		DRLG_L3PlaceRndSet(L6VERTLPOOL4, 20);
		DRLG_L3PlaceRndSet(L6VERTLPOOL5, 25);
		DRLG_L3PlaceRndSet(L6VERTLPOOL6, 30);
		DRLG_L3PlaceRndSet(L6VERTLPOOL7, 50);
		DRLG_L3PlaceRndSet(L6HORZLPOOL1, 10);
		DRLG_L3PlaceRndSet(L6HORZLPOOL2, 15);
		DRLG_L3PlaceRndSet(L6HORZLPOOL3, 15);
		DRLG_L3PlaceRndSet(L6HORZLPOOL4, 20);
		DRLG_L3PlaceRndSet(L6HORZLPOOL5, 25);
		DRLG_L3PlaceRndSet(L6HORZLPOOL6, 30);
		DRLG_L3PlaceRndSet(L6HORZLPOOL7, 50);
		DRLG_L3PlaceRndSet(L6SPOOL1, 15);
		DRLG_L3PlaceRndSet(L6SPOOL2, 20);
		DRLG_L3PlaceRndSet(L6SPOOL3, 25);
		DRLG_L3PlaceRndSet(L6SPOOL4, 30);
		DRLG_L3PlaceRndSet(L6SPOOL5, 50);
	} else
#endif
	{
		// assert(currLvl._dType == DTYPE_CAVES);
		FixL3HallofHeroes();
		DRLG_L3River();
		DRLG_PlaceThemeRooms(5, 10, themeTiles, 0);

		DRLG_L3Wood();
		//DRLG_L3PlaceRndSet(L3LTITE1, 20); - commented out because of a graphical glitch
		//DRLG_L3PlaceRndSet(L3LTITE2, 20);
		DRLG_L3PlaceRndSet(L3TITEBASE, 75);
		DRLG_L3PlaceRndSet(L3TITE2, 14);
		DRLG_L3PlaceRndSet(L3TITE3, 16);
		DRLG_L3PlaceRndSet(L3TITE8, 20);
		DRLG_L3PlaceRndSet(L3TITE9, 25);
		DRLG_L3PlaceRndSet(L3TITE10, 33);
		DRLG_L3PlaceRndSet(L3TITE11, 50);
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
	}
}
#if !USE_PATCH
static void DRLG_L3FixMap()
{
	uint16_t* lm = (uint16_t*)pSetPieces[0]._spData;

	if (pSetPieces[0]._sptype == SPT_LVL_PWATER) {
		// patch the map - Foulwatr.DUN
		// - separate subtiles for the automap
		lm[2 + 7 + 33 * 19] = SwapLE16(111);
		// protect island tiles from spawning additional monsters
		for (int y = 1; y < 7; y++) {
			for (int x = 7; x < 14; x++) {
				lm[2 + 19 * 37 + x + y * 19] = SwapLE16((3 << 8) | (3 << 10) | (3 << 12) | (3 << 14));
			}
		}
		// remove most of the monsters
		for (int y = 13; y < 61; y++) {
			for (int x = 4; x < 30; x++) {
				if (x == 6 && y == 33) {
					continue;
				}
				lm[2 + 19 * 37 + 19 * 37 * 2 * 2 + x + y * 19 * 2] = 0;
			}
		}
	}
}
#endif
static void DRLG_L3DrawPreMaps()
{
	for (int i = lengthof(pSetPieces) - 1; i >= 0; i--) {
		if (pSetPieces[i]._sptype == SPT_NONE)
			continue;
		if (setpiecedata[pSetPieces[i]._sptype]._spdPreDunFile != NULL) {
			MemFreeDbg(pSetPieces[i]._spData);
			pSetPieces[i]._spData = LoadFileInMem(setpiecedata[pSetPieces[i]._sptype]._spdPreDunFile);
			DRLG_DrawMap(i);
		}
	}
}

static void LoadL3Dungeon(const LevelData* lds)
{
	pWarps[DWARP_ENTRY]._wx = lds->dSetLvlDunX;
	pWarps[DWARP_ENTRY]._wy = lds->dSetLvlDunY;
	pWarps[DWARP_ENTRY]._wtype = lds->dSetLvlWarp;

	// load pre-dungeon
	pSetPieces[0]._spx = 0;
	pSetPieces[0]._spy = 0;
	pSetPieces[0]._sptype = lds->dSetLvlPiece;
	pSetPieces[0]._spData = LoadFileInMem(setpiecedata[pSetPieces[0]._sptype]._spdDunFile);
#if !USE_PATCH
	DRLG_L3FixMap();
#endif

	memset(drlgFlags, 0, sizeof(drlgFlags));
	static_assert(sizeof(dungeon[0][0]) == 1, "memset on dungeon does not work in LoadL3DungeonData.");
	memset(dungeon, BASE_MEGATILE_L3, sizeof(dungeon));

	DRLG_LoadSP(0, DEFAULT_MEGATILE_L3);
}

void CreateL3Dungeon()
{
	const LevelData* lds = &AllLevels[currLvl._dLevelNum];

	if (lds->dSetLvl) {
		LoadL3Dungeon(lds);
	} else {
		DRLG_LoadL3SP();
		DRLG_L3();
	}

#ifdef HELLFIRE
	if (currLvl._dType == DTYPE_NEST) {
		DRLG_L6Subs();
	} else 
#endif
	{
		DRLG_L3Subs();
	}

	memcpy(pdungeon, dungeon, sizeof(pdungeon));
	DRLG_L3DrawPreMaps();

	DRLG_L3InitTransVals();
	DRLG_PlaceMegaTiles(BASE_MEGATILE_L3);
}

DEVILUTION_END_NAMESPACE
