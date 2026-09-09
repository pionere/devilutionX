/**
 * @file scrollrt.cpp
 *
 * Implementation of functionality for rendering the dungeons, monsters and calling other render routines.
 */
#include "all.h"
#include "utils/display.h"
#include "plrctrls.h"
#include "engine/render/cel_render.h"
#include "engine/render/cl2_render.h"
#include "engine/render/dun_render.h"
#include "engine/render/text_render.h"

DEVILUTION_BEGIN_NAMESPACE

/** Specifies the player viewpoint of the map. */
GRID_POS32 myview;
ScrollStruct ScrollInfo;

/* Specifies the number of entries in the scene array. */
static unsigned numEntries;
/* Array to contain the entities to be drawn. */
static SceneEntry scene[(16 + 1) * (16 * 2 + 2 + 2) * 3 + MAXITEMS + MAXOBJECTS + MAXMISSILES + MAXMONSTERS + MAX_PLRS];

/**
  * Container to hold the cached properties of the viewport.
  *
  * _vColumns: the number of columns to draw to ensure the screen is covered. 
  * _vRows: the number of rows to draw to ensure the screen is covered.
  * _vOffsetX: the base X-offset to draw the tiles in the back buffer.
  * _vOffsetY: the base Y-offset to draw the tiles in the back buffer.
  * _vShiftX: the base offset to myview.subtile.x.
  * _vShiftY: the base offset to myview.subtile.y.
*/
ViewportStruct gsTileVp;

/**
 * Specifies the current light entry.
 */
int light_trn_index;

/**
 * Specifies whether transparency is active for the current CEL file being decoded.
 */
bool gbCelTransparencyActive;

#define BACK_CURSOR 0
#if BACK_CURSOR
/**
 * Cursor-size
 */
static int sgCursHgt;
static int sgCursWdt;

/**
 * Cursor-position
 */
static int sgCursX;
static int sgCursY;

/**
 * Buffer to store the cursor image.
 */
static BYTE sgSaveBack[MAX_CURSOR_AREA];
#endif
#if DEBUG_MODE
static unsigned guFrameCnt;
static unsigned guFrameRate;
static Uint32 guFpsStartTc;
/** Maps from facing direction to scroll-direction. */
static const int8_t dir2sdir[NUM_DIRS] = { SDIR_S, SDIR_SW, SDIR_W, SDIR_NW, SDIR_N, SDIR_NE, SDIR_E, SDIR_SE };

const char* const szMonModeAssert[NUM_MON_MODES] = {
	"standing",
	"walking (1)",
	"walking (2)",
	"attacking",
	"getting hit",
	"dying",
	"attacking (special)",
	"fading in",
	"fading out",
	"attacking (ranged)",
	"standing (special)",
	"attacking (special ranged)",
	"delaying",
	"charging",
	"stoned",
	"healing",
	"talking",
	"dead",
	"unused",
	"reserved",
};

const char* const szPlrModeAssert[NUM_PLR_MODES] = {
	"standing",
	"walking (1)",
	"walking (2)",
	"charging",
	"attacking (melee)",
	"attacking (ranged)",
	"blocking",
	"getting hit",
	"dying",
	"casting a spell",
	"changing levels"
};
#endif

POS32 DungeonToDunPos(int x, int y)
{
	x *= DUN_WIDTH;
	y *= DUN_WIDTH;

	x |= DUN_WIDTH / 2;
	y |= DUN_WIDTH / 2;

	return { x, y };
}

POS32 DungeonScreenToDunPos(int x, int y, int xoff, int yoff)
{
	int xo, yo;
	POS32 res = DungeonToDunPos(x, y);
	static_assert(((TILE_WIDTH / ASSET_MPL) << GRID_SHIFT) == GRID_WIDTH, "Grid position calculation must be adjusted.");

	// convert screen-offset to tile-offset
	xoff /= ASSET_MPL;
	yoff /= ASSET_MPL;

	yoff *= TILE_WIDTH / TILE_HEIGHT;

	xo = xoff + yoff;
	yo = yoff - xoff;

	xo *= DUN_WIDTH / (TILE_WIDTH / ASSET_MPL);
	yo *= DUN_WIDTH / (TILE_WIDTH / ASSET_MPL);

	res.x += xo;
	res.y += yo;

	return res;
}

POS32 DungeonToGridPos(int x, int y)
{
	int gx, gy;
	SET_GRID(gy, gx, y, x);

	gx *= GRID_WIDTH / 2;
	gy *= GRID_WIDTH / 2;

	// gx += GRID_WIDTH / 2;
	// gy += GRID_WIDTH / 2;

	// gx += GRID_WIDTH * (MAXDUNX / 2);
	return { gx, gy };
}

POS32 DungeonScreenToGridPos(int x, int y, int xoff, int yoff)
{
	POS32 res = DungeonToGridPos(x, y);

	xoff /= ASSET_MPL;
	yoff /= ASSET_MPL;

	yoff *= TILE_WIDTH / TILE_HEIGHT;

	res.x += xoff * (GRID_WIDTH / (TILE_WIDTH / ASSET_MPL));
	res.y += yoff * (GRID_WIDTH / (TILE_WIDTH / ASSET_MPL));
	return res;
}

/*
 * Convert grid-position to screen coordinates ignoring zoom and scrolling
 * @param gx Precise grid (shifted dungeon) X-position
 * @param gy Precise grid (shifted dungeon) Y-position
 * @return the screen x/y-coordinates
 */
POS32 GridToScreen(int gx, int gy)
{
	POS32 pos = { gx, gy };

//	pos.x /= (GRID_WIDTH / TILE_WIDTH);
//	pos.y /= (GRID_WIDTH / TILE_HEIGHT);

	pos.x >>= GRID_SHIFT;
	pos.y >>= GRID_SHIFT + 1;

	static_assert(TILE_WIDTH << GRID_SHIFT == GRID_WIDTH * ASSET_MPL, "grid to screen conversion must be adjusted I.");
	static_assert(TILE_HEIGHT << (GRID_SHIFT + 1) == GRID_WIDTH * ASSET_MPL, "grid to screen conversion must be adjusted II.");
	pos.x *= ASSET_MPL;
	pos.y *= ASSET_MPL;

	return pos;
}

POS32 ScreenOffset(int x, int y, int gx, int gy)
{
	POS32 gp = DungeonToGridPos(x, y);
	gx -= gp.x;
	gy -= gp.y;

	return GridToScreen(gx, gy);
}

POS32 DunScreenOffset(int x, int y, int dx, int dy)
{
	POS32 dp = DungeonToDunPos(x, y);
	dx -= dp.x;
	dy -= dp.y;

	POS32 gp;
	SET_GRID(gp.y, gp.x, dy, dx);

	gp.x /= DUN_WIDTH / (TILE_WIDTH / 2);
	gp.y /= DUN_WIDTH / (TILE_HEIGHT / 2);

	return gp;
}

/*
 * Return the screen position of the given grid-position (gx;gy).
 *
 * @param gx Precise grid (shifted dungeon) X-position
 * @param gy Precise grid (shifted dungeon) Y-position
 * @return the screen x/y-coordinates
 */
POS32 GetMousePosGrid(int gx, int gy)
{
	POS32 pos;
	POS32 dp = DungeonToGridPos(myview.subtile.x, myview.subtile.y);

	gx -= dp.x;
	gy -= dp.y;

	pos = GridToScreen(gx, gy);

	pos.x += ScrollInfo._sxoff;
	pos.y += ScrollInfo._syoff;

	if (gbZoomInFlag) {
		pos.x <<= 1;
		pos.y <<= 1;
	}

	pos.x += SCREEN_WIDTH / 2u;
	pos.y += SCREEN_HEIGHT / 2u;

	return pos;
}

/*
 * Return the screen position of the given dun-position.
 *
 * @param pos Precise dungeon position
 * @return the screen x/y-coordinates
 */
POS32 GetMousePosDun(POS32 pos)
{
	POS32 dp = myview.dun;
	pos.x -= dp.x;
	pos.y -= dp.y;

	int gx = pos.x - pos.y;
	int gy = pos.x + pos.y;

	gx /= DUN_WIDTH / (TILE_WIDTH / 2);
	gy /= DUN_WIDTH / (TILE_HEIGHT / 2);

	if (gbZoomInFlag) {
		gx <<= 1;
		gy <<= 1;
	}

	gx += SCREEN_WIDTH / 2u;
	gy += SCREEN_HEIGHT / 2u;

	return { gx, gy };
}

void UpdateScrollInfo(int pnum)
{
	if (pnum == mypnum) {
#if FOLLOW
		// TODO: follow with the cursor if a monster is selected? (does not work well with upscale)
		int dx = plr._px - myview.subtile.x;
		int dy = plr._py - myview.subtile.y;
#endif
		myview.subtile.x = plr._px;
		myview.subtile.y = plr._py;
		myview.dun = plr._ppos;
		myview.gx = plr._pgx;
		myview.gy = plr._pgy;

		POS32 sp = ScreenOffset(myview.subtile.x, myview.subtile.y, plr._pgx, plr._pgy);
#if FOLLOW
		POS32 dp = DungeonScreenToGridPos(dx, dy, ScrollInfo._sxoff + sp.x, ScrollInfo._syoff + sp.y);
		dp = GridToScreen(dp.x, dp.y);
		if (gbActionBtnDown != 0 && (dp.x | dp.y) != 0 && MON_VALID(pcursmonst))
			SetCursorPos(MousePos.x - dp.x, MousePos.y - dp.y);
#endif
		ScrollInfo._sxoff = -sp.x;
		ScrollInfo._syoff = -sp.y;
#if DEBUG_MODE
//		for (int i = 0; i < lengthof(dir2sdir); i++)
//			assert(dir2sdir[i] == 1 + i);
#endif
//		ScrollInfo._sdir = (ScrollInfo._sxoff == 0 && ScrollInfo._syoff == 0) ? SDIR_NONE : (1 + /*OPPOSITE(*/plr._pdir/*)*/); // == dir2sdir[dir];
	}
}

/**
 * @brief Clear cursor state
 */
void ClearCursor() // CODE_FIX: this was supposed to be in cursor.cpp
{
#if BACK_CURSOR
	sgCursWdt = 0;
#endif
}

/**
 * @brief Remove the cursor from the back buffer
 */
static void scrollrt_remove_back_buffer_cursor()
{
#if BACK_CURSOR
	int i;
	BYTE *src, *dst;

	if (sgCursWdt == 0) {
		return;
	}

	assert(gpBuffer != NULL);
	src = sgSaveBack;
	dst = &gpBuffer[SCREENXY(sgCursX, sgCursY)];
	for (i = sgCursHgt; i != 0; i--) {
		memcpy(dst, src, sgCursWdt);
		src += sgCursWdt;
		dst += BUFFER_WIDTH;
	}

	sgCursWdt = 0;
#endif
}

