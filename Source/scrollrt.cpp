/**
 * @file scrollrt.cpp
 *
 * Implementation of functionality for rendering the dungeons, monsters and calling other render routines.
 */
#include "all.h"
#include "utils/display.h"
#include "plrctrls.h"
#include "engine/render/cl2_render.h"
#include "engine/render/dun_render.h"

DEVILUTION_BEGIN_NAMESPACE

/** Specifies the player viewpoint X-coordinate of the map. */
int ViewX;
/** Specifies the player viewpoint Y-coordinate of the map. */
int ViewY;
ScrollStruct ScrollInfo;

/**
  * Container to hold the cached properties of the viewport.
  *
  * _vColumns: the number of columns to draw to ensure the screen is covered. 
  * _vRows: the number of rows to draw to ensure the screen is covered.
  * _vOffsetX: the base X-offset to draw the tiles in the back buffer.
  * _vOffsetY: the base Y-offset to draw the tiles in the back buffer.
  * _vShiftX: the base offset to ViewX.
  * _vShiftY: the base offset to ViewY.
*/
ViewportStruct gsTileVp;

/**
 * Specifies the current light entry.
 */
int light_trn_index;

/**
 * Specifies the current draw mode.
 */
static BOOLEAN gbPreFlag;

/**
 * Cursor-size
 */
int sgCursHgt;
int sgCursWdt;
int sgCursHgtOld;
int sgCursWdtOld;

/**
 * Cursor-position
 */
int sgCursX;
int sgCursY;
int sgCursXOld;
int sgCursYOld;

/**
 * Specifies whether transparency is active for the current CEL file being decoded.
 */
bool gbCelTransparencyActive;
void (*DrawPlrProc)(int, int, int, int, int, BYTE*, int, int, int, int);
/**
 * Buffer to store the cursor image.
 */
BYTE sgSaveBack[MAX_CURSOR_AREA];

//bool dRendered[MAXDUNX][MAXDUNY];
static unsigned guFrameCnt;
static unsigned guFrameRate;
static Uint32 guFpsStartTc;

#if DEBUG_MODE
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

/**
 * @brief Clear cursor state
 */
void ClearCursor() // CODE_FIX: this was supposed to be in cursor.cpp
{
	sgCursWdt = 0;
	sgCursWdtOld = 0;
}

/**
 * @brief Remove the cursor from the back buffer
 */
static void scrollrt_remove_back_buffer_cursor()
{
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

	sgCursXOld = sgCursX;
	sgCursYOld = sgCursY;
	sgCursWdtOld = sgCursWdt;
	sgCursHgtOld = sgCursHgt;
	sgCursWdt = 0;
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
	int i, mx, my, frame;
	BYTE *src, *dst, *cCels;

	assert(sgCursWdt == 0);

	if (pcursicon <= CURSOR_NONE) {
		return;
	}
	assert(cursW != 0 && cursH != 0);

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
	if (sgbControllerActive && !IsMovingMouseCursorWithController() && pcursicon != CURSOR_TELEPORT
	 && (gnNumActiveWindows == 0 || (gaActiveWindows[gnNumActiveWindows - 1] != WND_INV && (gaActiveWindows[gnNumActiveWindows - 1] != WND_CHAR || !gbLvlUp))))
		return;
#endif

	mx = MousePos.x - 1;
	if (mx < 0 - cursW - 1) {
		return;
	}
	if (mx > SCREEN_WIDTH - 1) {
		return;
	}
	my = MousePos.y - 1;
	if (my < 0 - cursH - 1) {
		return;
	}
	if (my > SCREEN_HEIGHT - 1) {
		return;
	}

	sgCursX = mx;
	sgCursWdt = sgCursX + cursW + 1;
	if (sgCursWdt > SCREEN_WIDTH - 1) {
		sgCursWdt = SCREEN_WIDTH - 1;
	}
	sgCursX &= ~3;
	sgCursWdt |= 3;
	sgCursWdt -= sgCursX;
	sgCursWdt++;

	sgCursY = my;
	sgCursHgt = sgCursY + cursH + 1;
	if (sgCursHgt > SCREEN_HEIGHT - 1) {
		sgCursHgt = SCREEN_HEIGHT - 1;
	}
	sgCursHgt -= sgCursY;
	sgCursHgt++;

	assert((unsigned)(sgCursWdt * sgCursHgt) <= sizeof(sgSaveBack));
	assert(gpBuffer != NULL);
	dst = sgSaveBack;
	src = &gpBuffer[SCREENXY(sgCursX, sgCursY)];

	for (i = sgCursHgt; i != 0; i--, dst += sgCursWdt, src += BUFFER_WIDTH) {
		memcpy(dst, src, sgCursWdt);
	}

	mx++;
	mx += SCREEN_X;
	my++;
	my += cursH + SCREEN_Y - 1;

	frame = pcursicon;
	cCels = pCursCels;
	if (frame < CURSOR_FIRSTITEM) {
		CelClippedDrawLightTbl(mx, my, cCels, frame, cursW, 0);
	} else {
		scrollrt_draw_item(&myplr._pHoldItem, true, mx, my, cCels, frame, cursW);
	}
}

