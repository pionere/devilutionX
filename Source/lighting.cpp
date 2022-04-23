/**
 * @file lighting.cpp
 *
 * Implementation of light and vision.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/* Maximum offset in a tile */
#define MAX_OFFSET		8
/* Maximum tile-distance till the precalculated tables are maintained. */
#define MAX_TILE_DIST	(MAX_LIGHT_RAD + 1)
/* Maximum light-distance till the precalculated tables are maintained. */
#define MAX_LIGHT_DIST	(MAX_TILE_DIST * MAX_OFFSET - 1)

/* The list of the indices of the active visions. */
BYTE visionactive[MAXVISION];
/* The list of visions/views in the game. */
LightListStruct VisionList[MAXVISION];
/* The list of the indices of the active light-sources. */
BYTE lightactive[MAXLIGHTS];
/* The list of light-sources in the game. */
LightListStruct LightList[MAXLIGHTS];
/* The number of visions/views in the game. */
int numvision;
/* The number of light-sources in the game. */
int numlights;
/* Specifies whether the visions should be re-processed. */
static bool _gbDovision;
/* Specifies whether the lights should be re-processed. */
static bool gbDolighting;
/*
 * Precalculated darkness (inverse brightness) levels for each light radius at a given distance.
 *  darkTable[lr][dist]
 * unit of dist is 0.125 tile width
 */
static BYTE darkTable[MAX_LIGHT_RAD + 1][MAX_LIGHT_DIST + 1];
/*
 * Precalculated distances from each offsets to a point in one quadrant.
 *  distMatrix[offy][offx][dy][dx]
 * Note: the distance value is limited to MAX_LIGHT_DIST to reduce the necessary
 *		checks in DoLighting.
 */
static BYTE distMatrix[MAX_OFFSET][MAX_OFFSET][MAX_TILE_DIST][MAX_TILE_DIST];
/*
 * In-game color translation tables.
 * 0-MAXDARKNESS: inverse brightness translations.
 * MAXDARKNESS+1: RED color translation.
 * MAXDARKNESS+2: GRAY color translation.
 * MAXDARKNESS+3: CORAL color translation.
 * MAXDARKNESS+4.. translations of unique monsters.
 */
BYTE ColorTrns[NUM_COLOR_TRNS][256];

/**
 * CrawlTable specifies X- and Y-coordinate deltas from a missile target coordinate.
 *
 * n=4
 *
 *    y
 *    ^
 *    |  1
 *    | 3#4
 *    |  2
 *    +-----> x
 *
 * n=16
 *
 *    y
 *    ^
 *    |  314
 *    | B7 8C
 *    | F # G
 *    | D9 AE
 *    |  526
 *    +-------> x
 */