void scrollrt_draw_item(const ItemStruct* is, bool outline, int sx, int sy, const BYTE* pCelBuff, int nCel, int nWidth)
{
	BYTE col, trans;

	col = ICOL_YELLOW;
	if (is->_iMagical != ITEM_QUALITY_NORMAL) {
		col = ICOL_BLUE;
	}
	if (!is->_iStatFlag) {
		col = ICOL_RED;
	}

	if (outline) {
		CelClippedDrawOutline(col, sx, sy, pCelBuff, nCel, nWidth);
	}
	trans = col != ICOL_RED ? 0 : COLOR_TRN_RED;
	CelClippedDrawLightTbl(sx, sy, pCelBuff, nCel, nWidth, trans);
}

/**
 * @brief Draw the cursor on the back buffer
 */
static void scrollrt_draw_cursor()
{
	int mx, my, frame;
	BYTE* cCels;
#if BACK_CURSOR
	int i, cx, cy, cw, ch;
	BYTE *src, *dst;
	assert(sgCursWdt == 0);
#endif
	if (pcursicon <= CURSOR_NONE) {
		return;
	}
	assert(cursW != 0 && cursH != 0);

	mx = MousePos.x;
	my = MousePos.y;
	// shift the cursor of the items CURSOR_HOTSPOT
	if (pcursicon >= CURSOR_FIRSTITEM) {
		mx -= cursW >> 1;
		my -= cursH >> 1;
	}
	// limit the mouse to the screen
	if (mx <= 0 - cursW) {
		return;
	}
	if (mx >= SCREEN_WIDTH) {
		return;
	}
	if (my <= 0 - cursH) {
		return;
	}
	if (my >= SCREEN_HEIGHT) {
		return;
	}
#if BACK_CURSOR
	cx = mx;
	cw = cx + cursW;
	// cut the cursor on the right side
	//if (cw > SCREEN_WIDTH) {
	//	cw = SCREEN_WIDTH;
	//}
	// cut the cursor on the left side
	//if (cx <= 0) {
	//	cx = 0;
	//} else {
		// draw to 4-byte aligned blocks
		cx &= ~3;
		cw -= cx;
	//}
	// draw with 4-byte alignment
	cw += 3;
	cw &= ~3;

	cy = my;
	ch = cy + cursH;
	// cut the cursor on the bottom
	//if (ch > SCREEN_HEIGHT) {
	//	ch = SCREEN_HEIGHT;
	//}
	// cut the cursor on the top
	//if (cy <= 0) {
	//	cy = 0;
	//} else {
		ch -= cy;
	//}

	sgCursX = cx;
	sgCursY = cy;

	sgCursWdt = cw;
	sgCursHgt = ch;

	assert((unsigned)(cw * ch) <= sizeof(sgSaveBack));
	assert(gpBuffer != NULL);
	dst = sgSaveBack;
	src = &gpBuffer[SCREENXY(cx, cy)];

	for (i = ch; i != 0; i--, dst += cw, src += BUFFER_WIDTH) {
		memcpy(dst, src, cw);
	}
#endif
	mx += SCREEN_X;
	my += cursH + SCREEN_Y - 1;

	frame = pcursicon;
	cCels = pCursCels;
	if (frame < CURSOR_FIRSTITEM) {
		CelClippedDrawLightTbl(mx, my, cCels, frame, cursW, 0);
	} else {
		scrollrt_draw_item(&myplr._pHoldItem, true, mx, my, cCels, frame, cursW);
	}
}

static unsigned SubtileZOrderAt(int x, int y)
{
	int sx = x - y;
	int sy = x + y;
	static_assert(MAXDUNX < 128 && MAXDUNY < 128, "Higher bits of the zOrder might overflow.");
	sx ^= 128;
	sx &= 0xFF;
	// assert(sy <= 0xFF);
	return (sx << 16) | (sy << 24);
}

static unsigned SubtileZOrder(POS32 pos)
{
	int x = (unsigned)pos.x / DUN_WIDTH;
	int y = (unsigned)pos.y / DUN_WIDTH;
	return SubtileZOrderAt(x, y);
}

static unsigned OffsetZOrder(POS32 pos)
{
	int x = (pos.x >> DUN_SHIFT) & ((DUN_WIDTH >> DUN_SHIFT) - 1);
	int y = (pos.y >> DUN_SHIFT) & ((DUN_WIDTH >> DUN_SHIFT) - 1);
	// int sx = x - y;
	int sy = x + y;
	static_assert((DUN_WIDTH >> DUN_SHIFT) <= (1 << ZOR_SHIFT), "Lower bits of the zOrder might overflow.");
	return sy >> 1;
}

/**
 * @brief add a missile to the scene-array
 * @param mi id of the missile
 * @param lightIdx light index at the missile's position
 * @param zorder zorder in the scene
 * @param entry the scene-array entry after which the missile should be added
 */
static void scene_addMissile(int mi, int lightIdx, unsigned zorder, SceneEntry* entry)
{
	BYTE trans;
	const MissileStruct* mis = &missile[mi];

	trans = mis->_miUniqTrans == 0 ? (mis->_miLightFlag ? lightIdx : 0) : mis->_miUniqTrans;

	scene[numEntries].scType = SCT_MISSILE;
	scene[numEntries].scLight = lightIdx;
	scene[numEntries].scTrans = trans; // gbCelTransparencyActive;
	scene[numEntries].scPosx = mis->_migx;
	scene[numEntries].scPosy = mis->_migy;
	scene[numEntries].scIdx = mi;

	scene[numEntries].scZOrder = zorder;
	scene[numEntries].scNext = entry->scNext;

	entry->scNext = numEntries;

	numEntries++;
#ifdef DEBUG
	assert(numEntries <= lengthof(scene));
#endif
}

/**
 * @brief Render a missile sprite
 * @param entry the scene entry of the missile
 */
static void DrawSceneMissile(const SceneEntry &entry)
{
	int mx = entry.scPosx;
	int my = entry.scPosy;
	int mi = entry.scIdx;
	BYTE trans = entry.scTrans;
	int nCel, nWidth;
	const BYTE* pCelBuff;
	// assert(entry.scIdx == SCT_MISSILE);
	// assert((unsigned)mi < MAXMISSILES);
	const MissileStruct* mis = &missile[mi];

	pCelBuff = mis->_miAnimData;
	if (pCelBuff == NULL) {
		dev_fatal("Draw Missile type %d: NULL Cel Buffer", mis->_miType);
	}
	nCel = mis->_miAnimFrame;
#if DEBUG_MODE
	int frames = (mis->_miType == MIS_RHINO || mis->_miType == MIS_CHARGE) ? LOAD_LE32(pCelBuff) : reinterpret_cast<const CelAnimBuf*>(pCelBuff)->caFrameCnt;
	if (nCel < 1 || frames > 50 || nCel > frames) {
		dev_fatal("Draw Missile frame %d of %d, type %d", nCel, frames, mis->_miType);
	}
#endif
	nWidth = mis->_miAnimWidth;
	mx -= nWidth / 2u;
	my += mis->_mizoff;
	Cl2DrawLightTbl(mx, my, pCelBuff, nCel, nWidth, trans);
}

/**
 * @brief add a monster to the scene-array
 * @param mnum id of the monster
 * @param bFlag flags to draw
 * @param lightIdx light index at the monster's position
 * @param zorder zorder in the scene
 * @param entry the scene-array entry after which the monster should be added
 */
static void scene_addMonster(int mnum, BYTE bFlag, int lightIdx, unsigned zorder, SceneEntry* entry)
{
	const MonsterStruct* mon;
	BYTE trans;
	BYTE visFlag = bFlag & BFLAG_VISIBLE;
	// assert((unsigned)mnum < MAXMONSTERS);
	if (!visFlag && myplr._pTimer[PLTR_INFRAVISION] <= 0/* !myplr._pInfraFlag */)
		return;

	mon = &monsters[mnum];
	if (mon->_mFlags & MFLAG_HIDDEN) {
		return;
	}

	if (!visFlag || (myplr._pTimer[PLTR_INFRAVISION] > 0/* myplr._pInfraFlag */ && lightIdx > 8))
		trans = COLOR_TRN_RED;
	else if (mon->_mmode == MM_STONE)
		trans = COLOR_TRN_GRAY;
	else
		trans = lightIdx;

	scene[numEntries].scType = SCT_MONSTER;
	scene[numEntries].scLight = lightIdx;
	scene[numEntries].scTrans = trans; // gbCelTransparencyActive;
	scene[numEntries].scPosx = mon->_mgx;
	scene[numEntries].scPosy = mon->_mgy;
	scene[numEntries].scIdx = mnum;

	scene[numEntries].scZOrder = zorder;
	scene[numEntries].scNext = entry->scNext;

	entry->scNext = numEntries;

	numEntries++;
#ifdef DEBUG
	assert(numEntries <= lengthof(scene));
#endif
}

/**
 * @brief Render a monster sprite
 * @param entry the scene entry of the monster
 */
static void DrawSceneMonster(const SceneEntry &entry)
{
	int mx = entry.scPosx;
	int my = entry.scPosy;
	int mnum = entry.scIdx;
	BYTE trans = entry.scTrans;
	int nCel, nWidth;
	const BYTE* pCelBuff;
	// assert(entry.scIdx == SCT_MONSTER || entry.scIdx == SCT_DEAD_MONSTER);
	// assert((unsigned)mnum < MAXMONSTERS);
	const MonsterStruct* mon = &monsters[mnum];

	pCelBuff = mon->_mAnimData;
	if (pCelBuff == NULL) {
		dev_fatal("Draw Monster \"%s\": NULL Cel Buffer", mon->_mName);
	}

	nCel = mon->_mAnimFrame;
#if DEBUG_MODE
	int frames = LOAD_LE32(pCelBuff);
	if (nCel < 1 || frames > 50 || nCel > frames) {
		const char* szMode = "unknown action";
		if (mon->_mmode < lengthof(szMonModeAssert))
			szMode = szMonModeAssert[mon->_mmode];
		dev_fatal(
			"Draw Monster \"%s\" %s: facing %d, frame %d of %d",
			mon->_mName,
			szMode,
			mon->_mdir,
			nCel,
			frames);
	}
#endif
	nWidth = mon->_mAnimWidth;
	mx -= nWidth / 2u;
	if (mnum == pcursmonst) {
		Cl2DrawOutline(PAL16_RED + 9, mx, my, pCelBuff, nCel, nWidth);
	}
	Cl2DrawLightTbl(mx, my, pCelBuff, nCel, nWidth, trans);
}

/**
 * @brief add a dead monster to the scene-array
 * @param mnum id of the monster
 * @param bFlag flags to draw
 * @param lightIdx light index at the monster's position
 * @param zorder zorder in the scene
 * @param entry the scene-array entry after which the monster should be added
 */
static void scene_addDeadMonsterEntry(int mnum, BYTE bFlag, int lightIdx, unsigned zorder, SceneEntry* entry)
{
	const MonsterStruct* mon = &monsters[mnum];
	// assert((unsigned)mnum < MAXMONSTERS);

	scene[numEntries].scType = SCT_DEAD_MONSTER;
	scene[numEntries].scLight = lightIdx;
	scene[numEntries].scTrans = lightIdx; // gbCelTransparencyActive;
	scene[numEntries].scPosx = mon->_mgx;
	scene[numEntries].scPosy = mon->_mgy;
	scene[numEntries].scIdx = mnum;

	scene[numEntries].scZOrder = zorder;
	scene[numEntries].scNext = entry->scNext;

	entry->scNext = numEntries;

	numEntries++;
#ifdef DEBUG
	assert(numEntries <= lengthof(scene));
#endif
}

