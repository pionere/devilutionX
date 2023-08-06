/**
 * @file automap.cpp
 *
 * Implementation of the in-game map overlay.
 */
#include "all.h"
#include "engine/render/raw_render.h"
#include "engine/render/text_render.h"

DEVILUTION_BEGIN_NAMESPACE

/* Maps from tile_id to automap type (_automap_types + _automap_flags). */
uint16_t automaptype[MAXTILES + 1];
/** Specifies whether the automap is enabled. */
bool gbAutomapflag;
/** Specifies whether the automap-data is valid. */
bool _gbAutomapData;
/** Tracks the explored areas of the map. */
BOOLEAN automapview[DMAXX][DMAXY];
/** Specifies the scale of the automap. */
unsigned AutoMapScale;
int AutoMapXOfs;
int AutoMapYOfs;
unsigned AmLine64;
unsigned AmLine32;
unsigned AmLine16;

/** color used to draw the player's arrow */
#define COLOR_PLAYER (PAL8_ORANGE + 1)
#define COLOR_FRIEND (PAL8_BLUE + 0)
#define COLOR_ENEMY  (PAL8_RED + 2)
/** color for bright map lines (doors, stairs etc.) */
#define COLOR_BRIGHT PAL8_YELLOW
/** color for dim map lines/dots */
#define COLOR_DIM (PAL16_YELLOW + 8)
// color for items on automap
#define COLOR_ITEM (PAL8_BLUE + 1)

#define MAP_SCALE_MAX 128
#define MAP_SCALE_MIN 64

/**
 * @brief Initializes the automap configuration.
 */
void InitAutomapOnce()
{
	gbAutomapflag = false;
	AutoMapScale = MAP_SCALE_MIN;
	InitAutomapScale();
	// these values are initialized by InitAutomap
	//_gbAutomapData = false;
	//memset(automaptype, 0, sizeof(automaptype));
	//memset(automapview, 0, sizeof(automapview));
	//AutoMapXOfs = 0;
	//AutoMapYOfs = 0;
}

/**
 * @brief Calculates the cached values of automap which are depending on the scale.
 */
void InitAutomapScale()
{
	AmLine64 = (AutoMapScale * TILE_WIDTH) / 128;
	AmLine32 = AmLine64 >> 1;
	AmLine16 = AmLine32 >> 1;
}

/**
 * @brief Initializes the automap of a dungeon level.
 *  1. Loads the mapping between tile IDs and automap shapes.
 *  2. Resets the offsets.
 */
