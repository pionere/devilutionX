/**
 * @file automap.cpp
 *
 * Implementation of the in-game map overlay.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/**
 * Maps from tile_id to automap type.
 * BUGFIX: only the first 256 elements are ever read
 */
WORD automaptype[512];
static int AutoMapX;
static int AutoMapY;
/** Specifies whether the automap is enabled. */
BOOL automapflag;
/** Tracks the explored areas of the map. */
BOOLEAN automapview[DMAXX][DMAXY];
/** Specifies the scale of the automap. */
int AutoMapScale;
int AutoMapXOfs;
int AutoMapYOfs;
int AmLine64;
int AmLine32;
int AmLine16;
int AmLine8;
int AmLine4;

/** color used to draw the player's arrow */
#define COLOR_PLAYER (PAL8_ORANGE + 1)
/** color for bright map lines (doors, stairs etc.) */
#define COLOR_BRIGHT PAL8_YELLOW
/** color for dim map lines/dots */
#define COLOR_DIM (PAL16_YELLOW + 8)
// color for items on automap
#define COLOR_ITEM (PAL8_BLUE + 1)

#define MAPFLAG_TYPE 0x000F
/** these are in the second byte */
#define MAPFLAG_VERTDOOR 0x01
#define MAPFLAG_HORZDOOR 0x02
#define MAPFLAG_VERTARCH 0x04
#define MAPFLAG_HORZARCH 0x08
#define MAPFLAG_VERTGRATE 0x10
#define MAPFLAG_HORZGRATE 0x20
#define MAPFLAG_DIRT 0x40
#define MAPFLAG_STAIRS 0x80

/**
 * @brief Initializes the automap.
 */
