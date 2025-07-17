/**
 * @file lighting.cpp
 *
 * Implementation of light and vision.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/* Maximum offset in a tile */
#define MAX_OFFSET 8
/* Maximum tile-distance till the precalculated tables are maintained. */
#define MAX_TILE_DIST (MAX_LIGHT_RAD + 1)
/* Maximum light-distance till the precalculated tables are maintained. */
#define MAX_LIGHT_DIST (MAX_TILE_DIST * MAX_OFFSET - 1)

/* The list of the indices of the active visions. */
BYTE visionactive[MAXVISION];
/* The list of visions/views in the game. */
LightListStruct VisionList[MAXVISION];
/* The list of the indices of the active light-sources. */
BYTE lightactive[MAXLIGHTS];
/* The list of light-sources in the game + one for temporary use. */
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
BYTE ColorTrns[NUM_COLOR_TRNS][NUM_COLORS];

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
const int8_t CrawlTable[1514] = {
	// clang-format off
	1,										//  0 - 0
	  0,  0,
	4,										//  1 - 3
	 -1,  0,    0, -1,    0,  1,    1,  0,
	16,										//  2 - 12
	 -1, -1,   -1,  1,    1, -1,    1,  1,
	 -2,  0,    0, -2,    0,  2,    2,  0,
	 -2, -1,   -2,  1,   -1, -2,   -1,  2,
	  1, -2,    1,  2,    2, -1,    2,  1,
	24,										//  3 - 45
	 -2, -2,   -2,  2,    2, -2,    2,  2,
	 -3,  0,    0, -3,    0,  3,    3,  0,
	 -3, -1,   -3,  1,   -1, -3,   -1,  3,
	  1, -3,    1,  3,    3, -1,    3,  1,
	 -3, -2,   -3,  2,   -2, -3,   -2,  3,
	  2, -3,    2,  3,    3, -2,    3,  2,
	24,										//  4 - 94
	 -4,  0,    0, -4,    0,  4,    4,  0,
	 -4, -1,   -4,  1,   -1, -4,   -1,  4,
	  1, -4,    1,  4,    4, -1,    4,  1,
	 -3, -3,   -3,  3,    3, -3,    3,  3,
	 -4, -2,   -4,  2,   -2, -4,   -2,  4,
	  2, -4,    2,  4,    4, -2,    4,  2,
	28,										//  5 - 143
	 -5,  0,   -4, -3,   -4,  3,   -3, -4,
	 -3,  4,    0, -5,    0,  5,    3, -4,
	  3,  4,    4, -3,    4,  3,    5,  0,
	 -5, -1,   -5,  1,   -1, -5,   -1,  5,
	  1, -5,    1,  5,    5, -1,    5,  1,
	 -5, -2,   -5,  2,   -2, -5,   -2,  5,
	  2, -5,    2,  5,    5, -2,    5,  2,
	40,										//  6 - 200
	 -4, -4,   -4,  4,    4, -4,    4,  4,
	 -5, -3,   -5,  3,   -3, -5,   -3,  5,
	  3, -5,    3,  5,    5, -3,    5,  3,
	 -6,  0,    0, -6,    0,  6,    6,  0,
	 -6, -1,   -6,  1,   -1, -6,   -1,  6,
	  1, -6,    1,  6,    6, -1,    6,  1,
	 -6, -2,   -6,  2,   -2, -6,   -2,  6,
	  2, -6,    2,  6,    6, -2,    6,  2,
	 -5, -4,   -5,  4,   -4, -5,   -4,  5,
	  4, -5,    4,  5,    5, -4,    5,  4,
	40,										//  7 - 281
	 -6, -3,   -6,  3,   -3, -6,   -3,  6,
	  3, -6,    3,  6,    6, -3,    6,  3,
	 -7,  0,    0, -7,    0,  7,    7,  0,
	 -7, -1,   -7,  1,   -5, -5,   -5,  5,
	 -1, -7,   -1,  7,    1, -7,    1,  7,
	  5, -5,    5,  5,    7, -1,    7,  1,
	 -6, -4,   -6,  4,   -4, -6,   -4,  6,
	  4, -6,    4,  6,    6, -4,    6,  4,
	 -7, -2,   -7,  2,   -2, -7,   -2,  7,
	  2, -7,    2,  7,    7, -2,    7,  2,
	48,										//  8 - 362
	 -7, -3,   -7,  3,   -3, -7,   -3,  7,
	  3, -7,    3,  7,    7, -3,    7,  3,
	 -6, -5,   -6,  5,   -5, -6,   -5,  6,
	  5, -6,    5,  6,    6, -5,    6,  5,
	 -8,  0,    0, -8,    0,  8,    8,  0,
	 -8, -1,   -8,  1,   -7, -4,   -7,  4,
	 -4, -7,   -4,  7,   -1, -8,   -1,  8,
	  1, -8,    1,  8,    4, -7,    4,  7,
	  7, -4,    7,  4,    8, -1,    8,  1,
	 -8, -2,   -8,  2,   -2, -8,   -2,  8,
	  2, -8,    2,  8,    8, -2,    8,  2,
	 -6, -6,   -6,  6,    6, -6,    6,  6,
	68,										//  9 - 459
	 -8, -3,   -8,  3,   -3, -8,   -3,  8,
	  3, -8,    3,  8,    8, -3,    8,  3,
	 -7, -5,   -7,  5,   -5, -7,   -5,  7,
	  5, -7,    5,  7,    7, -5,    7,  5,
	 -8, -4,   -8,  4,   -4, -8,   -4,  8,
	  4, -8,    4,  8,    8, -4,    8,  4,
	 -9,  0,    0, -9,    0,  9,    9,  0,
	 -9, -1,   -9,  1,   -1, -9,   -1,  9,
	  1, -9,    1,  9,    9, -1,    9,  1,
	 -9, -2,   -9,  2,   -7, -6,   -7,  6,
	 -6, -7,   -6,  7,   -2, -9,   -2,  9,
	  2, -9,    2,  9,    6, -7,    6,  7,
	  7, -6,    7,  6,    9, -2,    9,  2,
	 -8, -5,   -8,  5,   -5, -8,   -5,  8,
	  5, -8,    5,  8,    8, -5,    8,  5,
	 -9, -3,   -9,  3,   -3, -9,   -3,  9,
	  3, -9,    3,  9,    9, -3,    9,  3,
	56,										// 10 - 596
	 -9, -4,   -9,  4,   -4, -9,   -4,  9,
	  4, -9,    4,  9,    9, -4,    9,  4,
	 -7, -7,   -7,  7,    7, -7,    7,  7,
	-10,  0,   -8, -6,   -8,  6,   -6, -8,
	 -6,  8,    0,-10,    0, 10,    6, -8,
	  6,  8,    8, -6,    8,  6,   10,  0,
	-10, -1,  -10,  1,   -1,-10,   -1, 10,
	  1,-10,    1, 10,   10, -1,   10,  1,
	-10, -2,  -10,  2,   -2,-10,   -2, 10,
	  2,-10,    2, 10,   10, -2,   10,  2,
	 -9, -5,   -9,  5,   -5, -9,   -5,  9,
	  5, -9,    5,  9,    9, -5,    9,  5,
	-10, -3,  -10,  3,   -3,-10,   -3, 10,
	  3,-10,    3, 10,   10, -3,   10,  3,
	72,										// 11 - 709
	 -8, -7,   -8,  7,   -7, -8,   -7,  8,
	  7, -8,    7,  8,    8, -7,    8,  7,
	-10, -4,  -10,  4,   -4,-10,   -4, 10,
	  4,-10,    4, 10,   10, -4,   10,  4,
	 -9, -6,   -9,  6,   -6, -9,   -6,  9,
	  6, -9,    6,  9,    9, -6,    9,  6,
	-11,  0,    0,-11,    0, 11,   11,  0,
	-11, -1,  -11,  1,   -1,-11,   -1, 11,
	  1,-11,    1, 11,   11, -1,   11,  1,
	-11, -2,  -11,  2,  -10, -5,  -10,  5,
	 -5,-10,   -5, 10,   -2,-11,   -2, 11,
	  2,-11,    2, 11,    5,-10,    5, 10,
	 10, -5,   10,  5,   11, -2,   11,  2,
	 -8, -8,   -8,  8,    8, -8,    8,  8,
	-11, -3,  -11,  3,   -9, -7,   -9,  7,
	 -7, -9,   -7,  9,   -3,-11,   -3, 11,
	  3,-11,    3, 11,    7, -9,    7,  9,
	  9, -7,    9,  7,   11, -3,   11,  3,
	68,										// 12 - 854
	-10, -6,  -10,  6,   -6,-10,   -6, 10,
	  6,-10,    6, 10,   10, -6,   10,  6,
	-11, -4,  -11,  4,   -4,-11,   -4, 11,
	  4,-11,    4, 11,   11, -4,   11,  4,
	-12,  0,    0,-12,    0, 12,   12,  0,
	-12, -1,  -12,  1,   -9, -8,   -9,  8,
	 -8, -9,   -8,  9,   -1,-12,   -1, 12,
	  1,-12,    1, 12,    8, -9,    8,  9,
	  9, -8,    9,  8,   12, -1,   12,  1,
	-11, -5,  -11,  5,   -5,-11,   -5, 11,
	  5,-11,    5, 11,   11, -5,   11,  5,
	-12, -2,  -12,  2,   -2,-12,   -2, 12,
	  2,-12,    2, 12,   12, -2,   12,  2,
	-10, -7,  -10,  7,   -7,-10,   -7, 10,
	  7,-10,    7, 10,   10, -7,   10,  7,
	-12, -3,  -12,  3,   -3,-12,   -3, 12,
	  3,-12,    3, 12,   12, -3,   12,  3,
	88,										// 13 - 991
	-11, -6,  -11,  6,   -6,-11,   -6, 11,
	  6,-11,    6, 11,   11, -6,   11,  6,
	-12, -4,  -12,  4,   -4,-12,   -4, 12,
	  4,-12,    4, 12,   12, -4,   12,  4,
	 -9, -9,   -9,  9,    9, -9,    9,  9,
	-10, -8,  -10,  8,   -8,-10,   -8, 10,
	  8,-10,    8, 10,   10, -8,   10,  8,
	-13,  0,  -12, -5,  -12,  5,   -5,-12,
	 -5, 12,    0,-13,    0, 13,    5,-12,
	  5, 12,   12, -5,   12,  5,   13,  0,
	-13, -1,  -13,  1,  -11, -7,  -11,  7,
	 -7,-11,   -7, 11,   -1,-13,   -1, 13,
	  1,-13,    1, 13,    7,-11,    7, 11,
	 11, -7,   11,  7,   13, -1,   13,  1,
	-13, -2,  -13,  2,   -2,-13,   -2, 13,
	  2,-13,    2, 13,   13, -2,   13,  2,
	-13, -3,  -13,  3,   -3,-13,   -3, 13,
	  3,-13,    3, 13,   13, -3,   13,  3,
	-12, -6,  -12,  6,   -6,-12,   -6, 12,
	  6,-12,    6, 12,   12, -6,   12,  6,
	-10, -9,  -10,  9,   -9,-10,   -9, 10,
	  9,-10,    9, 10,   10, -9,   10,  9,
	88,										// 14 - 1168
	-13, -4,  -13,  4,  -11, -8,  -11,  8,
	 -8,-11,   -8, 11,   -4,-13,   -4, 13,
	  4,-13,    4, 13,    8,-11,    8, 11,
	 11, -8,   11,  8,   13, -4,   13,  4,
	-12, -7,  -12,  7,   -7,-12,   -7, 12,
	  7,-12,    7, 12,   12, -7,   12,  7,
	-13, -5,  -13,  5,   -5,-13,   -5, 13,
	  5,-13,    5, 13,   13, -5,   13,  5,
	-14,  0,    0,-14,    0, 14,   14,  0,
	-14, -1,  -14,  1,   -1,-14,   -1, 14,
	  1,-14,    1, 14,   14, -1,   14,  1,
	-14, -2,  -14,  2,  -10,-10,  -10, 10,
	 -2,-14,   -2, 14,    2,-14,    2, 14,
	 10,-10,   10, 10,   14, -2,   14,  2,
	-11, -9,  -11,  9,   -9,-11,   -9, 11,
	  9,-11,    9, 11,   11, -9,   11,  9,
	-14, -3,  -14,  3,  -13, -6,  -13,  6,
	 -6,-13,   -6, 13,   -3,-14,   -3, 14,
	  3,-14,    3, 14,    6,-13,    6, 13,
	 13, -6,   13,  6,   14, -3,   14,  3,
	-12, -8,  -12,  8,   -8,-12,   -8, 12,
	  8,-12,    8, 12,   12, -8,   12,  8,
	84,										// 15 - 1345
	-14, -4,  -14,  4,   -4,-14,   -4, 14,
	  4,-14,    4, 14,   14, -4,   14,  4,
	-13, -7,  -13,  7,   -7,-13,   -7, 13,
	  7,-13,    7, 13,   13, -7,   13,  7,
	-14, -5,  -14,  5,  -11,-10,  -11, 10,
	-10,-11,  -10, 11,   -5,-14,   -5, 14,
	  5,-14,    5, 14,   10,-11,   10, 11,
	 11,-10,   11, 10,   14, -5,   14,  5,
	-15,  0,  -12, -9,  -12,  9,   -9,-12,
	 -9, 12,    0,-15,    0, 15,    9,-12,
	  9, 12,   12, -9,   12,  9,   15,  0,
	-15, -1,  -15,  1,   -1,-15,   -1, 15,
	  1,-15,    1, 15,   15, -1,   15,  1,
	-15, -2,  -15,  2,   -2,-15,   -2, 15,
	  2,-15,    2, 15,   15, -2,   15,  2,
	-14, -6,  -14,  6,   -6,-14,   -6, 14,
	  6,-14,    6, 14,   14, -6,   14,  6,
	-13, -8,  -13,  8,   -8,-13,   -8, 13,
	  8,-13,    8, 13,   13, -8,   13,  8,
	-15, -3,  -15,  3,   -3,-15,   -3, 15,
	  3,-15,    3, 15,   15, -3,   15,  3,
	// clang-format on
};

