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

#define MAP_SHIFT 7
#define MAP_GRID_WIDTH (1 << MAP_SHIFT)

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
/* Specifies the tile-width at the given map-scale. (AutoMapScale * TILE_WIDTH) / MAP_GRID_WIDTH */
#if TILE_WIDTH <= MAP_GRID_WIDTH
static_assert(MAP_GRID_WIDTH % TILE_WIDTH == 0, "The calculation of MAP_TILE_WIDTH is incorrect.");
#define MAP_TILE_WIDTH (AutoMapScale / (MAP_GRID_WIDTH / TILE_WIDTH))
#else
static_assert(TILE_WIDTH % MAP_GRID_WIDTH == 0, "The calculation of MAP_TILE_WIDTH is incorrect.");
#define MAP_TILE_WIDTH (AutoMapScale * (TILE_WIDTH / MAP_GRID_WIDTH))
#endif

/** color used to draw the player's arrow */
#define COLOR_PLAYER (PAL8_ORANGE + 1)
#define COLOR_FRIEND (PAL8_BLUE + 0)
#define COLOR_ENEMY  (PAL8_RED + 2)
#define COLOR_DEAD   (PAL16_GRAY + 0)
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
static void AutomapUp()
{
	SHIFT_GRID(AutoMapXOfs, AutoMapYOfs, 0, -2);
}

/**
 * @brief Scrolls the automap downwards.
 */
static void AutomapDown()
{
	SHIFT_GRID(AutoMapXOfs, AutoMapYOfs, 0, 2);
}

/**
 * @brief Scrolls the automap leftwards.
 */
static void AutomapLeft()
{
	SHIFT_GRID(AutoMapXOfs, AutoMapYOfs, -2, 0);
}

/**
 * @brief Scrolls the automap rightwards.
 */
static void AutomapRight()
{
	SHIFT_GRID(AutoMapXOfs, AutoMapYOfs, 2, 0);
}

void AutomapMove(int dir)
{
	switch (dir) {
	case MDIR_UP:    AutomapUp();    break;
	case MDIR_DOWN:  AutomapDown();  break;
	case MDIR_LEFT:  AutomapLeft();  break;
	case MDIR_RIGHT: AutomapRight(); break;
	default: ASSUME_UNREACHABLE;     break;
	}
}

/**
 * @brief Increases the zoom level of the automap.
 */