/**
 * @brief add a towner to the scene-array
 * @param mnum id of the towner
 * @param lightIdx light index at the towner's position
 * @param zorder zorder in the scene
 * @param entry the scene-array entry after which the towner should be added
 */
static void scene_addTowner(int mnum, int lightIdx, unsigned zorder, SceneEntry* entry)
{
	const MonsterStruct* tw = &monsters[mnum];
	// assert(mnum < numtowners);

	scene[numEntries].scType = SCT_TOWNER;
	scene[numEntries].scLight = lightIdx;
	scene[numEntries].scTrans = FALSE; // gbCelTransparencyActive;
	scene[numEntries].scPosx = tw->_mgx;
	scene[numEntries].scPosy = tw->_mgy;
	scene[numEntries].scIdx = mnum;

	scene[numEntries].scZOrder = zorder;
	scene[numEntries].scNext = entry->scNext;

	entry->scNext = numEntries;

	numEntries++;
#ifdef DEBUG
	assert(numEntries <= lengthof(scene));
#endif
}

/**
 * @brief Render a towner sprite
 * @param entry the scene entry of the towner
 */
static void DrawSceneTowner(const SceneEntry &entry)
{
	int tx = entry.scPosx;
	int ty = entry.scPosy;
	int tnum = entry.scIdx;
	int nCel, nWidth;
	const BYTE* pCelBuff;
	// assert(entry.scIdx == SCT_TOWNER);
	// assert((unsigned)tnum < numtowners);
	const MonsterStruct* tw = &monsters[tnum];

	pCelBuff = tw->_mAnimData;
	if (pCelBuff == NULL) {
		dev_fatal("Draw Towner \"%s\": NULL Cel Buffer", tw->_mName);
	}
	nCel = tw->_mAnimFrame;
	nWidth = tw->_mAnimWidth;
	tx -= nWidth / 2u;
	if (tnum == pcursmonst) {
		CelClippedDrawOutline(PAL16_BEIGE + 6, tx, ty, pCelBuff, nCel, nWidth);
	}
	CelClippedDrawLightTbl(tx, ty, pCelBuff, nCel, nWidth, 0);
}

/**
 * @brief add a player to the scene-array
 * @param pnum id of the player
 * @param bFlag flags to draw
 * @param lightIdx light index at the player's position
 * @param zorder zorder in the scene
 * @param entry the scene-array entry after which the player should be added
 */
static void scene_addPlayer(int pnum, BYTE bFlag, int lightIdx, unsigned zorder, SceneEntry* entry)
{
	BYTE visFlag = bFlag & BFLAG_VISIBLE;
	BYTE trans;
	// assert(pnum < MAX_PLRS);
	if (!visFlag && myplr._pTimer[PLTR_INFRAVISION] <= 0/* !myplr._pInfraFlag */)
		return;

	if (pnum == mypnum) {
		trans = 0;
	} else if (!visFlag || (myplr._pTimer[PLTR_INFRAVISION] > 0/* myplr._pInfraFlag */ && lightIdx > 8)) {
		trans = COLOR_TRN_RED;
	} else {
		trans = lightIdx;
		trans = trans <= 5 ? 0 : (trans - 5);
	}

	scene[numEntries].scType = plr._pHitPoints != 0 ? SCT_PLAYER : SCT_DEAD_PLAYER;
	scene[numEntries].scLight = lightIdx;
	scene[numEntries].scTrans = trans; // gbCelTransparencyActive;
	scene[numEntries].scPosx = plr._pgx;
	scene[numEntries].scPosy = plr._pgy;
	scene[numEntries].scIdx = pnum;

	scene[numEntries].scZOrder = zorder;
	scene[numEntries].scNext = entry->scNext;

	entry->scNext = numEntries;

	numEntries++;
#ifdef DEBUG
	assert(numEntries <= lengthof(scene));
#endif
}

/**
 * @brief Render a player sprite
 * @param entry the scene entry of the player
 */
static void DrawScenePlayer(const SceneEntry &entry)
{
	int px = entry.scPosx;
	int py = entry.scPosy;
	int pnum = entry.scIdx;
	BYTE trans = entry.scTrans;
	int nCel, nWidth;
	const BYTE* pCelBuff;
	// assert(pnum < MAX_PLRS);
	// assert(entry.scIdx == SCT_PLAYER || entry.scIdx == SCT_DEAD_PLAYER);

	pCelBuff = plr._pAnimData;
	if (pCelBuff == NULL) {
		dev_fatal("Draw Player %d \"%s\": NULL Cel Buffer", pnum, plr._pName);
	}
	nCel = plr._pAnimFrame;
#if DEBUG_MODE
	int frames = LOAD_LE32(pCelBuff);
	if (nCel < 1 || frames > 50 || nCel > frames) {
		const char* szMode = "unknown action";
		if (plr._pmode < lengthof(szPlrModeAssert))
			szMode = szPlrModeAssert[plr._pmode];
		dev_fatal(
			"Draw Player %d \"%s\" %s(%d): facing %d, frame %d of %d",
			pnum,
			plr._pName,
			szMode,
			plr._pmode,
			plr._pdir,
			nCel,
			frames);
	}
#endif
	nWidth = plr._pAnimWidth;
	px -= nWidth / 2u;
	if (pnum == pcursplr)
		Cl2DrawOutline(PAL16_BEIGE + 5, px, py, pCelBuff, nCel, nWidth);
	/*if (plr.pManaShield != 0)
		Cl2DrawLightTbl(
		    px + nWidth / 2u - misanimdim[MFILE_MANASHLD][1],
		    py,
		    misanimdata[MFILE_MANASHLD][0],
		    1,
		    misanimdim[MFILE_MANASHLD][0], trans);*/
	Cl2DrawLightTbl(px, py, pCelBuff, nCel, nWidth, trans);
}

/**
 * @brief add an object to the scene-array
 * @param oi id of the object
 * @param lightIdx light index at the object's position
 * @param zorder zorder in the scene
 * @param entry the scene-array entry after which the object should be added
 */
static void scene_addObject(int oi, int lightIdx, unsigned zorder, SceneEntry* entry)
{
	const ObjectStruct* os;
	// assert((unsigned)oi < MAXOBJECTS);
	os = &objects[oi];

	scene[numEntries].scType = SCT_OBJECT;
	scene[numEntries].scLight = lightIdx;
	scene[numEntries].scTrans = FALSE; // gbCelTransparencyActive;
	scene[numEntries].scPosx = os->_ogx;
	scene[numEntries].scPosy = os->_ogy;
	scene[numEntries].scIdx = oi;

	scene[numEntries].scZOrder = zorder;
	scene[numEntries].scNext = entry->scNext;

	entry->scNext = numEntries;

	numEntries++;
#ifdef DEBUG
	assert(numEntries <= lengthof(scene));
#endif
}

/**
 * @brief Render an object sprite
 * @param entry the scene entry of the object
 */
static void DrawSceneObject(const SceneEntry &entry)
{
	int ox = entry.scPosx;
	int oy = entry.scPosy;
	int oi = entry.scIdx;
	int nGfxCel, nAnimCel, nWidth;
	const BYTE* pCelBuff;
	// assert(entry.scIdx == SCT_OBJECT);
	// assert((unsigned)oi < MAXOBJECTS);
	const ObjectStruct* os = &objects[oi];

	pCelBuff = os->_oAnimData;
	if (pCelBuff == NULL) {
		dev_fatal("Draw Object type %d: NULL Cel Buffer", os->_otype);
	}

	nGfxCel = os->_oGfxFrame;
	nAnimCel = os->_oAnimFrame;
#if DEBUG_MODE
	int frames = LOAD_LE32(pCelBuff);
	if (nGfxCel > frames) {
		dev_fatal("Draw Object Gfx: frame %d of %d, type %d", nGfxCel, frames, os->_otype);
	}
	if (nAnimCel > frames) {
		dev_fatal("Draw Object Anim: frame %d of %d, type %d", nAnimCel, frames, os->_otype);
	}
#endif
	nWidth = os->_oAnimWidth;
	ox -= nWidth / 2u;
	if (oi == pcursobj) {
		if (nGfxCel > 0) {
			CelClippedDrawOutline(PAL16_YELLOW + 2, ox, oy, pCelBuff, nGfxCel, nWidth);
		}
		if (nAnimCel > 0) {
			CelClippedDrawOutline(PAL16_YELLOW + 2, ox, oy, pCelBuff, nAnimCel, nWidth);
		}
	}
	if (nGfxCel > 0) {
		CelClippedDrawLightTbl(ox, oy, pCelBuff, nGfxCel, nWidth, light_trn_index);
	}
	if (nAnimCel > 0) {
		CelClippedDrawLightTbl(ox, oy, pCelBuff, nAnimCel, nWidth, light_trn_index);
	}
}

/**
 * @brief add a dungeon subtile to the scene-array
 * @param pn piece number
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 */
static void scene_addCell(int pn, int x, int y, int sx, int sy)
{
	uint16_t i, limit;
	int tmp;
	tmp = sy - SCREEN_Y;
	tmp = (unsigned)(tmp + 1 + (MICRO_HEIGHT - 1)) / MICRO_HEIGHT;
	tmp *= TILE_WIDTH / MICRO_WIDTH;
	limit = tmp <= MicroTileLen ? tmp : MicroTileLen;
	/*limit = MicroTileLen;
	while (sy - limit * (MICRO_HEIGHT / (TILE_WIDTH / MICRO_WIDTH)) <= SCREEN_Y - MICRO_HEIGHT) {
		limit -= (TILE_WIDTH / MICRO_WIDTH);
	}*/
	/*i = 0;
	while (sy > SCREEN_Y + SCREEN_HEIGHT + MICRO_HEIGHT) {
		sy -= MICRO_HEIGHT;
		i += (TILE_WIDTH / MICRO_WIDTH);
	}*/
	tmp = sy - (SCREEN_Y + SCREEN_HEIGHT + TILE_HEIGHT - 1);
	i = 0;
	if (tmp >= 0) {
		// starting from below the bottom -> skip tiles
		tmp = 1 + (unsigned)tmp / TILE_HEIGHT;
		sy -= TILE_HEIGHT * tmp;
		i = tmp * (TILE_WIDTH / MICRO_WIDTH) * (TILE_HEIGHT / MICRO_HEIGHT);
	}

	scene[numEntries].scType = SCT_CELL;
	scene[numEntries].scLight = light_trn_index;
	scene[numEntries].scTrans = gbCelTransparencyActive;
	scene[numEntries].scPosx = sx;
	scene[numEntries].scPosy = sy;
	scene[numEntries].scIdx = pn;
	scene[numEntries].scCellIdxFrom = i;
	scene[numEntries].scCellIdxTo = limit;

	scene[numEntries].scZOrder = SubtileZOrderAt(x, y) | (ZOR_CELL << ZOR_SHIFT);
	scene[numEntries].scNext = numEntries + 1;

	numEntries++;
#ifdef DEBUG
	assert(numEntries <= lengthof(scene));
#endif
}