const char CrawlTable[2749] = {
	1,										//  0 - 0
	  0,  0,
	4,										//  1 - 3
	  0,  1,    0, -1,   -1,  0,    1,  0,
	16,										//  2 - 12
	  0,  2,    0, -2,   -1,  2,    1,  2,
	 -1, -2,    1, -2,   -1,  1,    1,  1,
	 -1, -1,    1, -1,   -2,  1,    2,  1,
	 -2, -1,    2, -1,   -2,  0,    2,  0,
	24,						 				//  3 - 45
	  0,  3,    0, -3,   -1,  3,    1,  3,
	 -1, -3,    1, -3,   -2,  3,    2,  3,
	 -2, -3,    2, -3,   -2,  2,    2,  2,
	 -2, -2,    2, -2,   -3,  2,    3,  2,
	 -3, -2,    3, -2,   -3,  1,    3,  1,
	 -3, -1,    3, -1,   -3,  0,    3,  0,
	32,										//  4 - 94
	  0,  4,    0, -4,   -1,  4,    1,  4,
	 -1, -4,    1, -4,   -2,  4,    2,  4,
	 -2, -4,    2, -4,   -3,  4,    3,  4,
	 -3, -4,    3, -4,   -3,  3,    3,  3,
	 -3, -3,    3, -3,   -4,  3,    4,  3,
	 -4, -3,    4, -3,   -4,  2,    4,  2,
	 -4, -2,    4, -2,   -4,  1,    4,  1,
	 -4, -1,    4, -1,   -4,  0,    4,  0,
	40,							 			//  5 - 159
	  0,  5,    0, -5,   -1,  5,    1,  5,
	 -1, -5,    1, -5,   -2,  5,    2,  5,
	 -2, -5,    2, -5,   -3,  5,    3,  5,
	 -3, -5,    3, -5,   -4,  5,    4,  5,
	 -4, -5,    4, -5,   -4,  4,    4,  4,
	 -4, -4,    4, -4,   -5,  4,    5,  4,
	 -5, -4,    5, -4,   -5,  3,    5,  3,
	 -5, -3,    5, -3,   -5,  2,    5,  2,
	 -5, -2,    5, -2,   -5,  1,    5,  1,
	 -5, -1,    5, -1,   -5,  0,    5,  0,
	48,										//  6 - 240
	  0,  6,    0, -6,   -1,  6,    1,  6,
	 -1, -6,    1, -6,   -2,  6,    2,  6,
	 -2, -6,    2, -6,   -3,  6,    3,  6,
	 -3, -6,    3, -6,   -4,  6,    4,  6,
	 -4, -6,    4, -6,   -5,  6,    5,  6,
	 -5, -6,    5, -6,   -5,  5,    5,  5,
	 -5, -5,    5, -5,   -6,  5,    6,  5,
	 -6, -5,    6, -5,   -6,  4,    6,  4,
	 -6, -4,    6, -4,   -6,  3,    6,  3,
	 -6, -3,    6, -3,   -6,  2,    6,  2,
	 -6, -2,    6, -2,   -6,  1,    6,  1,
	 -6, -1,    6, -1,   -6,  0,    6,  0,
	56,										//  7 - 337
	  0,  7,    0, -7,   -1,  7,    1,  7,
	 -1, -7,    1, -7,   -2,  7,    2,  7,
	 -2, -7,    2, -7,   -3,  7,    3,  7,
	 -3, -7,    3, -7,   -4,  7,    4,  7,
	 -4, -7,    4, -7,   -5,  7,    5,  7,
	 -5, -7,    5, -7,   -6,  7,    6,  7,
	 -6, -7,    6, -7,   -6,  6,    6,  6,
	 -6, -6,    6, -6,   -7,  6,    7,  6,
	 -7, -6,    7, -6,   -7,  5,    7,  5,
	 -7, -5,    7, -5,   -7,  4,    7,  4,
	 -7, -4,    7, -4,   -7,  3,    7,  3,
	 -7, -3,    7, -3,   -7,  2,    7,  2,
	 -7, -2,    7, -2,   -7,  1,    7,  1,
	 -7, -1,    7, -1,   -7,  0,    7,  0,
	64,										//  8 - 450
	  0,  8,    0, -8,   -1,  8,    1,  8,
	 -1, -8,    1, -8,   -2,  8,    2,  8,
	 -2, -8,    2, -8,   -3,  8,    3,  8,
	 -3, -8,    3, -8,   -4,  8,    4,  8,
	 -4, -8,    4, -8,   -5,  8,    5,  8,
	 -5, -8,    5, -8,   -6,  8,    6,  8,
	 -6, -8,    6, -8,   -7,  8,    7,  8,
	 -7, -8,    7, -8,   -7,  7,    7,  7,
	 -7, -7,    7, -7,   -8,  7,    8,  7,
	 -8, -7,    8, -7,   -8,  6,    8,  6,
	 -8, -6,    8, -6,   -8,  5,    8,  5,
	 -8, -5,    8, -5,   -8,  4,    8,  4,
	 -8, -4,    8, -4,   -8,  3,    8,  3,
	 -8, -3,    8, -3,   -8,  2,    8,  2,
	 -8, -2,    8, -2,   -8,  1,    8,  1,
	 -8, -1,    8, -1,   -8,  0,    8,  0,
	72,										//  9 - 579
	  0,  9,    0, -9,   -1,  9,    1,  9,
	 -1, -9,    1, -9,   -2,  9,    2,  9,
	 -2, -9,    2, -9,   -3,  9,    3,  9,
	 -3, -9,    3, -9,   -4,  9,    4,  9,
	 -4, -9,    4, -9,   -5,  9,    5,  9,
	 -5, -9,    5, -9,   -6,  9,    6,  9,
	 -6, -9,    6, -9,   -7,  9,    7,  9,
	 -7, -9,    7, -9,   -8,  9,    8,  9,
	 -8, -9,    8, -9,   -8,  8,    8,  8,
	 -8, -8,    8, -8,   -9,  8,    9,  8,
	 -9, -8,    9, -8,   -9,  7,    9,  7,
	 -9, -7,    9, -7,   -9,  6,    9,  6,
	 -9, -6,    9, -6,   -9,  5,    9,  5,
	 -9, -5,    9, -5,   -9,  4,    9,  4,
	 -9, -4,    9, -4,   -9,  3,    9,  3,
	 -9, -3,    9, -3,   -9,  2,    9,  2,
	 -9, -2,    9, -2,   -9,  1,    9,  1,
	 -9, -1,    9, -1,   -9,  0,    9,  0,
	80,										// 10 - 724
	  0, 10,    0,-10,   -1, 10,    1, 10,
	 -1,-10,    1,-10,   -2, 10,    2, 10,
	 -2,-10,    2,-10,   -3, 10,    3, 10,
	 -3,-10,    3,-10,   -4, 10,    4, 10,
	 -4,-10,    4,-10,   -5, 10,    5, 10,
	 -5,-10,    5,-10,   -6, 10,    6, 10,
	 -6,-10,    6,-10,   -7, 10,    7, 10,
	 -7,-10,    7,-10,   -8, 10,    8, 10,
	 -8,-10,    8,-10,   -9, 10,    9, 10,
	 -9,-10,    9,-10,   -9,  9,    9,  9,
	 -9, -9,    9, -9,  -10,  9,   10,  9,
	-10, -9,   10, -9,  -10,  8,   10,  8,
	-10, -8,   10, -8,  -10,  7,   10,  7,
	-10, -7,   10, -7,  -10,  6,   10,  6,
	-10, -6,   10, -6,  -10,  5,   10,  5,
	-10, -5,   10, -5,  -10,  4,   10,  4,
	-10, -4,   10, -4,  -10,  3,   10,  3,
	-10, -3,   10, -3,  -10,  2,   10,  2,
	-10, -2,   10, -2,  -10,  1,   10,  1,
	-10, -1,   10, -1,  -10,  0,   10,  0,
	88,										// 11 - 885
	  0, 11,    0,-11,   -1, 11,    1, 11,
	 -1,-11,    1,-11,   -2, 11,    2, 11,
	 -2,-11,    2,-11,   -3, 11,    3, 11,
	 -3,-11,    3,-11,   -4, 11,    4, 11,
	 -4,-11,    4,-11,   -5, 11,    5, 11,
	 -5,-11,    5,-11,   -6, 11,    6, 11,
	 -6,-11,    6,-11,   -7, 11,    7, 11,
	 -7,-11,    7,-11,   -8, 11,    8, 11,
	 -8,-11,    8,-11,   -9, 11,    9, 11,
	 -9,-11,    9,-11,  -10, 11,   10, 11,
	-10,-11,   10,-11,  -10, 10,   10, 10,
	-10,-10,   10,-10,  -11, 10,   11, 10,
	-11,-10,   11,-10,  -11,  9,   11,  9,
	-11, -9,   11, -9,  -11,  8,   11,  8,
	-11, -8,   11, -8,  -11,  7,   11,  7,
	-11, -7,   11, -7,  -11,  6,   11,  6,
	-11, -6,   11, -6,  -11,  5,   11,  5,
	-11, -5,   11, -5,  -11,  4,   11,  4,
	-11, -4,   11, -4,  -11,  3,   11,  3,
	-11, -3,   11, -3,  -11,  2,   11,  2,
	-11, -2,   11, -2,  -11,  1,   11,  1,
	-11, -1,   11, -1,  -11,  0,   11,  0,
	96,										// 12 - 1062
	  0, 12,    0,-12,   -1, 12,    1, 12,
	 -1,-12,    1,-12,   -2, 12,    2, 12,
	 -2,-12,    2,-12,   -3, 12,    3, 12,
	 -3,-12,    3,-12,   -4, 12,    4, 12,
	 -4,-12,    4,-12,   -5, 12,    5, 12,
	 -5,-12,    5,-12,   -6, 12,    6, 12,
	 -6,-12,    6,-12,   -7, 12,    7, 12,
	 -7,-12,    7,-12,   -8, 12,    8, 12,
	 -8,-12,    8,-12,   -9, 12,    9, 12,
	 -9,-12,    9,-12,  -10, 12,   10, 12,
	-10,-12,   10,-12,  -11, 12,   11, 12,
	-11,-12,   11,-12,  -11, 11,   11, 11,
	-11,-11,   11,-11,  -12, 11,   12, 11,
	-12,-11,   12,-11,  -12, 10,   12, 10,
	-12,-10,   12,-10,  -12,  9,   12,  9,
	-12, -9,   12, -9,  -12,  8,   12,  8,
	-12, -8,   12, -8,  -12,  7,   12,  7,
	-12, -7,   12, -7,  -12,  6,   12,  6,
	-12, -6,   12, -6,  -12,  5,   12,  5,
	-12, -5,   12, -5,  -12,  4,   12,  4,
	-12, -4,   12, -4,  -12,  3,   12,  3,
	-12, -3,   12, -3,  -12,  2,   12,  2,
	-12, -2,   12, -2,  -12,  1,   12,  1,
	-12, -1,   12, -1,  -12,  0,   12,  0,
	104,									// 13 - 1255
	  0, 13,    0,-13,   -1, 13,    1, 13,
	 -1,-13,    1,-13,   -2, 13,    2, 13,
	 -2,-13,    2,-13,   -3, 13,    3, 13,
	 -3,-13,    3,-13,   -4, 13,    4, 13,
	 -4,-13,    4,-13,   -5, 13,    5, 13,
	 -5,-13,    5,-13,   -6, 13,    6, 13,
	 -6,-13,    6,-13,   -7, 13,    7, 13,
	 -7,-13,    7,-13,   -8, 13,    8, 13,
	 -8,-13,    8,-13,   -9, 13,    9, 13,
	 -9,-13,    9,-13,  -10, 13,   10, 13,
	-10,-13,   10,-13,  -11, 13,   11, 13,
	-11,-13,   11,-13,  -12, 13,   12, 13,
	-12,-13,   12,-13,  -12, 12,   12, 12,
	-12,-12,   12,-12,  -13, 12,   13, 12,
	-13,-12,   13,-12,  -13, 11,   13, 11,
	-13,-11,   13,-11,  -13, 10,   13, 10,
	-13,-10,   13,-10,  -13,  9,   13,  9,
	-13, -9,   13, -9,  -13,  8,   13,  8,
	-13, -8,   13, -8,  -13,  7,   13,  7,
	-13, -7,   13, -7,  -13,  6,   13,  6,
	-13, -6,   13, -6,  -13,  5,   13,  5,
	-13, -5,   13, -5,  -13,  4,   13,  4,
	-13, -4,   13, -4,  -13,  3,   13,  3,
	-13, -3,   13, -3,  -13,  2,   13,  2,
	-13, -2,   13, -2,  -13,  1,   13,  1,
	-13, -1,   13, -1,  -13,  0,   13,  0,
	112,									// 14 - 1464
	  0, 14,    0,-14,   -1, 14,    1, 14,
	 -1,-14,    1,-14,   -2, 14,    2, 14,
	 -2,-14,    2,-14,   -3, 14,    3, 14,
	 -3,-14,    3,-14,   -4, 14,    4, 14,
	 -4,-14,    4,-14,   -5, 14,    5, 14,
	 -5,-14,    5,-14,   -6, 14,    6, 14,
	 -6,-14,    6,-14,   -7, 14,    7, 14,
	 -7,-14,    7,-14,   -8, 14,    8, 14,
	 -8,-14,    8,-14,   -9, 14,    9, 14,
	 -9,-14,    9,-14,  -10, 14,   10, 14,
	-10,-14,   10,-14,  -11, 14,   11, 14,
	-11,-14,   11,-14,  -12, 14,   12, 14,
	-12,-14,   12,-14,  -13, 14,   13, 14,
	-13,-14,   13,-14,  -13, 13,   13, 13,
	-13,-13,   13,-13,  -14, 13,   14, 13,
	-14,-13,   14,-13,  -14, 12,   14, 12,
	-14,-12,   14,-12,  -14, 11,   14, 11,
	-14,-11,   14,-11,  -14, 10,   14, 10,
	-14,-10,   14,-10,  -14,  9,   14,  9,
	-14, -9,   14, -9,  -14,  8,   14,  8,
	-14, -8,   14, -8,  -14,  7,   14,  7,
	-14, -7,   14, -7,  -14,  6,   14,  6,
	-14, -6,   14, -6,  -14,  5,   14,  5,
	-14, -5,   14, -5,  -14,  4,   14,  4,
	-14, -4,   14, -4,  -14,  3,   14,  3,
	-14, -3,   14, -3,  -14,  2,   14,  2,
	-14, -2,   14, -2,  -14,  1,   14,  1,
	-14, -1,   14, -1,  -14,  0,   14,  0,
	120,									// 15 - 1689
	  0, 15,    0,-15,   -1, 15,    1, 15,
	 -1,-15,    1,-15,   -2, 15,    2, 15,
	 -2,-15,    2,-15,   -3, 15,    3, 15,
	 -3,-15,    3,-15,   -4, 15,    4, 15,
	 -4,-15,    4,-15,   -5, 15,    5, 15,
	 -5,-15,    5,-15,   -6, 15,    6, 15,
	 -6,-15,    6,-15,   -7, 15,    7, 15,
	 -7,-15,    7,-15,   -8, 15,    8, 15,
	 -8,-15,    8,-15,   -9, 15,    9, 15,
	 -9,-15,    9,-15,  -10, 15,   10, 15,
	-10,-15,   10,-15,  -11, 15,   11, 15,
	-11,-15,   11,-15,  -12, 15,   12, 15,
	-12,-15,   12,-15,  -13, 15,   13, 15,
	-13,-15,   13,-15,  -14, 15,   14, 15,
	-14,-15,   14,-15,  -14, 14,   14, 14,
	-14,-14,   14,-14,  -15, 14,   15, 14,
	-15,-14,   15,-14,  -15, 13,   15, 13,
	-15,-13,   15,-13,  -15, 12,   15, 12,
	-15,-12,   15,-12,  -15, 11,   15, 11,
	-15,-11,   15,-11,  -15, 10,   15, 10,
	-15,-10,   15,-10,  -15,  9,   15,  9,
	-15, -9,   15, -9,  -15,  8,   15,  8,
	-15, -8,   15, -8,  -15,  7,   15,  7,
	-15, -7,   15, -7,  -15,  6,   15,  6,
	-15, -6,   15, -6,  -15,  5,   15,  5,
	-15, -5,   15, -5,  -15,  4,   15,  4,
	-15, -4,   15, -4,  -15,  3,   15,  3,
	-15, -3,   15, -3,  -15,  2,   15,  2,
	-15, -2,   15, -2,  -15,  1,   15,  1,
	-15, -1,   15, -1,  -15,  0,   15,  0,
	(char)128,								// 16 - 1930
	0, 16, 0, -16, -1, 16, 1, 16,
	-1, -16, 1, -16, -2, 16, 2, 16,
	-2, -16, 2, -16, -3, 16, 3, 16,
	-3, -16, 3, -16, -4, 16, 4, 16,
	-4, -16, 4, -16, -5, 16, 5, 16,
	-5, -16, 5, -16, -6, 16, 6, 16,
	-6, -16, 6, -16, -7, 16, 7, 16,
	-7, -16, 7, -16, -8, 16, 8, 16,
	-8, -16, 8, -16, -9, 16, 9, 16,
	-9, -16, 9, -16, -10, 16, 10, 16,
	-10, -16, 10, -16, -11, 16, 11, 16,
	-11, -16, 11, -16, -12, 16, 12, 16,
	-12, -16, 12, -16, -13, 16, 13, 16,
	-13, -16, 13, -16, -14, 16, 14, 16,
	-14, -16, 14, -16, -15, 16, 15, 16,
	-15, -16, 15, -16, -15, 15, 15, 15,
	-15, -15, 15, -15, -16, 15, 16, 15,
	-16, -15, 16, -15, -16, 14, 16, 14,
	-16, -14, 16, -14, -16, 13, 16, 13,
	-16, -13, 16, -13, -16, 12, 16, 12,
	-16, -12, 16, -12, -16, 11, 16, 11,
	-16, -11, 16, -11, -16, 10, 16, 10,
	-16, -10, 16, -10, -16, 9, 16, 9,
	-16, -9, 16, -9, -16, 8, 16, 8,
	-16, -8, 16, -8, -16, 7, 16, 7,
	-16, -7, 16, -7, -16, 6, 16, 6,
	-16, -6, 16, -6, -16, 5, 16, 5,
	-16, -5, 16, -5, -16, 4, 16, 4,
	-16, -4, 16, -4, -16, 3, 16, 3,
	-16, -3, 16, -3, -16, 2, 16, 2,
	-16, -2, 16, -2, -16, 1, 16, 1,
	-16, -1, 16, -1, -16, 0, 16, 0,
	(char)136,								// 17 - 2187
	0, 17, 0, -17, -1, 17, 1, 17,
	-1, -17, 1, -17, -2, 17, 2, 17,
	-2, -17, 2, -17, -3, 17, 3, 17,
	-3, -17, 3, -17, -4, 17, 4, 17,
	-4, -17, 4, -17, -5, 17, 5, 17,
	-5, -17, 5, -17, -6, 17, 6, 17,
	-6, -17, 6, -17, -7, 17, 7, 17,
	-7, -17, 7, -17, -8, 17, 8, 17,
	-8, -17, 8, -17, -9, 17, 9, 17,
	-9, -17, 9, -17, -10, 17, 10, 17,
	-10, -17, 10, -17, -11, 17, 11, 17,
	-11, -17, 11, -17, -12, 17, 12, 17,
	-12, -17, 12, -17, -13, 17, 13, 17,
	-13, -17, 13, -17, -14, 17, 14, 17,
	-14, -17, 14, -17, -15, 17, 15, 17,
	-15, -17, 15, -17, -16, 17, 16, 17,
	-16, -17, 16, -17, -16, 16, 16, 16,
	-16, -16, 16, -16, -17, 16, 17, 16,
	-17, -16, 17, -16, -17, 15, 17, 15,
	-17, -15, 17, -15, -17, 14, 17, 14,
	-17, -14, 17, -14, -17, 13, 17, 13,
	-17, -13, 17, -13, -17, 12, 17, 12,
	-17, -12, 17, -12, -17, 11, 17, 11,
	-17, -11, 17, -11, -17, 10, 17, 10,
	-17, -10, 17, -10, -17, 9, 17, 9,
	-17, -9, 17, -9, -17, 8, 17, 8,
	-17, -8, 17, -8, -17, 7, 17, 7,
	-17, -7, 17, -7, -17, 6, 17, 6,
	-17, -6, 17, -6, -17, 5, 17, 5,
	-17, -5, 17, -5, -17, 4, 17, 4,
	-17, -4, 17, -4, -17, 3, 17, 3,
	-17, -3, 17, -3, -17, 2, 17, 2,
	-17, -2, 17, -2, -17, 1, 17, 1,
	-17, -1, 17, -1, -17, 0, 17, 0,
	(char)144,								// 18 - 2460
	0, 18, 0, -18, -1, 18, 1, 18,
	-1, -18, 1, -18, -2, 18, 2, 18,
	-2, -18, 2, -18, -3, 18, 3, 18,
	-3, -18, 3, -18, -4, 18, 4, 18,
	-4, -18, 4, -18, -5, 18, 5, 18,
	-5, -18, 5, -18, -6, 18, 6, 18,
	-6, -18, 6, -18, -7, 18, 7, 18,
	-7, -18, 7, -18, -8, 18, 8, 18,
	-8, -18, 8, -18, -9, 18, 9, 18,
	-9, -18, 9, -18, -10, 18, 10, 18,
	-10, -18, 10, -18, -11, 18, 11, 18,
	-11, -18, 11, -18, -12, 18, 12, 18,
	-12, -18, 12, -18, -13, 18, 13, 18,
	-13, -18, 13, -18, -14, 18, 14, 18,
	-14, -18, 14, -18, -15, 18, 15, 18,
	-15, -18, 15, -18, -16, 18, 16, 18,
	-16, -18, 16, -18, -17, 18, 17, 18,
	-17, -18, 17, -18, -17, 17, 17, 17,
	-17, -17, 17, -17, -18, 17, 18, 17,
	-18, -17, 18, -17, -18, 16, 18, 16,
	-18, -16, 18, -16, -18, 15, 18, 15,
	-18, -15, 18, -15, -18, 14, 18, 14,
	-18, -14, 18, -14, -18, 13, 18, 13,
	-18, -13, 18, -13, -18, 12, 18, 12,
	-18, -12, 18, -12, -18, 11, 18, 11,
	-18, -11, 18, -11, -18, 10, 18, 10,
	-18, -10, 18, -10, -18, 9, 18, 9,
	-18, -9, 18, -9, -18, 8, 18, 8,
	-18, -8, 18, -8, -18, 7, 18, 7,
	-18, -7, 18, -7, -18, 6, 18, 6,
	-18, -6, 18, -6, -18, 5, 18, 5,
	-18, -5, 18, -5, -18, 4, 18, 4,
	-18, -4, 18, -4, -18, 3, 18, 3,
	-18, -3, 18, -3, -18, 2, 18, 2,
	-18, -2, 18, -2, -18, 1, 18, 1,
	-18, -1, 18, -1, -18, 0, 18, 0
};

