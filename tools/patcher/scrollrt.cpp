/**
 * @file scrollrt.cpp
 *
 * Implementation of functionality for rendering the dungeons, monsters and calling other render routines.
 */
#include "all.h"
#include "utils/display.h"
//#include "plrctrls.h"
#include "engine/render/cel_render.h"
#if GAME
#include "engine/render/cl2_render.h"
#include "engine/render/dun_render.h"
#include "engine/render/text_render.h"
#endif

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
#if GAME
	} else {
		scrollrt_draw_item(&myplr._pHoldItem, true, mx, my, cCels, frame, cursW);
#endif // GAME
	}
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

DEVILUTION_END_NAMESPACE