/**
 * @brief Render a cell
 * @param entry the scene entry of the cell
 */
static void DrawSceneCell(const SceneEntry &entry)
{
	int sx = entry.scPosx;
	int sy = entry.scPosy;
	int pn = entry.scIdx;
	uint16_t i = entry.scCellIdxFrom;
	uint16_t limit = entry.scCellIdxTo;
	BYTE* dst;
	uint16_t levelCelBlock;
	uint16_t* pMap;
	int tmp, mask;
	// assert(entry.scIdx == SCT_CELL);
	dst = &gpBuffer[BUFFERXY(sx, sy)];

	pMap = &pSubtiles[pn][i];
	tmp = microFlags[pn];
	tmp &= gbCelTransparencyActive ? ~0 : ~(TMIF_LEFT_WALL_TRANS | TMIF_RIGHT_WALL_TRANS | TMIF_WALL_TRANS);
	if (i == 0) {
		if (tmp & TMIF_LEFT_REDRAW) {
			for (int j = 0; j < ASSET_MPL; j++) {
				for (int k = 0; k < ASSET_MPL; k++) {
					levelCelBlock = pMap[k + j * ASSET_MPL * 2];
					if (levelCelBlock != 0) {
						mask = DMT_NONE;
#if ASSET_MPL == 1
						if (tmp & TMIF_LEFT_WALL_TRANS) {
							mask = DMT_LTFLOOR;
						} else if (tmp & TMIF_LEFT_FOLIAGE) {
							mask = DMT_LFLOOR;
						}
#elif ASSET_MPL == 2
						if (tmp & TMIF_LEFT_WALL_TRANS) {
							static int leftTrnMasksEven[ASSET_MPL][ASSET_MPL] = {
								// clang-format off
								DMT_NONE,              DMT_NONE,
								DMT_FLOOR_TRN_UP_LEFT, DMT_LTFLOOR,
								// clang-format on
							};
							mask = leftTrnMasksEven[j][k];
						} else if (tmp & TMIF_LEFT_FOLIAGE) {
							static int leftFolMasksEven[ASSET_MPL][ASSET_MPL] = {
								// clang-format off
								DMT_FLOOR_LOW_BOTTOM_LEFT, DMT_FLOOR_BOTTOM_LEFT,
								DMT_FLOOR_UP_TOP_LEFT,     DMT_FLOOR_TOP_LEFT,
								// clang-format on
							};
							mask = leftFolMasksEven[j][k];
						}
#elif ASSET_MPL == 3
						if (tmp & TMIF_LEFT_WALL_TRANS) {
							static int leftTrnMasksOdd[ASSET_MPL][ASSET_MPL] = {
								// clang-format off
								DMT_NONE,    DMT_NONE,              DMT_NONE,
								DMT_LTFLOOR, DMT_NONE,              DMT_NONE,
								DMT_TWALL,   DMT_FLOOR_TRN_UP_LEFT, DMT_LTFLOOR,
								// clang-format on
							};
							mask = leftTrnMasksOdd[j][k];
						} else if (tmp & TMIF_LEFT_FOLIAGE) {
							static int leftFolMasksOdd[ASSET_MPL][ASSET_MPL] = {
								// clang-format off
								DMT_NONE,   DMT_FLOOR_LOW_BOTTOM_LEFT, DMT_FLOOR_BOTTOM_LEFT,
								DMT_LFLOOR, DMT_EMPTY,                 DMT_EMPTY,
								DMT_NONE,   DMT_FLOOR_UP_TOP_LEFT,     DMT_FLOOR_TOP_LEFT,
								// clang-format on
							};
							mask = leftFolMasksOdd[j][k];
						}
#elif ASSET_MPL == 4
						if (tmp & TMIF_LEFT_WALL_TRANS) {
							static int leftTrnMasksEven[ASSET_MPL][ASSET_MPL] = {
								// clang-format off
								DMT_NONE,              DMT_NONE,    DMT_NONE,              DMT_NONE,
								DMT_NONE,              DMT_NONE,    DMT_NONE,              DMT_NONE,
								DMT_FLOOR_TRN_UP_LEFT, DMT_LTFLOOR, DMT_NONE,              DMT_NONE,
								DMT_TWALL,             DMT_TWALL,   DMT_FLOOR_TRN_UP_LEFT, DMT_LTFLOOR,
								// clang-format on
							};
							mask = leftTrnMasksEven[j][k];
						} else if (tmp & TMIF_LEFT_FOLIAGE) {
							static int leftFolMasksEven[ASSET_MPL][ASSET_MPL] = {
								// clang-format off
								DMT_NONE,                  DMT_NONE,              DMT_FLOOR_LOW_BOTTOM_LEFT, DMT_FLOOR_BOTTOM_LEFT,
							    DMT_FLOOR_LOW_BOTTOM_LEFT, DMT_FLOOR_BOTTOM_LEFT, DMT_EMPTY,                 DMT_EMPTY,
								DMT_FLOOR_UP_TOP_LEFT,     DMT_FLOOR_TOP_LEFT,    DMT_EMPTY,                 DMT_EMPTY,
								DMT_NONE,                  DMT_NONE,              DMT_FLOOR_UP_TOP_LEFT,     DMT_FLOOR_TOP_LEFT,
								// clang-format on
							};
							mask = leftFolMasksEven[j][k];
						}
#else
					bool ASSET_EVEN = (ASSET_MPL & 1) == 0;
					static int leftTrnMasksEven[] = {
						// clang-format off
						DMT_NONE, DMT_NONE, DMT_FLOOR_TRN_UP_LEFT, DMT_LTFLOOR,
						DMT_NONE, DMT_NONE, DMT_NONE /*DMT_LTFLOOR*/, DMT_NONE /*DMT_LTFLOOR*/,
						DMT_TWALL, DMT_TWALL, DMT_TWALL, DMT_TWALL,
						DMT_TWALL, DMT_TWALL, DMT_TWALL, DMT_TWALL,
						DMT_NONE, DMT_NONE/*, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,*/
						// clang-format on
					};
					static int leftTrnMasksOdd[] = {
						// clang-format off
						DMT_NONE, DMT_NONE, DMT_LTFLOOR, DMT_FLOOR_TRN_UP_LEFT,
						DMT_NONE, DMT_NONE, DMT_LTFLOOR, DMT_LTFLOOR,
						DMT_TWALL, DMT_TWALL, DMT_TWALL, DMT_TWALL,
						DMT_TWALL, DMT_TWALL, DMT_TWALL, DMT_TWALL,
						DMT_NONE, DMT_NONE/*, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,*/
						// clang-format on
					};
					static int leftFolMasksEven[] = {
						// clang-format off
						DMT_EMPTY, DMT_EMPTY, DMT_FLOOR_UP_TOP_LEFT, DMT_FLOOR_TOP_LEFT,
						DMT_FLOOR_LOW_BOTTOM_LEFT, DMT_FLOOR_BOTTOM_LEFT, DMT_NONE /*DMT_LFLOOR*/, DMT_NONE /*DMT_LFLOOR*/,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE/*, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,*/
						// clang-format on
					};
					static int leftFolMasksOdd[] = {
						// clang-format off
						DMT_EMPTY, DMT_EMPTY, DMT_FLOOR_TOP_LEFT, DMT_FLOOR_UP_TOP_LEFT,
						DMT_FLOOR_BOTTOM_LEFT, DMT_FLOOR_LOW_BOTTOM_LEFT, DMT_LFLOOR, DMT_LFLOOR,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE/*, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,*/
						// clang-format on
					};
					mask = (k & 1)                                 // odd
					 | ((j == (ASSET_MPL + k) / 2) << 1)           // onUpperTop
					 | (((j + 1) == (ASSET_MPL - k + 1) / 2) << 2) // onLowerBottom
					 | ((j > (ASSET_MPL + k) / 2) << 3)            // upperTop
					 | (((j + 1) < (ASSET_MPL - k + 1) / 2) << 4); // lowerBottom
					if (tmp & TMIF_LEFT_WALL_TRANS) {
						if (ASSET_EVEN)
							mask = leftTrnMasksEven[mask];
						else
							mask = leftTrnMasksOdd[mask];
					} else if (tmp & TMIF_LEFT_FOLIAGE) {
						if (ASSET_EVEN)
							mask = leftFolMasksEven[mask];
						else
							mask = leftFolMasksOdd[mask];
					} else {
						mask = DMT_NONE;
					}
#endif /* ASSET_MPL */
						RenderMicro(dst + MICRO_WIDTH * k - j * BUFFER_WIDTH * MICRO_HEIGHT, levelCelBlock, mask);
					}
				}
			}
		}
		pMap += ASSET_MPL;
		if (tmp & TMIF_RIGHT_REDRAW) {
			for (int j = 0; j < ASSET_MPL; j++) {
				for (int k = 0; k < ASSET_MPL; k++) {
					levelCelBlock = pMap[k + j * ASSET_MPL * 2];
					if (levelCelBlock != 0) {
						mask = DMT_NONE;
#if ASSET_MPL == 1
						if (tmp & TMIF_RIGHT_WALL_TRANS) {
							mask = DMT_RTFLOOR; // &RightMask[MICRO_HEIGHT - 1];
						} else if (tmp & TMIF_RIGHT_FOLIAGE) {
							mask = DMT_RFLOOR; // &RightFoliageMask[MICRO_HEIGHT - 1];
						}
#elif ASSET_MPL == 2
						if (tmp & TMIF_RIGHT_WALL_TRANS) {
							static int rightTrnMasksEven[ASSET_MPL][ASSET_MPL] = {
								// clang-format off
								DMT_NONE,    DMT_NONE,
								DMT_RTFLOOR, DMT_FLOOR_TRN_UP_RIGHT,
								// clang-format on
							};
							mask = rightTrnMasksEven[j][k];
						} else if (tmp & TMIF_RIGHT_FOLIAGE) {
							static int rightFolMasksEven[ASSET_MPL][ASSET_MPL] = {
								// clang-format off
								DMT_FLOOR_BOTTOM_RIGHT, DMT_FLOOR_LOW_BOTTOM_RIGHT,
								DMT_FLOOR_TOP_RIGHT,    DMT_FLOOR_UP_TOP_RIGHT,
								// clang-format on
							};
							mask = rightFolMasksEven[j][k];
						}
#elif ASSET_MPL == 3
						if (tmp & TMIF_RIGHT_WALL_TRANS) {
							static int rightTrnMasksOdd[ASSET_MPL][ASSET_MPL] = {
								// clang-format off
								DMT_NONE,    DMT_NONE,               DMT_NONE,
								DMT_NONE,    DMT_NONE,               DMT_RTFLOOR,
								DMT_RTFLOOR, DMT_FLOOR_TRN_UP_RIGHT, DMT_TWALL,
								// clang-format on
							};
							mask = rightTrnMasksOdd[j][k];
						} else if (tmp & TMIF_RIGHT_FOLIAGE) {
							static int rightFolMasksOdd[ASSET_MPL][ASSET_MPL] = {
								// clang-format off
								DMT_FLOOR_BOTTOM_RIGHT, DMT_FLOOR_LOW_BOTTOM_RIGHT, DMT_NONE,
								DMT_EMPTY,              DMT_EMPTY,                  DMT_RFLOOR,
								DMT_FLOOR_TOP_RIGHT,    DMT_FLOOR_UP_TOP_RIGHT,     DMT_NONE,
								// clang-format on
							};
							mask = rightFolMasksOdd[j][k];
						}
#elif ASSET_MPL == 4
						if (tmp & TMIF_RIGHT_WALL_TRANS) {
							static int rightTrnMasksEven[ASSET_MPL][ASSET_MPL] = {
								// clang-format off
								DMT_NONE,    DMT_NONE,               DMT_NONE,    DMT_NONE,
								DMT_NONE,    DMT_NONE,               DMT_NONE,    DMT_NONE,
								DMT_NONE,    DMT_NONE,               DMT_RTFLOOR, DMT_FLOOR_TRN_UP_RIGHT,
								DMT_RTFLOOR, DMT_FLOOR_TRN_UP_RIGHT, DMT_TWALL,   DMT_TWALL,
								// clang-format on
							};
							mask = rightTrnMasksEven[j][k];
						} else if (tmp & TMIF_RIGHT_FOLIAGE) {
							static int rightFolMasksEven[ASSET_MPL][ASSET_MPL] = {
								// clang-format off
								DMT_FLOOR_BOTTOM_RIGHT, DMT_FLOOR_LOW_BOTTOM_RIGHT, DMT_NONE,               DMT_NONE,
								DMT_EMPTY,              DMT_EMPTY,                  DMT_FLOOR_BOTTOM_RIGHT, DMT_FLOOR_LOW_BOTTOM_RIGHT,
								DMT_EMPTY,              DMT_EMPTY,                  DMT_FLOOR_TOP_RIGHT,    DMT_FLOOR_UP_TOP_RIGHT,
								DMT_FLOOR_TOP_RIGHT,    DMT_FLOOR_UP_TOP_RIGHT,     DMT_NONE,               DMT_NONE,
								// clang-format on
							};
							mask = rightFolMasksEven[j][k];
						}
#else
					bool ASSET_EVEN = (ASSET_MPL & 1) == 0;
					static int rightTrnMasksEven[] = {
						// clang-format off
						DMT_NONE, DMT_NONE, DMT_FLOOR_TRN_UP_RIGHT, DMT_RTFLOOR,
						DMT_NONE, DMT_NONE, DMT_NONE /*DMT_RTFLOOR*/, DMT_NONE /*DMT_RTFLOOR*/,
						DMT_TWALL, DMT_TWALL, DMT_TWALL, DMT_TWALL,
						DMT_TWALL, DMT_TWALL, DMT_TWALL, DMT_TWALL,
						DMT_NONE, DMT_NONE/*, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,*/
						// clang-format on
					};
					static int rightTrnMasksOdd[] = {
						// clang-format off
						DMT_NONE, DMT_NONE, DMT_RTFLOOR, DMT_FLOOR_TRN_UP_RIGHT,
						DMT_NONE, DMT_NONE, DMT_RTFLOOR, DMT_RTFLOOR,
						DMT_TWALL, DMT_TWALL, DMT_TWALL, DMT_TWALL,
						DMT_TWALL, DMT_TWALL, DMT_TWALL, DMT_TWALL,
						DMT_NONE, DMT_NONE/*, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,*/
						// clang-format on
					};
					static int rightFolMasksEven[] = {
						// clang-format off
						DMT_EMPTY, DMT_EMPTY, DMT_FLOOR_UP_TOP_RIGHT, DMT_FLOOR_TOP_RIGHT,
						DMT_FLOOR_LOW_BOTTOM_RIGHT, DMT_FLOOR_BOTTOM_RIGHT, DMT_NONE /*DMT_RFLOOR*/, DMT_NONE /*DMT_RFLOOR*/,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE/*, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,*/
						// clang-format on
					};
					static int rightFolMasksOdd[] = {
						// clang-format off
						DMT_EMPTY, DMT_EMPTY, DMT_FLOOR_TOP_RIGHT, DMT_FLOOR_UP_TOP_RIGHT,
						DMT_FLOOR_BOTTOM_RIGHT, DMT_FLOOR_LOW_BOTTOM_RIGHT, DMT_RFLOOR, DMT_RFLOOR,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE/*, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,
						DMT_NONE, DMT_NONE, DMT_NONE, DMT_NONE,*/
						// clang-format on
					};
					mask = ((ASSET_MPL - 1 - k) & 1)                                 // odd
					 | ((j == (ASSET_MPL + (ASSET_MPL - 1 - k)) / 2) << 1)           // onUpperTop
					 | (((j + 1) == (ASSET_MPL - (ASSET_MPL - 1 - k) + 1) / 2) << 2) // onLowerBottom
					 | ((j > (ASSET_MPL + (ASSET_MPL - 1 - k)) / 2) << 3)            // upperTop
					 | (((j + 1) < (ASSET_MPL - (ASSET_MPL - 1 - k) + 1) / 2) << 4); // lowerBottom
					if (tmp & TMIF_RIGHT_WALL_TRANS) {
						if (ASSET_EVEN)
							mask = rightTrnMasksEven[mask];
						else
							mask = rightTrnMasksOdd[mask];
					} else if (tmp & TMIF_RIGHT_FOLIAGE) {
						if (ASSET_EVEN)
							mask = rightFolMasksEven[mask];
						else
							mask = rightFolMasksOdd[mask];
					} else {
						mask = DMT_NONE;
					}
#endif  /* ASSET_MPL */
						RenderMicro(dst + ASSET_MPL * MICRO_WIDTH + MICRO_WIDTH * k - j * BUFFER_WIDTH * MICRO_HEIGHT, levelCelBlock, mask);
					}
				}
			}
		}
		pMap += (TILE_WIDTH / MICRO_WIDTH) * (TILE_HEIGHT / MICRO_HEIGHT) - ASSET_MPL;
		dst -= BUFFER_WIDTH * TILE_HEIGHT;
		i = (TILE_WIDTH / MICRO_WIDTH) * (TILE_HEIGHT / MICRO_HEIGHT); // ASSET_MPL
	}

	mask = (tmp & TMIF_WALL_TRANS) ? DMT_TWALL : DMT_NONE;
	while (i < limit) {
		for (int j = 0; j < (TILE_WIDTH / MICRO_WIDTH); j++) {
			levelCelBlock = *pMap;
			if (levelCelBlock != 0) {
				RenderMicro(dst, levelCelBlock, mask);
			}
			pMap++;
			dst += MICRO_WIDTH;
		}
		i += (TILE_WIDTH / MICRO_WIDTH);
		dst -= TILE_WIDTH + BUFFER_WIDTH * MICRO_HEIGHT;
	}
}