/** Indices of CrawlTable to select the entries at a given distance. */
const int CrawlNum[19] = { 0, 3, 12, 45, 94, 159, 240, 337, 450, 579, 724, 885, 1062, 1255, 1464, 1689, 1930, 2187, 2460 };

static void RotateRadius(int *x, int *y, int *dx, int *dy, int *lx, int *ly, int *bx, int *by)
{
	int swap;

	*bx = 0;
	*by = 0;

	swap = *dx;
	*dx = 7 - *dy;
	*dy = swap;
	swap = *lx;
	*lx = 7 - *ly;
	*ly = swap;

	*x = *dx - *lx;
	*y = *dy - *ly;

	if (*x < 0) {
		*x += 8;
		*bx = 1;
	}
	if (*y < 0) {
		*y += 8;
		*by = 1;
	}
}

void DoLighting(int nXPos, int nYPos, int nRadius, unsigned lnum)
{
	int x, y, xoff, yoff;
	int min_x, max_x, min_y, max_y;
	int dist_x, dist_y, light_x, light_y, block_x, block_y, temp_x, temp_y;
	BYTE (&dark)[128] = darkTable[nRadius];
	BYTE v, radius_block;

	xoff = 0;
	yoff = 0;
	light_x = 0;
	light_y = 0;
	block_x = 0;
	block_y = 0;

	if (lnum < MAXLIGHTS) {
		xoff = LightList[lnum]._xoff;
		yoff = LightList[lnum]._yoff;
		if (xoff < 0) {
			xoff += 8;
			nXPos--;
		} else if (xoff >= 8) {
			xoff -= 8;
			nXPos++;
		}
		if (yoff < 0) {
			yoff += 8;
			nYPos--;
		} else if (yoff >= 8) {
			yoff -= 8;
			nYPos++;
		}
		assert((unsigned)xoff < 8);
		assert((unsigned)yoff < 8);
	}

	dist_x = xoff;
	dist_y = yoff;

	static_assert(DBORDERX >= MAX_LIGHT_RAD + 1, "DoLighting expects a large enough border I.");
	static_assert(DBORDERY >= MAX_LIGHT_RAD + 1, "DoLighting expects a large enough border II.");
	assert(MAX_LIGHT_RAD <= MAXDUNX - nXPos);
	//max_x = MAX_LIGHT_RAD; //std::min(15, MAXDUNX - nXPos);
	assert(MAX_LIGHT_RAD <= MAXDUNY - nYPos);
	//max_y = MAX_LIGHT_RAD; //std::min(15, MAXDUNY - nYPos);
	assert(MAX_LIGHT_RAD <= nXPos + 1);
	//min_x = MAX_LIGHT_RAD; //std::min(15, nXPos + 1);
	assert(MAX_LIGHT_RAD <= nYPos + 1);
	//min_y = MAX_LIGHT_RAD; //std::min(15, nYPos + 1);

	nRadius++;
	min_x = min_y = max_x = max_y = std::min(MAX_LIGHT_RAD, nRadius);

	BYTE (&dist0)[MAX_TILE_DIST][MAX_TILE_DIST] = distMatrix[yoff][xoff];
	// Add light to (0;0)
	{
			radius_block = dist0[0][0];
			//assert(radius_block <= MAX_LIGHT_DIST);
			//if (radius_block <= MAX_LIGHT_DIST) {
				temp_x = nXPos + 0;
				temp_y = nYPos + 0;
				v = dark[radius_block];
				if (v < dLight[temp_x][temp_y])
					dLight[temp_x][temp_y] = v;
			//}
	}
	// Add light to the I. (+;+) quadrant
	for (y = 0; y < max_y; y++) {
		for (x = 1; x < max_x; x++) {
			radius_block = dist0[y][x];
			//assert(radius_block <= MAX_LIGHT_DIST);
			//if (radius_block <= MAX_LIGHT_DIST) {
				temp_x = nXPos + x;
				temp_y = nYPos + y;
				v = dark[radius_block];
				if (v < dLight[temp_x][temp_y])
					dLight[temp_x][temp_y] = v;
			//}
		}
	}
	RotateRadius(&xoff, &yoff, &dist_x, &dist_y, &light_x, &light_y, &block_x, &block_y);
	// Add light to the II. (+;-) quadrant
	BYTE (&dist1)[MAX_TILE_DIST][MAX_TILE_DIST] = distMatrix[yoff][xoff];
	for (y = 0; y < max_x; y++) {
		for (x = 1; x < min_y; x++) {
			radius_block = dist1[y + block_y][x + block_x];
			//assert(radius_block <= MAX_LIGHT_DIST);
			//if (radius_block <= MAX_LIGHT_DIST) {
				temp_x = nXPos + y;
				temp_y = nYPos - x;
				v = dark[radius_block];
				if (v < dLight[temp_x][temp_y])
					dLight[temp_x][temp_y] = v;
			//}
		}
	}
	RotateRadius(&xoff, &yoff, &dist_x, &dist_y, &light_x, &light_y, &block_x, &block_y);
	// Add light to the III. (-;-) quadrant
	BYTE (&dist2)[MAX_TILE_DIST][MAX_TILE_DIST] = distMatrix[yoff][xoff];
	for (y = 0; y < min_y; y++) {
		for (x = 1; x < min_x; x++) {
			radius_block = dist2[y + block_y][x + block_x];
			//assert(radius_block <= MAX_LIGHT_DIST);
			//if (radius_block <= MAX_LIGHT_DIST) {
				temp_x = nXPos - x;
				temp_y = nYPos - y;
				v = dark[radius_block];
				if (v < dLight[temp_x][temp_y])
					dLight[temp_x][temp_y] = v;
			//}
		}
	}
	RotateRadius(&xoff, &yoff, &dist_x, &dist_y, &light_x, &light_y, &block_x, &block_y);
	// Add light to the IV. (-;+) quadrant
	BYTE (&dist3)[MAX_TILE_DIST][MAX_TILE_DIST] = distMatrix[yoff][xoff];
	for (y = 0; y < min_x; y++) {
		for (x = 1; x < max_y; x++) {
			radius_block = dist3[y + block_y][x + block_x];
			//assert(radius_block <= MAX_LIGHT_DIST);
			//if (radius_block <= MAX_LIGHT_DIST) {
				temp_x = nXPos - y;
				temp_y = nYPos + x;
				v = dark[radius_block];
				if (v < dLight[temp_x][temp_y])
					dLight[temp_x][temp_y] = v;
			//}
		}
	}
}

