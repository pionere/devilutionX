/**
 * @file automap.cpp
 *
 * Implementation of the in-game map overlay.
 */
#include "all.h"
#include "engine/render/raw_render.h"
#include "engine/render/text_render.h"

DEVILUTION_BEGIN_NAMESPACE

#define MAP_SCALE_MAX 128
#define MAP_SCALE_MIN 16
#define MAP_SCALE_NORMAL 64
#define MAP_SCALE_MINI 32

#define MAP_MINI_WIDTH 160
#define MAP_MINI_HEIGHT 120

#define AUTOMAP_VALID (automaptype[0] == 0)

static_assert(MAP_SCALE_MAX <= UCHAR_MAX, "Mapscale values are stored in one byte.");
/** Specifies whether the automap is enabled (_automap_mode). */
BYTE gbAutomapflag = AMM_NONE;
/* The scale of the mini-automap. */
BYTE MiniMapScale = MAP_SCALE_MINI;
/* The scale of the normal-automap. */
BYTE NormalMapScale = MAP_SCALE_NORMAL;
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

/**
 * @brief Initializes the automap configuration.
 */
void InitAutomapOnce()
{
	gbAutomapflag = AMM_NONE;
	AutoMapScale = MAP_SCALE_NORMAL;
	InitAutomapScale();
	// these values are initialized by InitAutomap
	//_gbAutomapData = false;
	//memset(automaptype, 0, sizeof(automaptype));
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
	/* commented out because the flags are reset in gendung.cpp anyway
	static_assert(sizeof(dFlags) == MAXDUNX * MAXDUNY, "Linear traverse of dFlags does not work in InitAutomap.");
	pTmp = &dFlags[0][0];
	for (i = 0; i < MAXDUNX * MAXDUNY; i++, pTmp++) {
		assert((*pTmp & BFLAG_EXPLORED) == 0);
		*pTmp &= ~BFLAG_EXPLORED;
	}*/

	AutoMapXOfs = 0;
	AutoMapYOfs = 0;
}

bool IsAutomapActive()
{
	return gbAutomapflag != AMM_NONE && AUTOMAP_VALID;
}

/**
 * @brief Display/Hides the automap.
 */
void ToggleAutomap()
{
	gbAutomapflag++;
	if (gbAutomapflag == NUM_AMMS) {
		gbAutomapflag = AMM_NONE;
	} else if (gbAutomapflag == AMM_MINI) {
		AutoMapScale = MiniMapScale;
	} else {
		AutoMapScale = NormalMapScale;
	}
	//if (gbAutomapflag != AMM_NONE) {
		InitAutomapScale();
	//}
	//if (gbAutomapflag != AMM_NONE) {
		AutoMapXOfs = 0;
		AutoMapYOfs = 0;
	//}
}

/**
 * @brief Scrolls the automap upwards.
 */
void AutomapUp()
{
	SHIFT_GRID(AutoMapXOfs, AutoMapYOfs, 0, -2);
}

/**
 * @brief Scrolls the automap downwards.
 */
void AutomapDown()
{
	SHIFT_GRID(AutoMapXOfs, AutoMapYOfs, 0, 2);
}

/**
 * @brief Scrolls the automap leftwards.
 */
void AutomapLeft()
{
	SHIFT_GRID(AutoMapXOfs, AutoMapYOfs, -2, 0);
}

/**
 * @brief Scrolls the automap rightwards.
 */
void AutomapRight()
{
	SHIFT_GRID(AutoMapXOfs, AutoMapYOfs, 2, 0);
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
		if (gbAutomapflag == AMM_MINI) {
			MiniMapScale = AutoMapScale;
		} else { // if (gbAutomapflag == AMM_NORMAL) {
			// assert(gbAutomapflag != AMM_NONE);
			NormalMapScale = AutoMapScale;
		}
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
		if (gbAutomapflag == AMM_MINI) {
			MiniMapScale = AutoMapScale;
		} else { // if (gbAutomapflag == AMM_NORMAL) {
			// assert(gbAutomapflag != AMM_NONE);
			NormalMapScale = AutoMapScale;
		}
	}
}

static void DrawAutomapExtern(int sx, int sy)
{
	unsigned d32 = AmLine32;
	unsigned d8 = (d32 >> 2);

	DrawPixel(sx, sy - d8, COLOR_DIM);
}

static void DrawAutomapStairs(int sx, int sy)
{
	unsigned d32 = AmLine32;
	unsigned d16 = (d32 >> 1), d8 = (d32 >> 2), d4 = (d32 >> 3);

	DrawLine(sx - d16 + d8, sy - d16 + d4, sx + d8, sy - d16 + d4 + d8, COLOR_BRIGHT);
	DrawLine(sx - d16, sy - d8, sx, sy, COLOR_BRIGHT);
}