void InitLvlAutomap()
{
	size_t dwTiles, i;
	BYTE* pAFile;
	uint16_t* lm;
	const char* mapData;

	/* commented out because the flags are reset in gendung.cpp anyway
	static_assert(sizeof(dFlags) == MAXDUNX * MAXDUNY, "Linear traverse of dFlags does not work in InitAutomap.");
	pTmp = &dFlags[0][0];
	for (i = 0; i < MAXDUNX * MAXDUNY; i++, pTmp++) {
		assert((*pTmp & BFLAG_EXPLORED) == 0);
		*pTmp &= ~BFLAG_EXPLORED;
	}*/

	mapData = levelfiledata[AllLevels[currLvl._dLevelIdx].dfindex].dAutomapData;
	_gbAutomapData = mapData != NULL;
	if (!_gbAutomapData) {
		memset(automaptype, 0, sizeof(automaptype));
		return;
	}

	pAFile = LoadFileInMem(mapData, &dwTiles);

	dwTiles /= 2;
	assert(dwTiles < (size_t)lengthof(automaptype));

	lm = (uint16_t*)pAFile;
	for (i = 1; i <= dwTiles; i++) {
		automaptype[i] = SwapLE16(*lm);
		// assert((automaptype[i] & MAF_TYPE) < 13); required by DrawAutomapTile and SetAutomapView
		lm++;
	}

	mem_free_dbg(pAFile);
#if !USE_PATCH
	// patch dAutomapData - L1.AMP
	if (currLvl._dType == DTYPE_CATHEDRAL) {
		// separate pillar tile
		automaptype[28] = MWT_PILLAR; // automaptype[15]
		// new shadows
		// - shadows created by fixCathedralShadows
		automaptype[145] = automaptype[11];
		automaptype[147] = automaptype[6];
		automaptype[149] = automaptype[12];
		automaptype[149] = automaptype[12];
		automaptype[150] = automaptype[2];
		automaptype[151] = automaptype[12];
		automaptype[152] = automaptype[36];
		automaptype[154] = automaptype[7];
		automaptype[155] = automaptype[2];
		automaptype[157] = automaptype[35];
		automaptype[159] = automaptype[13];
		automaptype[164] = automaptype[13];
		automaptype[165] = automaptype[13];
		// - shadows for the banner setpiece
		automaptype[56] = automaptype[1];
		automaptype[55] = automaptype[1];
		automaptype[54] = automaptype[60];
		automaptype[53] = automaptype[58];
		// - shadows for the vile setmap
		automaptype[52] = automaptype[2];
		automaptype[51] = automaptype[2];
		automaptype[50] = automaptype[1];
		automaptype[49] = automaptype[17];
		automaptype[48] = automaptype[11];
		automaptype[47] = automaptype[2];
		automaptype[46] = automaptype[7];
	}
	// patch dAutomapData - L2.AMP
	if (currLvl._dType == DTYPE_CATACOMBS) {
		// fix automap type
		automaptype[42] &= ~MAF_EAST_ARCH; // not a horizontal arch
		automaptype[156] = MWT_NONE; // no door is placed
		automaptype[157] = MWT_NONE;
		// separate pillar tile
		automaptype[52] = MWT_PILLAR;
		// new shadows
		automaptype[17] = automaptype[5];
		// automaptype[18] = automaptype[5];
		automaptype[34] = automaptype[6];
		automaptype[35] = automaptype[7];
		automaptype[36] = automaptype[9];
		automaptype[37] = automaptype[9];
		// automaptype[44] = automaptype[3];
		// automaptype[46] = automaptype[3];
		// automaptype[47] = automaptype[3];
		// automaptype[48] = automaptype[3];
		// automaptype[49] = automaptype[3];
		// automaptype[95] = automaptype[3];
		// automaptype[96] = automaptype[3];
		// automaptype[100] = automaptype[3];
	}
	// patch dAutomapData - L3.AMP
	if (currLvl._dType == DTYPE_CAVES) {
		// new shadows
		automaptype[144] = automaptype[151];
		automaptype[145] = automaptype[152];
	}
	// patch dAutomapData - L4.AMP
	if (currLvl._dType == DTYPE_HELL) {
		// fix automap types
		automaptype[27] = MAF_EXTERN | MWT_NORTH_EAST_END;
		automaptype[28] = MAF_EXTERN | MWT_NORTH_WEST_END;
		automaptype[52] |= MAF_WEST_GRATE;
		automaptype[56] |= MAF_EAST_GRATE;
		automaptype[7] = MWT_NORTH_WEST_END;
		automaptype[8] = MWT_NORTH_EAST_END;
		automaptype[83] = MWT_NORTH_WEST_END;
		// new shadow-types
		automaptype[61] = automaptype[2];
		automaptype[62] = automaptype[2];
		automaptype[76] = automaptype[15];
		automaptype[129] = automaptype[15];
		automaptype[130] = automaptype[56];
		automaptype[131] = automaptype[56];
		automaptype[132] = automaptype[8];
		automaptype[133] = automaptype[8];
		automaptype[134] = automaptype[14];
		automaptype[135] = automaptype[14];
	}
#ifdef HELLFIRE
	// patch dAutomapData - L5.AMP
	if (currLvl._dType == DTYPE_CRYPT) {
		// fix automap types
		automaptype[20] = MAF_EXTERN | MWT_CORNER;
		automaptype[23] = MAF_EXTERN | MWT_NORTH_WEST_END;
		automaptype[24] = MAF_EXTERN | MWT_NORTH_EAST_END;
		// fix automap of the entrance
		automaptype[47] = MAF_STAIRS | MWT_NORTH_WEST;
		automaptype[50] = MWT_NORTH_WEST;
		automaptype[48] = MAF_STAIRS | MWT_NORTH;
		automaptype[51] = MWT_NORTH_WEST_END;
		automaptype[52] = MAF_EXTERN;
		automaptype[53] = MAF_STAIRS | MWT_NORTH;
		automaptype[54] = MAF_EXTERN;
		automaptype[56] = MWT_NONE;
		automaptype[58] = MAF_EXTERN | MWT_NORTH_WEST_END;
		// separate pillar tile
		automaptype[28] = MWT_PILLAR; // automaptype[15]
		// new shadows
		// - shadows created by fixCryptShadows
		automaptype[109] = MWT_NORTH_WEST;
		automaptype[110] = MWT_NORTH_WEST;
		automaptype[111] = MAF_WEST_ARCH | MWT_NORTH_WEST;
		automaptype[215] = MAF_WEST_GRATE | MWT_NORTH_WEST;
		// - 'add' new shadow-types with glow
		automaptype[216] = MAF_WEST_ARCH | MWT_NORTH_WEST;
		// - 'add' new shadow-types with horizontal arches
		automaptype[71] = MWT_NORTH_EAST;
		automaptype[80] = MWT_NORTH_EAST;
		automaptype[81] = MAF_EAST_ARCH | MWT_NORTH_EAST;
		automaptype[82] = MAF_EAST_ARCH | MWT_NORTH_EAST;
		automaptype[83] = MAF_EAST_GRATE | MWT_NORTH_EAST;
		automaptype[84] = MAF_EAST_GRATE | MWT_NORTH_EAST;
		automaptype[85] = MWT_NORTH_EAST;
		automaptype[86] = MWT_NORTH_EAST;
		automaptype[87] = MAF_EAST_DOOR | MWT_NORTH_EAST;
		automaptype[88] = MAF_EAST_DOOR | MWT_NORTH_EAST;
	}
#endif // HELLFIRE
#endif // !USE_PATCH
	memset(automapview, 0, sizeof(automapview));

	AutoMapXOfs = 0;
	AutoMapYOfs = 0;
}