static void DoUnLight(int nXPos, int nYPos, int nRadius)
{
	int x, y, min_x, min_y, max_x, max_y;

	nRadius++;
	min_y = nYPos - nRadius;
	max_y = nYPos + nRadius;
	min_x = nXPos - nRadius;
	max_x = nXPos + nRadius;
	static_assert(DBORDERY >= MAX_LIGHT_RAD + 1, "DoUnLight skips limit-checks assuming large enough border I.");
	assert(min_y >= 0);
	assert(max_y <= MAXDUNY);
	static_assert(DBORDERX >= MAX_LIGHT_RAD + 1, "DoUnLight skips limit-checks assuming large enough border II.");
	assert(min_x >= 0);
	assert(max_x <= MAXDUNX);

	for (y = min_y; y < max_y; y++) {
		for (x = min_x; x < max_x; x++) {
			dLight[x][y] = dPreLight[x][y];
		}
	}
}

void DoUnVision(int nXPos, int nYPos, int nRadius)
{
	int i, j, x1, y1, x2, y2;

	nRadius++;
	y1 = nYPos - nRadius;
	y2 = nYPos + nRadius;
	x1 = nXPos - nRadius;
	x2 = nXPos + nRadius;

	static_assert(DBORDERY >= MAX_LIGHT_RAD + 1, "DoUnVision skips limit-checks assuming large enough border I.");
	assert(y1 >= 0);
	assert(y2 <= MAXDUNY);
	static_assert(DBORDERX >= MAX_LIGHT_RAD + 1, "DoUnVision skips limit-checks assuming large enough border II.");
	assert(x1 >= 0);
	assert(x2 <= MAXDUNX);

	for (i = x1; i < x2; i++) {
		for (j = y1; j < y2; j++) {
			dFlags[i][j] &= ~(BFLAG_ALERT | BFLAG_VISIBLE);
		}
	}
}