/**
 * @brief Render a missile sprite
 * @param mis Pointer to MissileStruct struct
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 */
static void DrawMissilePrivate(MissileStruct* mis, int sx, int sy)
{
	int mx, my, nCel, frames, nWidth;
	BYTE trans;
	BYTE* pCelBuff;

	if (mis->_miPreFlag != gbPreFlag || !mis->_miDrawFlag)
		return;

	mx = sx + mis->_mixoff - mis->_miAnimXOffset;
	my = sy + mis->_miyoff;
	pCelBuff = mis->_miAnimData;
	if (pCelBuff == NULL) {
		dev_fatal("Draw Missile type %d: NULL Cel Buffer", mis->_miType);
	}
	nCel = mis->_miAnimFrame;
	frames = SwapLE32(*(uint32_t*)pCelBuff);
	if (nCel < 1 || frames > 50 || nCel > frames) {
		dev_fatal("Draw Missile frame %d of %d, type %d", nCel, frames, mis->_miType);
	}
	nWidth = mis->_miAnimWidth;
	trans = mis->_miUniqTrans == 0 ? (mis->_miLightFlag ? light_trn_index : 0) : mis->_miUniqTrans;
	Cl2DrawLightTbl(mx, my, pCelBuff, nCel, nWidth, trans);
}

/**
 * @brief Render a missile sprites for a given tile
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 */
static void DrawMissile(int mi, int x, int y, int sx, int sy)
{
	int i;
	MissileStruct* mis;

	if (mi != MIS_MULTI) {
		mis = &missile[mi - 1];
		DrawMissilePrivate(mis, sx, sy);
		return;
	}

	for (i = 0; i < nummissiles; i++) {
		assert((unsigned)missileactive[i] < MAXMISSILES);
		mis = &missile[missileactive[i]];
		if (mis->_mix != x || mis->_miy != y)
			continue;
		DrawMissilePrivate(mis, sx, sy);
	}
}

/**
 * @brief Render a monster sprite
 * @param mnum Id of monster
 * @param bFlag flags to draw
 * @param mx Back buffer coordinate
 * @param my Back buffer coordinate
 */
static void DrawMonster(int mnum, BYTE bFlag, int sx, int sy)
{
	MonsterStruct* mon;
	int mx, my, nCel, nWidth;
	BYTE trans;
	BYTE visFlag = bFlag & BFLAG_VISIBLE;
	BYTE* pCelBuff;

	if (!visFlag && !myplr._pInfraFlag)
		return;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("Draw Monster: tried to draw illegal monster %d", mnum);
	}

	mon = &monsters[mnum];
	if (mon->_mFlags & MFLAG_HIDDEN) {
		return;
	}

	mx = sx + mon->_mxoff - mon->_mAnimXOffset;
	my = sy + mon->_myoff;

	pCelBuff = mon->_mAnimData;
	if (pCelBuff == NULL) {
		dev_fatal("Draw Monster \"%s\": NULL Cel Buffer", mon->_mName);
	}

	nCel = mon->_mAnimFrame;
#if DEBUG_MODE
	int frames = SwapLE32(*(uint32_t*)pCelBuff);
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
	if (mnum == pcursmonst) {
		Cl2DrawOutline(PAL16_RED + 9, mx, my, pCelBuff, nCel, nWidth);
	}
	if (!visFlag || (myplr._pInfraFlag && light_trn_index > 8))
		trans = COLOR_TRN_RED;
	else if (mon->_mmode == MM_STONE)
		trans = COLOR_TRN_GRAY;
	else if (mon->_muniqtrans != 0)
		trans = mon->_muniqtrans;
	else
		trans = light_trn_index;
	Cl2DrawLightTbl(mx, my, pCelBuff, nCel, nWidth, trans);
}

/**
 * @brief Render a sprite of a dead monster
 * @param mnum Id of monster
 * @param mx Back buffer coordinate
 * @param my Back buffer coordinate
 */
