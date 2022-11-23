/**
 * @file automap.cpp
 *
 * Implementation of the in-game map overlay.
 */
#include "all.h"
#include "engine/render/automap_render.hpp"

DEVILUTION_BEGIN_NAMESPACE

/**
 * Maps from tile_id to automap type.
 */
uint16_t automaptype[256];
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
unsigned AmLine8;
unsigned AmLine4;

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

// base mapflags set in the corresponding .AMP file (only the lower byte is used)
#define MAPFLAG_TYPE      0x00FF
#define MAPFLAG_VERTDOOR  0x0100
#define MAPFLAG_HORZDOOR  0x0200
#define MAPFLAG_VERTARCH  0x0400
#define MAPFLAG_HORZARCH  0x0800
#define MAPFLAG_VERTGRATE 0x1000
#define MAPFLAG_HORZGRATE 0x2000
#define MAPFLAG_DIRT      0x4000
#define MAPFLAG_STAIRS    0x8000
// calculated mapflags
#define MAPFLAG_DOVERT      0x0010
#define MAPFLAG_DOHORZ      0x0020
#define MAPFLAG_DOVERT_CAVE 0x0040
#define MAPFLAG_DOHORZ_CAVE 0x0080

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
	AmLine8 = AmLine16 >> 1;
	AmLine4 = AmLine8 >> 1;
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

	mapData = AllLevels[currLvl._dLevelIdx].dAutomapData;
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
		// assert((automaptype[i] & MAPFLAG_TYPE) < 13); required by DrawAutomapTile and SetAutomapView
		lm++;
	}

	mem_free_dbg(pAFile);

	// patch dAutomapData - L2.AMP
	if (currLvl._dType == DTYPE_CATACOMBS) {
		automaptype[42] &= ~MAPFLAG_HORZARCH;
		automaptype[156] &= ~(MAPFLAG_VERTDOOR | MAPFLAG_TYPE);
		automaptype[157] &= ~(MAPFLAG_HORZDOOR | MAPFLAG_TYPE);
	}
	// patch dAutomapData - L4.AMP
	if (currLvl._dType == DTYPE_HELL) {
		automaptype[52] |= MAPFLAG_VERTGRATE;
		automaptype[56] |= MAPFLAG_HORZGRATE;
	}

	memset(automapview, 0, sizeof(automapview));

	AutoMapXOfs = 0;
	AutoMapYOfs = 0;
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
		AmLine8 = AmLine16 >> 1;
		AmLine4 = AmLine8 >> 1;
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
		AmLine8 = AmLine16 >> 1;
		AmLine4 = AmLine8 >> 1;
	}
}

static void DrawAutomapHorzDoor(int x, int y)
{
	int d16 = AmLine16, d8 = AmLine8;

	AutomapDrawLine(x - d16, y - d8, x - d8, y - AmLine4, COLOR_DIM);
	AutomapDrawLine(x + d16, y + d8, x + d8, y + AmLine4, COLOR_DIM);
	AutomapDrawLine(x, y - d8, x - d16, y, COLOR_BRIGHT);
	AutomapDrawLine(x, y - d8, x + d16, y, COLOR_BRIGHT);
	AutomapDrawLine(x, y + d8, x - d16, y, COLOR_BRIGHT);
	AutomapDrawLine(x, y + d8, x + d16, y, COLOR_BRIGHT);
}

static void DrawAutomapVertDoor(int x, int y)
{
	int d16 = AmLine16, d8 = AmLine8;

	AutomapDrawLine(x + d16, y - d8, x + d8, y - AmLine4, COLOR_DIM);
	AutomapDrawLine(x - d16, y + d8, x - d8, y + AmLine4, COLOR_DIM);
	AutomapDrawLine(x, y - d8, x - d16, y, COLOR_BRIGHT);
	AutomapDrawLine(x, y - d8, x + d16, y, COLOR_BRIGHT);
	AutomapDrawLine(x, y + d8, x - d16, y, COLOR_BRIGHT);
	AutomapDrawLine(x, y + d8, x + d16, y, COLOR_BRIGHT);
}