static bool doautomap;
static BYTE vFlags;
static bool LightPos(int x1, int y1)
{
	//int nTrans;
	assert(IN_DUNGEON_AREA(x1, y1));
	dFlags[x1][y1] |= vFlags;
	if (doautomap) {
		if (!(dFlags[x1][y1] & BFLAG_EXPLORED)) {
			dFlags[x1][y1] |= BFLAG_EXPLORED;
			SetAutomapView(x1, y1);
		}
	}
	return !nBlockTable[dPiece[x1][y1]];
	/* skip this to not make tiles transparent based on visible tv values. only the tv of the player's tile should matter.
	if (nBlockTable[dPiece[x1][y1]])
		return false;
	nTrans = dTransVal[x1][y1];
	if (nTrans != 0) {
		TransList[nTrans] = true;
	}
	return true;*/
}

/*
 * Mark tiles alert/visible/explored starting from nXPos:nYpos using raytracing algorithm.
 *
 * @param nXPos: the starting x-coordinate
 * @param nYPos: the starting y-coordinate
 * @param nRadius: the maximum distance where the tile might be alert/visible
 * @param local: whether it is called for the local player
 *    true: the tiles are marked alert, visible and explored + TransList updated
 *    false: the tiles are marked alert
 */
void DoVision(int nXPos, int nYPos, int nRadius, bool local)
{
	const char* cr;
	int i, x1, y1, limit;
	int d, dx, dy, xinc, yinc;
	vFlags = local ? BFLAG_VISIBLE | BFLAG_ALERT : BFLAG_ALERT;
	doautomap = local;

	assert(IN_DUNGEON_AREA(nXPos, nYPos));
	dFlags[nXPos][nYPos] |= vFlags;
	if (doautomap) {
		if (!(dFlags[nXPos][nYPos] & BFLAG_EXPLORED)) {
			dFlags[nXPos][nYPos] |= BFLAG_EXPLORED;
			SetAutomapView(nXPos, nYPos);
		}
		i = dTransVal[nXPos][nYPos];
		if (i != 0) {
			TransList[i] = true;
		}
	}
	nRadius = 2 * (nRadius + 1);
	cr = &CrawlTable[CrawlNum[15]];
	for (i = (BYTE)*cr; i > 0; i--) {
		x1 = nXPos;
		y1 = nYPos;
		limit = nRadius;
		dx = *++cr;
		dy = *++cr;

		// find out step size and direction on the y coordinate
		xinc = dx < 0 ? -1 : 1;
		yinc = dy < 0 ? -1 : 1;

		dy = abs(dy);
		dx = abs(dx);
		if (dx >= dy) {
			assert(dx != 0);

			// multiply by 2 so we round up
			dy *= 2;
			d = 0;
			do {
				d += dy;
				if (d >= dx) {
					d -= 2 * dx; // multiply by 2 to support rounding
					y1 += yinc;
					limit--;
				}
				x1 += xinc;
				limit -= 2;
				if (limit <= 0)
					break;
			} while (LightPos(x1, y1));
		} else {
			// multiply by 2 so we round up
			dx *= 2;
			d = 0;
			do {
				d += dx;
				if (d >= dy) {
					d -= 2 * dy; // multiply by 2 to support rounding
					x1 += xinc;
					limit--;
				}
				y1 += yinc;
				limit -= 2;
				if (limit <= 0)
					break;
			} while (LightPos(x1, y1));
		}
	}
}