bool IsAutomapActive()
{
	return gbAutomapflag && _gbAutomapData;
}

/**
 * @brief Display/Hides the automap.
 */
void ToggleAutomap()
{
	gbAutomapflag = !gbAutomapflag;
	//if (gbAutomapflag) {
		AutoMapXOfs = 0;
		AutoMapYOfs = 0;
	//}
}

/**
 * @brief Scrolls the automap upwards.
 */
void AutomapUp()
{
	AutoMapXOfs--;
	AutoMapYOfs--;
}

/**
 * @brief Scrolls the automap downwards.
 */
void AutomapDown()
{
	AutoMapXOfs++;
	AutoMapYOfs++;
}

/**
 * @brief Scrolls the automap leftwards.
 */
void AutomapLeft()
{
	AutoMapXOfs--;
	AutoMapYOfs++;
}

/**
 * @brief Scrolls the automap rightwards.
 */
void AutomapRight()
{
	AutoMapXOfs++;
	AutoMapYOfs--;
}

/**
 * @brief Increases the zoom level of the automap.
 */
void AutomapZoomIn()
{
	if (AutoMapScale < MAP_SCALE_MAX) {
		AutoMapScale += 16;
		AmLine64 = (AutoMapScale * TILE_WIDTH) / 128;
		AmLine32 = AmLine64 >> 1;
		AmLine16 = AmLine32 >> 1;
	}
}

/**
 * @brief Decreases the zoom level of the automap.
 */
void AutomapZoomOut()
{
	if (AutoMapScale > MAP_SCALE_MIN) {
		AutoMapScale -= 16;
		AmLine64 = (AutoMapScale * TILE_WIDTH) / 128;
		AmLine32 = AmLine64 >> 1;
		AmLine16 = AmLine32 >> 1;
	}
}

static void DrawAutomapDirt(int x, int y)
{
	unsigned d32 = AmLine32;
	unsigned d16 = (d32 >> 1), d8 = (d32 >> 2), d4 = (d32 >> 3);

	/*            07
	          01      03
	      05      00      06
	  09      02      04      10
	      11      08      12
	          13      14
	              15
	*/
	DrawPixel(x, y, COLOR_DIM);                 // 00
	DrawPixel(x - d8, y - d4, COLOR_DIM);       // 01
	DrawPixel(x - d8, y + d4, COLOR_DIM);       // 02
	DrawPixel(x + d8, y - d4, COLOR_DIM);       // 03
	DrawPixel(x + d8, y + d4, COLOR_DIM);       // 04
	DrawPixel(x - d16, y, COLOR_DIM);           // 05
	DrawPixel(x + d16, y, COLOR_DIM);           // 06
	DrawPixel(x, y - d8, COLOR_DIM);            // 07
	DrawPixel(x, y + d8, COLOR_DIM);            // 08
	DrawPixel(x - d32 + d8, y + d4, COLOR_DIM); // 09
	DrawPixel(x + d32 - d8, y + d4, COLOR_DIM); // 10
	DrawPixel(x - d16, y + d8, COLOR_DIM);      // 11
	DrawPixel(x + d16, y + d8, COLOR_DIM);      // 12
	DrawPixel(x - d8, y + d16 - d4, COLOR_DIM); // 13
	DrawPixel(x + d8, y + d16 - d4, COLOR_DIM); // 14
	DrawPixel(x, y + d16, COLOR_DIM);           // 15
}

static void DrawAutomapStairs(int x, int y)
{
	unsigned d32 = AmLine32;
	unsigned d16 = (d32 >> 1), d8 = (d32 >> 2), d4 = (d32 >> 3);

	DrawLine(x - d8, y - d16 + d4, x + d32 - d8, y + d4, COLOR_BRIGHT);
	DrawLine(x - d16, y - d8, x + d16, y + d8, COLOR_BRIGHT);
	DrawLine(x - d32 + d8, y - d4, x + d8, y + d16 - d4, COLOR_BRIGHT);
	DrawLine(x - d32, y, x, y + d16, COLOR_BRIGHT);
}