static void DrawAutomapDoorDiamond(int dir, int sx, int sy)
{
	unsigned d32 = AmLine32;
	unsigned d16 = (d32 >> 1), d8 = (d32 >> 2), d4 = (d32 >> 3);

	if (dir == 0) { // WEST
		sx -= d8;
		sy -= d4;
	} else {        // EAST
		sx += d8;
		sy -= d4;
	}

	DrawLine(sx - d16 + 2, sy - d8, sx - 1, sy - d16 + 2, COLOR_BRIGHT); // top left
	DrawLine(sx, sy - d16 + 2, sx + d16 - 3, sy - d8, COLOR_BRIGHT);     // top right
	DrawLine(sx - d16 + 2, sy - d8 + 1, sx - 1, sy - 1, COLOR_BRIGHT);   // bottom left
	DrawLine(sx, sy - 1, sx + d16 - 3, sy - d8 + 1, COLOR_BRIGHT);       // bottom right
}

/**
 * @brief Renders the given automap shape at the specified screen coordinates.
 */
void DrawAutomapTile(int sx, int sy, BYTE automap_type)
{
	switch (automap_type & MAT_TYPE) {
	case MAT_NONE:
		break;
	case MAT_EXTERN:
		DrawAutomapExtern(sx, sy);
		break;
	case MAT_STAIRS:
		DrawAutomapStairs(sx, sy);
		break;
	case MAT_DOOR_WEST:
		DrawAutomapDoorDiamond(0, sx, sy);
		break;
	case MAT_DOOR_EAST:
		DrawAutomapDoorDiamond(1, sx, sy);
		break;
	}

	unsigned d32 = AmLine32;
	unsigned d16 = (d32 >> 1);
	unsigned d8 = (d32 >> 2);
	if (automap_type & MAT_WALL_NW) {
		DrawLine(sx - d16, sy - d8, sx - 1, sy - d16 + 1, COLOR_DIM);
	}
	if (automap_type & MAT_WALL_NE) {
		DrawLine(sx, sy - d16 + 1, sx + d16 - 1, sy - d8, COLOR_DIM);
	}
	if (automap_type & MAT_WALL_SW) {
		DrawLine(sx - d16, sy - d8 + 1, sx - 1, sy, COLOR_DIM);
	}
	if (automap_type & MAT_WALL_SE) {
		DrawLine(sx, sy, sx + d16 - 1, sy - d8 + 1, COLOR_DIM);
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

	x = AutoMapXOfs + ViewX;
	y = AutoMapYOfs + ViewY;
	xoff = (ScrollInfo._sxoff * (int)AutoMapScale / 128 >> 1) + SCREEN_WIDTH / 2 + SCREEN_X - (x - y) * d16;
	yoff = (ScrollInfo._syoff * (int)AutoMapScale / 128 >> 1) + SCREEN_HEIGHT / 2 + SCREEN_Y - (x + y) * (d16 >> 1) - (d16 >> 1);

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
	px -= AutoMapXOfs + ViewX;
	py -= AutoMapYOfs + ViewY;

	//x = (p->_pxoff * (int)AutoMapScale / 128 >> 1) + (ScrollInfo._sxoff * (int)AutoMapScale / 128 >> 1) + (px - py) * d16 + SCREEN_WIDTH / 2 + SCREEN_X;
	//y = (p->_pyoff * (int)AutoMapScale / 128 >> 1) + (ScrollInfo._syoff * (int)AutoMapScale / 128 >> 1) + (px + py) * (d16 >> 1) + SCREEN_HEIGHT / 2 + SCREEN_Y;

	if (gbAutomapflag == AMM_NORMAL) {
		x = SCREEN_CENTERX(0);
		y = SCREEN_CENTERY(0);
	} else {
		x = SCREEN_X + SCREEN_WIDTH - MAP_MINI_WIDTH / 2;
		y = SCREEN_Y + MAP_MINI_HEIGHT / 2;
	}
	x += ((p->_pxoff + ScrollInfo._sxoff) * (int)AutoMapScale / 128 >> 1) + (px - py) * d16;
	y += ((p->_pyoff + ScrollInfo._syoff) * (int)AutoMapScale / 128 >> 1) + (px + py) * (d16 >> 1);

	//y -= (d16 >> 1);

	if (y < SCREEN_Y || y >= SCREEN_HEIGHT + SCREEN_Y || x < SCREEN_X || x >= SCREEN_WIDTH + SCREEN_X)
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
static BYTE GetAutomapType(int x, int y, bool view)
{
	if ((unsigned)x >= MAXDUNX || (unsigned)y >= MAXDUNY) {
		return MAT_NONE;
	}
	if (view && !(dFlags[x][y] & BFLAG_EXPLORED)) {
		return MAT_NONE;
	}

	return automaptype[dPiece[x][y]];
}

/**
 * @brief Renders game info, such as the name of the current level, and in multi player the name of the game and the game password.
 */
static void DrawAutomapText()
{
	if (!currLvl._dDynLvl)
		PrintGameStr(SCREEN_X + 8, SCREEN_Y + 20, AllLevels[currLvl._dLevelNum].dLevelName, COL_GOLD);
}

/**
 * @brief Renders the automap on screen.
 */
static void DrawAutomapContent()
{
	int sx, sy, mapx, mapy;
	int i, j, cells;
	unsigned d64 = AmLine64;

	//gpBufEnd = &gpBuffer[BUFFERXY(0, SCREEN_Y + SCREEN_HEIGHT)];

	// calculate the map center in the dungeon matrix
	mapx = ViewX & ~1;
	mapx += AutoMapXOfs;
	if (mapx < DBORDERX) {
		AutoMapXOfs -= mapx - DBORDERX;
		mapx = DBORDERX;
	} else if (mapx > DBORDERX + (DSIZEX - 2)) {
		AutoMapXOfs -= mapx - (DBORDERX + (DSIZEX - 2));
		mapx = DBORDERX + (DSIZEX - 2);
	}

	mapy = ViewY & ~1;
	mapy += AutoMapYOfs;
	if (mapy < DBORDERY) {
		AutoMapYOfs -= mapy - DBORDERY;
		mapy = DBORDERY;
	} else if (mapy > DBORDERY + (DSIZEY - 2)) {
		AutoMapYOfs -= mapy - (DBORDERY + (DSIZEY - 2));
		mapy = DBORDERY + (DSIZEY - 2);
	}

	// assert(d64 <= (MAP_SCALE_MAX * TILE_WIDTH) / 128);
	//static_assert(BORDER_LEFT >= (MAP_SCALE_MAX * TILE_WIDTH) / 128, "Make sure the automap-renderer does not have to check for clipping I."); - unnecessary, since the cells are limited to the screen
	//static_assert(BORDER_TOP >= (MAP_SCALE_MAX * TILE_WIDTH) / 128, "Make sure the automap-renderer does not have to check for clipping III.");
	//static_assert(BORDER_BOTTOM >= (MAP_SCALE_MAX * TILE_WIDTH) / 128 / 2, "Make sure the automap-renderer does not have to check for clipping IV.");

	// find an odd number of tiles which fits to the screen
	// assert(SCREEN_WIDTH >= d64);
	if (gbAutomapflag == AMM_NORMAL)
		cells = std::min(SCREEN_WIDTH, SCREEN_HEIGHT * 2);
	else
		cells = MAP_MINI_WIDTH;
	cells = 2 * ((cells - d64) / (2 * d64)) + 1;

	/*if ((SCREEN_WIDTH / 2) % d64)
		cells++;
	if ((SCREEN_WIDTH / 2) % d64 >= (AutoMapScale << 5) / 128)
		cells++;

	if (ScrollInfo._sxoff + ScrollInfo._syoff)
		cells++;*/

	// find the starting dungeon coordinates
	mapx -= (cells & ~1) * 2; // mapx - (xcells / 2 + ycells / 2);
	//mapy -= 1;          // mapy - (ycells / 2 - xcells / 2);

	// calculate the center of the tile on the screen
	if (gbAutomapflag == AMM_NORMAL) {
		sx = SCREEN_CENTERX(0);
		sy = SCREEN_CENTERY(0);
	} else {
		sx = SCREEN_X + SCREEN_WIDTH - MAP_MINI_WIDTH / 2;
		sy = SCREEN_Y + MAP_MINI_HEIGHT / 2;
	}
	sx -= d64 * (cells >> 1);
	sy -= (d64 >> 1) * (cells >> 1);
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

	// draw the subtiles
	cells *= 2;
	unsigned d32 = d64 >> 1;
	for (i = 0; i < 2 * cells; i++) { // foreach ycells
		int x = sx;

		for (j = 0; j < cells; j++) { // foreach xcells
			BYTE maptype = GetAutomapType(mapx, mapy, true);
			if (maptype != MAT_NONE)
				DrawAutomapTile(x, sy, maptype);
			SHIFT_GRID(mapx, mapy, 1, 0);
			x += d32;
		}
		// Return to start of row
		SHIFT_GRID(mapx, mapy, -cells, 0);

		// move to the next row
		if (i & 1) {
			mapy++;

			sx -= (d32 >> 1);
			sy += (d32 >> 2);
		} else {
			mapx++;

			sx += (d32 >> 1);
			sy += (d32 >> 2);
		}
	}

	for (int pnum = 0; pnum < MAX_PLRS; pnum++) {
		if (plr._pDunLevel == myplr._pDunLevel && plr._pActive && !plr._pLvlChanging) {
			if (plr._pTeam == myplr._pTeam)
				DrawAutomapPlr(pnum, pnum == mypnum ? COLOR_PLAYER : COLOR_FRIEND);
#if INET_MODE
			else
#else
			else if ((dFlags[plr._px][plr._py] & BFLAG_VISIBLE) || myplr._pTimer[PLTR_INFRAVISION] > 0/* || myplr._pInfraFlag*/)
#endif
				DrawAutomapPlr(pnum, COLOR_ENEMY);
		}
	}
	//if (AutoMapShowItems)
	//	SearchAutomapItem();
	//gpBufEnd = &gpBuffer[BUFFERXY(0, SCREEN_Y + SCREEN_HEIGHT)];
}

void DrawAutomap()
{
	if (AUTOMAP_VALID) {
		DrawAutomapContent();
	}

	DrawAutomapText();
}

/**
 * @brief Marks the given coordinate as within view on the automap.
 */
void SetAutomapView(int xx, int yy)
{
	// assert(IN_DUNGEON_AREA(xx, yy));
	dFlags[xx][yy] |= BFLAG_EXPLORED;

	BYTE maptype = automaptype[dungeon[xx][yy]]; // GetAutomapType(xx, yy, false);

	static_assert(DBORDERX != 0 && DBORDERY != 0, "SetAutomapView skips border checks.");
	BYTE mapftr = maptype & MAT_TYPE;
	if (maptype & MAT_WALL_NW) {
		if (mapftr == MAT_EXTERN) {
			if ((automaptype[dungeon[xx - 1][yy + 1]] & MAT_TYPE) == MAT_EXTERN)
				dFlags[xx][yy + 1] |= BFLAG_EXPLORED; // reveal corner-tile from NE to south
		} else if ((automaptype[dungeon[xx - 1][yy]] & MAT_TYPE) == MAT_EXTERN) {
			dFlags[xx - 1][yy] |= BFLAG_EXPLORED; // reveal extern-tile to NW
			if ((automaptype[dungeon[xx][yy + 1]] & MAT_TYPE) == MAT_EXTERN) {
				dFlags[xx - 1][yy + 1] |= BFLAG_EXPLORED; // reveal corner-tile to W
			}
			if ((maptype & MAT_WALL_NE) && (automaptype[dungeon[xx][yy - 1]] & MAT_TYPE) == MAT_EXTERN) {
				dFlags[xx - 1][yy - 1] |= BFLAG_EXPLORED; // reveal extern-tile to N
			}
		}
	}
	if (maptype & MAT_WALL_NE) {
		if (mapftr == MAT_EXTERN) {
			// if ((automaptype[dungeon[xx + 1][yy]] & MAT_TYPE) == MAT_EXTERN) - should be covered from the other direction (NE)
			//	dFlags[xx + 1][yy] |= BFLAG_EXPLORED; // reveal corner-tile from NW to south
		} else if ((automaptype[dungeon[xx][yy - 1]] & MAT_TYPE) == MAT_EXTERN) {
			dFlags[xx][yy - 1] |= BFLAG_EXPLORED; // reveal extern-tile to NE
			if ((automaptype[dungeon[xx + 1][yy]] & MAT_TYPE) == MAT_EXTERN) {
				dFlags[xx + 1][yy - 1] |= BFLAG_EXPLORED; // reveal corner-tile to E
			}
		}
	}
	if (maptype & MAT_WALL_SW) {
		// assert(mapftr != MAT_EXTERN);
		if ((automaptype[dungeon[xx][yy + 1]] & MAT_TYPE) == MAT_EXTERN) {
			dFlags[xx][yy + 1] |= BFLAG_EXPLORED; // reveal extern-tile to SW
			if ((maptype & MAT_WALL_SE) && (automaptype[dungeon[xx + 1][yy]] & MAT_TYPE) == MAT_EXTERN) {
				dFlags[xx + 1][yy + 1] |= BFLAG_EXPLORED; // reveal corner-tile to S
			}
		}
	}
	if (maptype & MAT_WALL_SE) {
		// assert(mapftr != MAT_EXTERN);
		if ((automaptype[dungeon[xx + 1][yy]] & MAT_TYPE) == MAT_EXTERN) {
			dFlags[xx + 1][yy] |= BFLAG_EXPLORED; // reveal extern-tile to SE
		}
	}
}

DEVILUTION_END_NAMESPACE