static void DrawDeadMonsterHelper(MonsterStruct* mon, int sx, int sy)
{
	int mx, my, nCel, nWidth;
	BYTE trans;
	BYTE* pCelBuff;

	mx = sx /*+ mon->_mxoff*/ - mon->_mAnimXOffset;
	my = sy /*+ mon->_myoff*/;

	pCelBuff = mon->_mAnimData;
	if (pCelBuff == NULL) {
		dev_fatal("Draw Dead Monster \"%s\": NULL Cel Buffer", mon->_mName);
	}
	nCel = mon->_mAnimFrame;
#if DEBUG_MODE
	int frames = SwapLE32(*(uint32_t*)pCelBuff);
	if (nCel < 1 || frames > 50 || nCel > frames) {
		dev_fatal("Draw Dead Monster frame %d of %d, name:%s", nCel, frames, mon->_mName);
	}
#endif
	nWidth = mon->_mAnimWidth;
	trans = mon->_muniqtrans == 0 ? light_trn_index : mon->_muniqtrans;
	Cl2DrawLightTbl(mx, my, pCelBuff, nCel, nWidth, trans);
}

static void DrawDeadMonster(int mnum, int x, int y, int sx, int sy)
{
	int i;
	MonsterStruct* mon;

	if (light_trn_index >= MAXDARKNESS)
		return;

	if (mnum != DEAD_MULTI) {
		mon = &monsters[mnum - 1];
		DrawDeadMonsterHelper(mon, sx, sy);
		return;
	}

	for (i = 0; i < MAXMONSTERS; i++) {
		mon = &monsters[i];
		if (mon->_mmode != MM_DEAD || mon->_mx != x || mon->_my != y)
			continue;
		DrawDeadMonsterHelper(mon, sx, sy);
	}
}

/**
 * @brief Render a towner sprite
 * @param mnum Id of towner
 * @param bFlag flags to draw
 * @param mx Back buffer coordinate
 * @param my Back buffer coordinate
 */
static void DrawTowner(int tnum, BYTE bFlag, int sx, int sy)
{
	MonsterStruct* tw;
	int tx, nCel, nWidth;
	BYTE* pCelBuff;

	tw = &monsters[tnum];
	tx = sx - tw->_mAnimXOffset;
	pCelBuff = tw->_mAnimData;
	if (pCelBuff == NULL) {
		dev_fatal("Draw Towner \"%s\": NULL Cel Buffer", tw->_mName);
	}
	nCel = tw->_mAnimFrame;
	nWidth = tw->_mAnimWidth;
	if (tnum == pcursmonst) {
		CelClippedDrawOutline(PAL16_BEIGE + 6, tx, sy, pCelBuff, nCel, nWidth);
	}
	CelClippedDrawLightTbl(tx, sy, pCelBuff, nCel, nWidth, 0);
}

/**
 * @brief Render a player sprite
 * @param pnum Player id
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param px Back buffer coordinate
 * @param py Back buffer coordinate
 * @param pCelBuff sprite buffer
 * @param nCel frame
 * @param nWidth width
 */
static void DrawPlayer(int pnum, BYTE bFlag, int sx, int sy)
{
	int px, py, nCel, nWidth;
	BYTE visFlag = bFlag & BFLAG_VISIBLE;
	BYTE trans;
	BYTE* pCelBuff;

	if (visFlag || myplr._pInfraFlag) {
		px = sx + plr._pxoff - plr._pAnimXOffset;
		py = sy + plr._pyoff;
		pCelBuff = plr._pAnimData;
		if (pCelBuff == NULL) {
			dev_fatal("Draw Player %d \"%s\": NULL Cel Buffer", pnum, plr._pName);
		}
		nCel = plr._pAnimFrame;
#if DEBUG_MODE
		int frames = SwapLE32(*(uint32_t*)pCelBuff);
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
		if (pnum == pcursplr)
			Cl2DrawOutline(PAL16_BEIGE + 5, px, py, pCelBuff, nCel, nWidth);
		if (pnum == mypnum) {
			trans = 0;
		} else if (!visFlag || (myplr._pInfraFlag && light_trn_index > 8)) {
			trans = COLOR_TRN_RED;
		} else {
			trans = light_trn_index;
			trans = trans <= 5 ? 0 : (trans - 5);
			/*if (plr.pManaShield != 0)
				Cl2DrawLightTbl(
				    px + plr._pAnimXOffset - misfiledata[MFILE_MANASHLD].mfAnimXOffset,
				    py,
				    misanimdata[MFILE_MANASHLD][0],
				    1,
				    misfiledata[MFILE_MANASHLD].mfAnimWidth, trans);*/
		}
		Cl2DrawLightTbl(px, py, pCelBuff, nCel, nWidth, trans);
	}
}