void MakeLightTable()
{
	unsigned i, j, k, shade, l1, l2, cnt, rem, div;
	BYTE col, max;
	BYTE* tbl;
	BYTE blood[16];

	tbl = ColorTrns[0];
	for (i = 0; i < MAXDARKNESS; i++) {
		static_assert(MAXDARKNESS == 15, "Shade calculation requires MAXDARKNESS to be 15.");
		// shade calculation is simplified by using a fix MAXDARKNESS value.
		// otherwise the correct calculation would be as follows:
		//	shade = (i * 15 + (MAXDARKNESS + 1) / 2) / MAXDARKNESS;
		shade = i;
		// light trns of the level palette
		*tbl++ = 0;
		for (j = 0; j < 8; j++) {
			col = 16 * j + shade;
			max = 16 * j + 15;
			for (k = 0; k < 16; k++) {
				if (k != 0 || j != 0) {
					*tbl++ = col;
				}
				if (col < max) {
					col++;
				} else {
					max = 0;
					col = 0;
				}
			}
		}
		// light trns of the standard palette
		//  (PAL8_BLUE, PAL8_RED, PAL8_YELLOW, PAL8_ORANGE)
		for (j = 16; j < 20; j++) {
			col = 8 * j + (shade >> 1);
			max = 8 * j + 7;
			for (k = 0; k < 8; k++) {
				*tbl++ = col;
				if (col < max) {
					col++;
				} else {
					max = 0;
					col = 0;
				}
			}
		}
		//  (PAL16_BEIGE, PAL16_BLUE, PAL16_YELLOW, PAL16_ORANGE, PAL16_RED, PAL16_GRAY)
		for (j = 10; j < 16; j++) {
			col = 16 * j + shade;
			max = 16 * j + 15;
			for (k = 0; k < 16; k++) {
				*tbl++ = col;
				if (col < max) {
					col++;
				} else {
					max = 0;
					col = 0;
				}
				if (col == 255) {
					max = 0;
					col = 0;
				}
			}
		}
	}

	// assert(tbl == ColorTrns[MAXDARKNESS]);
	tbl = ColorTrns[0];
	memset(ColorTrns[MAXDARKNESS], 0, 256);

	if (currLvl._dType == DTYPE_HELL) {
		for (i = 0; i < MAXDARKNESS; i++) {
			l1 = MAXDARKNESS - i;
			l2 = l1;
			div = 15 / l1;
			rem = 15 % l1;
			cnt = 0;
			blood[0] = 0;
			col = 1;
			for (j = 1; j < 16; j++) {
				blood[j] = col;
				l2 += rem;
				if (l2 > l1 && j < 15) {
					j++;
					blood[j] = col;
					l2 -= l1;
				}
				cnt++;
				if (cnt == div) {
					col++;
					cnt = 0;
				}
			}
			*tbl++ = 0;
			for (j = 1; j <= 15; j++) {
				*tbl++ = blood[j];
			}
			for (j = 15; j > 0; j--) {
				*tbl++ = blood[j];
			}
			*tbl++ = 1;
			tbl += 224;
		}
		/**tbl++ = 0;
		for (j = 0; j < 31; j++) {
			*tbl++ = 1;
		}
		tbl += 224;*/
#ifdef HELLFIRE
	} else if (currLvl._dType == DTYPE_NEST || currLvl._dType == DTYPE_CRYPT) {
		for (i = 0; i < MAXDARKNESS; i++) {
			*tbl++ = 0;
			for (j = 1; j < 16; j++)
				*tbl++ = j;
			tbl += 240;
		}
		/**tbl++ = 0;
		for (j = 1; j < 16; j++)
			*tbl++ = 1;
		tbl += 240;*/
#endif
	}
}

