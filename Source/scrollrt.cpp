/**
 * @file scrollrt.cpp
 *
 * Implementation of functionality for rendering the dungeons, monsters and calling other render routines.
 */
#include "all.h"
#include "plrctrls.h"
#include "engine/render/cl2_render.hpp"
#include "engine/render/dun_render.hpp"

DEVILUTION_BEGIN_NAMESPACE

/** Specifies the player viewpoint X-coordinate of the map. */
int ViewX;
/** Specifies the player viewpoint Y-coordinate of the map. */
int ViewY;
ScrollStruct ScrollInfo;

/**
 * Specifies the current light entry.
 */
int light_table_index;

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
/**
 * Specifies whether foliage (tile has extra content that overlaps previous tile) being rendered.
 */
bool gbCelFoliageActive;
/**
 * Specifies the current dungeon piece ID of the level, as used during rendering of the level tiles.
 */
int level_piece_id;
void (*DrawPlrProc)(int, int, int, int, int, BYTE *, int, int, int, int);
/**
 * Buffer to store the cursor image.
 */
BYTE sgSaveBack[MAX_CURSOR_AREA];

//bool dRendered[MAXDUNX][MAXDUNY];
/**
 * Specfies whether the FPS counter is shown.
 */
bool gbFrameflag = false;
static unsigned guFrameCnt;
static unsigned guFrameRate;
static Uint32 guFpsStartTc;

/* data */

/* used in 1.00 debug */
#ifdef _DEBUG
const char *const szMonModeAssert[18] = {
	"standing",
	"walking (1)",
	"walking (2)",
	"walking (3)",
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
	"talking"
};