/**
 * @brief Render a player sprite
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 */
void DrawDeadPlayer(int x, int y, int sx, int sy)
{
	int pnum;
	dFlags[x][y] &= ~BFLAG_DEAD_PLAYER;

	for (pnum = 0; pnum < MAX_PLRS; pnum++) {
		if (plr._pActive && plr._pHitPoints < (1 << 6) && plr._pDunLevel == currLvl._dLevelIdx && plr._px == x && plr._py == y) {
#if DEBUG_MODE
			BYTE* pCelBuff = plr._pAnimData;
			if (pCelBuff == NULL) {
				dev_fatal("Draw Dead Player %d \"%s\": NULL Cel Buffer", pnum, plr._pName);
			}
			int nCel = plr._pAnimFrame;
			int frames = SwapLE32(*(uint32_t*)pCelBuff);
			if (nCel < 1 || frames > 50 || nCel > frames) {
				dev_fatal("Draw Dead Player %d \"%s\": facing %d, frame %d of %d", pnum, plr._pName, plr._pdir, nCel, frames);
			}
#endif
			dFlags[x][y] |= BFLAG_DEAD_PLAYER;
			DrawPlayer(pnum, dFlags[x][y], sx, sy);
		}
	}
}

/**
 * @brief Render an object sprite
 * @param oi the id of the object
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param ox Back buffer coordinate
 * @param oy Back buffer coordinate
 */
static void DrawObject(int oi, int x, int y, int ox, int oy)
{
	ObjectStruct* os;
	int sx, sy, xx, yy, nCel, nWidth;
	bool mainTile;
	BYTE trans;
	BYTE* pCelBuff;

	if (light_trn_index >= MAXDARKNESS)
		return;
	// assert(oi != 0);
	mainTile = oi >= 0;
	oi = oi >= 0 ? oi - 1 : -(oi + 1);
	assert((unsigned)oi < MAXOBJECTS);
	os = &objects[oi];
	if (os->_oPreFlag != gbPreFlag)
		return;
	sx = ox - os->_oAnimXOffset;
	sy = oy;
	if (!mainTile) {
		xx = os->_ox - x;
		yy = os->_oy - y;
		sx += (xx * (TILE_WIDTH / 2)) - (yy * (TILE_WIDTH / 2));
		sy += (yy * (TILE_HEIGHT / 2)) + (xx * (TILE_HEIGHT / 2));
	}

	pCelBuff = os->_oAnimData;
	if (pCelBuff == NULL) {
		dev_fatal("Draw Object type %d: NULL Cel Buffer", os->_otype);
	}

	nCel = os->_oAnimFrame;
#if DEBUG_MODE
	int frames = pCelBuff->ciFrameCnt;
	if (nCel < 1 || frames > 50 || nCel > frames) {
		dev_fatal("Draw Object: frame %d of %d, type %d", nCel, frames, os->_otype);
	}
#endif
	nWidth = os->_oAnimWidth;
	if (oi == pcursobj) {
		CelClippedDrawOutline(PAL16_YELLOW + 2, sx, sy, pCelBuff, nCel, nWidth);
	}
	trans = os->_oLightFlag ? light_trn_index : 0;
	CelClippedDrawLightTbl(sx, sy, pCelBuff, nCel, nWidth, trans);
}