void InitAutomapOnce()
{
	automapflag = FALSE;
	AutoMapScale = 50;
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
	BYTE b1, b2;
	DWORD dwTiles;
	BYTE *pAFile, *pTmp;
	DWORD i;

	memset(automaptype, 0, sizeof(automaptype));

	switch (leveltype) {
	case DTYPE_TOWN:
		return;
	case DTYPE_CATHEDRAL:
#ifdef HELLFIRE
		if (currlevel >= 21)
			pAFile = LoadFileInMem("NLevels\\L5Data\\L5.AMP", &dwTiles);
		else
#endif
			pAFile = LoadFileInMem("Levels\\L1Data\\L1.AMP", &dwTiles);
		break;
	case DTYPE_CATACOMBS:
		pAFile = LoadFileInMem("Levels\\L2Data\\L2.AMP", &dwTiles);
		break;
	case DTYPE_CAVES:
#ifdef HELLFIRE
		if (currlevel >= 17)
			pAFile = LoadFileInMem("NLevels\\L6Data\\L6.AMP", &dwTiles);
		else
#endif
			pAFile = LoadFileInMem("Levels\\L3Data\\L3.AMP", &dwTiles);
		break;
	case DTYPE_HELL:
		pAFile = LoadFileInMem("Levels\\L4Data\\L4.AMP", &dwTiles);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	dwTiles /= 2;
	pTmp = pAFile;

	for (i = 1; i <= dwTiles; i++) {
		b1 = *pTmp++;
		b2 = *pTmp++;
		automaptype[i] = b1 + (b2 << 8);
	}

	mem_free_dbg(pAFile);
	memset(automapview, 0, sizeof(automapview));

	static_assert(sizeof(dFlags) == MAXDUNX * MAXDUNY, "Linear traverse of dFlags does not work in InitAutomap.");
	pTmp = (BYTE*)&dFlags[0][0];
	for (i = 0; i < MAXDUNX * MAXDUNY; i++, pTmp++)
		*pTmp &= ~BFLAG_EXPLORED;
}

/**
 * @brief Display/Hides the automap.
 */
void ToggleAutomap()
{
	if (!automapflag) {
		AutoMapXOfs = 0;
		AutoMapYOfs = 0;
		automapflag = TRUE;
	} else {
		automapflag = FALSE;
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
	if (AutoMapScale < 200) {
		AutoMapScale += 5;
		AmLine64 = (AutoMapScale << 6) / 100;
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
	if (AutoMapScale > 50) {
		AutoMapScale -= 5;
		AmLine64 = (AutoMapScale << 6) / 100;
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
	BOOL do_vert;
	BOOL do_horz;
	BOOL do_cave_horz;
	BOOL do_cave_vert;
	int x1, y1, x2, y2;

	BYTE flags = automap_type >> 8;

	if (flags & MAPFLAG_DIRT) {
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

	if (flags & MAPFLAG_STAIRS) {
		DrawLine(sx - AmLine8, sy - AmLine8 - AmLine4, sx + AmLine8 + AmLine16, sy + AmLine4, COLOR_BRIGHT);
		DrawLine(sx - AmLine16, sy - AmLine8, sx + AmLine16, sy + AmLine8, COLOR_BRIGHT);
		DrawLine(sx - AmLine16 - AmLine8, sy - AmLine4, sx + AmLine8, sy + AmLine8 + AmLine4, COLOR_BRIGHT);
		DrawLine(sx - AmLine32, sy, sx, sy + AmLine16, COLOR_BRIGHT);
	}

	do_vert = FALSE;
	do_horz = FALSE;
	do_cave_horz = FALSE;
	do_cave_vert = FALSE;
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
		do_vert = TRUE;
		break;
	case 3:
	case 6:
		do_horz = TRUE;
		break;
	case 4:
		do_vert = TRUE;
		do_horz = TRUE;
		break;
	case 8:
		do_vert = TRUE;
		do_cave_horz = TRUE;
		break;
	case 9:
		do_horz = TRUE;
		do_cave_vert = TRUE;
		break;
	case 10:
		do_cave_horz = TRUE;
		break;
	case 11:
		do_cave_vert = TRUE;
		break;
	case 12:
		do_cave_horz = TRUE;
		do_cave_vert = TRUE;
		break;
	}

	if (do_vert) {                      // right-facing obstacle
		if (flags & MAPFLAG_VERTDOOR) { // two wall segments with a door in the middle
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
		if (flags & MAPFLAG_VERTGRATE) { // right-facing half-wall
			DrawLine(sx - AmLine16, sy - AmLine8, sx - AmLine32, sy, COLOR_DIM);
			flags |= MAPFLAG_VERTARCH;
		}
		if (flags & MAPFLAG_VERTARCH) { // window or passable column
			x1 = sx - AmLine16;
			y1 = sy - AmLine16;
			x2 = x1 + AmLine32;
			y2 = sy - AmLine8;

			DrawLine(sx, y1, x1, y2, COLOR_DIM);
			DrawLine(sx, y1, x2, y2, COLOR_DIM);
			DrawLine(sx, sy, x1, y2, COLOR_DIM);
			DrawLine(sx, sy, x2, y2, COLOR_DIM);
		}
		if ((flags & (MAPFLAG_VERTDOOR | MAPFLAG_VERTGRATE | MAPFLAG_VERTARCH)) == 0)
			DrawLine(sx, sy - AmLine16, sx - AmLine32, sy, COLOR_DIM);
	}

	if (do_horz) { // left-facing obstacle
		if (flags & MAPFLAG_HORZDOOR) {
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
		if (flags & MAPFLAG_HORZGRATE) {
			DrawLine(sx + AmLine16, sy - AmLine8, sx + AmLine32, sy, COLOR_DIM);
			flags |= MAPFLAG_HORZARCH;
		}
		if (flags & MAPFLAG_HORZARCH) {
			x1 = sx - AmLine16;
			y1 = sy - AmLine16;
			x2 = x1 + AmLine32;
			y2 = sy - AmLine8;

			DrawLine(sx, y1, x1, y2, COLOR_DIM);
			DrawLine(sx, y1, x2, y2, COLOR_DIM);
			DrawLine(sx, sy, x1, y2, COLOR_DIM);
			DrawLine(sx, sy, x2, y2, COLOR_DIM);
		}
		if ((flags & (MAPFLAG_HORZDOOR | MAPFLAG_HORZGRATE | MAPFLAG_HORZARCH)) == 0)
			DrawLine(sx, sy - AmLine16, sx + AmLine32, sy, COLOR_DIM);
	}

	// for caves the horz/vert flags are switched
	if (do_cave_horz) {
		if (flags & MAPFLAG_VERTDOOR) {
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
		if (flags & MAPFLAG_HORZDOOR) {
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
	xoff = (ScrollInfo._sxoff * AutoMapScale / 100 >> 1) + SCREEN_WIDTH / 2 + SCREEN_X - (x - y) * AmLine16;
	yoff = (ScrollInfo._syoff * AutoMapScale / 100 >> 1) + (SCREEN_HEIGHT - PANEL_HEIGHT) / 2 + SCREEN_Y - (x + y) * AmLine8 - AmLine8;
	if (PANELS_COVER) {
		if (invflag || sbookflag)
			xoff -= 160;
		if (chrflag || questlog)
			xoff += 160;
	}

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
static void DrawAutomapPlr()
{
	PlayerStruct *p;
	int px, py;
	int x, y;

	p = &plr[myplr];
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

	x = (p->_pxoff * AutoMapScale / 100 >> 1) + (ScrollInfo._sxoff * AutoMapScale / 100 >> 1) + (px - py) * AmLine16 + SCREEN_WIDTH / 2 + SCREEN_X;
	y = (p->_pyoff * AutoMapScale / 100 >> 1) + (ScrollInfo._syoff * AutoMapScale / 100 >> 1) + (px + py) * AmLine8 + (SCREEN_HEIGHT - PANEL_HEIGHT) / 2 + SCREEN_Y;

	if (PANELS_COVER) {
		if (invflag || sbookflag)
			x -= SCREEN_WIDTH / 4;
		if (chrflag || questlog)
			x += SCREEN_WIDTH / 4;
	}
	y -= AmLine8;

	switch (p->_pdir) {
	case DIR_N:
		DrawLine(x, y, x, y - AmLine16, COLOR_PLAYER);
		DrawLine(x, y - AmLine16, x - AmLine4, y - AmLine8, COLOR_PLAYER);
		DrawLine(x, y - AmLine16, x + AmLine4, y - AmLine8, COLOR_PLAYER);
		break;
	case DIR_NE:
		DrawLine(x, y, x + AmLine16, y - AmLine8, COLOR_PLAYER);
		DrawLine(x + AmLine16, y - AmLine8, x + AmLine8, y - AmLine8, COLOR_PLAYER);
		DrawLine(x + AmLine16, y - AmLine8, x + AmLine8 + AmLine4, y, COLOR_PLAYER);
		break;
	case DIR_E:
		DrawLine(x, y, x + AmLine16, y, COLOR_PLAYER);
		DrawLine(x + AmLine16, y, x + AmLine8, y - AmLine4, COLOR_PLAYER);
		DrawLine(x + AmLine16, y, x + AmLine8, y + AmLine4, COLOR_PLAYER);
		break;
	case DIR_SE:
		DrawLine(x, y, x + AmLine16, y + AmLine8, COLOR_PLAYER);
		DrawLine(x + AmLine16, y + AmLine8, x + AmLine8 + AmLine4, y, COLOR_PLAYER);
		DrawLine(x + AmLine16, y + AmLine8, x + AmLine8, y + AmLine8, COLOR_PLAYER);
		break;
	case DIR_S:
		DrawLine(x, y, x, y + AmLine16, COLOR_PLAYER);
		DrawLine(x, y + AmLine16, x + AmLine4, y + AmLine8, COLOR_PLAYER);
		DrawLine(x, y + AmLine16, x - AmLine4, y + AmLine8, COLOR_PLAYER);
		break;
	case DIR_SW:
		DrawLine(x, y, x - AmLine16, y + AmLine8, COLOR_PLAYER);
		DrawLine(x - AmLine16, y + AmLine8, x - AmLine4 - AmLine8, y, COLOR_PLAYER);
		DrawLine(x - AmLine16, y + AmLine8, x - AmLine8, y + AmLine8, COLOR_PLAYER);
		break;
	case DIR_W:
		DrawLine(x, y, x - AmLine16, y, COLOR_PLAYER);
		DrawLine(x - AmLine16, y, x - AmLine8, y - AmLine4, COLOR_PLAYER);
		DrawLine(x - AmLine16, y, x - AmLine8, y + AmLine4, COLOR_PLAYER);
		break;
	case DIR_NW:
		DrawLine(x, y, x - AmLine16, y - AmLine8, COLOR_PLAYER);
		DrawLine(x - AmLine16, y - AmLine8, x - AmLine8, y - AmLine8, COLOR_PLAYER);
		DrawLine(x - AmLine16, y - AmLine8, x - AmLine4 - AmLine8, y, COLOR_PLAYER);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

/**
 * @brief Returns the automap shape at the given coordinate.
 */
static WORD GetAutomapType(int x, int y, BOOL view)
{
	WORD rv;

	if (view && x == -1 && y >= 0 && y < DMAXY && automapview[0][y]) {
		if (GetAutomapType(0, y, FALSE) & (MAPFLAG_DIRT << 8)) {
			return 0;
		} else {
			return MAPFLAG_DIRT << 8;
		}
	}

	if (view && y == -1 && x >= 0 && x < DMAXY && automapview[x][0]) {
		if (GetAutomapType(x, 0, FALSE) & (MAPFLAG_DIRT << 8)) {
			return 0;
		} else {
			return MAPFLAG_DIRT << 8;
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

	rv = automaptype[(BYTE)dungeon[x][y]];
	if (rv == 7) {
		if (GetAutomapType(x - 1, y, FALSE) & (MAPFLAG_HORZARCH << 8)) {
			if (GetAutomapType(x, y - 1, FALSE) & (MAPFLAG_VERTARCH << 8)) {
				rv = 1;
			}
		}
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
	const char *dstr;

	if (gbMaxPlayers != 1) {
		snprintf(desc, sizeof(desc), "game: %s", szPlayerName);
		PrintGameStr(8, nextline, desc, COL_GOLD);
		nextline += 15;
		if (szPlayerDescript[0] != '\0') {
			snprintf(desc, sizeof(desc), "password: %s", szPlayerDescript);
			PrintGameStr(8, nextline, desc, COL_GOLD);
			nextline += 15;
		}
	}
	if (setlevel) {
		dstr = quest_level_names[setlvlnum];
	} else if (currlevel != 0) {
#ifdef HELLFIRE
		if (currlevel >= 17 && currlevel <= 24) {
			if (currlevel <= 20)
				snprintf(desc, sizeof(desc), "Level: Nest %i", currlevel - 16);
			else
				snprintf(desc, sizeof(desc), "Level: Crypt %i", currlevel - 20);
		} else
#endif
			snprintf(desc, sizeof(desc), "Level: %i", currlevel);
		dstr = desc;
	} else {
		dstr = "Tristram";
	}
	PrintGameStr(8, nextline, dstr, COL_GOLD);
}

/**
 * @brief Renders the automap on screen.
 */
void DrawAutomap()
{
	int cells;
	int sx, sy;
	int i, j, d;
	int mapx, mapy;

	if (leveltype == DTYPE_TOWN) {
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

	d = (AutoMapScale << 6) / 100;
	cells = 2 * (SCREEN_WIDTH / 2 / d) + 1;
	if ((SCREEN_WIDTH / 2) % d)
		cells++;
	if ((SCREEN_WIDTH / 2) % d >= (AutoMapScale << 5) / 100)
		cells++;

	if (ScrollInfo._sxoff + ScrollInfo._syoff)
		cells++;
	mapx = AutoMapX - cells;
	mapy = AutoMapY - 1;

	sx = SCREEN_WIDTH / 2 + SCREEN_X - AmLine64 * (cells >> 1);
	sy = (SCREEN_HEIGHT - PANEL_HEIGHT) / 2 + SCREEN_Y - AmLine32 * (cells >> 1);
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

	sx += AutoMapScale * ScrollInfo._sxoff / 100 >> 1;
	sy += AutoMapScale * ScrollInfo._syoff / 100 >> 1;
	if (PANELS_COVER) {
		if (invflag || sbookflag) {
			sx -= SCREEN_WIDTH / 4;
		}
		if (chrflag || questlog) {
			sx += SCREEN_WIDTH / 4;
		}
	}

	for (i = 0; i <= cells + 1; i++) {
		int x = sx;
		int y;

		for (j = 0; j < cells; j++) {
			WORD maptype = GetAutomapType(mapx + j, mapy - j, TRUE);
			if (maptype != 0)
				DrawAutomapTile(x, sy, maptype);
			x += AmLine64;
		}
		mapy++;
		x = sx - AmLine32;
		y = sy + AmLine16;
		for (j = 0; j <= cells; j++) {
			WORD maptype = GetAutomapType(mapx + j, mapy - j, TRUE);
			if (maptype != 0)
				DrawAutomapTile(x, y, maptype);
			x += AmLine64;
		}
		mapx++;
		sy += AmLine32;
	}
	DrawAutomapPlr();
	if (AutoMapShowItems)
		SearchAutomapItem();
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

	maptype = GetAutomapType(xx, yy, FALSE);
	solid = maptype & (MAPFLAG_DIRT << 8);

	switch (maptype & MAPFLAG_TYPE) {
	case 2:
		if (solid) {
			if (GetAutomapType(xx, yy + 1, FALSE) == ((MAPFLAG_DIRT << 8) | 0x07))
				automapview[xx][yy + 1] = TRUE;
		} else if (GetAutomapType(xx - 1, yy, FALSE) & (MAPFLAG_DIRT << 8)) {
			automapview[xx - 1][yy] = TRUE;
		}
		break;
	case 3:
		if (solid) {
			if (GetAutomapType(xx + 1, yy, FALSE) == ((MAPFLAG_DIRT << 8) | 0x07))
				automapview[xx + 1][yy] = TRUE;
		} else if (GetAutomapType(xx, yy - 1, FALSE) & (MAPFLAG_DIRT << 8)) {
			automapview[xx][yy - 1] = TRUE;
		}
		break;
	case 4:
		if (solid) {
			if (GetAutomapType(xx, yy + 1, FALSE) == ((MAPFLAG_DIRT << 8) | 0x07))
				automapview[xx][yy + 1] = TRUE;
			if (GetAutomapType(xx + 1, yy, FALSE) == ((MAPFLAG_DIRT << 8) | 0x07))
				automapview[xx + 1][yy] = TRUE;
		} else {
			if (GetAutomapType(xx - 1, yy, FALSE) & (MAPFLAG_DIRT << 8))
				automapview[xx - 1][yy] = TRUE;
			if (GetAutomapType(xx, yy - 1, FALSE) & (MAPFLAG_DIRT << 8))
				automapview[xx][yy - 1] = TRUE;
			if (GetAutomapType(xx - 1, yy - 1, FALSE) & (MAPFLAG_DIRT << 8))
				automapview[xx - 1][yy - 1] = TRUE;
		}
		break;
	case 5:
		if (solid) {
			if (GetAutomapType(xx, yy - 1, FALSE) & (MAPFLAG_DIRT << 8))
				automapview[xx][yy - 1] = TRUE;
			if (GetAutomapType(xx, yy + 1, FALSE) == ((MAPFLAG_DIRT << 8) | 0x07))
				automapview[xx][yy + 1] = TRUE;
		} else if (GetAutomapType(xx - 1, yy, FALSE) & (MAPFLAG_DIRT << 8)) {
			automapview[xx - 1][yy] = TRUE;
		}
		break;
	case 6:
		if (solid) {
			if (GetAutomapType(xx - 1, yy, FALSE) & (MAPFLAG_DIRT << 8))
				automapview[xx - 1][yy] = TRUE;
			if (GetAutomapType(xx + 1, yy, FALSE) == ((MAPFLAG_DIRT << 8) | 0x07))
				automapview[xx + 1][yy] = TRUE;
		} else if (GetAutomapType(xx, yy - 1, FALSE) & (MAPFLAG_DIRT << 8)) {
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
	AmLine64 = (AutoMapScale << 6) / 100;
	AmLine32 = AmLine64 >> 1;
	AmLine16 = AmLine32 >> 1;
	AmLine8 = AmLine16 >> 1;
	AmLine4 = AmLine8 >> 1;
}

DEVILUTION_END_NAMESPACE
