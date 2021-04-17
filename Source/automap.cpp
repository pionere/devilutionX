/**
 * @file automap.cpp
 *
 * Implementation of the in-game map overlay.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/**
 * Maps from tile_id to automap type.
 */
WORD automaptype[256];
static int AutoMapX;
static int AutoMapY;
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
#define COLOR_ENEMY (PAL8_RED + 2)
/** color for bright map lines (doors, stairs etc.) */
#define COLOR_BRIGHT PAL8_YELLOW
/** color for dim map lines/dots */
#define COLOR_DIM (PAL16_YELLOW + 8)
// color for items on automap
#define COLOR_ITEM (PAL8_BLUE + 1)

#define MAPFLAG_TYPE 0x000F
#define MAPFLAG_VERTDOOR 0x0100
#define MAPFLAG_HORZDOOR 0x0200
#define MAPFLAG_VERTARCH 0x0400
#define MAPFLAG_HORZARCH 0x0800
#define MAPFLAG_VERTGRATE 0x1000
#define MAPFLAG_HORZGRATE 0x2000
#define MAPFLAG_DIRT 0x4000
#define MAPFLAG_STAIRS 0x8000

/**
 * @brief Initializes the automap.
 */
void InitAutomapOnce()
{
	gbAutomapflag = false;
	AutoMapScale = 64;
	AmLine64 = 32;
	AmLine32 = 16;
	AmLine16 = 8;
	AmLine8 = 4;
	AmLine4 = 2;
}

/**
 * @brief Loads the mapping between tile IDs and automap shapes.
 */
void InitAutomap()
{
	DWORD dwTiles;
	BYTE *pAFile, *pTmp;
	WORD *lm;
	DWORD i;
	const char* mapData;

	mapData = AllLevels[currLvl._dLevelIdx].dAutomapData;
	_gbAutomapData = mapData != NULL;
	if (!_gbAutomapData)
		return;

	memset(automaptype, 0, sizeof(automaptype));

	pAFile = LoadFileInMem(mapData, &dwTiles);

	dwTiles /= 2;
	assert(dwTiles < (DWORD)lengthof(automaptype));

	lm = (WORD*)pAFile;
	for (i = 1; i <= dwTiles; i++) {
		automaptype[i] = SwapLE16(*lm);
		lm++;
	}

	mem_free_dbg(pAFile);
	memset(automapview, 0, sizeof(automapview));

	static_assert(sizeof(dFlags) == MAXDUNX * MAXDUNY, "Linear traverse of dFlags does not work in InitAutomap.");
	pTmp = &dFlags[0][0];
	for (i = 0; i < MAXDUNX * MAXDUNY; i++, pTmp++)
		*pTmp &= ~BFLAG_EXPLORED;
}

/**
 * @brief Display/Hides the automap.
 */