static void DrawAutomapHorzDoor(int x, int y)
{
	unsigned d16 = AmLine16;
	unsigned d8 = (d16 >> 1), d4 = (d16 >> 2);

	DrawLine(x - d16, y - d8, x - d8, y - d4, COLOR_DIM); // left wall
	DrawLine(x + d8, y + d4, x + d16, y + d8, COLOR_DIM); // right wall

	// DrawAutomapDiamond(x, y + d8, COLOR_BRIGHT)
	DrawLine(x - d16, y, x, y + d8, COLOR_BRIGHT); // top left
	DrawLine(x, y + d8, x + d16, y, COLOR_BRIGHT); // top right
	DrawLine(x, y - d8, x + d16, y, COLOR_BRIGHT); // bottom right
	DrawLine(x - d16, y, x, y - d8, COLOR_BRIGHT); // bottom left
}

static void DrawAutomapVertDoor(int x, int y)
{
	unsigned d16 = AmLine16;
	unsigned d8 = (d16 >> 1), d4 = (d16 >> 2);

	DrawLine(x - d16, y + d8, x - d8, y + d4, COLOR_DIM); // left wall
	DrawLine(x + d8, y - d4, x + d16, y - d8, COLOR_DIM); // right wall

	// DrawAutomapDiamond(x, y + d8, COLOR_BRIGHT)
	DrawLine(x - d16, y, x, y + d8, COLOR_BRIGHT); // top left
	DrawLine(x, y + d8, x + d16, y, COLOR_BRIGHT); // top right
	DrawLine(x, y - d8, x + d16, y, COLOR_BRIGHT); // bottom right
	DrawLine(x - d16, y, x, y - d8, COLOR_BRIGHT); // bottom left
}

static void DrawAutomapDiamond(int x, int y)
{
	int d16 = AmLine16, y2;

	y2 = y - (d16 >> 1);

	DrawLine(x - d16, y2, x, y, COLOR_DIM);       // top left
	DrawLine(x, y, x + d16, y2, COLOR_DIM);       // top right
	DrawLine(x, y - d16, x + d16, y2, COLOR_DIM); // bottom right
	DrawLine(x - d16, y2, x, y - d16, COLOR_DIM); // bottom left
}

/**
 * @brief Renders the given automap shape at the specified screen coordinates.
 */