/** Indices of CrawlTable to select the entries at a given distance. */
const int CrawlNum[16] = { 0, 3, 12, 45, 94, 143, 200, 281, 362, 459, 596, 709, 854, 991, 1168, 1345 };

static void RotateRadius(int* ox, int* oy, int* dx, int* dy, int* bx, int* by)
{
	int nx, ny;

	nx = - *dy;
	ny = *dx;

	*dx = nx;
	*dy = ny;

	*bx = nx < 0 ? 1 : 0;
	if (*bx == 1) {
		nx += 8;
	}
	*by = ny < 0 ? 1 : 0;
	if (*by == 1) {
		ny += 8;
	}
	*ox = nx;
	*oy = ny;
}

static void DoLighting(unsigned lnum)
{
	LightListStruct* lis = &LightList[lnum];
	int x, y, xoff, yoff;
	int min_x, max_x, min_y, max_y;
	int baseOffX, baseOffY, block_x, block_y, temp_x, temp_y;
	int nXPos = lis->_lx;
	int nYPos = lis->_ly;
	int nRadius = lis->_lradius;
	BYTE (&dark)[128] = darkTable[nRadius];
	BYTE v, radius_block;

	xoff = lis->_lxoff;
	yoff = lis->_lyoff;
	if (xoff < 0) {
		xoff += MAX_OFFSET;
		nXPos--;
	} else if (xoff >= MAX_OFFSET) {
		xoff -= MAX_OFFSET;
		nXPos++;
	}
	if (yoff < 0) {
		yoff += MAX_OFFSET;
		nYPos--;
	} else if (yoff >= MAX_OFFSET) {
		yoff -= MAX_OFFSET;
		nYPos++;
	}
	assert((unsigned)xoff < MAX_OFFSET);
	assert((unsigned)yoff < MAX_OFFSET);

	baseOffX = xoff;
	baseOffY = yoff;

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
	RotateRadius(&xoff, &yoff, &baseOffX, &baseOffY, &block_x, &block_y);
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
	RotateRadius(&xoff, &yoff, &baseOffX, &baseOffY, &block_x, &block_y);
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
	RotateRadius(&xoff, &yoff, &baseOffX, &baseOffY, &block_x, &block_y);
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

static void DoUnLight(LightListStruct* lis)
{
	int x, y, xoff, yoff, min_x, min_y, max_x, max_y;
	int nXPos = lis->_lunx + lis->_lunxoff;
	int nYPos = lis->_luny + lis->_lunyoff;
	int nRadius = lis->_lunr;

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

	lis->_lunx = lis->_lx;
	lis->_luny = lis->_ly;
	lis->_lunr = lis->_lradius;
	xoff = lis->_lxoff;
	yoff = lis->_lyoff;
	lis->_lunxoff = 0;
	if (xoff < 0) {
		lis->_lunxoff = -1;
	} else if (xoff >= 8) {
		lis->_lunxoff = 1;
	}
	lis->_lunyoff = 0;
	if (yoff < 0) {
		lis->_lunyoff = -1;
	} else if (yoff >= 8) {
		lis->_lunyoff = 1;
	}
	lis->_lunflag = false;
}

BYTE *srcDark;
static bool LightPos(int x1, int y1, int radius_block)
{
	assert(IN_DUNGEON_AREA(x1, y1));

	// int yoff = 0;
	// int xoff = 0;
	// BYTE (&dist0)[MAX_TILE_DIST][MAX_TILE_DIST] = distMatrix[yoff][xoff];
	// BYTE radius_block = dist0[abs(nYPos - y1)][abs(nXPos - x1)];
	// BYTE v = srcDark[radius_block];
	BYTE v = srcDark[radius_block];
	if (v < dLight[x1][y1])
		dLight[x1][y1] = v;

	return !nBlockTable[dPiece[x1][y1]];
}

void TraceLightSource(int nXPos, int nYPos, int nRadius)
{
	const int8_t* cr;
	int i, x1, y1, limit;
	int d, dx, dy, xinc, yinc;

	srcDark = darkTable[nRadius];
	BYTE v = srcDark[0];
	if (v < dLight[nXPos][nYPos])
		dLight[nXPos][nYPos] = v;

	nRadius = 2 * (nRadius + 1) * 8 * 16;
	static_assert(INT_MAX / (2 * 8 * 16) > MAX_LIGHT_RAD, "Light tracing overflows in TraceLightSource.");
	static_assert(MAX_OFFSET == 8, "Light tracing shift must be adjusted in TraceLightSource.");
	static_assert(lengthof(CrawlNum) > 15, "TraceLightSource uses CrawlTable/CrawlNum up to radius 15.");
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
					limit -= 1 * 109; // 1 * 7;
				}
				x1 += xinc;
				limit -= 2 * 8 * 16;
				if (limit <= 0)
					break;
			} while (LightPos(x1, y1, (nRadius - limit) >> (1 + 4))); // * MAX_OFFSET / (2 * 8 * 16)
		} else {
			// multiply by 2 so we round up
			dx *= 2;
			d = 0;
			do {
				d += dx;
				if (d >= dy) {
					d -= 2 * dy; // multiply by 2 to support rounding
					x1 += xinc;
					limit -= 1 * 109; // 1 * 7;
				}
				y1 += yinc;
				limit -= 2 * 8 * 16;
				if (limit <= 0)
					break;
			} while (LightPos(x1, y1, (nRadius - limit) >> (1 + 4))); // * MAX_OFFSET / (2 * 8 * 16)
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
static bool ViewPos(int x1, int y1)
{
	//int nTrans;
	assert(IN_DUNGEON_AREA(x1, y1));
	dFlags[x1][y1] |= vFlags;
	bool result = !nBlockTable[dPiece[x1][y1]];
	if (doautomap) {
		if (!(dFlags[x1][y1] & BFLAG_EXPLORED)) {
			SetAutomapView(x1, y1);
		}
	}
	return result;
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
	const int8_t* cr;
	int i, x1, y1, limit;
	int d, dx, dy, xinc, yinc;
	vFlags = local ? BFLAG_VISIBLE | BFLAG_ALERT : BFLAG_ALERT;
	doautomap = local;

	assert(IN_DUNGEON_AREA(nXPos, nYPos));
	dFlags[nXPos][nYPos] |= vFlags;
	if (local) {
		/*if (!(dFlags[nXPos][nYPos] & BFLAG_EXPLORED)) { -- not necessary, because the same tile is going to be checked by one of the other subtiles
			SetAutomapView(nXPos, nYPos);
		}*/
		i = dTransVal[nXPos][nYPos];
		if (i != 0) {
			TransList[i] = true;
		}
	}
	nRadius = 2 * (nRadius + 1);
	static_assert(lengthof(CrawlNum) > 15, "DoVision uses CrawlTable/CrawlNum up to radius 15.");
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
			} while (ViewPos(x1, y1));
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
			} while (ViewPos(x1, y1));
		}
	}
}
#if 0
void MakeLightTable()
{
	unsigned i, j, k, shade;
	BYTE col, max;
	BYTE* tbl;

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
			if (max == 255) {
				max = 254;
			}
			for (k = 0; k < 16; k++) {
				*tbl++ = col;
				if (col < max) {
					col++;
				} else {
					max = 0;
					col = 0;
				}
			}
		}
	}

	// assert(tbl == ColorTrns[MAXDARKNESS]);
	tbl = ColorTrns[0];
	memset(ColorTrns[MAXDARKNESS], 0, sizeof(ColorTrns[MAXDARKNESS]));

	if (currLvl._dType == DTYPE_HELL) {
		for (i = 0; i <= MAXDARKNESS; i++) {
			shade = i;
			col = 1;
			*tbl++ = 0;
			for (k = 1; k < 16; k++) {
				*tbl++ = col;
				if (shade > 0) {
					shade--;
				} else {
					col++;
				}
			}
			shade = i;
			col = 16 * 1 + shade;
			max = 16 * 1 + 15;
			for (k = 0; k < 16; k++) {
				*tbl++ = col;
				if (col < max) {
					col++;
				} else {
					max = 1;
					col = 1;
				}
			}
			tbl += NUM_COLORS - 32;
		}
#ifdef HELLFIRE
	} else if (currLvl._dType == DTYPE_CAVES || currLvl._dType == DTYPE_CRYPT) {
#else
	} else if (currLvl._dType == DTYPE_CAVES) {
#endif
		for (i = 0; i <= MAXDARKNESS; i++) {
			*tbl++ = 0;
			for (j = 1; j < 32; j++)
				*tbl++ = j;
			tbl += NUM_COLORS - 32;
		}
#ifdef HELLFIRE
	} else if (currLvl._dType == DTYPE_NEST) {
		for (i = 0; i <= MAXDARKNESS; i++) {
			*tbl++ = 0;
			for (j = 1; j < 16; j++)
				*tbl++ = j;
			tbl += NUM_COLORS - 16;
		}
#endif
	}
}
#endif // 0
void InitLighting()
{
	//BYTE* tbl;
	//int i, j, k, l;
	//BYTE col;
	//double fs, fa;

	LoadFileWithMem("Levels\\TownData\\Town.TRS", ColorTrns[0]);
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
		memset(darkTable[0], MAXDARKNESS, lengthof(darkTable[0]));
		for (i = 1, k = MAX_OFFSET; i <= MAX_LIGHT_RAD; i++, k += MAX_OFFSET) {
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
				fa = (MAX_OFFSET * k - j);
				fa *= fa;
				for (l = 0; l < MAX_TILE_DIST; l++) {
					fs = (MAX_OFFSET * l - i);
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

void InitLvlLighting()
{
	int i;

	numlights = 0;
	gbDolighting = false;

	for (i = 0; i < MAXLIGHTS; i++) {
		lightactive[i] = i;
	}
}

unsigned AddLight(int x, int y, int r)
{
	LightListStruct* lis;
	int lnum;

	static_assert(NO_LIGHT >= MAXLIGHTS, "Handling of lights expects NO_LIGHT out of the [0..MAXLIGHTS) range");
	lnum = NO_LIGHT;

	if (numlights < MAXLIGHTS) {
		lnum = lightactive[numlights++];
		lis = &LightList[lnum];
		lis->_lunx = lis->_lx = x;
		lis->_luny = lis->_ly = y;
		lis->_lunr = lis->_lradius = r;
		lis->_lunxoff = 0;
		lis->_lunyoff = 0;
		lis->_lxoff = 0;
		lis->_lyoff = 0;
		lis->_ldel = false;
		lis->_lunflag = false;
		gbDolighting = true;
	}

	return lnum;
}

void AddUnLight(unsigned lnum)
{
	if (lnum >= MAXLIGHTS)
		return;

	LightList[lnum]._ldel = true;
	gbDolighting = true;
}

void ChangeLightRadius(unsigned lnum, int r)
{
	LightListStruct* lis;

	if (lnum >= MAXLIGHTS)
		return;

	lis = &LightList[lnum];
	lis->_lunflag = true;
	lis->_lradius = r;
	gbDolighting = true;
}

void ChangeLightXY(unsigned lnum, int x, int y)
{
	LightListStruct* lis;

	if (lnum >= MAXLIGHTS)
		return;

	lis = &LightList[lnum];
	lis->_lunflag = true;
	lis->_lx = x;
	lis->_ly = y;
	gbDolighting = true;
}

void ChangeLightScreenOff(unsigned lnum, int xsoff, int ysoff)
{
	LightListStruct* lis;
	int xoff, yoff;

	if (lnum >= MAXLIGHTS)
		return;
	// convert screen-offset to tile-offset
	xoff = xsoff + 2 * ysoff;
	yoff = 2 * ysoff - xsoff;

	xoff = xoff / (TILE_WIDTH / 8); // ASSET_MPL * 8 ?
	yoff = yoff / (TILE_WIDTH / 8);

	lis = &LightList[lnum];
	lis->_lunflag = true;
	lis->_lxoff = xoff;
	lis->_lyoff = yoff;
	gbDolighting = true;
}

/*
 * Same as ChangeLightXY, but also sets the x/y-offsets to zero.
 */
void ChangeLightXYOff(unsigned lnum, int x, int y)
{
	LightListStruct* lis;

	if (lnum >= MAXLIGHTS)
		return;

	lis = &LightList[lnum];
	lis->_lunflag = true;
	lis->_lx = x;
	lis->_ly = y;
	lis->_lxoff = 0;
	lis->_lyoff = 0;
	gbDolighting = true;
}

void CondChangeLightXY(unsigned lnum, int x, int y)
{
	LightListStruct* lis;

	if (lnum >= MAXLIGHTS)
		return;

	lis = &LightList[lnum];
	if (lis->_lx == x && lis->_ly == y)
		return;

	lis->_lunflag = true;
	lis->_lx = x;
	lis->_ly = y;
	gbDolighting = true;
}

void CondChangeLightScreenOff(unsigned lnum, int xsoff, int ysoff)
{
	LightListStruct* lis;
	int xoff, yoff;
	int lx, ly;
	int offx, offy;

	if (lnum >= MAXLIGHTS)
		return;
	lis = &LightList[lnum];
	// convert screen-offset to tile-offset
	xoff = xsoff + 2 * ysoff;
	yoff = 2 * ysoff - xsoff;

	xoff = xoff / (TILE_WIDTH / 8); // ASSET_MPL * 8 ?
	yoff = yoff / (TILE_WIDTH / 8);
	// check if offset-change is meaningful
	lx = xoff + (lis->_lx << 3);
	ly = yoff + (lis->_ly << 3);
	offx = lis->_lxoff + (lis->_lx << 3);
	offy = lis->_lyoff + (lis->_ly << 3);

	if (abs(lx - offx) < 3 && abs(ly - offy) < 3)
		return;

	lis->_lunflag = true;
	lis->_lxoff = xoff;
	lis->_lyoff = yoff;
	gbDolighting = true;
}

void ChangeLight(unsigned lnum, int x, int y, int r)
{
	LightListStruct* lis;

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
	LightListStruct* lis;
	int i, j;
	BYTE temp;

	if (gbDolighting) {
		for (i = 0; i < numlights; i++) {
			lis = &LightList[lightactive[i]];
			if (lis->_ldel | lis->_lunflag) {
				DoUnLight(lis);
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
			DoLighting(j);
		}

		gbDolighting = false;
	}
}

void InitLvlVision()
{
	int i;

	numvision = 0;
	_gbDovision = false;

	for (i = 0; i < MAXVISION; i++) {
		visionactive[i] = i;
	}
	static_assert(false == 0, "InitLvlVision fills TransList with 0 instead of false values.");
	memset(TransList, 0, sizeof(TransList));
}

void RedoLightAndVision()
{
	//if (deltaload)
	//	return;

	gbDolighting = true;
	_gbDovision = true;
}

void LightAndVisionDone()
{
	gbDolighting = false;
	_gbDovision = false;
}

int AddVision(int x, int y, int r, bool mine)
{
	LightListStruct* vis;
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
	if (vnum >= MAXVISION)
		return;

	VisionList[vnum]._ldel = true;
	_gbDovision = true;
}

void ChangeVisionRadius(unsigned vnum, int r)
{
	LightListStruct* vis;

	if (vnum >= MAXVISION)
		return;

	vis = &VisionList[vnum];
	vis->_lunflag = true;
	vis->_lradius = r;
	_gbDovision = true;
}

void ChangeVisionXY(unsigned vnum, int x, int y)
{
	LightListStruct* vis;

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
	LightListStruct* vis;
	int i;
	BYTE temp;

	if (!_gbDovision)
		return;

	// skip vision calculation in town
	// if (currLvl._dLevelIdx != DLV_TOWN) {
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
	// }

	_gbDovision = false;
}

void lighting_update_caves()
{
	int i, j;
	BYTE col;
	BYTE* tbl;

	tbl = ColorTrns[0];

	for (j = 0; j <= MAXDARKNESS; j++) {
		col = tbl[1];
		for (i = 1; i < 31; i++) {
			tbl[i] = tbl[i + 1];
		}
		tbl[i] = col;

		tbl += NUM_COLORS;
	}
}

void lighting_update_hell()
{
	lighting_update_caves();
	/*int i, j;
	BYTE col;
	BYTE* tbl;

	// assert(currLvl._dType == DTYPE_HELL);

	tbl = ColorTrns[0];

	for (j = 0; j <= MAXDARKNESS; j++) {
		tbl++;
		col = *tbl;
		for (i = 0; i < 30; i++) {
			tbl[0] = tbl[1];
			tbl++;
		}
		*tbl++ = col;
		tbl += NUM_COLORS - 32;
	}*/
}

#ifdef HELLFIRE
static int cryptCycleCounter = 3;
void lighting_update_crypt()
{
	int i, j;
	BYTE col;
	BYTE* tbl;

	if (--cryptCycleCounter == 0) {
		cryptCycleCounter = 3;

		tbl = ColorTrns[0];

		for (j = 0; j <= MAXDARKNESS; j++) {
			col = tbl[15];
			for (i = 15; i > 1; i--) {
				tbl[i] = tbl[i - 1];
			}
			tbl[i] = col;

			tbl += NUM_COLORS;
		}
	}

	tbl = ColorTrns[0];

	for (j = 0; j <= MAXDARKNESS; j++) {
		col = tbl[31];
		for (i = 31; i > 16; i--) {
			tbl[i] = tbl[i - 1];
		}
		tbl[i] = col;

		tbl += NUM_COLORS;
	}
}

static int nestCycleCounter = 3;
void lighting_update_nest()
{
	int i, j;
	BYTE col;
	BYTE* tbl;

	// assert(currLvl._dType == DTYPE_NEST);
	if (--nestCycleCounter != 0) {
		return;
	}
	nestCycleCounter = 3;

	tbl = ColorTrns[0];

	for (j = 0; j <= MAXDARKNESS; j++) {
		col = tbl[8];
		for (i = 8; i > 1; i--) {
			tbl[i] = tbl[i - 1];
		}
		tbl[i] = col;

		col = tbl[15];
		for (i = 15; i > 9; i--) {
			tbl[i] = tbl[i - 1];
		}
		tbl[i] = col;

		tbl += NUM_COLORS;
	}
}
#endif

DEVILUTION_END_NAMESPACE