void ToggleAutomap()
{
	if (!gbAutomapflag) {
		AutoMapXOfs = 0;
		AutoMapYOfs = 0;
		gbAutomapflag = true;
	} else {
		gbAutomapflag = false;
	}
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
	if (AutoMapScale < 256) {
		AutoMapScale += 12;
		AmLine64 = (AutoMapScale << 6) / 128;
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
	if (AutoMapScale > 64) {
		AutoMapScale -= 12;
		AmLine64 = (AutoMapScale << 6) / 128;
		AmLine32 = AmLine64 >> 1;
		AmLine16 = AmLine32 >> 1;
		AmLine8 = AmLine16 >> 1;
		AmLine4 = AmLine8 >> 1;
	}
}

/**
 * @brief Renders the given automap shape at the specified screen coordinates.
 */
static void DrawAutomapTile(int sx, int sy, WORD automap_type)
{
	bool do_vert, do_horz, do_cave_horz, do_cave_vert;
	int x1, y1, x2, y2;

	if (automap_type & MAPFLAG_DIRT) {
		ENG_set_pixel(sx, sy, COLOR_DIM);
		ENG_set_pixel(sx - AmLine8, sy - AmLine4, COLOR_DIM);
		ENG_set_pixel(sx - AmLine8, sy + AmLine4, COLOR_DIM);
		ENG_set_pixel(sx + AmLine8, sy - AmLine4, COLOR_DIM);
		ENG_set_pixel(sx + AmLine8, sy + AmLine4, COLOR_DIM);
		ENG_set_pixel(sx - AmLine16, sy, COLOR_DIM);
		ENG_set_pixel(sx + AmLine16, sy, COLOR_DIM);
		ENG_set_pixel(sx, sy - AmLine8, COLOR_DIM);
		ENG_set_pixel(sx, sy + AmLine8, COLOR_DIM);
		ENG_set_pixel(sx + AmLine8 - AmLine32, sy + AmLine4, COLOR_DIM);
		ENG_set_pixel(sx - AmLine8 + AmLine32, sy + AmLine4, COLOR_DIM);
		ENG_set_pixel(sx - AmLine16, sy + AmLine8, COLOR_DIM);
		ENG_set_pixel(sx + AmLine16, sy + AmLine8, COLOR_DIM);
		ENG_set_pixel(sx - AmLine8, sy + AmLine16 - AmLine4, COLOR_DIM);
		ENG_set_pixel(sx + AmLine8, sy + AmLine16 - AmLine4, COLOR_DIM);
		ENG_set_pixel(sx, sy + AmLine16, COLOR_DIM);
	}

	if (automap_type & MAPFLAG_STAIRS) {
		DrawLine(sx - AmLine8, sy - AmLine8 - AmLine4, sx + AmLine8 + AmLine16, sy + AmLine4, COLOR_BRIGHT);
		DrawLine(sx - AmLine16, sy - AmLine8, sx + AmLine16, sy + AmLine8, COLOR_BRIGHT);
		DrawLine(sx - AmLine16 - AmLine8, sy - AmLine4, sx + AmLine8, sy + AmLine8 + AmLine4, COLOR_BRIGHT);
		DrawLine(sx - AmLine32, sy, sx, sy + AmLine16, COLOR_BRIGHT);
	}

	do_vert = false;
	do_horz = false;
	do_cave_horz = false;
	do_cave_vert = false;
	switch (automap_type & MAPFLAG_TYPE) {
	case 1: // stand-alone column or other unpassable object
		x1 = sx - AmLine16;
		y1 = sy - AmLine16;
		x2 = x1 + AmLine32;
		y2 = sy - AmLine8;
		DrawLine(sx, y1, x1, y2, COLOR_DIM);
		DrawLine(sx, y1, x2, y2, COLOR_DIM);
		DrawLine(sx, sy, x1, y2, COLOR_DIM);
		DrawLine(sx, sy, x2, y2, COLOR_DIM);
		break;
	case 2:
	case 5:
		do_vert = true;
		break;
	case 3:
	case 6:
		do_horz = true;
		break;
	case 4:
		do_vert = true;
		do_horz = true;
		break;
	case 8:
		do_vert = true;
		do_cave_horz = true;
		break;
	case 9:
		do_horz = true;
		do_cave_vert = true;
		break;
	case 10:
		do_cave_horz = true;
		break;
	case 11:
		do_cave_vert = true;
		break;
	case 12:
		do_cave_horz = true;
		do_cave_vert = true;
		break;
	}

	if (do_vert) {                      // right-facing obstacle
		if (automap_type & MAPFLAG_VERTDOOR) { // two wall segments with a door in the middle
			x1 = sx - AmLine32;
			x2 = sx - AmLine16;
			y1 = sy - AmLine16;
			y2 = sy - AmLine8;

			DrawLine(sx, y1, sx - AmLine8, y1 + AmLine4, COLOR_DIM);
			DrawLine(x1, sy, x1 + AmLine8, sy - AmLine4, COLOR_DIM);
			DrawLine(x2, y1, x1, y2, COLOR_BRIGHT);
			DrawLine(x2, y1, sx, y2, COLOR_BRIGHT);
			DrawLine(x2, sy, x1, y2, COLOR_BRIGHT);
			DrawLine(x2, sy, sx, y2, COLOR_BRIGHT);
		}
		if (automap_type & MAPFLAG_VERTGRATE) { // right-facing half-wall
			DrawLine(sx - AmLine16, sy - AmLine8, sx - AmLine32, sy, COLOR_DIM);
		}
		if (automap_type & (MAPFLAG_VERTGRATE | MAPFLAG_VERTARCH)) { // window or passable column
			x1 = sx - AmLine16;
			y1 = sy - AmLine16;
			x2 = x1 + AmLine32;
			y2 = sy - AmLine8;

			DrawLine(sx, y1, x1, y2, COLOR_DIM);
			DrawLine(sx, y1, x2, y2, COLOR_DIM);
			DrawLine(sx, sy, x1, y2, COLOR_DIM);
			DrawLine(sx, sy, x2, y2, COLOR_DIM);
		}
		if ((automap_type & (MAPFLAG_VERTDOOR | MAPFLAG_VERTGRATE | MAPFLAG_VERTARCH)) == 0)
			DrawLine(sx, sy - AmLine16, sx - AmLine32, sy, COLOR_DIM);
	}

	if (do_horz) { // left-facing obstacle
		if (automap_type & MAPFLAG_HORZDOOR) {
			x1 = sx + AmLine16;
			x2 = sx + AmLine32;
			y1 = sy - AmLine16;
			y2 = sy - AmLine8;

			DrawLine(sx, y1, sx + AmLine8, y1 + AmLine4, COLOR_DIM);
			DrawLine(x2, sy, x2 - AmLine8, sy - AmLine4, COLOR_DIM);
			DrawLine(x1, y1, sx, y2, COLOR_BRIGHT);
			DrawLine(x1, y1, x2, y2, COLOR_BRIGHT);
			DrawLine(x1, sy, sx, y2, COLOR_BRIGHT);
			DrawLine(x1, sy, x2, y2, COLOR_BRIGHT);
		}
		if (automap_type & MAPFLAG_HORZGRATE) {
			DrawLine(sx + AmLine16, sy - AmLine8, sx + AmLine32, sy, COLOR_DIM);
		}
		if (automap_type & (MAPFLAG_HORZGRATE | MAPFLAG_HORZARCH)) {
			x1 = sx - AmLine16;
			y1 = sy - AmLine16;
			x2 = x1 + AmLine32;
			y2 = sy - AmLine8;

			DrawLine(sx, y1, x1, y2, COLOR_DIM);
			DrawLine(sx, y1, x2, y2, COLOR_DIM);
			DrawLine(sx, sy, x1, y2, COLOR_DIM);
			DrawLine(sx, sy, x2, y2, COLOR_DIM);
		}
		if ((automap_type & (MAPFLAG_HORZDOOR | MAPFLAG_HORZGRATE | MAPFLAG_HORZARCH)) == 0)
			DrawLine(sx, sy - AmLine16, sx + AmLine32, sy, COLOR_DIM);
	}

	// for caves the horz/vert flags are switched
	if (do_cave_horz) {
		if (automap_type & MAPFLAG_VERTDOOR) {
			x1 = sx - AmLine32;
			x2 = sx - AmLine16;
			y1 = sy + AmLine16;
			y2 = sy + AmLine8;

			DrawLine(sx, y1, sx - AmLine8, y1 - AmLine4, COLOR_DIM);
			DrawLine(x1, sy, x1 + AmLine8, sy + AmLine4, COLOR_DIM);
			DrawLine(x2, y1, x1, y2, COLOR_BRIGHT);
			DrawLine(x2, y1, sx, y2, COLOR_BRIGHT);
			DrawLine(x2, sy, x1, y2, COLOR_BRIGHT);
			DrawLine(x2, sy, sx, y2, COLOR_BRIGHT);
		} else
			DrawLine(sx, sy + AmLine16, sx - AmLine32, sy, COLOR_DIM);
	}

	if (do_cave_vert) {
		if (automap_type & MAPFLAG_HORZDOOR) {
			x1 = sx + AmLine16;
			x2 = sx + AmLine32;
			y1 = sy + AmLine16;
			y2 = sy + AmLine8;

			DrawLine(sx, y1, sx + AmLine8, y1 - AmLine4, COLOR_DIM);
			DrawLine(x2, sy, x2 - AmLine8, sy + AmLine4, COLOR_DIM);
			DrawLine(x1, y1, sx, y2, COLOR_BRIGHT);
			DrawLine(x1, y1, x2, y2, COLOR_BRIGHT);
			DrawLine(x1, sy, sx, y2, COLOR_BRIGHT);
			DrawLine(x1, sy, x2, y2, COLOR_BRIGHT);
		} else
			DrawLine(sx, sy + AmLine16, sx + AmLine32, sy, COLOR_DIM);
	}
}

static void DrawAutomapItem(int x, int y, BYTE color)
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
	PlayerStruct *p;
	int x, y;
	int x1, y1, x2, y2, xoff, yoff;
	int i, j;

	x = 2 * AutoMapXOfs + ViewX;
	y = 2 * AutoMapYOfs + ViewY;
	xoff = (ScrollInfo._sxoff * (int)AutoMapScale / 128 >> 1) + SCREEN_WIDTH / 2 + SCREEN_X - (x - y) * AmLine16;
	yoff = (ScrollInfo._syoff * (int)AutoMapScale / 128 >> 1) + VIEWPORT_HEIGHT / 2 + SCREEN_Y - (x + y) * AmLine8 - AmLine8;

	p = &plr[myplr];
	if (p->_pmode == PM_WALK3) {
		x = p->_pfutx;
		y = p->_pfuty;
		if (p->_pdir == DIR_W)
			x++;
		else
			y++;
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
	if (x2 > MAXDUNX)
		x2 = MAXDUNX;

	y2 = y + 8;
	if (y2 > MAXDUNY)
		y2 = MAXDUNY;

	for (i = x1; i < x2; i++) {
		for (j = y1; j < y2; j++) {
			if (dItem[i][j] != 0) {
				x = xoff + (i - j) * AmLine16;
				y = yoff + (i + j) * AmLine8;
				DrawAutomapItem(x, y, COLOR_ITEM);
			}
		}
	}
}

/**
 * @brief Renders an arrow on the automap, centered on and facing the direction of the player.
 */
static void DrawAutomapPlr(int pnum, int playerColor)
{
	PlayerStruct *p;
	int px, py;
	int x, y;

	p = &plr[pnum];
	if (p->_pmode == PM_WALK3) {
		px = p->_pfutx;
		py = p->_pfuty;
		if (p->_pdir == DIR_W)
			px++;
		else
			py++;
	} else {
		px = p->_px;
		py = p->_py;
	}
	px -= 2 * AutoMapXOfs + ViewX;
	py -= 2 * AutoMapYOfs + ViewY;

	x = (p->_pxoff * (int)AutoMapScale / 128 >> 1) + (ScrollInfo._sxoff * (int)AutoMapScale / 128 >> 1) + (px - py) * AmLine16 + SCREEN_WIDTH / 2 + SCREEN_X;
	y = (p->_pyoff * (int)AutoMapScale / 128 >> 1) + (ScrollInfo._syoff * (int)AutoMapScale / 128 >> 1) + (px + py) * AmLine8 + VIEWPORT_HEIGHT / 2 + SCREEN_Y;

	y -= AmLine8;

	switch (p->_pdir) {
	case DIR_N:
		DrawLine(x, y, x, y - AmLine16, playerColor);
		DrawLine(x, y - AmLine16, x - AmLine4, y - AmLine8, playerColor);
		DrawLine(x, y - AmLine16, x + AmLine4, y - AmLine8, playerColor);
		break;
	case DIR_NE:
		DrawLine(x, y, x + AmLine16, y - AmLine8, playerColor);
		DrawLine(x + AmLine16, y - AmLine8, x + AmLine8, y - AmLine8, playerColor);
		DrawLine(x + AmLine16, y - AmLine8, x + AmLine8 + AmLine4, y, playerColor);
		break;
	case DIR_E:
		DrawLine(x, y, x + AmLine16, y, playerColor);
		DrawLine(x + AmLine16, y, x + AmLine8, y - AmLine4, playerColor);
		DrawLine(x + AmLine16, y, x + AmLine8, y + AmLine4, playerColor);
		break;
	case DIR_SE:
		DrawLine(x, y, x + AmLine16, y + AmLine8, playerColor);
		DrawLine(x + AmLine16, y + AmLine8, x + AmLine8 + AmLine4, y, playerColor);
		DrawLine(x + AmLine16, y + AmLine8, x + AmLine8, y + AmLine8, playerColor);
		break;
	case DIR_S:
		DrawLine(x, y, x, y + AmLine16, playerColor);
		DrawLine(x, y + AmLine16, x + AmLine4, y + AmLine8, playerColor);
		DrawLine(x, y + AmLine16, x - AmLine4, y + AmLine8, playerColor);
		break;
	case DIR_SW:
		DrawLine(x, y, x - AmLine16, y + AmLine8, playerColor);
		DrawLine(x - AmLine16, y + AmLine8, x - AmLine4 - AmLine8, y, playerColor);
		DrawLine(x - AmLine16, y + AmLine8, x - AmLine8, y + AmLine8, playerColor);
		break;
	case DIR_W:
		DrawLine(x, y, x - AmLine16, y, playerColor);
		DrawLine(x - AmLine16, y, x - AmLine8, y - AmLine4, playerColor);
		DrawLine(x - AmLine16, y, x - AmLine8, y + AmLine4, playerColor);
		break;
	case DIR_NW:
		DrawLine(x, y, x - AmLine16, y - AmLine8, playerColor);
		DrawLine(x - AmLine16, y - AmLine8, x - AmLine8, y - AmLine8, playerColor);
		DrawLine(x - AmLine16, y - AmLine8, x - AmLine4 - AmLine8, y, playerColor);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

/**
 * @brief Returns the automap shape at the given coordinate.
 */
static WORD GetAutomapType(int x, int y, bool view)
{
	WORD rv;

	if (view && x == -1 && y >= 0 && y < DMAXY && automapview[0][y]) {
		if (GetAutomapType(0, y, false) & MAPFLAG_DIRT) {
			return 0;
		} else {
			return MAPFLAG_DIRT;
		}
	}

	if (view && y == -1 && x >= 0 && x < DMAXY && automapview[x][0]) {
		if (GetAutomapType(x, 0, false) & MAPFLAG_DIRT) {
			return 0;
		} else {
			return MAPFLAG_DIRT;
		}
	}

	if (x < 0 || x >= DMAXX) {
		return 0;
	}
	if (y < 0 || y >= DMAXX) {
		return 0;
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
	char desc[256];
	int nextline = 20;

	if (gbMaxPlayers != 1) {
		PrintGameStr(8, nextline, szGameName, COL_GOLD);
		nextline += 15;
		if (szGamePassword[0] != '\0') {
			snprintf(desc, sizeof(desc), "password: %s", szGamePassword);
			PrintGameStr(8, nextline, desc, COL_GOLD);
			nextline += 15;
		}
	}

	PrintGameStr(8, nextline, AllLevels[currLvl._dLevelIdx].dLevelName, COL_GOLD);
}

/**
 * @brief Renders the automap on screen.
 */
void DrawAutomap()
{
	int cells;
	int sx, sy;
	int i, j, mapx, mapy;
	unsigned d;

	if (!_gbAutomapData) {
		DrawAutomapText();
		return;
	}

	gpBufEnd = &gpBuffer[BUFFER_WIDTH * (SCREEN_Y + VIEWPORT_HEIGHT)];

	AutoMapX = (ViewX - DBORDERX) >> 1;
	AutoMapX += AutoMapXOfs;
	if (AutoMapX < 0) {
		AutoMapXOfs -= AutoMapX;
		AutoMapX = 0;
	} else if (AutoMapX > (DMAXX - 1)) {
		AutoMapXOfs -= AutoMapX - (DMAXX - 1);
		AutoMapX = DMAXX - 1;
	}

	AutoMapY = (ViewY - DBORDERY) >> 1;
	AutoMapY += AutoMapYOfs;
	if (AutoMapY < 0) {
		AutoMapYOfs -= AutoMapY;
		AutoMapY = 0;
	} else if (AutoMapY > (DMAXY - 1)) {
		AutoMapYOfs -= AutoMapY - (DMAXY - 1);
		AutoMapY = DMAXY - 1;
	}

	d = (AutoMapScale << 6) / 128;
	cells = 2 * (SCREEN_WIDTH / 2 / d) + 1;
	if ((SCREEN_WIDTH / 2) % d)
		cells++;
	if ((SCREEN_WIDTH / 2) % d >= (AutoMapScale << 5) / 128)
		cells++;

	if (ScrollInfo._sxoff + ScrollInfo._syoff)
		cells++;
	mapx = AutoMapX - cells;
	mapy = AutoMapY - 1;

	sx = SCREEN_WIDTH / 2 + SCREEN_X - AmLine64 * (cells >> 1);
	sy = VIEWPORT_HEIGHT / 2 + SCREEN_Y - AmLine32 * (cells >> 1);
	if (cells & 1) {
		sy -= AmLine32;
	} else {
		sx += AmLine32;
		sy -= AmLine16;
	}
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

	for (i = 0; i <= cells + 1; i++) {
		int x = sx;
		int y;

		for (j = 0; j < cells; j++) {
			WORD maptype = GetAutomapType(mapx + j, mapy - j, true);
			if (maptype != 0)
				DrawAutomapTile(x, sy, maptype);
			x += AmLine64;
		}
		mapy++;
		x = sx - AmLine32;
		y = sy + AmLine16;
		for (j = 0; j <= cells; j++) {
			WORD maptype = GetAutomapType(mapx + j, mapy - j, true);
			if (maptype != 0)
				DrawAutomapTile(x, y, maptype);
			x += AmLine64;
		}
		mapx++;
		sy += AmLine32;
	}

	for (int pnum = 0; pnum < MAX_PLRS; pnum++) {
		if (plr[pnum].plrlevel == plr[myplr].plrlevel && plr[pnum].plractive && !plr[pnum]._pLvlChanging) {
			if (plr[pnum]._pTeam == plr[myplr]._pTeam)
				DrawAutomapPlr(pnum, pnum == myplr ? COLOR_PLAYER : COLOR_FRIEND);
			else if ((dFlags[plr[pnum]._px][plr[pnum]._py] & BFLAG_LIT) || plr[myplr]._pInfraFlag)
				DrawAutomapPlr(pnum, COLOR_ENEMY);
		}
	}
	//if (AutoMapShowItems)
	//	SearchAutomapItem();
	DrawAutomapText();
	gpBufEnd = &gpBuffer[BUFFER_WIDTH * (SCREEN_Y + SCREEN_HEIGHT)];
}

/**
 * @brief Marks the given coordinate as within view on the automap.
 */
void SetAutomapView(int x, int y)
{
	WORD maptype, solid;
	int xx, yy;

	xx = (x - DBORDERX) >> 1;
	yy = (y - DBORDERY) >> 1;

	if (xx < 0 || xx >= DMAXX || yy < 0 || yy >= DMAXY) {
		return;
	}

	automapview[xx][yy] = TRUE;

	maptype = GetAutomapType(xx, yy, false);
	solid = maptype & MAPFLAG_DIRT;

	switch (maptype & MAPFLAG_TYPE) {
	case 2:
		if (solid) {
			if (GetAutomapType(xx, yy + 1, false) == (MAPFLAG_DIRT | 0x07))
				automapview[xx][yy + 1] = TRUE;
		} else if (GetAutomapType(xx - 1, yy, false) & MAPFLAG_DIRT) {
			automapview[xx - 1][yy] = TRUE;
		}
		break;
	case 3:
		if (solid) {
			if (GetAutomapType(xx + 1, yy, false) == (MAPFLAG_DIRT | 0x07))
				automapview[xx + 1][yy] = TRUE;
		} else if (GetAutomapType(xx, yy - 1, false) & MAPFLAG_DIRT) {
			automapview[xx][yy - 1] = TRUE;
		}
		break;
	case 4:
		if (solid) {
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
		if (solid) {
			if (GetAutomapType(xx, yy - 1, false) & MAPFLAG_DIRT)
				automapview[xx][yy - 1] = TRUE;
			if (GetAutomapType(xx, yy + 1, false) == (MAPFLAG_DIRT | 0x07))
				automapview[xx][yy + 1] = TRUE;
		} else if (GetAutomapType(xx - 1, yy, false) & MAPFLAG_DIRT) {
			automapview[xx - 1][yy] = TRUE;
		}
		break;
	case 6:
		if (solid) {
			if (GetAutomapType(xx - 1, yy, false) & MAPFLAG_DIRT)
				automapview[xx - 1][yy] = TRUE;
			if (GetAutomapType(xx + 1, yy, false) == (MAPFLAG_DIRT | 0x07))
				automapview[xx + 1][yy] = TRUE;
		} else if (GetAutomapType(xx, yy - 1, false) & MAPFLAG_DIRT) {
			automapview[xx][yy - 1] = TRUE;
		}
		break;
	}
}

/**
 * @brief Resets the zoom level of the automap.
 */
void AutomapZoomReset()
{
	AutoMapXOfs = 0;
	AutoMapYOfs = 0;
	AmLine64 = (AutoMapScale << 6) / 128;
	AmLine32 = AmLine64 >> 1;
	AmLine16 = AmLine32 >> 1;
	AmLine8 = AmLine16 >> 1;
	AmLine4 = AmLine8 >> 1;
}

DEVILUTION_END_NAMESPACE