void InitLightGFX()
{
	//BYTE* tbl;
	//int i, j, k, l;
	//BYTE col;
	//double fs, fa;

	LoadFileWithMem("PlrGFX\\Infra.TRN", ColorTrns[COLOR_TRN_RED]);
	LoadFileWithMem("PlrGFX\\Stone.TRN", ColorTrns[COLOR_TRN_GRAY]);
	LoadFileWithMem("PlrGFX\\Coral.TRN", ColorTrns[COLOR_TRN_CORAL]);

	/*tbl = ColorTrns[COLOR_TRN_CORAL];
	for (i = 0; i < 8; i++) {
		for (col = 226; col < 239; col++) {
			if (i != 0 || col != 226) {
				*tbl++ = col;
			} else {
				*tbl++ = 0;
			}
		}
		*tbl++ = 0;
		*tbl++ = 0;
		*tbl++ = 0;
	}
	for (i = 0; i < 4; i++) {
		col = 224;
		for (j = 224; j < 239; j += 2) {
			*tbl++ = col;
			col += 2;
		}
	}
	for (i = 0; i < 6; i++) {
		for (col = 224; col < 239; col++) {
			*tbl++ = col;
		}
		*tbl++ = 0;
	}*/

/*#ifdef HELLFIRE
	if (currLvl._dType == DTYPE_NEST || currLvl._dType == DTYPE_CRYPT) {
		for (i = 0; i < 16; i++) {
			for (j = 128; j > 0; j--) {
				k = 15 - ((i + 1) * (j * j)) / (128 * 128);
				k -= i >> 1;
				if (k < 0)
					k = 0;
				darkTable[i][128 - j] = k;
			}
		}
	} else
#endif*/
	LoadFileWithMem("Meta\\Dark.tbl", &darkTable[0][0]);
	/*{
		memset(darkTable[0], MAXDARKNESS, MAX_LIGHT_DIST);
		for (i = 1, k = 2 * MAX_OFFSET; i <= MAX_LIGHT_RAD; i++, k += MAX_OFFSET) {
			for (j = 0; j <= MAX_LIGHT_DIST; j++) {
				if (j >= k) {
					darkTable[i][j] = MAXDARKNESS;
				} else {
					darkTable[i][j] = (MAXDARKNESS * j) / k;
				}
			}
		}
	}*/
	LoadFileWithMem("Meta\\Dist.tbl", &distMatrix[0][0][0][0]);
	/*for (j = 0; j < MAX_OFFSET; j++) {
		for (i = 0; i < MAX_OFFSET; i++) {
			for (k = 0; k < MAX_TILE_DIST; k++) {
				fa = (MAX_OFFSET * k - i);
				fa *= fa;
				for (l = 0; l < MAX_TILE_DIST; l++) {
					fs = (MAX_OFFSET * l - j);
					fs *= fs;
					fs = sqrt(fs + fa);
					// round to nearest int
					col = fs + 0.5;
					// limit to MAX_LIGHT_DIST to reduce the necessary checks in DoLighting
					static_assert(MAX_LIGHT_DIST <= UCHAR_MAX, "Distance can not be stored in a BYTE.");
					col = std::min((BYTE)MAX_LIGHT_DIST, col);
					distMatrix[j][i][k][l] = col;
				}
			}
		}
	}*/
}

#if DEBUG_MODE
void ToggleLighting()
{
	int pnum;

	lightflag = !lightflag;

	if (lightflag) {
		memset(dLight, 0, sizeof(dLight));
	} else {
		memcpy(dLight, dPreLight, sizeof(dLight));
		for (pnum = 0; pnum < MAX_PLRS; pnum++) {
			if (plr._pActive && plr._pDunLevel == currLvl._dLevelIdx) {
				DoLighting(plr._px, plr._py, plr._pLightRad, NO_LIGHT);
			}
		}
	}
}

#endif

void InitLighting()
{
	int i;

	numlights = 0;
	gbDolighting = false;
#if DEBUG_MODE
	lightflag = false;
#endif

	for (i = 0; i < MAXLIGHTS; i++) {
		lightactive[i] = i;
	}
}

unsigned AddLight(int x, int y, int r)
{
	LightListStruct *lis;
	int lnum;

#if DEBUG_MODE
	if (lightflag)
		return NO_LIGHT;
#endif
	static_assert(NO_LIGHT >= MAXLIGHTS, "Handling of lights expects NO_LIGHT out of the [0..MAXLIGHTS) range");
	lnum = NO_LIGHT;

	if (numlights < MAXLIGHTS) {
		lnum = lightactive[numlights++];
		lis = &LightList[lnum];
		lis->_lunx = lis->_lx = x;
		lis->_luny = lis->_ly = y;
		lis->_lunr = lis->_lradius = r;
		lis->_xoff = 0;
		lis->_yoff = 0;
		lis->_ldel = false;
		lis->_lunflag = false;
		gbDolighting = true;
	}

	return lnum;
}

void AddUnLight(unsigned lnum)
{
#if DEBUG_MODE
	if (lightflag)
		return;
#endif
	if (lnum >= MAXLIGHTS)
		return;

	LightList[lnum]._ldel = true;
	gbDolighting = true;
}

void ChangeLightRadius(unsigned lnum, int r)
{
	LightListStruct *lis;

#if DEBUG_MODE
	if (lightflag)
		return;
#endif
	if (lnum >= MAXLIGHTS)
		return;

	lis = &LightList[lnum];
	lis->_lunflag = true;
	lis->_lradius = r;
	gbDolighting = true;
}

void ChangeLightXY(unsigned lnum, int x, int y)
{
	LightListStruct *lis;

#if DEBUG_MODE
	if (lightflag)
		return;
#endif
	if (lnum >= MAXLIGHTS)
		return;

	lis = &LightList[lnum];
	lis->_lunflag = true;
	lis->_lx = x;
	lis->_ly = y;
	gbDolighting = true;
}