static void DrawAutomapDiamond(int x, int y)
{
	int d16 = AmLine16, y2;

	y2 = y - AmLine8;

	AutomapDrawLine(x, y - d16, x + d16, y2, COLOR_DIM);
	AutomapDrawLine(x, y - d16, x - d16, y2, COLOR_DIM);
	AutomapDrawLine(x, y, x - d16, y2, COLOR_DIM);
	AutomapDrawLine(x, y, x + d16, y2, COLOR_DIM);
}

/**
 * @brief Renders the given automap shape at the specified screen coordinates.
 */
static void DrawAutomapTile(int sx, int sy, uint16_t automap_type)
{
	uint8_t type;

	if (automap_type & MAPFLAG_DIRT) {
		AutomapDrawPixel(sx, sy, COLOR_DIM);
		AutomapDrawPixel(sx - AmLine8, sy - AmLine4, COLOR_DIM);
		AutomapDrawPixel(sx - AmLine8, sy + AmLine4, COLOR_DIM);
		AutomapDrawPixel(sx + AmLine8, sy - AmLine4, COLOR_DIM);
		AutomapDrawPixel(sx + AmLine8, sy + AmLine4, COLOR_DIM);
		AutomapDrawPixel(sx - AmLine16, sy, COLOR_DIM);
		AutomapDrawPixel(sx + AmLine16, sy, COLOR_DIM);
		AutomapDrawPixel(sx, sy - AmLine8, COLOR_DIM);
		AutomapDrawPixel(sx, sy + AmLine8, COLOR_DIM);
		AutomapDrawPixel(sx + AmLine8 - AmLine32, sy + AmLine4, COLOR_DIM);
		AutomapDrawPixel(sx - AmLine8 + AmLine32, sy + AmLine4, COLOR_DIM);
		AutomapDrawPixel(sx - AmLine16, sy + AmLine8, COLOR_DIM);
		AutomapDrawPixel(sx + AmLine16, sy + AmLine8, COLOR_DIM);
		AutomapDrawPixel(sx - AmLine8, sy + AmLine16 - AmLine4, COLOR_DIM);
		AutomapDrawPixel(sx + AmLine8, sy + AmLine16 - AmLine4, COLOR_DIM);
		AutomapDrawPixel(sx, sy + AmLine16, COLOR_DIM);
	}

	if (automap_type & MAPFLAG_STAIRS) {
		AutomapDrawLine(sx - AmLine8, sy - AmLine8 - AmLine4, sx + AmLine8 + AmLine16, sy + AmLine4, COLOR_BRIGHT);
		AutomapDrawLine(sx - AmLine16, sy - AmLine8, sx + AmLine16, sy + AmLine8, COLOR_BRIGHT);
		AutomapDrawLine(sx - AmLine16 - AmLine8, sy - AmLine4, sx + AmLine8, sy + AmLine8 + AmLine4, COLOR_BRIGHT);
		AutomapDrawLine(sx - AmLine32, sy, sx, sy + AmLine16, COLOR_BRIGHT);
	}

	type = automap_type & MAPFLAG_TYPE;
	automap_type &= ~MAPFLAG_TYPE;
	switch (type) {
	case 0:
	case 7:
		break;
	case 1: // stand-alone column or other unpassable object
		DrawAutomapDiamond(sx, sy);
		break;
	case 2:
	case 5:
		automap_type |= MAPFLAG_DOVERT;
		break;
	case 3:
	case 6:
		automap_type |= MAPFLAG_DOHORZ;
		break;
	case 4:
		automap_type |= MAPFLAG_DOHORZ | MAPFLAG_DOVERT;
		break;
	case 8:
		automap_type |= MAPFLAG_DOVERT | MAPFLAG_DOHORZ_CAVE;
		break;
	case 9:
		automap_type |= MAPFLAG_DOHORZ | MAPFLAG_DOVERT_CAVE;
		break;
	case 10:
		automap_type |= MAPFLAG_DOHORZ_CAVE;
		break;
	case 11:
		automap_type |= MAPFLAG_DOVERT_CAVE;
		break;
	case 12:
		automap_type |= MAPFLAG_DOHORZ_CAVE | MAPFLAG_DOVERT_CAVE;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	if (automap_type & MAPFLAG_DOVERT) { // right-facing obstacle
		if (automap_type & MAPFLAG_VERTDOOR) { // two wall segments with a door in the middle
			DrawAutomapVertDoor(sx - AmLine16, sy - AmLine8);
		}
		if (automap_type & MAPFLAG_VERTGRATE) { // right-facing half-wall
			AutomapDrawLine(sx - AmLine16, sy - AmLine8, sx - AmLine32, sy, COLOR_DIM);
		}
		if (automap_type & (MAPFLAG_VERTGRATE | MAPFLAG_VERTARCH)) { // window or passable column
			DrawAutomapDiamond(sx, sy);
		}
		if ((automap_type & (MAPFLAG_VERTDOOR | MAPFLAG_VERTGRATE | MAPFLAG_VERTARCH)) == 0)
			AutomapDrawLine(sx, sy - AmLine16, sx - AmLine32, sy, COLOR_DIM);
	}

	if (automap_type & MAPFLAG_DOHORZ) { // left-facing obstacle
		if (automap_type & MAPFLAG_HORZDOOR) {
			DrawAutomapHorzDoor(sx + AmLine16, sy - AmLine8);
		}
		if (automap_type & MAPFLAG_HORZGRATE) {
			AutomapDrawLine(sx + AmLine16, sy - AmLine8, sx + AmLine32, sy, COLOR_DIM);
		}
		if (automap_type & (MAPFLAG_HORZGRATE | MAPFLAG_HORZARCH)) {
			DrawAutomapDiamond(sx, sy);
		}
		if ((automap_type & (MAPFLAG_HORZDOOR | MAPFLAG_HORZGRATE | MAPFLAG_HORZARCH)) == 0)
			AutomapDrawLine(sx, sy - AmLine16, sx + AmLine32, sy, COLOR_DIM);
	}

	// for caves the horz/vert flags are switched
	if (automap_type & MAPFLAG_DOHORZ_CAVE) {
		if (automap_type & MAPFLAG_VERTDOOR) {
			DrawAutomapHorzDoor(sx - AmLine16, sy + AmLine8);
		} else
			AutomapDrawLine(sx, sy + AmLine16, sx - AmLine32, sy, COLOR_DIM);
	}

	if (automap_type & MAPFLAG_DOVERT_CAVE) {
		if (automap_type & MAPFLAG_HORZDOOR) {
			DrawAutomapVertDoor(sx + AmLine16, sy + AmLine8);
		} else
			AutomapDrawLine(sx, sy + AmLine16, sx + AmLine32, sy, COLOR_DIM);
	}
}

/*static void DrawAutomapItem(int x, int y, BYTE color)
{
	int x1, y1, x2, y2;

	x1 = x - AmLine32 / 2;
	y1 = y - AmLine16 / 2;
	x2 = x1 + AmLine64 / 2;
	y2 = y1 + AmLine32 / 2;
	AutomapDrawLine(x, y1, x1, y, color);
	AutomapDrawLine(x, y1, x2, y, color);
	AutomapDrawLine(x, y2, x1, y, color);
	AutomapDrawLine(x, y2, x2, y, color);
}

static void SearchAutomapItem()
{
	PlayerStruct* p;
	int x, y;
	int x1, y1, x2, y2, xoff, yoff;
	int i, j;

	x = 2 * AutoMapXOfs + ViewX;
	y = 2 * AutoMapYOfs + ViewY;
	xoff = (ScrollInfo._sxoff * (int)AutoMapScale / 128 >> 1) + SCREEN_WIDTH / 2 + SCREEN_X - (x - y) * AmLine16;
	yoff = (ScrollInfo._syoff * (int)AutoMapScale / 128 >> 1) + VIEWPORT_HEIGHT / 2 + SCREEN_Y - (x + y) * AmLine8 - AmLine8;

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
				x = xoff + (i - j) * AmLine16;
				y = yoff + (i + j) * AmLine8;
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

	p = &plr;
	px = p->_px;
	py = p->_py;
	px -= 2 * AutoMapXOfs + ViewX;
	py -= 2 * AutoMapYOfs + ViewY;

	//x = (p->_pxoff * (int)AutoMapScale / 128 >> 1) + (ScrollInfo._sxoff * (int)AutoMapScale / 128 >> 1) + (px - py) * AmLine16 + SCREEN_WIDTH / 2 + SCREEN_X;
	//y = (p->_pyoff * (int)AutoMapScale / 128 >> 1) + (ScrollInfo._syoff * (int)AutoMapScale / 128 >> 1) + (px + py) * AmLine8 + VIEWPORT_HEIGHT / 2 + SCREEN_Y;

	x = ((p->_pxoff + ScrollInfo._sxoff) * (int)AutoMapScale / 128 >> 1) + (px - py) * AmLine16 + SCREEN_WIDTH / 2 + SCREEN_X;
	y = ((p->_pyoff + ScrollInfo._syoff) * (int)AutoMapScale / 128 >> 1) + (px + py) * AmLine8 + VIEWPORT_HEIGHT / 2 + SCREEN_Y;

	//y -= AmLine8;

	if (y < SCREEN_Y || y >= VIEWPORT_HEIGHT + SCREEN_Y || x < SCREEN_X || x >= SCREEN_WIDTH + SCREEN_X)
		return;

	static_assert(BORDER_LEFT >= (MAP_SCALE_MAX * TILE_WIDTH) / 128 / 4, "Make sure the automap-renderer does not have to check for clipping V.");
	static_assert(BORDER_TOP >= (MAP_SCALE_MAX * TILE_WIDTH) / 128 / 4, "Make sure the automap-renderer does not have to check for clipping VII.");
	static_assert(BORDER_BOTTOM >= (MAP_SCALE_MAX * TILE_WIDTH) / 128 / 4, "Make sure the automap-renderer does not have to check for clipping VIII.");

	switch (p->_pdir) {
	case DIR_N:
		AutomapDrawLine(x, y, x, y - AmLine16, playerColor);
		AutomapDrawLine(x, y - AmLine16, x - AmLine4, y - AmLine8, playerColor);
		AutomapDrawLine(x, y - AmLine16, x + AmLine4, y - AmLine8, playerColor);
		break;
	case DIR_NE:
		AutomapDrawLine(x, y, x + AmLine16, y - AmLine8, playerColor);
		AutomapDrawLine(x + AmLine16, y - AmLine8, x + AmLine4, y - AmLine8, playerColor);
		AutomapDrawLine(x + AmLine16, y - AmLine8, x + AmLine8 + AmLine4, y, playerColor);
		break;
	case DIR_E:
		AutomapDrawLine(x, y, x + AmLine16, y, playerColor);
		AutomapDrawLine(x + AmLine16, y, x + AmLine8, y - AmLine4, playerColor);
		AutomapDrawLine(x + AmLine16, y, x + AmLine8, y + AmLine4, playerColor);
		break;
	case DIR_SE:
		AutomapDrawLine(x, y, x + AmLine16, y + AmLine8, playerColor);
		AutomapDrawLine(x + AmLine16, y + AmLine8, x + AmLine8 + AmLine4, y, playerColor);
		AutomapDrawLine(x + AmLine16, y + AmLine8, x + AmLine4, y + AmLine8, playerColor);
		break;
	case DIR_S:
		AutomapDrawLine(x, y, x, y + AmLine16, playerColor);
		AutomapDrawLine(x, y + AmLine16, x + AmLine4, y + AmLine8, playerColor);
		AutomapDrawLine(x, y + AmLine16, x - AmLine4, y + AmLine8, playerColor);
		break;
	case DIR_SW:
		AutomapDrawLine(x, y, x - AmLine16, y + AmLine8, playerColor);
		AutomapDrawLine(x - AmLine16, y + AmLine8, x - AmLine4 - AmLine8, y, playerColor);
		AutomapDrawLine(x - AmLine16, y + AmLine8, x - AmLine4, y + AmLine8, playerColor);
		break;
	case DIR_W:
		AutomapDrawLine(x, y, x - AmLine16, y, playerColor);
		AutomapDrawLine(x - AmLine16, y, x - AmLine8, y - AmLine4, playerColor);
		AutomapDrawLine(x - AmLine16, y, x - AmLine8, y + AmLine4, playerColor);
		break;
	case DIR_NW:
		AutomapDrawLine(x, y, x - AmLine16, y - AmLine8, playerColor);
		AutomapDrawLine(x - AmLine16, y - AmLine8, x - AmLine4, y - AmLine8, playerColor);
		AutomapDrawLine(x - AmLine16, y - AmLine8, x - AmLine4 - AmLine8, y, playerColor);
		break;
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
	uint16_t rv;

	if ((unsigned)x >= DMAXX) {
		return x == -1 && view
			&& (unsigned)y < DMAXY && automapview[0][y] ? MAPFLAG_DIRT : 0;
	}
	if ((unsigned)y >= DMAXY) {
		return y == -1 && view
			&& (unsigned)x < DMAXX && automapview[x][0] ? MAPFLAG_DIRT : 0;
	}

	if (!automapview[x][y] && view) {
		return 0;
	}

	rv = automaptype[dungeon[x][y]];
	if (rv == 7) { // TODO: check for (rv & 7) == 7 instead to ignore the high bits?
		// TODO: this feels like a hack. A better logic would be to check around
		//  the tile and see if there is a wall. That way the automaptype of the 
		//  pillars of the BONECHAMBER-stairs (39/40/41+42) could be set to 7 as well.
		//  Which would eliminate the ugly corners in the catacombs.
		// The check would be more 'expensive' though.
		//if (x >= 1 && y >= 1) {
		//	if ((automaptype[dungeon[x - 1][y]] & MAPFLAG_HORZARCH)
		//	 && (automaptype[dungeon[x][y - 1]] & MAPFLAG_VERTARCH))
		//		rv = 1;
			if (x >= 2 && y >= 2) {
				if ((automaptype[dungeon[x - 2][y]] & MAPFLAG_HORZARCH)
				 && (automaptype[dungeon[x][y - 2]] & MAPFLAG_VERTARCH))
					rv = 1;
			}
		//}
	}
	return rv;
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

	// assert(AmLine64 <= (MAP_SCALE_MAX * TILE_WIDTH) / 128);
	static_assert(BORDER_LEFT >= (MAP_SCALE_MAX * TILE_WIDTH) / 128, "Make sure the automap-renderer does not have to check for clipping I.");
	static_assert(BORDER_TOP >= (MAP_SCALE_MAX * TILE_WIDTH) / 128, "Make sure the automap-renderer does not have to check for clipping III.");
	static_assert(BORDER_BOTTOM >= (MAP_SCALE_MAX * TILE_WIDTH) / 128 / 2, "Make sure the automap-renderer does not have to check for clipping IV.");

	// find an odd number of tiles which fits to the width
	// assert(SCREEN_WIDTH >= AmLine64);
	cells = 2 * ((SCREEN_WIDTH - AmLine64) / (2 * AmLine64)) + 1;
	// make sure it fits to height as well
	// assert(cells < 2 * (VIEWPORT_HEIGHT - AmLine32) / (2 * AmLine32)) + 1);

	/*if ((SCREEN_WIDTH / 2) % AmLine64)
		cells++;
	if ((SCREEN_WIDTH / 2) % AmLine64 >= (AutoMapScale << 5) / 128)
		cells++;

	if (ScrollInfo._sxoff + ScrollInfo._syoff)
		cells++;*/

	// find the starting dungeon coordinates
	mapx -= (cells & ~1); // mapx - (xcells / 2 + ycells / 2);
	//mapy -= 1;          // mapy - (ycells / 2 - xcells / 2);

	// calculate the center of the tile on the screen
	sx = SCREEN_WIDTH / 2 + SCREEN_X - AmLine64 * (cells >> 1);
	sy = VIEWPORT_HEIGHT / 2 + SCREEN_Y - AmLine32 * (cells >> 1);
	/*if (cells & 1) {
		sy -= AmLine32;
	} else {
		sx += AmLine32;
		sy -= AmLine16;
	}*/
	if (ViewX & 1) {
		sx -= AmLine16;
		sy -= AmLine8;
	}
	if (ViewY & 1) {
		sx += AmLine16;
		sy -= AmLine8;
	}

	sx += ((int)AutoMapScale * ScrollInfo._sxoff / 128) >> 1;
	sy += ((int)AutoMapScale * ScrollInfo._syoff / 128) >> 1;

	// select the bottom edge of the tile
	sy += AmLine16;

	// draw the tiles, two rows at a time
	for (i = 0; i < cells; i++) { // foreach ycells
		int x = sx;

		for (j = 0; j < cells; j++) { // foreach xcells 1.
			uint16_t maptype = GetAutomapType(mapx, mapy, true);
			if (maptype != 0)
				DrawAutomapTile(x, sy, maptype);
			SHIFT_GRID(mapx, mapy, 1, 0);
			x += AmLine64;
		}
		// Return to start of row
		SHIFT_GRID(mapx, mapy, -cells, 0);

		mapy++;
		x = sx + AmLine32;
		sy += AmLine16;
		for (j = 1; j < cells; j++) { // foreach xcells 2.
			SHIFT_GRID(mapx, mapy, 1, 0);
			uint16_t maptype = GetAutomapType(mapx, mapy, true);
			if (maptype != 0)
				DrawAutomapTile(x, sy, maptype);
			x += AmLine64;
		}
		// Return to start of row
		SHIFT_GRID(mapx, mapy, -(cells - 1), 0);
		mapx++;
		sy += AmLine16;
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

	switch (maptype & MAPFLAG_TYPE) {
	case 0:
	case 1:
		break;
	case 2:
		//if (solid) {
		if (maptype & MAPFLAG_DIRT) {
			if (GetAutomapType(xx, yy + 1, false) == (MAPFLAG_DIRT | 0x07))
				automapview[xx][yy + 1] = TRUE;
		} else if (GetAutomapType(xx - 1, yy, false) & MAPFLAG_DIRT) {
			automapview[xx - 1][yy] = TRUE;
		}
		break;
	case 3:
		//if (solid) {
		if (maptype & MAPFLAG_DIRT) {
			if (GetAutomapType(xx + 1, yy, false) == (MAPFLAG_DIRT | 0x07))
				automapview[xx + 1][yy] = TRUE;
		} else if (GetAutomapType(xx, yy - 1, false) & MAPFLAG_DIRT) {
			automapview[xx][yy - 1] = TRUE;
		}
		break;
	case 4:
		//if (solid) {
		if (maptype & MAPFLAG_DIRT) {
			if (GetAutomapType(xx, yy + 1, false) == (MAPFLAG_DIRT | 0x07))
				automapview[xx][yy + 1] = TRUE;
			if (GetAutomapType(xx + 1, yy, false) == (MAPFLAG_DIRT | 0x07))
				automapview[xx + 1][yy] = TRUE;
		} else {
			if (GetAutomapType(xx - 1, yy, false) & MAPFLAG_DIRT)
				automapview[xx - 1][yy] = TRUE;
			if (GetAutomapType(xx, yy - 1, false) & MAPFLAG_DIRT)
				automapview[xx][yy - 1] = TRUE;
			if (GetAutomapType(xx - 1, yy - 1, false) & MAPFLAG_DIRT)
				automapview[xx - 1][yy - 1] = TRUE;
		}
		break;
	case 5:
		//if (solid) {
		if (maptype & MAPFLAG_DIRT) {
			if (GetAutomapType(xx, yy - 1, false) & MAPFLAG_DIRT)
				automapview[xx][yy - 1] = TRUE;
			if (GetAutomapType(xx, yy + 1, false) == (MAPFLAG_DIRT | 0x07))
				automapview[xx][yy + 1] = TRUE;
		} else if (GetAutomapType(xx - 1, yy, false) & MAPFLAG_DIRT) {
			automapview[xx - 1][yy] = TRUE;
		}
		break;
	case 6:
		//if (solid) {
		if (maptype & MAPFLAG_DIRT) {
			if (GetAutomapType(xx - 1, yy, false) & MAPFLAG_DIRT)
				automapview[xx - 1][yy] = TRUE;
			if (GetAutomapType(xx + 1, yy, false) == (MAPFLAG_DIRT | 0x07))
				automapview[xx + 1][yy] = TRUE;
		} else if (GetAutomapType(xx, yy - 1, false) & MAPFLAG_DIRT) {
			automapview[xx][yy - 1] = TRUE;
		}
		break;
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

DEVILUTION_END_NAMESPACE