/**
 * @brief Render a cell
 * @param pn piece number
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 */
static void drawCell(int pn, int sx, int sy)
{
	BYTE* dst;
	uint16_t levelCelBlock, i, limit;
	uint16_t* pMap;
	int tmp, mask;

	if (sx <= SCREEN_X - TILE_WIDTH || sx >= SCREEN_X + SCREEN_WIDTH)
		return; // starting from too far to the left or right -> skip

	tmp = sy - SCREEN_Y;
	if (tmp < 0)
		return; // starting from above the top -> skip
	tmp = (unsigned)(tmp + 1 + (MICRO_HEIGHT - 1)) / MICRO_HEIGHT;
	tmp *= TILE_WIDTH / MICRO_WIDTH;
	limit = tmp <= MicroTileLen ? tmp : MicroTileLen;
	/*limit = MicroTileLen;
	while (sy - limit * (MICRO_HEIGHT / (TILE_WIDTH / MICRO_WIDTH)) <= SCREEN_Y - MICRO_HEIGHT) {
		limit -= (TILE_WIDTH / MICRO_WIDTH);
	}*/
	/*i = 0;
	while (sy > SCREEN_Y + VIEWPORT_HEIGHT + MICRO_HEIGHT) {
		sy -= MICRO_HEIGHT;
		i += (TILE_WIDTH / MICRO_WIDTH);
	}*/
	tmp = sy - (SCREEN_Y + VIEWPORT_HEIGHT + TILE_HEIGHT - 1);
	i = 0;
	if (tmp >= 0) {
		// starting from below the bottom -> skip tiles
		tmp = 1 + (unsigned)tmp / TILE_HEIGHT;
		sy -= TILE_HEIGHT * tmp;
		i = tmp * (TILE_WIDTH / MICRO_WIDTH) * (TILE_HEIGHT / MICRO_HEIGHT);
		if (i >= limit)
			return; // not enough microtiles to affect the screen -> skip
	}
	dst = &gpBuffer[sx + BUFFER_WIDTH * sy];

	pMap = &pMicroPieces[pn][i];
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
 * @brief Render a floor tiles
 * @param pn piece number
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 */
static void drawFloor(int pn, int sx, int sy)
{
	BYTE *dst, tmp;
	uint16_t levelCelBlock;
	uint16_t* pMap;

	if (sx <= SCREEN_X - TILE_WIDTH || sx >= SCREEN_X + SCREEN_WIDTH)
		return; // starting from too far to the left or right -> skip

	if (sy < SCREEN_Y || sy >= SCREEN_Y + VIEWPORT_HEIGHT + TILE_HEIGHT - 1)
		return; // starting from above the top or below the bottom -> skip

	dst = &gpBuffer[sx + BUFFER_WIDTH * sy];

	pMap = &pMicroPieces[pn][0];
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
 * @brief Draw item for a given tile
 * @param y dPiece coordinate
 * @param x dPiece coordinate
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 */
static void DrawItem(int ii, int sx, int sy)
{
	int nCel;
	ItemStruct* is;
	BYTE* pCelBuff;

	assert(ii > 0);

	ii--;

	is = &items[ii];
	if (is->_iPostDraw == gbPreFlag)
		return;

	pCelBuff = is->_iAnimData;
	if (pCelBuff == NULL) {
		dev_fatal("Draw Item \"%s\": NULL Cel Buffer", is->_iName);
	}
	nCel = is->_iAnimFrame;
#if DEBUG_MODE
	int frames = pCelBuff->ciFrameCnt;
	if (nCel < 1 || frames > 50 || nCel > frames) {
		dev_fatal("Draw Item \"%s\": frame %d of %d, type %d", is->_iName, nCel, frames, is->_itype);
	}
#endif
	sx -= ITEM_ANIM_XOFFSET; //is->_iAnimXOffset;
	if (ii == pcursitem) {
		CelClippedDrawOutline(ICOL_BLUE, sx, sy, pCelBuff, nCel, ITEM_ANIM_WIDTH); // is->_iAnimWidth);
	}
	CelClippedDrawLightTbl(sx, sy, pCelBuff, nCel, ITEM_ANIM_WIDTH, light_trn_index); //is->_iAnimWidth);
}

/**
 * @brief Draw a towner or a monster depending on the level
 * @param bFlag flags
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 */
static void DrawMonsterHelper(int mnum, BYTE bFlag, int sx, int sy)
{
	if (currLvl._dType != DTYPE_TOWN)
		DrawMonster(mnum, bFlag, sx, sy);
	else
		DrawTowner(mnum, bFlag, sx, sy);
}

/**
 * @brief Render object sprites
 * @param sx dPiece coordinate
 * @param sy dPiece coordinate
 * @param dx Back buffer coordinate
 * @param dy Back buffer coordinate
 */
static void scrollrt_draw_dungeon(int sx, int sy, int dx, int dy)
{
	int mpnum;
	BYTE bv, bFlag;

	assert((unsigned)sx < MAXDUNX);
	assert((unsigned)sy < MAXDUNY);

	//if (dRendered[sx][sy])
	//	return;
	//dRendered[sx][sy] = true;

	gbPreFlag = TRUE;
	bFlag = dFlags[sx][sy];
	light_trn_index = dLight[sx][sy];
	gbCelTransparencyActive = TransList[dTransVal[sx][sy]];

	mpnum = dPiece[sx][sy];
	drawCell(mpnum, dx, dy);

#if DEBUG_MODE
	if (visiondebug && (bFlag & BFLAG_VISIBLE)) {
		CelClippedDrawLightTbl(dx, dy, pSquareCel, 1, TILE_WIDTH, 0);
	}
#endif

	if (bFlag & BFLAG_MISSILE_PRE) {
		mpnum = dMissile[sx][sy];
		assert(mpnum != 0);
		DrawMissile(mpnum, sx, sy, dx, dy);
	}

	bv = dDead[sx][sy];
	if (bv != 0)
		DrawDeadMonster(bv, sx, sy, dx, dy);
	mpnum = dObject[sx][sy];
	if (mpnum != 0)
		DrawObject(mpnum, sx, sy, dx, dy);
	bv = dItem[sx][sy];
	if (bv != 0)
		DrawItem(bv, dx, dy);
	if (bFlag & BFLAG_DEAD_PLAYER) {
		DrawDeadPlayer(sx, sy, dx, dy);
	}
	gbPreFlag = FALSE;
	mpnum = dPlayer[sx][sy];
	if (mpnum > 0)
		DrawPlayer(mpnum - 1, bFlag, dx, dy);
	mpnum = dMonster[sx][sy];
	if (mpnum > 0)
		DrawMonsterHelper(mpnum - 1, bFlag, dx, dy);
	mpnum = dMissile[sx][sy];
	if (mpnum != 0)
		DrawMissile(mpnum, sx, sy, dx, dy);
	mpnum = dObject[sx][sy];
	if (mpnum != 0)
		DrawObject(mpnum, sx, sy, dx, dy);
	bv = dItem[sx][sy];
	if (bv != 0)
		DrawItem(bv, dx, dy);

	if (currLvl._dType != DTYPE_TOWN) {
		bv = dSpecial[sx][sy];
		if (bv != 0) {
			CelClippedDrawLightTrans(dx, dy, pSpecialCels, bv, TILE_WIDTH);
		}
	} else {
		// Tree leaves should always cover player when entering or leaving the tile,
		// So delay the rendering until after the next row is being drawn.
		// This could probably have been better solved by sprites in screen space.
		if (sx > 0 && sy > 0) {
			bv = dSpecial[sx - 1][sy - 1];
			if (bv != 0 && dy > TILE_HEIGHT + SCREEN_Y) {
				CelDraw(dx, (dy - TILE_HEIGHT), (CelImageBuf*)pSpecialCels, bv);
			}
		}
	}
}

/**
 * @brief Render a row of tiles
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param rows Number of rows
 * @param columns Tile in a row
 */
static void scrollrt_drawFloor(int x, int y, int sx, int sy, int rows, int columns)
{
	//int pn;

	assert(gpBuffer != NULL);

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			if (IN_DUNGEON_AREA(x, y)) {
				//pn = dPiece[x][y];
				//assert(pn != 0);
				//if (pn != 0) {
					//if ((microFlags[pn] & (~(TMIF_WALL_TRANS))) != (TMIF_LEFT_REDRAW | TMIF_RIGHT_REDRAW))
						light_trn_index = dLight[x][y];
						drawFloor(dPiece[x][y], sx, sy);
					//}
				//} else {
				//	world_draw_black_tile(sx, sy);
				//}
			} else {
				world_draw_black_tile(sx, sy);
			}
			SHIFT_GRID(x, y, 1, 0);
			sx += TILE_WIDTH;
		}
		// Return to start of row
		SHIFT_GRID(x, y, -columns, 0);
		sx -= columns * TILE_WIDTH;

		// Jump to next row
		sy += TILE_HEIGHT / 2;
		if (i & 1) {
			x++;
			columns--;
			sx += TILE_WIDTH / 2;
		} else {
			y++;
			columns++;
			sx -= TILE_WIDTH / 2;
		}
	}
}