/**
 * @brief Render a special frame of a subtile
 * @param entry the scene entry of the special frame
 */
static void DrawSceneSpecial(const SceneEntry &entry)
{
	int sx = entry.scPosx;
	int sy = entry.scPosy;
	int nCel = entry.scIdx;
	// assert(entry.scIdx == SCT_SPECIAL);
	CelClippedDrawLightTrans(sx, sy, pSpecialsCel, nCel, TILE_WIDTH); // light_trn_index, gbCelTransparencyActive
}

/**
 * @brief add a floor subtile to the scene-array
 * @param pn piece number
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 */
static void scene_addFloorPiece(int pn, int x, int y, int sx, int sy)
{
	scene[numEntries].scType = SCT_FLOOR;
	scene[numEntries].scLight = light_trn_index;
	scene[numEntries].scTrans = FALSE;
	scene[numEntries].scPosx = sx;
	scene[numEntries].scPosy = sy;
	scene[numEntries].scIdx = pn;

	scene[numEntries].scZOrder = SubtileZOrderAt(x, y) | (ZOR_FLOOR << ZOR_SHIFT);
	scene[numEntries].scNext = numEntries + 1;

	numEntries++;
#ifdef DEBUG
	assert(numEntries <= lengthof(scene));
#endif
}

/**
 * @brief Render a floor subtile
 * @param entry the scene entry of the subtile
 */
static void DrawSceneFloor(const SceneEntry &entry)
{
	BYTE *dst, tmp;
	uint16_t levelCelBlock;
	uint16_t* pMap;
	int sx = entry.scPosx;
	int sy = entry.scPosy;
	int pn = entry.scIdx;
	// assert(entry.scIdx == SCT_FLOOR);
	dst = &gpBuffer[BUFFERXY(sx, sy)];

	pMap = &pSubtiles[pn][0];
	tmp = microFlags[pn];

	if ((tmp & (TMIF_LEFT_REDRAW | TMIF_LEFT_FOLIAGE)) != TMIF_LEFT_REDRAW) {
		for (int j = 0; j < ASSET_MPL; j++) {
			for (int i = 0; i < ASSET_MPL; i++) {
				levelCelBlock = pMap[i + ASSET_MPL * 2 * j];
				if (levelCelBlock != 0) {
					RenderMicro(dst + MICRO_WIDTH * i - BUFFER_WIDTH * j * MICRO_HEIGHT, levelCelBlock, DMT_NONE);
				}
			}
		}
	}
	if ((tmp & (TMIF_RIGHT_REDRAW | TMIF_RIGHT_FOLIAGE)) != TMIF_RIGHT_REDRAW) {
		for (int j = 0; j < ASSET_MPL; j++) {
			for (int i = 0; i < ASSET_MPL; i++) {
				levelCelBlock = pMap[i + ASSET_MPL + ASSET_MPL * 2 * j];
				if (levelCelBlock != 0) {
					RenderMicro(dst + ASSET_MPL * MICRO_WIDTH + MICRO_WIDTH * i - BUFFER_WIDTH * j * MICRO_HEIGHT, levelCelBlock, DMT_NONE);
				}
			}
		}
	}
}

/**
 * @brief add an item to the scene-array
 * @param ii id of the item
 * @param lightIdx light index at the item's position
 * @param zorder zorder in the scene
 * @param entry the scene-array entry after which the item should be added
 */
static void scene_addItem(int ii, int lightIdx, unsigned zorder, SceneEntry* entry)
{
	const ItemStruct* is;

	is = &items[ii];

	scene[numEntries].scType = SCT_ITEM;
	scene[numEntries].scLight = lightIdx;
	scene[numEntries].scTrans = FALSE; // gbCelTransparencyActive;
	scene[numEntries].scPosx = is->_igx;
	scene[numEntries].scPosy = is->_igy;
	scene[numEntries].scIdx = ii;

	scene[numEntries].scZOrder = zorder;
	scene[numEntries].scNext = entry->scNext;

	entry->scNext = numEntries;

	numEntries++;
#ifdef DEBUG
	assert(numEntries <= lengthof(scene));
#endif
}

/**
 * @brief Draw item for a given tile
 * @param entry entry of the item
 */