static void DrawAutomapTile(int sx, int sy, uint16_t automap_type)
{
	uint8_t type;

	if (automap_type & MAF_EXTERN) {
		DrawAutomapDirt(sx, sy);
	}

	if (automap_type & MAF_STAIRS) {
		DrawAutomapStairs(sx, sy);
	}

	type = automap_type & MAF_TYPE;
	automap_type &= ~MAF_TYPE;
	switch (type) {
	case MWT_NONE:
	case MWT_CORNER:
		break;
	case MWT_PILLAR: // stand-alone column or other unpassable object
		DrawAutomapDiamond(sx, sy);
		break;
	case MWT_NORTH_WEST:
	case MWT_NORTH_WEST_END:
		automap_type |= MAF_DO_NORTH_WEST;
		break;
	case MWT_NORTH_EAST:
	case MWT_NORTH_EAST_END:
		automap_type |= MAF_DO_NORTH_EAST;
		break;
	case MWT_NORTH:
		automap_type |= MAF_DO_NORTH_EAST | MAF_DO_NORTH_WEST;
		break;
	case MWT_WEST:
		automap_type |= MAF_DO_NORTH_WEST | MAF_DO_SOUTH_WEST;
		break;
	case MWT_EAST:
		automap_type |= MAF_DO_NORTH_EAST | MAF_DO_SOUTH_EAST;
		break;
	case MWT_SOUTH_WEST:
		automap_type |= MAF_DO_SOUTH_WEST;
		break;
	case MWT_SOUTH_EAST:
		automap_type |= MAF_DO_SOUTH_EAST;
		break;
	case MWT_SOUTH:
		automap_type |= MAF_DO_SOUTH_WEST | MAF_DO_SOUTH_EAST;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	if (automap_type & MAF_DO_NORTH_WEST) {
		if (automap_type & MAF_WEST_DOOR) {
			unsigned d16 = AmLine16;
			unsigned d8 = (d16 >> 1);
			DrawAutomapVertDoor(sx - d16, sy - d8);
		}
		if (automap_type & MAF_WEST_GRATE) {
			unsigned d32 = AmLine32;
			unsigned d16 = (d32 >> 1), d8 = (d32 >> 2);
			DrawLine(sx - d16, sy - d8, sx - d32, sy, COLOR_DIM);
		}
		if (automap_type & (MAF_WEST_GRATE | MAF_WEST_ARCH)) {
			DrawAutomapDiamond(sx, sy);
		}
		if ((automap_type & (MAF_WEST_DOOR | MAF_WEST_GRATE | MAF_WEST_ARCH)) == 0) {
			unsigned d32 = AmLine32;
			unsigned d16 = (d32 >> 1);
			DrawLine(sx, sy - d16, sx - d32, sy, COLOR_DIM);
		}
	}

	if (automap_type & MAF_DO_NORTH_EAST) {
		if (automap_type & MAF_EAST_DOOR) {
			unsigned d16 = AmLine16;
			unsigned d8 = (d16 >> 1);
			DrawAutomapHorzDoor(sx + d16, sy - d8);
		}
		if (automap_type & MAF_EAST_GRATE) {
			unsigned d32 = AmLine32;
			unsigned d16 = (d32 >> 1), d8 = (d32 >> 2);
			DrawLine(sx + d16, sy - d8, sx + d32, sy, COLOR_DIM);
		}
		if (automap_type & (MAF_EAST_GRATE | MAF_EAST_ARCH)) {
			DrawAutomapDiamond(sx, sy);
		}
		if ((automap_type & (MAF_EAST_DOOR | MAF_EAST_GRATE | MAF_EAST_ARCH)) == 0) {
			unsigned d32 = AmLine32;
			unsigned d16 = (d32 >> 1);
			DrawLine(sx, sy - d16, sx + d32, sy, COLOR_DIM);
		}
	}

	if (automap_type & MAF_DO_SOUTH_WEST) {
		unsigned d32 = AmLine32;
		unsigned d16 = (d32 >> 1);
		if (automap_type & MAF_WEST_DOOR) {
			unsigned d8 = (d32 >> 2);
			DrawAutomapHorzDoor(sx - d16, sy + d8);
		} else {
			DrawLine(sx, sy + d16, sx - d32, sy, COLOR_DIM);
		}
	}

	if (automap_type & MAF_DO_SOUTH_EAST) {
		unsigned d32 = AmLine32;
		unsigned d16 = (d32 >> 1);
		if (automap_type & MAF_EAST_DOOR) {
			unsigned d8 = (d32 >> 2);
			DrawAutomapVertDoor(sx + d16, sy + d8);
		} else {
			DrawLine(sx, sy + d16, sx + d32, sy, COLOR_DIM);
		}
	}
}

/*static void DrawAutomapItem(int x, int y, BYTE color)
{
	int x1, y1, x2, y2;

	x1 = x - AmLine32 / 2;
	y1 = y - AmLine16 / 2;
	x2 = x1 + AmLine64 / 2;
	y2 = y1 + AmLine32 / 2;
	DrawLine(x, y1, x1, y, color);
	DrawLine(x, y1, x2, y, color);
	DrawLine(x, y2, x1, y, color);
	DrawLine(x, y2, x2, y, color);
}

static void SearchAutomapItem()
{
	PlayerStruct* p;
	int x, y;
	int x1, y1, x2, y2, xoff, yoff;
	int i, j;
	unsigned d16 = AmLine16;

	x = 2 * AutoMapXOfs + ViewX;
	y = 2 * AutoMapYOfs + ViewY;
	xoff = (ScrollInfo._sxoff * (int)AutoMapScale / 128 >> 1) + SCREEN_WIDTH / 2 + SCREEN_X - (x - y) * d16;
	yoff = (ScrollInfo._syoff * (int)AutoMapScale / 128 >> 1) + VIEWPORT_HEIGHT / 2 + SCREEN_Y - (x + y) * (d16 >> 1) - (d16 >> 1);

	p = &myplr;
	if (p->_pmode == PM_WALK2) {
		x = p->_poldx;
		y = p->_poldy;
	} else {
		x = p->_px;
		y = p->_py;
	}

	x1 = x - 8;
	if (x1 < 0)
		x1 = 0;

	y1 = y - 8;
	if (y1 < 0)
		y1 = 0;

	x2 = x + 8;
	if (x2 > MAXDUNX - 1)
		x2 = MAXDUNX - 1;

	y2 = y + 8;
	if (y2 > MAXDUNY - 1)
		y2 = MAXDUNY - 1;

	for (i = x1; i <= x2; i++) {
		for (j = y1; j <= y2; j++) {
			if (dItem[i][j] != 0) {
				x = xoff + (i - j) * d16;
				y = yoff + (i + j) * (d16 >> 1);
				DrawAutomapItem(x, y, COLOR_ITEM);
			}
		}
	}
}*/

/**
 * @brief Renders an arrow on the automap, centered on and facing the direction of the player.
 */
static void DrawAutomapPlr(int pnum, int playerColor)
{
	PlayerStruct* p;
	int px, py;
	int x, y;
	unsigned d16 = AmLine16;

	p = &plr;
	px = p->_px;
	py = p->_py;
	px -= 2 * AutoMapXOfs + ViewX;
	py -= 2 * AutoMapYOfs + ViewY;

	//x = (p->_pxoff * (int)AutoMapScale / 128 >> 1) + (ScrollInfo._sxoff * (int)AutoMapScale / 128 >> 1) + (px - py) * d16 + SCREEN_WIDTH / 2 + SCREEN_X;
	//y = (p->_pyoff * (int)AutoMapScale / 128 >> 1) + (ScrollInfo._syoff * (int)AutoMapScale / 128 >> 1) + (px + py) * (d16 >> 1) + VIEWPORT_HEIGHT / 2 + SCREEN_Y;

	x = ((p->_pxoff + ScrollInfo._sxoff) * (int)AutoMapScale / 128 >> 1) + (px - py) * d16 + SCREEN_WIDTH / 2 + SCREEN_X;
	y = ((p->_pyoff + ScrollInfo._syoff) * (int)AutoMapScale / 128 >> 1) + (px + py) * (d16 >> 1) + VIEWPORT_HEIGHT / 2 + SCREEN_Y;

	//y -= (d16 >> 1);

	if (y < SCREEN_Y || y >= VIEWPORT_HEIGHT + SCREEN_Y || x < SCREEN_X || x >= SCREEN_WIDTH + SCREEN_X)
		return;

	static_assert(BORDER_LEFT >= (MAP_SCALE_MAX * TILE_WIDTH) / 128 / 4, "Make sure the automap-renderer does not have to check for clipping V.");
	static_assert(BORDER_TOP >= (MAP_SCALE_MAX * TILE_WIDTH) / 128 / 4, "Make sure the automap-renderer does not have to check for clipping VII.");
	static_assert(BORDER_BOTTOM >= (MAP_SCALE_MAX * TILE_WIDTH) / 128 / 4, "Make sure the automap-renderer does not have to check for clipping VIII.");

	unsigned d8 = (d16 >> 1), d4 = (d16 >> 2);
	switch (p->_pdir) {
	case DIR_N: {
		DrawLine(x, y - d16, x, y, playerColor);
		DrawLine(x, y - d16, x - d4, y - d8, playerColor);
		DrawLine(x, y - d16, x + d4, y - d8, playerColor);
	} break;
	case DIR_NE: {
		DrawLine(x, y, x + d16, y - d8, playerColor);
		DrawLine(x + d4, y - d8, x + d16, y - d8, playerColor);
		DrawLine(x + d16, y - d8, x + d16 - d4, y, playerColor);
	} break;
	case DIR_E: {
		DrawLine(x, y, x + d16, y, playerColor);
		DrawLine(x + d8, y - d4, x + d16, y, playerColor);
		DrawLine(x + d8, y + d4, x + d16, y, playerColor);
	} break;
	case DIR_SE: {
		DrawLine(x, y, x + d16, y + d8, playerColor);
		DrawLine(x + d16 - d4, y, x + d16, y + d8, playerColor);
		DrawLine(x + d4, y + d8, x + d16, y + d8, playerColor);
	} break;
	case DIR_S: {
		DrawLine(x, y, x, y + d16, playerColor);
		DrawLine(x, y + d16, x + d4, y + d8, playerColor);
		DrawLine(x - d4, y + d8, x, y + d16, playerColor);
	} break;
	case DIR_SW: {
		DrawLine(x - d16, y + d8, x, y, playerColor);
		DrawLine(x - d16 + d4, y, x - d16, y + d8, playerColor);
		DrawLine(x - d16, y + d8, x - d4, y + d8, playerColor);
	} break;
	case DIR_W: {
		DrawLine(x - d16, y, x, y, playerColor);
		DrawLine(x - d16, y, x - d8, y - d4, playerColor);
		DrawLine(x - d16, y, x - d8, y + d4, playerColor);
	} break;
	case DIR_NW: {
		DrawLine(x - d16, y - d8, x, y, playerColor);
		DrawLine(x - d16, y - d8, x - d4, y - d8, playerColor);
		DrawLine(x - d16, y - d8, x - d16 + d4, y, playerColor);
	} break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

/**
 * @brief Returns the automap shape at the given coordinate.
 */
static uint16_t GetAutomapType(int x, int y, bool view)
{
	if ((unsigned)x >= DMAXX) {
		return x == -1 && view
			&& (unsigned)y < DMAXY && automapview[0][y] ? MAF_EXTERN : MWT_NONE;
	}
	if ((unsigned)y >= DMAXY) {
		return y == -1 && view
			&& (unsigned)x < DMAXX && automapview[x][0] ? MAF_EXTERN : MWT_NONE;
	}

	if (!automapview[x][y] && view) {
		return 0;
	}

	return automaptype[dungeon[x][y]];
}

/**
 * @brief Renders game info, such as the name of the current level, and in multi player the name of the game and the game password.
 */
static void DrawAutomapText()
{
	PrintGameStr(SCREEN_X + 8, SCREEN_Y + 20, AllLevels[currLvl._dLevelIdx].dLevelName, COL_GOLD);
}

/**
 * @brief Renders the automap on screen.
 */
void DrawAutomap()
{
	int sx, sy, mapx, mapy;
	int i, j, cells;
	unsigned d64 = AmLine64;

	if (!_gbAutomapData) {
		DrawAutomapText();
		return;
	}

	//gpBufEnd = &gpBuffer[BUFFER_WIDTH * (SCREEN_Y + VIEWPORT_HEIGHT)];

	// calculate the map center in the dungeon matrix
	mapx = (ViewX - DBORDERX) >> 1;
	mapx += AutoMapXOfs;
	if (mapx < 0) {
		AutoMapXOfs -= mapx;
		mapx = 0;
	} else if (mapx > (DMAXX - 1)) {
		AutoMapXOfs -= mapx - (DMAXX - 1);
		mapx = DMAXX - 1;
	}

	mapy = (ViewY - DBORDERY) >> 1;
	mapy += AutoMapYOfs;
	if (mapy < 0) {
		AutoMapYOfs -= mapy;
		mapy = 0;
	} else if (mapy > (DMAXY - 1)) {
		AutoMapYOfs -= mapy - (DMAXY - 1);
		mapy = DMAXY - 1;
	}

	// assert(d64 <= (MAP_SCALE_MAX * TILE_WIDTH) / 128);
	//static_assert(BORDER_LEFT >= (MAP_SCALE_MAX * TILE_WIDTH) / 128, "Make sure the automap-renderer does not have to check for clipping I."); - unnecessary, since the cells are limited to the screen
	//static_assert(BORDER_TOP >= (MAP_SCALE_MAX * TILE_WIDTH) / 128, "Make sure the automap-renderer does not have to check for clipping III.");
	//static_assert(BORDER_BOTTOM >= (MAP_SCALE_MAX * TILE_WIDTH) / 128 / 2, "Make sure the automap-renderer does not have to check for clipping IV.");

	// find an odd number of tiles which fits to the width
	// assert(SCREEN_WIDTH >= d64);
	cells = 2 * ((SCREEN_WIDTH - d64) / (2 * d64)) + 1;
	// make sure it fits to height as well
	// assert(cells < 2 * (VIEWPORT_HEIGHT - (d64 >> 1)) / (2 * (d64 >> 1))) + 1);

	/*if ((SCREEN_WIDTH / 2) % d64)
		cells++;
	if ((SCREEN_WIDTH / 2) % d64 >= (AutoMapScale << 5) / 128)
		cells++;

	if (ScrollInfo._sxoff + ScrollInfo._syoff)
		cells++;*/

	// find the starting dungeon coordinates
	mapx -= (cells & ~1); // mapx - (xcells / 2 + ycells / 2);
	//mapy -= 1;          // mapy - (ycells / 2 - xcells / 2);

	// calculate the center of the tile on the screen
	sx = SCREEN_WIDTH / 2 + SCREEN_X - d64 * (cells >> 1);
	sy = VIEWPORT_HEIGHT / 2 + SCREEN_Y - (d64 >> 1) * (cells >> 1);
	/*if (cells & 1) {
		sy -= (d64 >> 1);
	} else {
		sx += (d64 >> 1);
		sy -= (d64 >> 2);
	}*/
	if (ViewX & 1) {
		sx -= (d64 >> 2);
		sy -= (d64 >> 3);
	}
	if (ViewY & 1) {
		sx += (d64 >> 2);
		sy -= (d64 >> 3);
	}

	sx += ((int)AutoMapScale * ScrollInfo._sxoff / 128) >> 1;
	sy += ((int)AutoMapScale * ScrollInfo._syoff / 128) >> 1;

	// select the bottom edge of the tile
	sy += (d64 >> 2);

	// draw the tiles, two rows at a time
	for (i = 0; i < cells; i++) { // foreach ycells
		int x = sx;

		for (j = 0; j < cells; j++) { // foreach xcells 1.
			uint16_t maptype = GetAutomapType(mapx, mapy, true);
			if (maptype != MWT_NONE)
				DrawAutomapTile(x, sy, maptype);
			SHIFT_GRID(mapx, mapy, 1, 0);
			x += d64;
		}
		// Return to start of row
		SHIFT_GRID(mapx, mapy, -cells, 0);

		mapy++;
		x = sx + (d64 >> 1);
		sy += (d64 >> 2);
		for (j = 1; j < cells; j++) { // foreach xcells 2.
			SHIFT_GRID(mapx, mapy, 1, 0);
			uint16_t maptype = GetAutomapType(mapx, mapy, true);
			if (maptype != MWT_NONE)
				DrawAutomapTile(x, sy, maptype);
			x += d64;
		}
		// Return to start of row
		SHIFT_GRID(mapx, mapy, -(cells - 1), 0);
		mapx++;
		sy += (d64 >> 2);
	}

	for (int pnum = 0; pnum < MAX_PLRS; pnum++) {
		if (plr._pDunLevel == myplr._pDunLevel && plr._pActive && !plr._pLvlChanging) {
			if (plr._pTeam == myplr._pTeam)
				DrawAutomapPlr(pnum, pnum == mypnum ? COLOR_PLAYER : COLOR_FRIEND);
#if INET_MODE
			else
#else
			else if ((dFlags[plr._px][plr._py] & BFLAG_VISIBLE) || myplr._pInfraFlag)
#endif
				DrawAutomapPlr(pnum, COLOR_ENEMY);
		}
	}
	//if (AutoMapShowItems)
	//	SearchAutomapItem();
	DrawAutomapText();
	//gpBufEnd = &gpBuffer[BUFFER_WIDTH * (SCREEN_Y + SCREEN_HEIGHT)];
}

/**
 * @brief Marks the given coordinate as within view on the automap.
 */
void SetAutomapView(int x, int y)
{
	uint16_t maptype;
	int xx, yy;

	xx = (x - DBORDERX) >> 1;
	yy = (y - DBORDERY) >> 1;

	if (xx < 0 || xx >= DMAXX || yy < 0 || yy >= DMAXY) {
		return;
	}

	automapview[xx][yy] = TRUE;

	maptype = automaptype[dungeon[xx][yy]]; // GetAutomapType(xx, yy, false);

	switch (maptype & MAF_TYPE) {
	case MWT_NONE:
	case MWT_PILLAR:
		break;
	case MWT_NORTH_WEST:
		//if (solid) {
		if (maptype & MAF_EXTERN) {
			if (GetAutomapType(xx, yy + 1, false) == (MAF_EXTERN | MWT_CORNER))
				automapview[xx][yy + 1] = TRUE;
		} else if (GetAutomapType(xx - 1, yy, false) & MAF_EXTERN) {
			automapview[xx - 1][yy] = TRUE;
		}
		break;
	case MWT_NORTH_EAST:
		//if (solid) {
		if (maptype & MAF_EXTERN) {
			if (GetAutomapType(xx + 1, yy, false) == (MAF_EXTERN | MWT_CORNER))
				automapview[xx + 1][yy] = TRUE;
		} else if (GetAutomapType(xx, yy - 1, false) & MAF_EXTERN) {
			automapview[xx][yy - 1] = TRUE;
		}
		break;
	case MWT_NORTH:
		//if (solid) {
		if (maptype & MAF_EXTERN) {
			if (GetAutomapType(xx, yy + 1, false) == (MAF_EXTERN | MWT_CORNER))
				automapview[xx][yy + 1] = TRUE;
			if (GetAutomapType(xx + 1, yy, false) == (MAF_EXTERN | MWT_CORNER))
				automapview[xx + 1][yy] = TRUE;
		} else {
			if (GetAutomapType(xx - 1, yy, false) & MAF_EXTERN)
				automapview[xx - 1][yy] = TRUE;
			if (GetAutomapType(xx, yy - 1, false) & MAF_EXTERN)
				automapview[xx][yy - 1] = TRUE;
			if (GetAutomapType(xx - 1, yy - 1, false) & MAF_EXTERN)
				automapview[xx - 1][yy - 1] = TRUE;
		}
		break;
	case MWT_NORTH_WEST_END:
		//if (solid) {
		if (maptype & MAF_EXTERN) {
			if (GetAutomapType(xx, yy - 1, false) & MAF_EXTERN)
				automapview[xx][yy - 1] = TRUE;
			if (GetAutomapType(xx, yy + 1, false) == (MAF_EXTERN | MWT_CORNER))
				automapview[xx][yy + 1] = TRUE;
		} else if (GetAutomapType(xx - 1, yy, false) & MAF_EXTERN) {
			automapview[xx - 1][yy] = TRUE;
		}
		break;
	case MWT_NORTH_EAST_END:
		//if (solid) {
		if (maptype & MAF_EXTERN) {
			if (GetAutomapType(xx - 1, yy, false) & MAF_EXTERN)
				automapview[xx - 1][yy] = TRUE;
			if (GetAutomapType(xx + 1, yy, false) == (MAF_EXTERN | MWT_CORNER))
				automapview[xx + 1][yy] = TRUE;
		} else if (GetAutomapType(xx, yy - 1, false) & MAF_EXTERN) {
			automapview[xx][yy - 1] = TRUE;
		}
		break;
	case MWT_CORNER:
	case MWT_WEST:
	case MWT_EAST:
	case MWT_SOUTH_WEST:
	case MWT_SOUTH_EAST:
	case MWT_SOUTH:
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

DEVILUTION_END_NAMESPACE
