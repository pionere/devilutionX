/**
 * @file scrollrt.cpp
 *
 * Implementation of functionality for rendering the dungeons, monsters and calling other render routines.
 */
#include "all.h"
#include "utils/display.h"
#include "plrctrls.h"
#include "engine/render/cel_render.h"

DEVILUTION_BEGIN_NAMESPACE

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
BYTE sgSaveBack[MAX_CURSOR_AREA];

/**
 * @brief Clear cursor state
 */
void ClearCursor() // CODE_FIX: this was supposed to be in cursor.cpp
{
	sgCursWdt = 0;
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

	sgCursWdt = 0;
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
	if (sgbControllerActive && !IsMovingMouseCursorWithController())
		return;
#endif

	mx = MousePos.x;
	my = MousePos.y;
	// assert(pcursicon < CURSOR_FIRSTITEM); -- hotspot is always 0:0
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

	sgCursX = mx;
	sgCursWdt = sgCursX + cursW;
	// cut the cursor on the right side
	//if (sgCursWdt > SCREEN_WIDTH) {
	//	sgCursWdt = SCREEN_WIDTH;
	//}
	// cut the cursor on the left side
	//if (sgCursX <= 0) {
	//	sgCursX = 0;
	//} else {
		// draw to 4-byte aligned blocks
		sgCursX &= ~3;
		sgCursWdt -= sgCursX;
	//}
	// draw with 4-byte alignment
	sgCursWdt += 3;
	sgCursWdt &= ~3;

	sgCursY = my;
	sgCursHgt = sgCursY + cursH;
	// cut the cursor on the bottom
	//if (sgCursHgt > SCREEN_HEIGHT) {
	//	sgCursHgt = SCREEN_HEIGHT;
	//}
	// cut the cursor on the top
	//if (sgCursY <= 0) {
	//	sgCursY = 0;
	//} else {
		sgCursHgt -= sgCursY;
	//}

	assert((unsigned)(sgCursWdt * sgCursHgt) <= sizeof(sgSaveBack));
	assert(gpBuffer != NULL);
	dst = sgSaveBack;
	src = &gpBuffer[SCREENXY(sgCursX, sgCursY)];

	for (i = sgCursHgt; i != 0; i--, dst += sgCursWdt, src += BUFFER_WIDTH) {
		memcpy(dst, src, sgCursWdt);
	}

	mx += SCREEN_X;
	my += cursH + SCREEN_Y - 1;

	frame = pcursicon;
	cCels = pCursCels;

	CelClippedDraw(mx, my, cCels, frame, cursW);
}

/**
 * @brief Redraw screen
 * @param draw_cursor
 */
void scrollrt_render_screen(bool draw_cursor)
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