static void DrawSceneItem(const SceneEntry &entry)
{
	int sx = entry.scPosx;
	int sy = entry.scPosy;
	int ii = entry.scIdx;
	// assert(entry.scIdx == SCT_ITEM);
	// assert((unsigned)ii < MAXITEMS);
	const ItemStruct* is = &items[ii];
	int nGfxCel, nAnimCel, nWidth;
	const CelAnimBuf* pCelBuff;

	pCelBuff = is->_iAnimData;
	if (pCelBuff == NULL) {
		dev_fatal("Draw Item \"%d\": NULL Cel Buffer", is->_iIdx);
	}
	nGfxCel = is->_iGfxFrame;
	nAnimCel = is->_iAnimFrame;
#if DEBUG_MODE
	int frames = pCelBuff->caFrameCnt + 1;
	if (nGfxCel > frames) {
		dev_fatal("Draw Item \"%d\" Gfx: frame %d of %d, type %d, curs %d", is->_iIdx, nGfxCel, frames, is->_itype, is->_iCurs);
	}
	if (nAnimCel > frames) {
		dev_fatal("Draw Item \"%d\" Anim: frame %d of %d, type %d, curs %d", is->_iIdx, nAnimCel, frames, is->_itype, is->_iCurs);
	}
#endif
	nWidth = pCelBuff->caWidth;
	sx -= nWidth / 2u;
	if (ii == pcursitem) {
		if (nGfxCel > 0) {
			CelClippedDrawOutline(ICOL_BLUE, sx, sy, reinterpret_cast<const BYTE*>(pCelBuff), nGfxCel, nWidth);
		}
		// if (nAnimCel > 0) {
		//	CelClippedDrawOutline(ICOL_BLUE, sx, sy, reinterpret_cast<const BYTE*>(pCelBuff), nAnimCel, nWidth);
		// }
	}
	if (nGfxCel > 0) {
		CelClippedDrawLightTbl(sx, sy, reinterpret_cast<const BYTE*>(pCelBuff), nGfxCel, nWidth, light_trn_index);
	}
	if (nAnimCel > 0) {
		CelClippedDrawLightTbl(sx, sy, reinterpret_cast<const BYTE*>(pCelBuff), nAnimCel, nWidth, light_trn_index);
	}
}

/**
 * @brief add a special (dungeon) frame to the scene
 * @param bv id of the special frame
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 */
static void scene_addSpecialCell(BYTE bv, int x, int y, int sx, int sy)
{
	scene[numEntries].scType = SCT_SPECIAL;
	scene[numEntries].scLight = light_trn_index;
	scene[numEntries].scTrans = gbCelTransparencyActive;
	scene[numEntries].scPosx = sx;
	scene[numEntries].scPosy = sy;
	scene[numEntries].scIdx = bv;

	scene[numEntries].scZOrder = SubtileZOrderAt(x, y) | (ZOR_SPECIAL_CELL << ZOR_SHIFT);
	scene[numEntries].scNext = numEntries + 1;

	numEntries++;
#ifdef DEBUG
	assert(numEntries <= lengthof(scene));
#endif
}

/**
 * @brief add a dungeon subtile (and special frame) to the scene
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 */
static void scene_addDungeon(int x, int y, int sx, int sy)
{
	int mpnum;
	BYTE bv;

	assert((unsigned)x < MAXDUNX);
	assert((unsigned)y < MAXDUNY);

	light_trn_index = dLight[x][y];
	gbCelTransparencyActive = TransList[dTransVal[x][y]];

	mpnum = dPiece[x][y];
	scene_addCell(mpnum, x, y, sx, sy);

	bv = nSpecTrapTable[dPiece[x][y]] & PST_SPEC_TYPE;
	if (bv != 0) {
		scene_addSpecialCell(bv, x, y, sx, sy);
	}
}

/**
 * @brief add a row of tiles to the scene
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param mode the direction to go after the end on the first line (0: SW [0,1], 1" SE [1,0])
 */
static void scene_addFloor(int x, int y, int sx, int sy, int mode)
{
	//int pn;
	const int rightEnd = SCREEN_X + (gbZoomInFlag ? SCREEN_WIDTH / 2 : SCREEN_WIDTH);
	const int bottomEnd = SCREEN_Y + (gbZoomInFlag ? SCREEN_HEIGHT / 2 : SCREEN_HEIGHT) + TILE_HEIGHT - 1;
	int i = 0;
	do {
		int cx = sx, cy = sy;
		int xx = x, yy = y;
		do {
			if (IN_DUNGEON_AREA(xx, yy)) {
				//pn = dPiece[xx][yy];
				//assert(pn != 0);
				//if (pn != 0) {
					//if ((microFlags[pn] & (~(TMIF_WALL_TRANS))) != (TMIF_LEFT_REDRAW | TMIF_RIGHT_REDRAW))
						light_trn_index = dLight[xx][yy];
						scene_addFloorPiece(dPiece[xx][yy], xx, yy, cx, cy);
					//}
				//} else {
				//	world_draw_black_tile(cx, cy);
				//}
			//} else {
			//	world_draw_black_tile(cx, cy);
			}
			SHIFT_GRID(xx, yy, 1, 0);
			cx += TILE_WIDTH;
		} while (cx < rightEnd);
		// Jump to next row
		sy += TILE_HEIGHT / 2;
		if ((i & 1) != mode) {
			x++;
			sx += TILE_WIDTH / 2;
		} else {
			y++;
			sx -= TILE_WIDTH / 2;
		}
		i++;
	} while (sy < bottomEnd);
}

/**
 * @brief add dungeon subtiles and special frames to the scene
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param mode the direction to go after the end on the first line (0: SW [0,1], 1" SE [1,0])
 */
static void scene_addEntries(int x, int y, int sx, int sy, int mode)
{
	int i = 0;
	BYTE skips = 0;
	const int rightEnd = SCREEN_X + (gbZoomInFlag ? SCREEN_WIDTH / 2 : SCREEN_WIDTH);
	const int bottomEnd = SCREEN_Y + (gbZoomInFlag ? SCREEN_HEIGHT / 2 : SCREEN_HEIGHT) + TILE_HEIGHT - 1 + TILE_HEIGHT * ((unsigned)MicroTileLen / ((TILE_WIDTH / MICRO_WIDTH) * (TILE_HEIGHT / MICRO_HEIGHT)) - 1);

	do {
		int cx = sx, cy = sy;
		int xx = x, yy = y;
		do {
			if (IN_DUNGEON_AREA(xx, yy)) {
				assert(dPiece[xx][yy] != 0);
				if (/*dPiece[xx][yy] != 0 &&*/ !(skips & 1)) {
					scene_addDungeon(xx, yy, cx, cy);
				}
			}
			SHIFT_GRID(xx, yy, 1, 0);
			cx += TILE_WIDTH;
			skips >>= 1;
		} while (cx < rightEnd);
		// Jump to next row
		sy += TILE_HEIGHT / 2;
		if ((i & 1) != mode) {
			x++;
			sx += TILE_WIDTH / 2;
		} else {
			y++;
			sx -= TILE_WIDTH / 2;
		}
		i++;
	} while (sy < bottomEnd);
}

unsigned bSearchLess(unsigned From, unsigned To, unsigned zorder)
{
	if (From == To) return From;
	assert(To > From);
	unsigned curr = (From + To) / 2;
	assert(curr < numEntries);
	if (scene[curr].scZOrder < zorder) {
		return bSearchLess(curr + 1, To, zorder);
	} else {
		return bSearchLess(From, curr, zorder);
	}
}

static SceneEntry* scene_placeEntry(const RECT_AREA32 &vArea, const POS32 &pos, int width, unsigned numCells, unsigned numStaticEntries, unsigned zorder)
{
	const int height = 2 * 160 * ASSET_MPL * (DUN_WIDTH / TILE_HEIGHT);
	width = width /* / 2 * 2 */ * (DUN_WIDTH / TILE_WIDTH);
	POS32 sp;
	sp.x = pos.x - pos.y;
	sp.y = pos.x + pos.y;
	sp.x *= ASSET_MPL;
	sp.y *= ASSET_MPL;
	sp.y += 2 * (TILE_HEIGHT / 2) * (DUN_WIDTH / (TILE_WIDTH / ASSET_MPL));
	const bool inView = POS_IN_AREA(sp.x, sp.y, vArea.x1 - width + 1, vArea.y1, vArea.x2 + width - 1, vArea.y2 + height - 1);
	if (!inView)
		return NULL;
	unsigned idx = bSearchLess(numCells, numStaticEntries, zorder);
	// assert(idx >= numCells);
	// assert(numCells != 0);
	SceneEntry* cellEntry = &scene[idx - 1];
	while (true) {
		unsigned nextIdx = cellEntry->scNext;
		SceneEntry* nextEntry = &scene[nextIdx];
		if (nextEntry->scZOrder > zorder)
			break;
		cellEntry = nextEntry;
	}
	return cellEntry;
}