#define IsWall(x, y)     (/*dPiece[x][y] == 0 ||*/ nSolidTable[dPiece[x][y]] || dSpecial[x][y] != 0)
#define IsWalkable(x, y) (/*dPiece[x][y] != 0 &&*/ !nSolidTable[dPiece[x][y]])

/**
 * @brief Render a row of tile
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param rows Number of rows
 * @param columns Tile in a row
 */
static void scrollrt_draw(int x, int y, int sx, int sy, int rows, int columns)
{
	BYTE skips = 0;
	assert(gpBuffer != NULL);

	// Keep evaluating until MicroTiles can't affect screen
	rows += MicroTileLen;
	//memset(dRendered, 0, sizeof(dRendered));

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			if (IN_DUNGEON_AREA(x, y)) {
				if (x + 1 < MAXDUNX && y - 1 >= 0 && j != columns - 1 /*sx + TILE_WIDTH <= SCREEN_X + SCREEN_WIDTH*/) {
					// Render objects behind walls first to prevent sprites, that are moving
					// between tiles, from poking through the walls as they exceed the tile bounds.
					// A proper fix for this would probably be to layout the sceen and render by
					// sprite screen position rather than tile position.
					if (IsWall(x, y) && IsWall(x + 1, y)) { // Part of a wall aligned on the x-axis
						if (IsWalkable(x + 1, y - 1)) {     // Has walkable area behind it (to make sure it matches only the rightmost wall)
							scrollrt_draw_dungeon(x + 1, y - 1, sx + TILE_WIDTH, sy);
							skips |= 2;
						}
					}
				}
				assert(dPiece[x][y] != 0);
				if (/*dPiece[x][y] != 0 &&*/ !(skips & 1)) {
					scrollrt_draw_dungeon(x, y, sx, sy);
				}
			}
			SHIFT_GRID(x, y, 1, 0);
			sx += TILE_WIDTH;
			skips >>= 1;
		}
		// Return to start of row
		SHIFT_GRID(x, y, -columns, 0);
		sx -= columns * TILE_WIDTH;

		// Jump to next row
		sy += TILE_HEIGHT / 2;
		if (i & 1) {
			x++;
			columns--;
			sx += TILE_WIDTH / 2;
		} else {
			y++;
			columns++;
			sx -= TILE_WIDTH / 2;
		}
	}
}

