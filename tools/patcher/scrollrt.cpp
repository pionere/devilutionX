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

	CelClippedDraw(mx, my, cCels, frame, cursW);
}

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

DEVILUTION_END_NAMESPACE