void scene_insertEntries(unsigned numCells)
{
	// add CORK to simplify the insertion
	scene[numEntries].scType = SCT_DUMMY;
	scene[numEntries].scZOrder = UINT_MAX;
	numEntries++;

	const unsigned numStaticEntries = numEntries;
	RECT_AREA32 area;
	POS32 cp = myview.dun;
	int sx, sy, w, h;

	w = 2 * (SCREEN_WIDTH / (gbZoomInFlag ? 4 : 2)) * (DUN_WIDTH / (TILE_WIDTH / ASSET_MPL));
	h = 2 * (SCREEN_HEIGHT / (gbZoomInFlag ? 4 : 2)) * (DUN_WIDTH / (TILE_HEIGHT / ASSET_MPL));
	sx = cp.x - cp.y;
	sy = cp.x + cp.y;
	sx *= ASSET_MPL;
	sy *= ASSET_MPL;
	//sy += 2 * TILE_HEIGHT * (DUN_WIDTH / (TILE_WIDTH / ASSET_MPL));
	//if (gbZoomInFlag)
	//	sy += 2 * (TILE_HEIGHT / 4) * (DUN_WIDTH / (TILE_WIDTH / ASSET_MPL));
	area.x1 = sx - w;
	area.y1 = sy - h;
	area.x2 = sx + w;
	area.y2 = sy + h;
	// insert objects
	for (int oi = 0; oi < numobjects; ++oi) {
		// int mi = objectactive[i];
		ObjectStruct* os = &objects[oi];
		if (dObject[os->_ox][os->_oy] != oi + 1) continue;
		const unsigned zorder = SubtileZOrder(os->_opos) | (((!os->_oPreFlag) ? ZOR_OBJECT : ZOR_PRE_OBJECT) << ZOR_SHIFT) | OffsetZOrder(os->_opos);
		SceneEntry* entry = scene_placeEntry(area, os->_opos, os->_oAnimWidth, numCells, numStaticEntries, zorder);
		if (entry == NULL) continue;
		int lightIdx = dLight[(unsigned)os->_opos.x / DUN_WIDTH][(unsigned)os->_opos.y / DUN_WIDTH];
		scene_addObject(oi, lightIdx, zorder, entry);
	}
	// insert items
	for (int i = 0; i < numitems; i++) {
		int ii = itemactive[i];
		const ItemStruct* is = &items[ii];
		const unsigned zorder = SubtileZOrder(is->_ipos) | (/*is->_iPostDraw ? ZOR_ITEM : */ZOR_PRE_ITEM << ZOR_SHIFT) | OffsetZOrder(is->_ipos);
		SceneEntry* entry = scene_placeEntry(area, is->_ipos, is->_iAnimData->caWidth, numCells, numStaticEntries, zorder);
		if (entry == NULL) continue;
		int lightIdx = dLight[(unsigned)is->_ipos.x / DUN_WIDTH][(unsigned)is->_ipos.y / DUN_WIDTH];
		scene_addItem(ii, lightIdx, zorder, entry);
	}
	// insert monsters
	for (int mnum = 0; mnum < MAXMONSTERS; mnum++) {
		const MonsterStruct* mon = &monsters[mnum];
		if (mon->_mmode > MM_INGAME_LAST && mon->_mmode != MM_DEAD) continue;
		if (mon->_mmode == MM_CHARGE) continue;
		const unsigned zorder = SubtileZOrder(mon->_mpos) | ((mon->_mmode != MM_DEAD ? ZOR_MONSTER : ZOR_DEAD_MONSTER) << ZOR_SHIFT) | OffsetZOrder(mon->_mpos);
		SceneEntry* entry = scene_placeEntry(area, mon->_mpos, mon->_mAnimWidth, numCells, numStaticEntries, zorder);
		if (entry == NULL) continue;
		BYTE bFlag = dFlags[(unsigned)mon->_mpos.x / DUN_WIDTH][(unsigned)mon->_mpos.y / DUN_WIDTH];
		int lightIdx = dLight[(unsigned)mon->_mpos.x / DUN_WIDTH][(unsigned)mon->_mpos.y / DUN_WIDTH];
		if (mon->_mmode != MM_DEAD) {
			scene_addMonster(mnum, bFlag, lightIdx, zorder, entry);
		} else {
			scene_addDeadMonsterEntry(mnum, bFlag, lightIdx, zorder, entry);
		}
	}
	// insert towners
	for (int mnum = MAX_MINIONS; mnum < numtowners; mnum++) {
		const MonsterStruct* mon = &monsters[mnum];
		// if (mon->_mmode > MM_INGAME_LAST && mon->_mmode != MM_DEAD) continue;
		const unsigned zorder = SubtileZOrder(mon->_mpos) | (ZOR_MONSTER << ZOR_SHIFT) | OffsetZOrder(mon->_mpos);
		SceneEntry* entry = scene_placeEntry(area, mon->_mpos, mon->_mAnimWidth, numCells, numStaticEntries, zorder);
		if (entry == NULL) continue;
		int lightIdx = dLight[(unsigned)mon->_mpos.x / DUN_WIDTH][(unsigned)mon->_mpos.y / DUN_WIDTH];
		scene_addTowner(mnum, lightIdx, zorder, entry);
	}
	// insert players
	for (int pnum = 0; pnum < MAX_PLRS; pnum++) {
		if (!plr._pActive || plr._pLvlChanging || plr._pDunLevel != currLvl._dLevelIdx) continue;
		if (plr._pmode == PM_CHARGE) continue;
		const unsigned zorder = SubtileZOrder(plr._ppos) | (((plr._pHitPoints != 0) ? ZOR_PLAYER : ZOR_DEAD_PLAYER) << ZOR_SHIFT) | OffsetZOrder(plr._ppos);
		SceneEntry* entry = scene_placeEntry(area, plr._ppos, plr._pAnimWidth, numCells, numStaticEntries, zorder);
		if (entry == NULL) continue;
		BYTE bFlag = dFlags[(unsigned)plr._ppos.x / DUN_WIDTH][(unsigned)plr._ppos.y / DUN_WIDTH];
		int lightIdx = dLight[(unsigned)plr._ppos.x / DUN_WIDTH][(unsigned)plr._ppos.y / DUN_WIDTH];
		scene_addPlayer(pnum, bFlag, lightIdx, zorder, entry);
	}
	// insert missiles
	for (int i = 0; i < nummissiles; i++) {
		int mi = missileactive[i];
		MissileStruct* mis = &missile[mi];
		if (!mis->_miDrawFlag) continue;
		const unsigned zorder = SubtileZOrderAt(mis->_mix, mis->_miy) | (((!mis->_miPreFlag) ? ZOR_MISSILE : ZOR_PRE_MISSILE) << ZOR_SHIFT) | OffsetZOrder(mis->_mipos);
		SceneEntry* entry = scene_placeEntry(area, mis->_mipos, mis->_miAnimWidth, numCells, numStaticEntries, zorder);
		if (entry == NULL) continue;
		int lightIdx = dLight[(unsigned)mis->_mipos.x / DUN_WIDTH][(unsigned)mis->_mipos.y / DUN_WIDTH];
		scene_addMissile(mi, lightIdx, zorder, entry);
	}
}

/**
 * @brief Scale up the rendered part of the back buffer to take up the full view
 */
static void Zoom()
{
	int wdt = SCREEN_WIDTH / 2u;
	BYTE* src = &gpBuffer[SCREENXY(SCREEN_WIDTH / 2u - 1, SCREEN_HEIGHT / 2u - 1)];
	BYTE* dst = &gpBuffer[SCREENXY(SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1)];

	for (unsigned hgt = 0; hgt < SCREEN_HEIGHT / 2u; hgt++) {
		for (int i = 0; i < wdt; i++) {
			*dst-- = *src;
			*dst-- = *src;
			src--;
		}
		memcpy(dst - BUFFER_WIDTH, dst, wdt * 2 + 1);
		src -= BUFFER_WIDTH - wdt;
		dst -= 2 * (BUFFER_WIDTH - wdt);
	}
}

/**
 * @brief Calculate the offset needed for centering tiles in view area
 * @param offsetX Offset in pixels
 * @param offsetY Offset in pixels
 */
static void CalcTileOffset(int* offsetX, int* offsetY)
{
	unsigned x, y;

	x = SCREEN_WIDTH;
	y = SCREEN_HEIGHT;
	if (gbZoomInFlag) {
		x >>= 1;
		y >>= 1;
	}
	x %= TILE_WIDTH;
	y %= TILE_HEIGHT;

	x = ((TILE_WIDTH - x) % TILE_WIDTH) >> 1;
	y = ((TILE_HEIGHT - y) % TILE_HEIGHT) >> 1;

	*offsetX = x;
	*offsetY = y;
}

/**
 * @brief Calculate the needed diamond tile to cover the view area
 * @param rcolumns Tiles needed per row
 * @param rrows Both even and odd rows
 */
static void TilesInView(unsigned* rcolumns, unsigned* rrows)
{
	unsigned columns = (unsigned)(SCREEN_WIDTH - 1) / TILE_WIDTH + 1;
	unsigned rows = (unsigned)(SCREEN_HEIGHT - 1) / TILE_HEIGHT + 1;

	if (gbZoomInFlag) {
		// Half the number of tiles, rounded up
		columns++;
		columns >>= 1;
		rows++;
		rows >>= 1;
	}

	*rcolumns = columns;
	*rrows = rows;
}

static void CalcTileViewport()
{
	int xo, yo;

	// Adjust by player offset and tile grid alignment
	CalcTileOffset(&xo, &yo);
	gsTileVp._vOffsetX = xo - SCREEN_X;
	gsTileVp._vOffsetY = yo - (SCREEN_Y + TILE_HEIGHT / 2 - 1);

	TilesInView(&gsTileVp._vColumns, &gsTileVp._vRows);
	unsigned lrow = gsTileVp._vRows;

	// Center player tile on screen
	SET_GRID(gsTileVp._vShiftX, gsTileVp._vShiftY, 0 - (gsTileVp._vColumns / 2), 0 - (lrow / 2));

	gsTileVp._vRows *= 2;

	// Align grid
	if ((gsTileVp._vColumns & 1) == 0) {
		gsTileVp._vShiftY--; // Shift player row to one that can be centered without pixel offset
		if ((lrow & 1) == 0) {
			// Offset tile to vertically align the player when both rows and colums are even
			gsTileVp._vRows++;
			gsTileVp._vOffsetY += TILE_HEIGHT / 2;
		}
	} else if (/*(gsTileVp._vColumns & 1) &&*/ (lrow & 1)) {
		// Offset tile to vertically align the player when both rows and colums are odd
		SHIFT_GRID(gsTileVp._vShiftX, gsTileVp._vShiftY, 0, -1);
		gsTileVp._vRows++;
		gsTileVp._vOffsetY += TILE_HEIGHT / 2;
	}

	// Slightly lower the zoomed view
	if (gbZoomInFlag) {
		gsTileVp._vOffsetY -= TILE_HEIGHT / 4;
		if (yo < TILE_HEIGHT / 4)
			gsTileVp._vRows++;
	}

	gsTileVp._vRows++; // Cover lower edge saw tooth, right edge accounted for in scrollrt_draw()
}

static void CalcMouseViewport()
{
	// Adjust by player offset and tile grid alignment
	CalcTileOffset(&gsMouseVp._vOffsetX, &gsMouseVp._vOffsetY);

	// Convert to tile grid
	TilesInView(&gsMouseVp._vColumns, &gsMouseVp._vRows);
	unsigned lrow = gsMouseVp._vRows;

	// Center player tile on screen
	SET_GRID(gsMouseVp._vShiftX, gsMouseVp._vShiftY, 0 - (gsMouseVp._vColumns / 2), 0 - (lrow / 2));

	// Align grid
	if ((gsMouseVp._vColumns & 1) == 0) {
		if ((lrow & 1) == 0) {
			gsMouseVp._vOffsetY += TILE_HEIGHT / 2;
		}
	} else if (/*gsMouseVp._vColumns & 1 &&*/ lrow & 1) {
		gsMouseVp._vOffsetX -= TILE_WIDTH / 2;
	} else /*if (gsMouseVp._vColumns & 1 && (lrow & 1) == 0)*/ {
		gsMouseVp._vShiftY++;
	}

	if (gbZoomInFlag) {
		gsMouseVp._vOffsetY -= TILE_HEIGHT / 4;
	}
}

void CalcViewportGeometry()
{
	CalcTileViewport();
	CalcMouseViewport();
}

/**
 * @brief Configure render and process screen rows
 */