const char *const szPlrModeAssert[12] = {
	"standing",
	"walking (1)",
	"walking (2)",
	"walking (3)",
	"attacking (melee)",
	"attacking (ranged)",
	"blocking",
	"getting hit",
	"dying",
	"casting a spell",
	"changing levels",
	"quitting"
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
static void scrollrt_draw_cursor_back_buffer()
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

/**
 * @brief Draw the cursor on the back buffer
 */
static void scrollrt_draw_cursor_item()
{
	int i, mx, my, frame;
	BYTE *src, *dst, *cCels;
	BYTE col;

	assert(sgCursWdt == 0);

	if (pcurs <= CURSOR_NONE || cursW == 0 || cursH == 0) {
		return;
	}

#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	if (sgbControllerActive && !IsMovingMouseCursorWithController() && pcurs != CURSOR_TELEPORT && !gbInvflag && (!gbChrflag || !gbLvlUp))
		return;
#endif

	mx = MouseX - 1;
	if (mx < 0 - cursW - 1) {
		return;
	}
	if (mx > SCREEN_WIDTH - 1) {
		return;
	}
	my = MouseY - 1;
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

	assert(sgCursWdt * sgCursHgt <= sizeof sgSaveBack);
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
	gpBufEnd = &gpBuffer[BUFFER_WIDTH * (SCREEN_HEIGHT + SCREEN_Y) - cursW - 2];

	frame = pcurs;
	cCels = pCursCels;
	if (frame >= CURSOR_FIRSTITEM) {
		col = ICOL_YELLOW;
		if (myplr.HoldItem._iMagical != ITEM_QUALITY_NORMAL) {
			col = ICOL_BLUE;
		}
		if (!myplr.HoldItem._iStatFlag) {
			col = ICOL_RED;
		}

		CelDrawOutline(col, mx, my, cCels, frame, cursW);
		if (col != ICOL_RED) {
			CelClippedDraw(mx, my, cCels, frame, cursW);
		} else {
			CelDrawLightRed(mx, my, cCels, frame, cursW);
		}
	} else {
		CelClippedDraw(mx, my, cCels, frame, cursW);
	}
}

/**
 * @brief Render a missile sprite
 * @param mis Pointer to MissileStruct struct
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pre Is the sprite in the background
 */
static void DrawMissilePrivate(MissileStruct *mis, int sx, int sy, BOOL pre)
{
	int mx, my, nCel, frames;
	BYTE *pCelBuff;

	if (mis->_miPreFlag != pre || !mis->_miDrawFlag)
		return;

	mx = sx + mis->_mixoff - mis->_miAnimXOffset;
	my = sy + mis->_miyoff;
	pCelBuff = mis->_miAnimData;
	if (pCelBuff == NULL) {
		dev_fatal("Draw Missile 2 type %d: NULL Cel Buffer", mis->_miType);
	}
	nCel = mis->_miAnimFrame;
	frames = SwapLE32(*(uint32_t *)pCelBuff);
	if (nCel < 1 || frames > 50 || nCel > frames) {
		dev_fatal("Draw Missile 2: frame %d of %d, missile type==%d", nCel, frames, mis->_miType);
	}
	if (mis->_miUniqTrans != 0)
		Cl2DrawLightTbl(mx, my, pCelBuff, nCel, mis->_miAnimWidth, mis->_miUniqTrans);
	else if (mis->_miLightFlag)
		Cl2DrawLight(mx, my, pCelBuff, nCel, mis->_miAnimWidth);
	else
		Cl2Draw(mx, my, pCelBuff, nCel, mis->_miAnimWidth);
}

/**
 * @brief Render a missile sprites for a given tile
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pre Is the sprite in the background
 */
static void DrawMissile(int x, int y, int sx, int sy, BOOL pre)
{
	int i;
	MissileStruct *mis;

	if (dMissile[x][y] != -1) {
		mis = &missile[dMissile[x][y] - 1];
		DrawMissilePrivate(mis, sx, sy, pre);
		return;
	}

	for (i = 0; i < nummissiles; i++) {
		assert((unsigned)missileactive[i] < MAXMISSILES);
		mis = &missile[missileactive[i]];
		if (mis->_mix != x || mis->_miy != y)
			continue;
		DrawMissilePrivate(mis, sx, sy, pre);
	}
}

/**
 * @brief Render a monster sprite
 * @param mnum Id of monster
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param mx Back buffer coordinate
 * @param my Back buffer coordinate
 */
static void DrawMonster(int mnum, int x, int y, int sx, int sy)
{
	MonsterStruct *mon;
	int mx, my, nCel, nWidth;
	BYTE trans;
	BYTE litFlag = dFlags[x][y] & BFLAG_LIT;
	BYTE *pCelBuff;

	if (!litFlag && !myplr._pInfraFlag)
		return;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("Draw Monster: tried to draw illegal monster %d", mnum);
	}

	mon = &monster[mnum];
	if (mon->_mFlags & MFLAG_HIDDEN) {
		return;
	}

	mx = sx + mon->_mxoff - mon->_mAnimXOffset;
	my = sy + mon->_myoff;

	pCelBuff = mon->_mAnimData;
	if (pCelBuff == NULL) {
		dev_fatal("Draw Monster \"%s\": NULL Cel Buffer", mon->mName);
	}

	nCel = mon->_mAnimFrame;
#ifdef _DEBUG
	int frames = SwapLE32(*(uint32_t *)pCelBuff);
	if (nCel < 1 || frames > 50 || nCel > frames) {
		const char *szMode = "unknown action";
		if (mon->_mmode < lengthof(szMonModeAssert))
			szMode = szMonModeAssert[mon->_mmode];
		dev_fatal(
			"Draw Monster \"%s\" %s: facing %d, frame %d of %d",
			mon->mName,
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
	if (!litFlag || (myplr._pInfraFlag && light_table_index > 8))
		trans = LIGHTIDX_RED;
	else if (mon->_mmode == MM_STONE)
		trans = LIGHTIDX_GRAY;
	else if (mon->_uniqtype != 0)
		trans = mon->_uniqtrans;
	else {
		Cl2DrawLight(mx, my, pCelBuff, nCel, nWidth);
		return;
	}
	Cl2DrawLightTbl(mx, my, pCelBuff, nCel, nWidth, trans);
}

/**
 * @brief Render a towner sprite
 * @param mnum Id of towner
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param mx Back buffer coordinate
 * @param my Back buffer coordinate
 */
static void DrawTowner(int tnum, int x, int y, int sx, int sy)
{
	TownerStruct *tw;
	int tx, nCel, nWidth;
	BYTE *pCelBuff;

	tw = &towners[tnum];
	tx = sx - tw->_tAnimXOffset;
	pCelBuff = tw->_tAnimData;
	nCel = tw->_tAnimFrame;
	nWidth = tw->_tAnimWidth;
	if (tnum == pcursmonst) {
		CelDrawOutline(PAL16_BEIGE + 6, tx, sy, pCelBuff, nCel, nWidth);
	}
	CelClippedDraw(tx, sy, pCelBuff, nCel, nWidth);
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
static void DrawPlayer(int pnum, int x, int y, int sx, int sy)
{
	int px, py, nCel, nWidth, l;
	BYTE litFlag = dFlags[x][y] & BFLAG_LIT;
	BYTE *pCelBuff;

	if (litFlag || myplr._pInfraFlag) {
		px = sx + plr._pxoff - plr._pAnimXOffset;
		py = sy + plr._pyoff;
		pCelBuff = plr._pAnimData;
		if (pCelBuff == NULL) {
			dev_fatal("Drawing player %d \"%s\": NULL Cel Buffer", pnum, plr._pName);
		}
		nCel = plr._pAnimFrame;
#ifdef _DEBUG
		int frames = SwapLE32(*(uint32_t *)pCelBuff);
		if (nCel < 1 || frames > 50 || nCel > frames) {
			const char *szMode = "unknown action";
			if (plr._pmode <= PM_QUIT)
				szMode = szPlrModeAssert[plr._pmode];
			dev_fatal(
				"Drawing player %d \"%s\" %s: facing %d, frame %d of %d",
				pnum,
				plr._pName,
				szMode,
				plr._pdir,
				nCel,
				frames);
		}
#endif
		nWidth = plr._pAnimWidth;
		if (pnum == pcursplr)
			Cl2DrawOutline(PAL16_BEIGE + 5, px, py, pCelBuff, nCel, nWidth);
		if (pnum == mypnum) {
			Cl2Draw(px, py, pCelBuff, nCel, nWidth);
		} else if (!litFlag || (myplr._pInfraFlag && light_table_index > 8)) {
			Cl2DrawLightTbl(px, py, pCelBuff, nCel, nWidth, LIGHTIDX_RED);
		} else {
			l = light_table_index;
			if (light_table_index <= 5)
				light_table_index = 0;
			else
				light_table_index -= 5;
			Cl2DrawLight(px, py, pCelBuff, nCel, nWidth);
			/*if (plr.pManaShield != 0)
				Cl2DrawLight(
				    px + plr._pAnimXOffset - misfiledata[MFILE_MANASHLD].mfAnimXOffset,
				    py,
				    misanimdata[MFILE_MANASHLD][0],
				    1,
				    misfiledata[MFILE_MANASHLD].mfAnimWidth);*/
			light_table_index = l;
		}
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
#ifdef _DEBUG
			BYTE *pCelBuff = plr._pAnimData;
			if (pCelBuff == NULL) {
				dev_fatal("Drawing dead player %d \"%s\": NULL Cel Buffer", i, plr._pName);
			}
			int nCel = plr._pAnimFrame;
			int frames = SwapLE32(*(uint32_t *)pCelBuff);
			if (nCel < 1 || frames > 50 || nCel > frames) {
				dev_fatal("Drawing dead player %d \"%s\": facing %d, frame %d of %d", i, plr._pName, plr._pdir, nCel, frames);
			}
#endif
			dFlags[x][y] |= BFLAG_DEAD_PLAYER;
			DrawPlayer(pnum, x, y, sx, sy);
		}
	}
}

/**
 * @brief Render an object sprite
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param ox Back buffer coordinate
 * @param oy Back buffer coordinate
 * @param pre Is the sprite in the background
 */
static void DrawObject(int x, int y, int ox, int oy, BOOL pre)
{
	ObjectStruct *os;
	int sx, sy, xx, yy, nCel, frames;
	char oi;
	BYTE *pCelBuff;

	oi = dObject[x][y];
	if (oi == 0 || light_table_index >= LIGHTMAX)
		return;

	if (oi > 0) {
		oi--;
		os = &objects[oi];
		if (os->_oPreFlag != pre)
			return;
		sx = ox - os->_oAnimXOffset;
		sy = oy;
	} else {
		oi = -(oi + 1);
		os = &objects[oi];
		if (os->_oPreFlag != pre)
			return;
		xx = os->_ox - x;
		yy = os->_oy - y;
		sx = (xx * (TILE_WIDTH / 2)) + ox - os->_oAnimXOffset - (yy * (TILE_WIDTH / 2));
		sy = oy + (yy * (TILE_HEIGHT / 2)) + (xx * (TILE_HEIGHT / 2));
	}

	assert((unsigned char)oi < MAXOBJECTS);

	pCelBuff = os->_oAnimData;
	if (pCelBuff == NULL) {
		dev_fatal("Draw Object type %d: NULL Cel Buffer", os->_otype);
	}

	nCel = os->_oAnimFrame;
	frames = SwapLE32(*(uint32_t *)pCelBuff);
	if (nCel < 1 || frames > 50 || nCel > frames) {
		dev_fatal("Draw Object: frame %d of %d, object type==%d", nCel, frames, os->_otype);
	}

	if (oi == pcursobj)
		CelDrawOutline(PAL16_YELLOW + 2, sx, sy, pCelBuff, nCel, os->_oAnimWidth);
	if (os->_oLight) {
		CelClippedDrawLight(sx, sy, pCelBuff, nCel, os->_oAnimWidth);
	} else {
		CelClippedDraw(sx, sy, pCelBuff, nCel, os->_oAnimWidth);
	}
}

/**
 * @brief Render a cell
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 */
static void drawCell(int x, int y, int sx, int sy)
{
	BYTE *dst, i;
	uint16_t levelCelBlock;
	BYTE limit;
	MICROS *pMap;
	int tmp;

	if (sx <= SCREEN_X - TILE_WIDTH || sx >= SCREEN_X + SCREEN_WIDTH)
		return;

	tmp = (sy - SCREEN_Y + (TILE_HEIGHT / 2 + 1)) / (TILE_HEIGHT / 2);
	if (tmp <= 0)
		return;
	limit = tmp <= MicroTileLen ? tmp : MicroTileLen;
	/*limit = MicroTileLen;
	while (sy - limit * (TILE_HEIGHT / 2) <= SCREEN_Y - TILE_HEIGHT) {
		limit -= 2;
	}*/
	/*i = 0;
	while (sy > SCREEN_Y + VIEWPORT_HEIGHT + TILE_HEIGHT) {
		sy -= TILE_HEIGHT;
		i += 2;
	}*/
	tmp = (sy - (SCREEN_Y + VIEWPORT_HEIGHT + TILE_HEIGHT) + (TILE_HEIGHT - 1)) / TILE_HEIGHT;
	i = 0;
	if (tmp > 0) {
		sy -= TILE_HEIGHT * tmp;
		i = tmp * 2;
		if (i >= limit)
			return;
	}
	dst = &gpBuffer[sx + BUFFER_WIDTH * sy];

	pMap = &dpiece_defs_map_2[x][y];
	level_piece_id = dPiece[x][y];
	gbCelTransparencyActive = (nTransTable[level_piece_id] & TransList[dTransVal[x][y]]);
	gbCelFoliageActive = !nSolidTable[level_piece_id];

	for ( ; i < limit; i += 2) {
		levelCelBlock = pMap->mt[i];
		if (levelCelBlock != 0) {
			RenderTile(dst, levelCelBlock, i == 0 ? RADT_LEFT : RADT_NONE);
		}
		levelCelBlock = pMap->mt[i + 1];
		if (levelCelBlock != 0) {
			RenderTile(dst + TILE_WIDTH / 2, levelCelBlock, i == 0 ? RADT_RIGHT : RADT_NONE);
		}
		dst -= BUFFER_WIDTH * TILE_HEIGHT;
	}
}

/**
 * @brief Render a floor tiles
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 */
static void drawFloor(int x, int y, int sx, int sy)
{
	BYTE *dst;
	uint16_t levelCelBlock;
	MICROS *pMap;

	if (sx <= SCREEN_X - TILE_WIDTH || sx >= SCREEN_X + SCREEN_WIDTH)
		return;

	if (sy <= SCREEN_Y || sy >= SCREEN_Y + VIEWPORT_HEIGHT + TILE_HEIGHT)
		return;

	dst = &gpBuffer[sx + BUFFER_WIDTH * sy];

	gbCelTransparencyActive = false;
	gbCelFoliageActive = false;
	light_table_index = dLight[x][y];
	pMap = &dpiece_defs_map_2[x][y];

	levelCelBlock = pMap->mt[0];
	if (levelCelBlock != 0) {
		RenderTile(dst, levelCelBlock, RADT_LEFT);
	}
	levelCelBlock = pMap->mt[1];
	if (levelCelBlock != 0) {
		RenderTile(dst + TILE_WIDTH / 2, levelCelBlock, RADT_RIGHT);
	}
}

/**
 * @brief Draw item for a given tile
 * @param y dPiece coordinate
 * @param x dPiece coordinate
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pre Is the sprite in the background
 */
static void DrawItem(int x, int y, int sx, int sy, BOOL pre)
{
	int nCel, ii, frames;
	ItemStruct *is;
	BYTE *pCelBuff;

	ii = dItem[x][y];
	if (ii == 0)
		return;

	ii--;
	if ((unsigned)ii >= MAXITEMS) {
		dev_fatal("Invalid item (%d) to draw.", ii);
	}

	is = &items[ii];
	if (is->_iPostDraw == pre)
		return;

	pCelBuff = is->_iAnimData;
	if (pCelBuff == NULL) {
		dev_fatal("Draw Item \"%s\" 1: NULL Cel Buffer", is->_iName);
	}
	frames = SwapLE32(*(uint32_t *)pCelBuff);
	nCel = is->_iAnimFrame;
	if (nCel < 1 || frames > 50 || nCel > frames) {
		dev_fatal("Draw \"%s\" Item 1: frame %d of %d, item type==%d", is->_iName, nCel, frames, is->_itype);
	}

	sx -= ITEM_ANIM_XOFFSET; //is->_iAnimXOffset;
	if (ii == pcursitem) {
		CelDrawOutline(ICOL_BLUE, sx, sy, pCelBuff, nCel, ITEM_ANIM_WIDTH); // is->_iAnimWidth);
	}
	CelClippedDrawLight(sx, sy, pCelBuff, nCel, ITEM_ANIM_WIDTH); //is->_iAnimWidth);
}

/**
 * @brief Draw a towner or a monster depending on the level
 * @param y dPiece coordinate
 * @param x dPiece coordinate
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 */
static void DrawMonsterHelper(int mnum, int x, int y, int sx, int sy)
{
	if (currLvl._dType != DTYPE_TOWN)
		DrawMonster(mnum, x, y, sx, sy);
	else
		DrawTowner(mnum, x, y, sx, sy);
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
	int px, mpnum, nCel, frames;
	BYTE bFlag, bDead, bArch, bMap, dd;
	DeadStruct *pDeadGuy;
	BYTE *pCelBuff;

	assert((unsigned)sx < MAXDUNX);
	assert((unsigned)sy < MAXDUNY);

	//if (dRendered[sx][sy])
	//	return;
	//dRendered[sx][sy] = true;

	light_table_index = dLight[sx][sy];
	bFlag = dFlags[sx][sy];
	bDead = dDead[sx][sy];
	bMap = dTransVal[sx][sy];

	if (bFlag & BFLAG_PLAYERLR) {
		assert((unsigned)(sy - 1) < MAXDUNY);
		mpnum = dPlayer[sx][sy - 1];
		// assert(mpnum < 0);
		if (mpnum < 0)
			DrawPlayer(-(mpnum + 1), sx, sy - 1, dx, dy);
	}
	if (bFlag & BFLAG_MONSTLR) {
		assert((unsigned)(sy - 1) < MAXDUNY);
		mpnum = dMonster[sx][sy - 1];
		// assert(mpnum < 0);
		if (mpnum < 0)
			DrawMonsterHelper(-(mpnum + 1), sx, sy - 1, dx, dy);
	}

	drawCell(sx, sy, dx, dy);

#ifdef _DEBUG
	if (visiondebug && bFlag & BFLAG_LIT) {
		CelClippedDraw(dx, dy, pSquareCel, 1, 64);
	}
#endif

	if (gbMissilePreFlag && (bFlag & BFLAG_MISSILE)) {
		DrawMissile(sx, sy, dx, dy, TRUE);
	}

	if (light_table_index < LIGHTMAX && bDead != 0) {
		pDeadGuy = &dead[(bDead & 0x1F) - 1];
		dd = (bDead >> 5) & 7;
		px = dx - pDeadGuy->_deadXOffset;
		pCelBuff = pDeadGuy->_deadData[dd];
		if (pCelBuff == NULL) {
			dev_fatal("Dead body(%d) without Data(%d) to draw .", bDead, dd);
		}
		frames = SwapLE32(*(uint32_t *)pCelBuff);
		nCel = pDeadGuy->_deadFrame;
		if (nCel < 1 || frames > 50 || nCel > frames) {
			dev_fatal("Unclipped dead: frame %d of %d, deadnum==%d", nCel, frames, bDead);
		}
		if (pDeadGuy->_deadtrans != 0) {
			Cl2DrawLightTbl(px, dy, pCelBuff, nCel, pDeadGuy->_deadWidth, pDeadGuy->_deadtrans);
		} else {
			Cl2DrawLight(px, dy, pCelBuff, nCel, pDeadGuy->_deadWidth);
		}
	}
	DrawObject(sx, sy, dx, dy, TRUE);
	DrawItem(sx, sy, dx, dy, TRUE);
	if (bFlag & BFLAG_DEAD_PLAYER) {
		DrawDeadPlayer(sx, sy, dx, dy);
	}
	mpnum = dPlayer[sx][sy];
	if (mpnum > 0) {
		DrawPlayer(mpnum - 1, sx, sy, dx, dy);
	}
	mpnum = dMonster[sx][sy];
	if (mpnum > 0) {
		DrawMonsterHelper(mpnum - 1, sx, sy, dx, dy);
	}
	if (bFlag & BFLAG_MISSILE)
		DrawMissile(sx, sy, dx, dy, FALSE);
	DrawObject(sx, sy, dx, dy, FALSE);
	DrawItem(sx, sy, dx, dy, FALSE);

	if (currLvl._dType != DTYPE_TOWN) {
		bArch = dSpecial[sx][sy];
		if (bArch != 0) {
			gbCelTransparencyActive = TransList[bMap];
			CelClippedDrawLightTrans(dx, dy, pSpecialCels, bArch, 64);
		}
	} else {
		// Tree leaves should always cover player when entering or leaving the tile,
		// So delay the rendering until after the next row is being drawn.
		// This could probably have been better solved by sprites in screen space.
		if (sx > 0 && sy > 0 && dy > TILE_HEIGHT + SCREEN_Y) {
			bArch = dSpecial[sx - 1][sy - 1];
			if (bArch != 0) {
				CelDraw(dx, (dy - TILE_HEIGHT), pSpecialCels, bArch, 64);
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
	assert(gpBuffer != NULL);

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			if (IN_DUNGEON_AREA(x, y)) {
				level_piece_id = dPiece[x][y];
				assert(level_piece_id != 0);
				//if (level_piece_id != 0) {
					if (!nSolidTable[level_piece_id])
						drawFloor(x, y, sx, sy);
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

#define IsWall(x, y) (/*dPiece[x][y] == 0 ||*/ nSolidTable[dPiece[x][y]] || dSpecial[x][y] != 0)
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

	BYTE *src = &gpBuffer[nSrcOff];
	BYTE *dst = &gpBuffer[nDstOff];

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
 * @brief Gets the number of rows covered by the main panel
 */
int RowsCoveredByPanel()
{
	//if (SCREEN_WIDTH <= PANEL_WIDTH) {
		return 0;
	/*}

	int rows = PANEL_HEIGHT / TILE_HEIGHT;
	if (gbZoomInFlag) {
		rows /= 2;
	}

	return rows;*/
}

/**
 * @brief Calculate the offset needed for centering tiles in view area
 * @param offsetX Offset in pixels
 * @param offsetY Offset in pixels
 */
void CalcTileOffset(int *offsetX, int *offsetY)
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
void TilesInView(int *rcolumns, int *rrows)
{
	int columns = (SCREEN_WIDTH + TILE_WIDTH - 1) / TILE_WIDTH;
	int rows = (VIEWPORT_HEIGHT + TILE_HEIGHT - 1) / TILE_HEIGHT;

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

int tileOffsetX;
int tileOffsetY;
int tileShiftX;
int tileShiftY;
int tileColums;
int tileRows;

void CalcViewportGeometry()
{
	int xo, yo;

	// Adjust by player offset and tile grid alignment
	CalcTileOffset(&xo, &yo);
	tileOffsetX = SCREEN_X - xo;
	tileOffsetY = SCREEN_Y + TILE_HEIGHT / 2 - 1 - yo;

	TilesInView(&tileColums, &tileRows);
	int lrow = tileRows - RowsCoveredByPanel();

	// Center player tile on screen
	tileShiftX = 0;
	tileShiftY = 0;
	SHIFT_GRID(tileShiftX, tileShiftY, -tileColums / 2, -lrow / 2);

	tileRows *= 2;

	// Align grid
	if ((tileColums & 1) == 0) {
		tileShiftY--; // Shift player row to one that can be centered without pixel offset
		if ((lrow & 1) == 0) {
			// Offset tile to vertically align the player when both rows and colums are even
			tileRows++;
			tileOffsetY -= TILE_HEIGHT / 2;
		}
	} else if (/*(tileColums & 1) &&*/ (lrow & 1)) {
		// Offset tile to vertically align the player when both rows and colums are odd
		SHIFT_GRID(tileShiftX, tileShiftY, 0, -1);
		tileRows++;
		tileOffsetY -= TILE_HEIGHT / 2;
	}

	// Slightly lower the zoomed view
	if (gbZoomInFlag) {
		tileOffsetY += TILE_HEIGHT / 4;
		if (yo < TILE_HEIGHT / 4)
			tileRows++;
	}

	tileRows++; // Cover lower edge saw tooth, right edge accounted for in scrollrt_draw()
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
	sx = ScrollInfo._sxoff + tileOffsetX;
	sy = ScrollInfo._syoff + tileOffsetY;

	columns = tileColums;
	rows = tileRows;

	x = ViewX + tileShiftX;
	y = ViewY + tileShiftY;

	// Draw areas moving in and out of the screen
	switch (ScrollInfo._sdir) {
	case SDIR_NONE:
		break;
	case SDIR_N:
		sy -= TILE_HEIGHT;
		SHIFT_GRID(x, y, 0, -1);
		rows += 2;
		break;
	case SDIR_NE:
		sy -= TILE_HEIGHT;
		SHIFT_GRID(x, y, 0, -1);
		columns++;
		rows += 2;
		break;
	case SDIR_E:
		columns++;
		break;
	case SDIR_SE:
		columns++;
		rows++;
		break;
	case SDIR_S:
		rows += 2;
		break;
	case SDIR_SW:
		sx -= TILE_WIDTH;
		SHIFT_GRID(x, y, -1, 0);
		columns++;
		rows++;
		break;
	case SDIR_W:
		sx -= TILE_WIDTH;
		SHIFT_GRID(x, y, -1, 0);
		columns++;
		break;
	case SDIR_NW:
		sx -= TILE_WIDTH / 2;
		sy -= TILE_HEIGHT / 2;
		x--;
		columns++;
		rows++;
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
	//if (drawFlags & (REDRAW_MANA_FLASK | REDRAW_SPELL_ICON)) {
		DrawSkillIcons();
	//}
	DrawLifeFlask();
	DrawManaFlask();

	if (stextflag != STORE_NONE && !gbQtextflag)
		DrawSText();
	if (gbInvflag) {
		DrawInv();
	} else if (gbSbookflag) {
		DrawSpellBook();
	} else if (gbTeamFlag) {
		DrawTeamBook();
	}

	DrawDurIcon();

	//if (drawFlags & REDRAW_SPEED_BAR) {
		DrawInvBelt();
	//}

	if (gbChrflag) {
		DrawChr();
	} else if (gbQuestlog) {
		DrawQuestLog();
	}
	if (gbLvlUp && stextflag == STORE_NONE) {
		DrawLevelUpIcon();
	}
	if (gbQtextflag) {
		DrawQText();
	}
	if (gbSkillListFlag) {
		DrawSkillList();
	}
	if (gbShowTooltip || GetAsyncKeyState(DVL_VK_MENU)) {
		DrawInfoStr();
	}
	if (gbDropGoldFlag) {
		DrawGoldSplit(dropGoldValue);
	}
	if (gbHelpflag) {
		DrawHelp();
	}
	if (currmsg != EMSG_NONE) {
		DrawDiabloMsg();
	}
	if (gbDeathflag) {
		RedBack();
	} else if (gbGamePaused) {
		gmenu_draw_pause();
	}

#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	DrawControllerModifierHints();
#endif
	DrawPlrMsg();
	if (gmenu_is_active())
		gmenu_draw();
	if (gbDoomflag) {
		doom_draw();
	}
	if (gbTalkflag) {
		DrawTalkPan();
	}
	//if (drawFlags & REDRAW_CTRL_BUTTONS) {
		DrawCtrlBtns();
	//}
}

/**
 * @brief Render the whole screen black
 */
void ClearScreenBuffer()
{
	lock_buf(3);

	assert(pal_surface != NULL);

	SDL_Rect SrcRect = {
		SCREEN_X,
		SCREEN_Y,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
	};
	SDL_FillRect(pal_surface, &SrcRect, 0);

	unlock_buf(3);
}

#ifdef _DEBUG
/**
 * @brief Scroll the screen when mouse is close to the edge
 */
void ScrollView()
{
	bool scroll;

	if (pcurs >= CURSOR_FIRSTITEM)
		return;

	scroll = false;

	if (MouseX < 20) {
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
	if (MouseX > SCREEN_WIDTH - 20) {
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
	if (MouseY < 20) {
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
	if (MouseY > SCREEN_HEIGHT - 20) {
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
 * @brief Initialize the FPS meter
 */
/*void EnableFrameCount()
{
	gbFrameflag = !gbFrameflag;
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
	PrintGameStr(8, 65, tempstr, COL_RED);
}

/**
 * @brief Update part of the screen from the back buffer
 * @param dwX Back buffer coordinate
 * @param dwY Back buffer coordinate
 * @param dwWdt Back buffer coordinate
 * @param dwHgt Back buffer coordinate
 */
/*static void DoBlitScreen(int dwX, int dwY, int dwWdt, int dwHgt)
{
	SDL_Rect SrcRect = {
		dwX + SCREEN_X,
		dwY + SCREEN_Y,
		dwWdt,
		dwHgt,
	};
	SDL_Rect DstRect = {
		dwX,
		dwY,
		dwWdt,
		dwHgt,
	};

	BltFast(&SrcRect, &DstRect);
}*/
static void DoBlitScreen()
{
	SDL_Rect SrcRect = {
		SCREEN_X,
		SCREEN_Y,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
	};
	SDL_Rect DstRect = {
		0,
		0,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
	};

	BltFast(&SrcRect, &DstRect);
}

/**
 * @brief Check render pipeline and blit individual screen parts
 * @param dwHgt Section of screen to update from top to bottom
 * @param drawFlags Render parts of the screen
 */
/*static void DrawMain(int dwHgt, int drawFlags)
{
	int ysize;

	ysize = dwHgt;

	if (!gbActive) {
		return;
	}

	assert(ysize >= 0 && ysize <= SCREEN_HEIGHT);
	//ysize = SCREEN_HEIGHT;
	if (ysize > 0) {
		DoBlitScreen(0, 0, SCREEN_WIDTH, ysize);
	}
	if (ysize < SCREEN_HEIGHT) {
		//if (drawFlags & REDRAW_SPEED_BAR) {
			//DoBlitScreen(PANEL_LEFT + 204, PANEL_TOP + 5, 232, 28);
			DoBlitScreen(InvRect[SLOTXY_BELT_FIRST].X, SCREEN_HEIGHT - InvRect[SLOTXY_BELT_FIRST].Y - (INV_SLOT_SIZE_PX + 1), 2 * (INV_SLOT_SIZE_PX + 1), 4 * (INV_SLOT_SIZE_PX + 1));
		//}
		//if (drawFlags & REDRAW_DESCRIPTION) {
		//	DoBlitScreen(PANEL_LEFT + 176, PANEL_TOP + 46, 288, 60);
		//}
		//if (drawFlags & REDRAW_MANA_FLASK) {
			DoBlitScreen(PANEL_LEFT + 460, PANEL_TOP, 88, 72);
		//}
		//if (drawFlags & (REDRAW_MANA_FLASK | REDRAW_SPELL_ICON)) {
			DoBlitScreen(SCREEN_WIDTH - SPLICONLENGTH, SCREEN_HEIGHT - SPLICONLENGTH, SPLICONLENGTH, SPLICONLENGTH);
		//}
		//if (drawFlags & REDRAW_HP_FLASK) {
			DoBlitScreen(PANEL_LEFT + 96, PANEL_TOP, 88, 72);
		//}
		//if (drawFlags & REDRAW_CTRL_BUTTONS) {
			//DoBlitScreen(PANEL_LEFT + 8, PANEL_TOP + 5, 72, 119);
			//DoBlitScreen(PANEL_LEFT + 556, PANEL_TOP + 5, 72, 48);
			//if (!IsLocalGame) {
			//	DoBlitScreen(PANEL_LEFT + 84, PANEL_TOP + 91, 36, 32);
			//	DoBlitScreen(PANEL_LEFT + 524, PANEL_TOP + 91, 36, 32);
			//}
			int y = (!gabPanbtn[PANBTN_MAINMENU] && !(drawFlags & REDRAW_CTRL_BUTTONS) ? 1 : numpanbtns) * MENUBTN_HEIGHT;
			DoBlitScreen(0, SCREEN_HEIGHT - 1 - y, MENUBTN_WIDTH, y);
		//}

		if (sgCursWdtOld != 0) {
			DoBlitScreen(sgCursXOld, sgCursYOld, sgCursWdtOld, sgCursHgtOld);
		}
		if (sgCursWdt != 0) {
			DoBlitScreen(sgCursX, sgCursY, sgCursWdt, sgCursHgt);
		}
	}
}*/

/**
 * @brief Redraw screen
 * @param draw_cursor
 */
void scrollrt_draw_game_screen(bool draw_cursor)
{
	//int hgt;
	bool redraw;

	if (gbRedrawFlags == REDRAW_ALL) {
		gbRedrawFlags = 0;
		//hgt = SCREEN_HEIGHT;
		redraw = true;
	} else {
		//hgt = 0;
		redraw = false;
	}

	if (draw_cursor) {
		lock_buf(0);
		scrollrt_draw_cursor_item();
		unlock_buf(0);
	}

	//DrawMain(hgt, 0);
	if (redraw && gbActive) {
		DoBlitScreen();
	}

	if (draw_cursor) {
		lock_buf(0);
		scrollrt_draw_cursor_back_buffer();
		unlock_buf(0);
	}
	RenderPresent();
}

/**
 * @brief Render the game
 */
void DrawAndBlit()
{
	if (!gbRunGame) {
		return;
	}

	lock_buf(0);
	DrawView();
	scrollrt_draw_cursor_item();

	if (gbFrameflag)
		DrawFPS();

	unlock_buf(0);

	//DrawMain(hgt, drawFlags);
	if (gbActive) {
		DoBlitScreen();
	}

	lock_buf(0);
	scrollrt_draw_cursor_back_buffer();
	unlock_buf(0);
	RenderPresent();

	gbRedrawFlags = 0;
}

DEVILUTION_END_NAMESPACE