/**
 * @brief Scale up the rendered part of the back buffer to take up the full view
 */
static void Zoom()
{
	int wdt = SCREEN_WIDTH / 2;
	int nSrcOff = SCREENXY(SCREEN_WIDTH / 2 - 1, VIEWPORT_HEIGHT / 2 - 1);
	int nDstOff = SCREENXY(SCREEN_WIDTH - 1, VIEWPORT_HEIGHT - 1);

	BYTE* src = &gpBuffer[nSrcOff];
	BYTE* dst = &gpBuffer[nDstOff];

	for (int hgt = 0; hgt < VIEWPORT_HEIGHT / 2; hgt++) {
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
	y = VIEWPORT_HEIGHT;
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
 * @param columns Tiles needed per row
 * @param rows Both even and odd rows
 */
static void TilesInView(unsigned* rcolumns, unsigned* rrows)
{
	unsigned columns = (unsigned)(SCREEN_WIDTH + TILE_WIDTH - 1) / TILE_WIDTH;
	unsigned rows = (unsigned)(VIEWPORT_HEIGHT + TILE_HEIGHT - 1) / TILE_HEIGHT;

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
	gsTileVp._vShiftX = 0;
	gsTileVp._vShiftY = 0;
	SHIFT_GRID(gsTileVp._vShiftX, gsTileVp._vShiftY, 0 - (gsTileVp._vColumns / 2), 0 - (lrow / 2));

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
	gsMouseVp._vShiftX = 0;
	gsMouseVp._vShiftY = 0;
	SHIFT_GRID(gsMouseVp._vShiftX, gsMouseVp._vShiftY, 0 - (gsMouseVp._vColumns / 2), 0 - (lrow / 2));

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
static void DrawGame()
{
	int x, y, sx, sy, columns, rows;

	// Limit rendering to the view area
	//if (!gbZoomInFlag)
	//	gpBufEnd = &gpBuffer[SCREENXY(0, VIEWPORT_HEIGHT)];
	//else
	//	gpBufEnd = &gpBuffer[SCREENXY(0, VIEWPORT_HEIGHT / 2)];

	// Adjust by player offset and tile grid alignment
	sx = ScrollInfo._sxoff - gsTileVp._vOffsetX;
	sy = ScrollInfo._syoff - gsTileVp._vOffsetY;

	columns = gsTileVp._vColumns;
	rows = gsTileVp._vRows;

	x = ViewX + gsTileVp._vShiftX;
	y = ViewY + gsTileVp._vShiftY;

	// Draw areas moving in and out of the screen
	switch (ScrollInfo._sdir) {
	case SDIR_NONE:
		break;
	case SDIR_N:
	case SDIR_S:
		sy -= TILE_HEIGHT;
		SHIFT_GRID(x, y, 0, -1);
		rows += 2;
		break;
	case SDIR_NE:
	case SDIR_SW:
	case SDIR_SE:
	case SDIR_NW:
		sx -= TILE_WIDTH / 2;
		sy -= TILE_HEIGHT / 2;
		x--;
		columns++;
		rows++;
		break;
	case SDIR_E:
	case SDIR_W:
		sx -= TILE_WIDTH;
		SHIFT_GRID(x, y, -1, 0);
		columns++;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	scrollrt_drawFloor(x, y, sx, sy, rows, columns);
	scrollrt_draw(x, y, sx, sy, rows, columns);

	// Allow rendering to the whole screen
	//gpBufEnd = &gpBuffer[SCREENXY(0, SCREEN_HEIGHT)];

	if (gbZoomInFlag) {
		Zoom();
	}
}

/**
 * @brief Start rendering of screen, town variation
 */
static void DrawView()
{
	DrawGame();
	if (gbAutomapflag) {
		DrawAutomap();
	}
	//if (gbRedrawFlags & (REDRAW_MANA_FLASK | REDRAW_SPELL_ICON)) {
		DrawSkillIcons();
	//}
	DrawLifeFlask();
	DrawManaFlask();
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
		if (gbDropGoldFlag) {
			DrawGoldSplit(dropGoldValue);
		}
		if (gbSkillListFlag) {
			DrawSkillList();
		}
		if (gbShowTooltip || (SDL_GetModState() & KMOD_ALT)) {
			DrawInfoStr();
		}
		if (gbHelpflag) {
			DrawHelp();
		}
	}
	if (currmsg != EMSG_NONE) {
		DrawDiabloMsg();
	}
	if (gbDeathflag == MDM_DEAD) {
		RedBack();
	} else if (gbGamePaused) {
		gmenu_draw_pause();
	}

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
	DrawControllerModifierHints();
#endif
	DrawPlrMsg(true);
	if (gmenu_is_active())
		gmenu_draw();
	//if (gbDoomflag)
	//	doom_draw();
	//if (gbRedrawFlags & REDRAW_CTRL_BUTTONS) {
		DrawCtrlBtns();
	//}
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
		if (DSIZEY + DBORDERY - 1 <= ViewY || DBORDERX >= ViewX) {
			if (DSIZEY + DBORDERY - 1 > ViewY) {
				ViewY++;
				scroll = true;
			}
			if (DBORDERX < ViewX) {
				ViewX--;
				scroll = true;
			}
		} else {
			ViewY++;
			ViewX--;
			scroll = true;
		}
	}
	if (MousePos.x > SCREEN_WIDTH - 20) {
		if (DSIZEX + DBORDERX - 1 <= ViewX || DBORDERY >= ViewY) {
			if (DSIZEX + DBORDERX - 1 > ViewX) {
				ViewX++;
				scroll = true;
			}
			if (DBORDERY < ViewY) {
				ViewY--;
				scroll = true;
			}
		} else {
			ViewY--;
			ViewX++;
			scroll = true;
		}
	}
	if (MousePos.y < 20) {
		if (DBORDERY >= ViewY || DBORDERX >= ViewX) {
			if (DBORDERY < ViewY) {
				ViewY--;
				scroll = true;
			}
			if (DBORDERX < ViewX) {
				ViewX--;
				scroll = true;
			}
		} else {
			ViewX--;
			ViewY--;
			scroll = true;
		}
	}
	if (MousePos.y > SCREEN_HEIGHT - 20) {
		if (DSIZEY + DBORDERY - 1 <= ViewY || DSIZEX + DBORDERX - 1 <= ViewX) {
			if (DSIZEY + DBORDERY - 1 > ViewY) {
				ViewY++;
				scroll = true;
			}
			if (DSIZEX + DBORDERX - 1 > ViewX) {
				ViewX++;
				scroll = true;
			}
		} else {
			ViewX++;
			ViewY++;
			scroll = true;
		}
	}

	if (scroll)
		ScrollInfo._sdir = SDIR_NONE;
}
#endif

/**
 * @brief Toggle the FPS meter
 */
/*void EnableFrameCount()
{
	gbShowFPS = !gbShowFPS;
	guFpsStartTc = SDL_GetTicks();
}*/

/**
 * @brief Display the current average FPS over 1 sec
 */
static void DrawFPS()
{
	Uint32 currTc, deltaTc;

	guFrameCnt++;
	currTc = SDL_GetTicks();
	deltaTc = currTc - guFpsStartTc;
	if (deltaTc >= 1000) {
		guFpsStartTc = currTc;
		guFrameRate = 1000 * guFrameCnt / deltaTc;
		guFrameCnt = 0;
	}
	snprintf(tempstr, sizeof(tempstr), "%d FPS", guFrameRate);
	PrintGameStr(SCREEN_X + 8, SCREEN_Y + 65, tempstr, COL_RED);
}

/**
 * @brief Redraw screen
 * @param draw_cursor
 */
void scrollrt_draw_screen(bool draw_cursor)
{
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
	if (sgbControllerActive)
		draw_cursor = false;
#endif
	if (draw_cursor) {
		lock_buf(0);
		scrollrt_draw_cursor();
		unlock_buf(0);
	}

	if (gbWndActive) {
		BltFast();
	}

	if (draw_cursor) {
		lock_buf(0);
		scrollrt_remove_back_buffer_cursor();
		unlock_buf(0);
	}
	RenderPresent();
}

/**
 * @brief Render the game
 */
void scrollrt_draw_game()
{
	if (!gbRunGame) {
		return;
	}

	lock_buf(0);
	DrawView();
	scrollrt_draw_cursor();

	if (gbShowFPS)
		DrawFPS();

	unlock_buf(0);

	//DrawMain(hgt, gbRedrawFlags);
	if (gbWndActive) {
		BltFast();
	}

	lock_buf(0);
	scrollrt_remove_back_buffer_cursor();
	unlock_buf(0);
	RenderPresent();

	gbRedrawFlags = 0;
}

DEVILUTION_END_NAMESPACE