void ChangeLightOff(unsigned lnum, int xoff, int yoff)
{
	LightListStruct *lis;

#if DEBUG_MODE
	if (lightflag)
		return;
#endif
	if (lnum >= MAXLIGHTS)
		return;

	lis = &LightList[lnum];
	lis->_lunflag = true;
	lis->_xoff = xoff;
	lis->_yoff = yoff;
	gbDolighting = true;
}

/*
 * Same as ChangeLightXY, but also sets the x/y-offsets to zero.
 */
void ChangeLightXYOff(unsigned lnum, int x, int y)
{
	LightListStruct *lis;

#if DEBUG_MODE
	if (lightflag)
		return;
#endif
	if (lnum >= MAXLIGHTS)
		return;

	lis = &LightList[lnum];
	lis->_lunflag = true;
	lis->_lx = x;
	lis->_ly = y;
	lis->_xoff = 0;
	lis->_yoff = 0;
	gbDolighting = true;
}

void CondChangeLightOff(unsigned lnum, int xoff, int yoff)
{
	LightListStruct *lis;
	int lx, ly;
	int offx, offy;

#if DEBUG_MODE
	if (lightflag)
		return;
#endif
	if (lnum >= MAXLIGHTS)
		return;

	lis = &LightList[lnum];
	lx = xoff + (lis->_lx << 3);
	ly = yoff + (lis->_ly << 3);
	offx = lis->_xoff + (lis->_lx << 3);
	offy = lis->_yoff + (lis->_ly << 3);

	if (abs(lx - offx) < 3 && abs(ly - offy) < 3)
		return;

	lis->_lunflag = true;
	lis->_xoff = xoff;
	lis->_yoff = yoff;
	gbDolighting = true;
}

void ChangeLight(unsigned lnum, int x, int y, int r)
{
	LightListStruct *lis;

#if DEBUG_MODE
	if (lightflag)
		return;
#endif
	if (lnum >= MAXLIGHTS)
		return;

	lis = &LightList[lnum];
	lis->_lunflag = true;
	lis->_lx = x;
	lis->_ly = y;
	lis->_lradius = r;
	gbDolighting = true;
}

void ProcessLightList()
{
	LightListStruct *lis;
	int i, j;
	BYTE temp;

#if DEBUG_MODE
	if (lightflag)
		return;
#endif

	if (gbDolighting) {
		for (i = 0; i < numlights; i++) {
			lis = &LightList[lightactive[i]];
			if (lis->_ldel | lis->_lunflag) {
				DoUnLight(lis->_lunx, lis->_luny, lis->_lunr);
				lis->_lunflag = false;
				lis->_lunx = lis->_lx;
				lis->_luny = lis->_ly;
				lis->_lunr = lis->_lradius;
			}
		}
		for (i = 0; i < numlights; ) {
			if (LightList[lightactive[i]]._ldel) {
				numlights--;
				temp = lightactive[numlights];
				lightactive[numlights] = lightactive[i];
				lightactive[i] = temp;
			} else {
				i++;
			}
		}
		for (i = 0; i < numlights; i++) {
			j = lightactive[i];
			DoLighting(LightList[j]._lx, LightList[j]._ly, LightList[j]._lradius, j);
		}

		gbDolighting = false;
	}
}

void InitVision()
{
	int i;

	numvision = 0;
	_gbDovision = false;

	for (i = 0; i < MAXVISION; i++) {
		visionactive[i] = i;
	}
	static_assert(false == 0, "InitVision fills TransList with 0 instead of false values.");
	memset(TransList, 0, sizeof(TransList));
}

void RedoLightAndVision()
{
	//if (deltaload)
	//	return;

	gbDolighting = true;
	_gbDovision = true;
}

int AddVision(int x, int y, int r, bool mine)
{
	LightListStruct *vis;
	int vnum;

	assert(numvision < MAXVISION);
	vnum = visionactive[numvision++];
	vis = &VisionList[vnum];
	vis->_lunx = vis->_lx = x;
	vis->_luny = vis->_ly = y;
	vis->_lunr = vis->_lradius = r;
	vis->_ldel = false;
	vis->_lunflag = false;
	vis->_lmine = mine;
	_gbDovision = true;

	return vnum;
}

void AddUnVision(unsigned vnum)
{
	assert(vnum < MAXVISION);

	VisionList[vnum]._ldel = true;
	_gbDovision = true;
}

void ChangeVisionRadius(unsigned vnum, int r)
{
	LightListStruct *vis;

	if (vnum >= MAXVISION)
		return;

	vis = &VisionList[vnum];
	vis->_lunflag = true;
	vis->_lradius = r;
	_gbDovision = true;
}

void ChangeVisionXY(unsigned vnum, int x, int y)
{
	LightListStruct *vis;

	if (vnum >= MAXVISION)
		return;

	vis = &VisionList[vnum];
	vis->_lunflag = true;
	vis->_lx = x;
	vis->_ly = y;
	_gbDovision = true;
}

void ProcessVisionList()
{
	LightListStruct *vis;
	int i;
	BYTE temp;

	if (_gbDovision) {
		for (i = 0; i < numvision; i++) {
			vis = &VisionList[visionactive[i]];
			if (vis->_lunflag) {
				DoUnVision(vis->_lunx, vis->_luny, vis->_lunr);
				vis->_lunflag = false;
				vis->_lunx = vis->_lx;
				vis->_luny = vis->_ly;
				vis->_lunr = vis->_lradius;
			} else if (vis->_ldel) {
				DoUnVision(vis->_lx, vis->_ly, vis->_lradius);
			}
		}
		for (i = 0; i < numtrans; i++) {
			TransList[i] = false;
		}
		for (i = 0; i < numvision; ) {
			if (VisionList[visionactive[i]]._ldel) {
				numvision--;
				temp = visionactive[numvision];
				visionactive[numvision] = visionactive[i];
				visionactive[i] = temp;
			} else {
				i++;
			}
		}
		for (i = 0; i < numvision; i++) {
			vis = &VisionList[visionactive[i]];
			DoVision(vis->_lx, vis->_ly, vis->_lradius, vis->_lmine);
		}

		_gbDovision = false;
	}
}

void lighting_color_cycling()
{
	int i, j, l;
	BYTE col;
	BYTE *tbl;

	// assert(currLvl._dType == DTYPE_HELL);

	l = MAXDARKNESS;// + 1;
	tbl = ColorTrns[0];

	for (j = 0; j < l; j++) {
		tbl++;
		col = *tbl;
		for (i = 0; i < 30; i++) {
			tbl[0] = tbl[1];
			tbl++;
		}
		*tbl = col;
		tbl += 225;
	}
}

DEVILUTION_END_NAMESPACE