static void CreateScene()
{
	int x, y, sx, sy;

	const POS32 vp = myview.dun;
	const POS32 dso = DunScreenOffset(vp.x / DUN_WIDTH, vp.y / DUN_WIDTH, vp.x, vp.y);
	POS32 sp = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
	if (gbZoomInFlag) {
		sp.x /= 2u;
		sp.y /= 2u;
	}
	// Adjust by player offset and tile grid alignment
	sp.x -= dso.x + TILE_WIDTH / 2;
	sp.y -= dso.y + TILE_HEIGHT / 2;

	// Slightly lower the view
	sp.y += TILE_HEIGHT - 1;
	if (gbZoomInFlag) {
		sp.y += TILE_HEIGHT / 4;
	}
	// Find the subtile on the top-left corner
	// assert(sp.x > 0);
	// assert(sp.y > 0);
	// - subtiles to the left
	int dsx = (unsigned)(sp.x + TILE_WIDTH - 1) / TILE_WIDTH;
	// - subtile to the top
	int dsy = (unsigned)(sp.y - 0) / TILE_HEIGHT;
	// - calculate the delta to the top-left corner
	POS32 dt;
	SET_GRID(dt.x, dt.y, -dsx, -dsy);
	// - move to the starting subtile (screen coordinates)
	POS32 gp = sp;
	gp.x += dt.x * TILE_WIDTH / 2 - dt.y * TILE_WIDTH / 2;
	gp.y += dt.x * TILE_HEIGHT / 2 + dt.y * TILE_HEIGHT / 2;

	int mode;
	if (gp.y > TILE_HEIGHT / 2) {
		// missing pixels on the top
		if (gp.x > -TILE_WIDTH / 2) {
			// missing pixels on the top-left corner
			dt.x--;
			gp.x -= TILE_WIDTH / 2;
			gp.y -= TILE_HEIGHT / 2;
			mode = 1;
		} else {
			dt.y--;
			gp.x += TILE_WIDTH / 2;
			gp.y -= TILE_HEIGHT / 2;
			mode = 0;
		}
	} else {
		if (gp.x > -TILE_WIDTH / 2) {
			// missing pixels on the left side
			mode = 0;
		} else {
			mode = 1;
		}
	}
	// - move to the starting subtile (tile coordinates)
	x = vp.x / DUN_WIDTH + dt.x;
	y = vp.y / DUN_WIDTH + dt.y;

	// - adjust for the border (screen coordinates)
	sx = SCREEN_X + gp.x;
	sy = SCREEN_Y + gp.y;

	numEntries = 0;
	scene_addFloor(x, y, sx, sy, mode);

	const unsigned numCells = numEntries;
	scene_addEntries(x, y, sx, sy, mode);

	scene_insertEntries(numCells);

	// shift positions from grid to screen
	POS32 dp = DungeonToGridPos(x, y);

	dp = GridToScreen(dp.x, dp.y);

	int shx = (sx - dp.x);
	int shy = (sy - dp.y);

	for (unsigned i = 0; i < numEntries; i++) {
		SceneEntry* entry = &scene[i];

		switch (entry->scType) {
		case SCT_FLOOR:        break;
		case SCT_CELL:         break;
		case SCT_ITEM:
		case SCT_OBJECT:
		case SCT_MISSILE:
		case SCT_TOWNER:
		case SCT_MONSTER:
		case SCT_DEAD_MONSTER:
		case SCT_PLAYER:
		case SCT_DEAD_PLAYER: {
			POS32 sp = GridToScreen(entry->scPosx, entry->scPosy);

			entry->scPosx = sp.x + shx + TILE_WIDTH / 2;
			entry->scPosy = sp.y + shy;
		} break;
		case SCT_SPECIAL:      break;
		case SCT_DUMMY:        break;
		default: ASSUME_UNREACHABLE
		}
	}
}

static void DrawScene()
{
	ClearScreenBuffer();
	// Limit rendering to the view area
	//if (!gbZoomInFlag)
	//	gpBufEnd = &gpBuffer[SCREENXY(0, SCREEN_HEIGHT)];
	//else
	//	gpBufEnd = &gpBuffer[SCREENXY(0, SCREEN_HEIGHT / 2)];

	const SceneEntry *entry = &scene[0];
	for (unsigned i = 0; i < numEntries; i++) {
		gbCelTransparencyActive = entry->scTrans;
		light_trn_index = entry->scLight;
		switch (entry->scType) {
		case SCT_FLOOR:        DrawSceneFloor(*entry);       break; // light
		case SCT_CELL:         DrawSceneCell(*entry);        break; // light, transp
		case SCT_ITEM:         DrawSceneItem(*entry);        break; // light
		case SCT_OBJECT:       DrawSceneObject(*entry);      break; // light
		case SCT_MISSILE:      DrawSceneMissile(*entry);     break;
		case SCT_TOWNER:       DrawSceneTowner(*entry);      break;
		case SCT_MONSTER:
		case SCT_DEAD_MONSTER: DrawSceneMonster(*entry);     break;
		case SCT_PLAYER:
		case SCT_DEAD_PLAYER:  DrawScenePlayer(*entry);      break;
		case SCT_SPECIAL:      DrawSceneSpecial(*entry);     break; // light, transp
		case SCT_DUMMY:                                      break;
		default: ASSUME_UNREACHABLE
		}
		entry = &scene[entry->scNext];
	}
	// Allow rendering to the whole screen
	//gpBufEnd = &gpBuffer[SCREENXY(0, SCREEN_HEIGHT)];

	if (gbZoomInFlag) {
		Zoom();
	}
}

static void DrawPause()
{
	int x, light;

	// assert(GetHugeStringWidth("Pause") == 135);
	x = SCREEN_CENTERX(135);
	static_assert(MAXDARKNESS >= 4, "Blinking pause uses too many shades.");
	light = (SDL_GetTicks() / 256) % 4;
	PrintHugeString(x, SCREEN_CENTERY(TILE_HEIGHT * 4), "Pause", COL_GOLD + light);
}

/**
 * @brief Start rendering of screen, town variation
 */
static void DrawView()
{
	CreateScene();
	DrawScene();
	if (gbAutomapflag != AMM_NONE) {
		DrawAutomap();
	}
	DrawLifeFlask();
	DrawManaFlask();
	DrawGolemBar();
	//if (gbRedrawFlags & (REDRAW_RECALC_MANA | REDRAW_SPELL_ICON)) {
		DrawSkillIcons();
	//}
	DrawDurIcon();

	//if (gbRedrawFlags & REDRAW_SPEED_BAR) {
		DrawInvBelt();
	//}

	DrawPlrMsg(false);

	if (gbQtextflag) {
		DrawQText();
	} else if (stextflag != STORE_NONE) {
		DrawStore();
	} else {
		if (gbLvlUp) {
			DrawLevelUpIcon();
		}
		for (int i = 0; i < gnNumActiveWindows; i++) {
			switch (gaActiveWindows[i]) {
			case WND_INV:   DrawInv();       break;
			case WND_CHAR:  DrawChr();       break;
			case WND_BOOK:  DrawSpellBook(); break;
			case WND_TEAM:  DrawTeamBook();  break;
			case WND_QUEST: DrawQuestLog();  break;
			default: ASSUME_UNREACHABLE;     break;
			}
		}
		if (INVIDX_VALID(gbDropGoldIndex)) {
			DrawGoldSplit();
		}
		if (gbSkillListFlag) {
			DrawSkillList();
		}
		if (gbCampaignMapFlag != CMAP_NONE) {
			DrawCampaignMap();
		}
		if (gnVisibleHelpLines != 0) {
			DrawHelp();
		}
	}
	if (gbShowTooltip || (gbModBtnDown & ACTBTN_MASK(ACT_MODCTX))) {
		DrawInfoStr();
	}
	if (currmsg != EMSG_NONE) {
		DrawDiabloMsg();
	}
	if (gbDeathflag == MDM_DEAD) {
		RedBack();
	} else if (gnGamePaused != 0 && !gmenu_is_active()) {
		DrawPause();
	}

	//if (gbDoomflag)
	//	doom_draw();
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
	DrawControllerModifierHints();
#endif
#if HAS_TOUCHPAD
	DrawGamepad();
#endif
	DrawPlrMsg(true);
	if (gmenu_is_active())
		gamemenu_draw();
}

#if DEBUG_MODE
/**
 * @brief Scroll the screen when mouse is close to the edge
 */
void ScrollView()
{
	bool scroll;

	if (pcursicon >= CURSOR_FIRSTITEM)
		return;

	scroll = false;

	if (MousePos.x < 20) {
		if (DSIZEY + DBORDERY - 1 <= myview.subtile.y || DBORDERX >= myview.subtile.x) {
			if (DSIZEY + DBORDERY - 1 > myview.subtile.y) {
				myview.subtile.y++;
				scroll = true;
			}
			if (DBORDERX < myview.subtile.x) {
				myview.subtile.x--;
				scroll = true;
			}
		} else {
			myview.subtile.y++;
			myview.subtile.x--;
			scroll = true;
		}
	}
	if (MousePos.x > SCREEN_WIDTH - 20) {
		if (DSIZEX + DBORDERX - 1 <= myview.subtile.x || DBORDERY >= myview.subtile.y) {
			if (DSIZEX + DBORDERX - 1 > myview.subtile.x) {
				myview.subtile.x++;
				scroll = true;
			}
			if (DBORDERY < myview.subtile.y) {
				myview.subtile.y--;
				scroll = true;
			}
		} else {
			myview.subtile.y--;
			myview.subtile.x++;
			scroll = true;
		}
	}
	if (MousePos.y < 20) {
		if (DBORDERY >= myview.subtile.y || DBORDERX >= myview.subtile.x) {
			if (DBORDERY < myview.subtile.y) {
				myview.subtile.y--;
				scroll = true;
			}
			if (DBORDERX < myview.subtile.x) {
				myview.subtile.x--;
				scroll = true;
			}
		} else {
			myview.subtile.x--;
			myview.subtile.y--;
			scroll = true;
		}
	}
	if (MousePos.y > SCREEN_HEIGHT - 20) {
		if (DSIZEY + DBORDERY - 1 <= myview.subtile.y || DSIZEX + DBORDERX - 1 <= myview.subtile.x) {
			if (DSIZEY + DBORDERY - 1 > myview.subtile.y) {
				myview.subtile.y++;
				scroll = true;
			}
			if (DSIZEX + DBORDERX - 1 > myview.subtile.x) {
				myview.subtile.x++;
				scroll = true;
			}
		} else {
			myview.subtile.x++;
			myview.subtile.y++;
			scroll = true;
		}
	}

//	if (scroll)
//		ScrollInfo._sdir = SDIR_NONE;
}

/**
 * @brief Display the current average FPS over 1 sec
 */
static void DrawFPS()
{
	Uint32 currTc, deltaTc;

	guFrameCnt++;
	currTc = SDL_GetTicks();
	deltaTc = currTc - guFpsStartTc;
	if ((Sint32)deltaTc >= 1000) {
		guFpsStartTc = currTc;
		guFrameRate = 1000 * guFrameCnt / deltaTc;
		guFrameCnt = 0;
	}
	snprintf(tempstr, sizeof(tempstr), "%d FPS", guFrameRate);
	PrintGameStr(SCREEN_X + 8, SCREEN_Y + 65, tempstr, COL_RED);
}
#endif // DEBUG_MODE

/**
 * @brief Redraw screen
 * @param draw_cursor
 */
void scrollrt_render_screen(bool draw_cursor)
{
	if (gbWndActive) {
		if (draw_cursor) {
			lock_buf(0);
			scrollrt_draw_cursor();
			unlock_buf(0);
		}

		BltFast();

		if (draw_cursor) {
			lock_buf(0);
			scrollrt_remove_back_buffer_cursor();
			unlock_buf(0);
		}
	}
	RenderPresent();
}

/**
 * @brief Render the game
 */
void scrollrt_render_game()
{
	if (gbWndActive) {
		lock_buf(0);
		DrawView();
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
		if (plrctrls_draw_cursor())
#endif
			scrollrt_draw_cursor();

#if DEBUG_MODE
		DrawFPS();
#endif

		unlock_buf(0);

		BltFast();

		lock_buf(0);
		scrollrt_remove_back_buffer_cursor();
		unlock_buf(0);
		gbRedrawFlags = 0;
	}
	RenderPresent();
}

DEVILUTION_END_NAMESPACE