void AutomapZoomIn()
{
	if (gbAutomapflag != AMM_NONE && AutoMapScale < MAP_SCALE_MAX) {
		AutoMapScale += 16;
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
	if (gbAutomapflag != AMM_NONE && AutoMapScale > MAP_SCALE_MIN) {
		AutoMapScale -= 16;
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
	unsigned d32 = MAP_TILE_WIDTH >> 1;
	unsigned d8 = (d32 >> 2);

	DrawPixel(sx, sy - d8, COLOR_DIM);
}

static void DrawAutomapStairs(int sx, int sy)
{
	unsigned d32 = MAP_TILE_WIDTH >> 1;
	unsigned d16 = (d32 >> 1), d8 = (d32 >> 2), d4 = (d32 >> 3);

	DrawLine(sx - d16 + d8, sy - d16 + d4, sx + d8, sy - d16 + d4 + d8, COLOR_BRIGHT);
	DrawLine(sx - d16, sy - d8, sx, sy, COLOR_BRIGHT);
}

static void DrawAutomapDoorDiamond(int dir, int sx, int sy)
{
	unsigned d32 = MAP_TILE_WIDTH >> 1;
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

	unsigned d32 = MAP_TILE_WIDTH >> 1;
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

/*static void DrawAutomapItem(int sx, int sy)
{
	unsigned d32 = MAP_TILE_WIDTH / 2;
	unsigned d16 = (d32 >> 1), d8 = (d32 >> 2);
	DrawLine(sx - d16, sy - d16, sx - 1, sy - d16 + 1 - d8, COLOR_ITEM); // top left
	DrawLine(sx, sy - d16 + 1 - d8, sx + d16 - 1, sy - d16, COLOR_ITEM); // top right
	DrawLine(sx - d16, sy - d16 + 1, sx - 1, sy - d8, COLOR_ITEM);       // bottom left
	DrawLine(sx, sy - d8, sx + d16 - 1, sy - d16 + 1, COLOR_ITEM);       // bottom right
}*/
/**
 * @brief Renders an arrow on the automap, centered on and facing the direction of the player.
 */
static void DrawAutomapPlr(int pnum, int sx, int sy)
{
	PlayerStruct* p;
	int x, y;
	unsigned d16 = MAP_TILE_WIDTH / 4;

	p = &plr;

	x = sx;
	y = sy;

	POS32 sp = DunScreenOffset(p->_ppos);

	x += sp.x * (int)AutoMapScale >> (MAP_SHIFT + 1);
	y += sp.y * (int)AutoMapScale >> (MAP_SHIFT + 1);

	y -= d16;

	static_assert(BORDER_LEFT >= (MAP_SCALE_MAX * TILE_WIDTH) / MAP_GRID_WIDTH / 4, "Make sure the automap-renderer does not have to check for clipping V.");
	static_assert(BORDER_TOP >= (MAP_SCALE_MAX * TILE_WIDTH) / MAP_GRID_WIDTH / 4, "Make sure the automap-renderer does not have to check for clipping VII.");
	static_assert(BORDER_BOTTOM >= (MAP_SCALE_MAX * TILE_WIDTH) / MAP_GRID_WIDTH / 4, "Make sure the automap-renderer does not have to check for clipping VIII.");

	unsigned d8 = (MAP_TILE_WIDTH / 8), d4 = (MAP_TILE_WIDTH / 16), d2 = (MAP_TILE_WIDTH / 32), d3p32 = 3 * MAP_TILE_WIDTH / 32, d3p64 = 3 * MAP_TILE_WIDTH / 64;
	if (p->_pHitPoints != 0) {
		BYTE col = pnum == mypnum ? COLOR_PLAYER : (plr._pTeam == myplr._pTeam ? COLOR_FRIEND : COLOR_ENEMY);

		switch (p->_pdir) {
		case DIR_N: {
			unsigned dy = d3p32;
			DrawLine(x, y - dy, x, y + dy, col);
			DrawLine(x, y - dy, x - d4, y, col);
			DrawLine(x, y - dy, x + d4, y, col);
		} break;
		case DIR_NE: {
			unsigned dx = d3p32, dy = d3p64;
			DrawLine(x - dx, y + dy, x + dx, y - dy, col);
			DrawLine(x - d2, y - dy, x + dx, y - dy, col);
			DrawLine(x + dx, y - dy, x + d4, y + dy, col);
		} break;
		case DIR_E: {
			DrawLine(x - d8, y, x + d8, y, col);
			DrawLine(x, y - d4, x + d8, y, col);
			DrawLine(x, y + d4, x + d8, y, col);
		} break;
		case DIR_SE: {
			unsigned dx = d3p32, dy = d3p64;
			DrawLine(x - dx, y - dy, x + dx, y + dy, col);
			DrawLine(x - d2, y + dy, x + dx, y + dy, col);
			DrawLine(x + dx, y + dy, x + d4, y - dy, col);
		} break;
		case DIR_S: {
			unsigned dy = d3p32;
			DrawLine(x, y - dy, x, y + dy, col);
			DrawLine(x, y + dy, x + d4, y, col);
			DrawLine(x - d4, y, x, y + dy, col);
		} break;
		case DIR_SW: {
			unsigned dx = d3p32, dy = d3p64;
			DrawLine(x - dx, y + dy, x + dx, y - dy, col);
			DrawLine(x - dx, y + dy, x + d2, y + dy, col);
			DrawLine(x - dx, y + dy, x - d4, y - dy, col);
		} break;
		case DIR_W: {
			DrawLine(x - d8, y, x + d8, y, col);
			DrawLine(x - d8, y, x, y - d4, col);
			DrawLine(x - d8, y, x, y + d4, col);
		} break;
		case DIR_NW: {
			unsigned dx = d3p32, dy = d3p64;
			DrawLine(x - dx, y - dy, x + dx, y + dy, col);
			DrawLine(x - dx, y - dy, x + d2, y - d4, col);
			DrawLine(x - dx, y - dy, x - d4, y + d4, col);
		} break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
	} else {
		DrawLine(x - d8, y - d4, x + d8, y + d4, COLOR_DEAD);
		DrawLine(x - d8, y + d4, x + d8, y - d4, COLOR_DEAD);
	}
}

/**
 * @brief Returns the automap shape at the given coordinate.
 */
static BYTE GetAutomapType(int x, int y)
{
	// assert(IN_DUNGEON_AREA(x, y));
	if (!(dFlags[x][y] & BFLAG_EXPLORED)) {
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
	int pnum, i, j, cells;
	unsigned d64 = MAP_TILE_WIDTH;
	const POS32 vp = myview.dun;
	const POS32 mp = { (int)((unsigned)vp.x / DUN_WIDTH), (int)((unsigned)vp.y / DUN_WIDTH) };

	//gpBufEnd = &gpBuffer[BUFFERXY(0, SCREEN_Y + SCREEN_HEIGHT)];

	// calculate the map center in the dungeon matrix
	mapx = mp.x & ~1;
	mapx += AutoMapXOfs;
	if (mapx < DBORDERX) {
		AutoMapXOfs -= mapx - DBORDERX;
		mapx = DBORDERX;
	} else if (mapx > DBORDERX + (DSIZEX - 2)) {
		AutoMapXOfs -= mapx - (DBORDERX + (DSIZEX - 2));
		mapx = DBORDERX + (DSIZEX - 2);
	}

	mapy = mp.y & ~1;
	mapy += AutoMapYOfs;
	if (mapy < DBORDERY) {
		AutoMapYOfs -= mapy - DBORDERY;
		mapy = DBORDERY;
	} else if (mapy > DBORDERY + (DSIZEY - 2)) {
		AutoMapYOfs -= mapy - (DBORDERY + (DSIZEY - 2));
		mapy = DBORDERY + (DSIZEY - 2);
	}

	// assert(d64 <= (MAP_SCALE_MAX * TILE_WIDTH) / MAP_GRID_WIDTH);
	//static_assert(BORDER_LEFT >= (MAP_SCALE_MAX * TILE_WIDTH) / MAP_GRID_WIDTH, "Make sure the automap-renderer does not have to check for clipping I."); - unnecessary, since the cells are limited to the screen
	//static_assert(BORDER_TOP >= (MAP_SCALE_MAX * TILE_WIDTH) / MAP_GRID_WIDTH, "Make sure the automap-renderer does not have to check for clipping III.");
	//static_assert(BORDER_BOTTOM >= (MAP_SCALE_MAX * TILE_WIDTH) / MAP_GRID_WIDTH / 2, "Make sure the automap-renderer does not have to check for clipping IV.");

	// find an odd number of tiles which fits to the screen
	// assert(SCREEN_WIDTH >= d64);
	if (gbAutomapflag == AMM_NORMAL)
		cells = std::min(SCREEN_WIDTH, SCREEN_HEIGHT * 2);
	else
		cells = MAP_MINI_WIDTH;
	cells = 2 * ((cells - d64) / (2 * d64)) + 1;

	/*if ((SCREEN_WIDTH / 2) % d64)
		cells++;
	if ((SCREEN_WIDTH / 2) % d64 >= (AutoMapScale << 5) / MAP_GRID_WIDTH)
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
	if (mp.x & 1) {
		sx -= (d64 >> 2);
		sy -= (d64 >> 3);
	}
	if (mp.y & 1) {
		sx += (d64 >> 2);
		sy -= (d64 >> 3);
	}
	const POS32 sp = DunScreenOffset(vp);
	sx -= (sp.x * (int)AutoMapScale) >> (MAP_SHIFT + 1);
	sy -= (sp.y * (int)AutoMapScale) >> (MAP_SHIFT + 1);

	// select the bottom edge of the tile
	sy += (d64 >> 2);

	// - initialize an array for the players on the current level
	std::pair<POS32, POS32> aplrs[MAX_PLRS];
	for (pnum = 0; pnum < MAX_PLRS; pnum++) {
		POS32 ppos = { -1, -1 };
		if (plr._pActive && !plr._pLvlChanging && plr._pDunLevel == currLvl._dLevelIdx) {
#if !INET_MODE
			if (plr._pTeam == myplr._pTeam || (dFlags[mapx][mapy] & BFLAG_VISIBLE) || myplr._pTimer[PLTR_INFRAVISION] > 0/*|| myplr._pInfraFlag*/ || plr._pHitPoints == 0)
#endif
				ppos = { (int)((unsigned)plr._ppos.x / DUN_WIDTH), (int)((unsigned)plr._ppos.y / DUN_WIDTH) };
		}
		aplrs[pnum] = std::pair<POS32, POS32>(ppos, { 0, 0 });;
	}
	// - initialize an array for the items
	// POS32 aitempos[MAXITEMS];
	// int numaitems = 0;
	// draw the subtiles
	cells *= 2;
	unsigned d32 = d64 >> 1;
	for (i = 0; i < 2 * cells; i++) { // foreach ycells
		int x = sx;

		for (j = 0; j < cells; j++) { // foreach xcells
			if (IN_DUNGEON_AREA(mapx, mapy)) {
				BYTE maptype = GetAutomapType(mapx, mapy);
				if (maptype != MAT_NONE)
					DrawAutomapTile(x, sy, maptype);
				for (pnum = 0; pnum < MAX_PLRS; pnum++) {
					if (aplrs[pnum].first.x == mapx && aplrs[pnum].first.y == mapy) {
						aplrs[pnum].second.x = x;
						aplrs[pnum].second.y = sy;
					}
				}
				// for (int idx = 0; idx < numitems; idx++) {
				// 	int ii = itemactive[idx];
				// 	const ItemStruct* is = &items[ii];
				// 	if ((unsigned)is->_ipos.x / DUN_WIDTH == mapx && (unsigned)is->_ipos.y / DUN_WIDTH == mapy) {
				// 		aitempos[numaitems] = { sx, sy };
				// 		numaitems++;
				// 	}
				// }
			}
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
	// draw the items
	// if (AutoMapShowItems) {
	//	for (i = 0; i < numaitems; i++) {
	//		DrawAutomapItem(aitempos[i].x, aitempos[i].y);
	//	}
	// }
	// draw the players in a separate loop to have a consistent draw-order
	for (pnum = 0; pnum < MAX_PLRS; pnum++) {
		// if (aplrs[pnum].first.x < 0) continue;
		if (aplrs[pnum].second.x == 0) continue;
		DrawAutomapPlr(pnum, aplrs[pnum].second.x, aplrs[pnum].second.y);
	}
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

	BYTE maptype = automaptype[dungeon[xx][yy]]; // GetAutomapType(xx, yy);

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
