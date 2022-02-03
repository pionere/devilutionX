/**
 * @file automap_render.cpp
 *
 * Line drawing routines for the automap.
 */
#include "automap_render.hpp"

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#if DEBUG_MODE
/** automap pixel color 8-bit (palette entry) */
char gbPixelCol;
/** flip - if y < x */
bool _gbRotateMap;
/** valid - if x/y are in bounds */
bool _gbNotInView;
/** Number of times the current seed has been fetched */
#endif

/**
 * @brief Set the value of a single pixel in the back buffer, DOES NOT checks bounds
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param col Color index from current palette
 */
void AutomapDrawPixel(int sx, int sy, BYTE col)
{
	BYTE *dst;

	//assert(gpBuffer != NULL);

	//if (sy < SCREEN_Y || sy >= SCREEN_HEIGHT + SCREEN_Y || sx < SCREEN_X || sx >= SCREEN_WIDTH + SCREEN_X)
	//	return;

	dst = &gpBuffer[sx + BUFFER_WIDTH * sy];
	//if (dst < gpBufEnd && dst > gpBufStart)
		*dst = col;
}

#if DEBUG_MODE
/**
 * @brief Set the value of a single pixel in the back buffer to that of gbPixelCol, checks bounds
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 */
void engine_draw_pixel(int sx, int sy)
{
	BYTE *dst;

	assert(gpBuffer != NULL);

	if (_gbRotateMap) {
		if (_gbNotInView && (sx < 0 || sx >= SCREEN_HEIGHT + SCREEN_Y || sy < SCREEN_X || sy >= SCREEN_WIDTH + SCREEN_X))
			return;
		dst = &gpBuffer[sy + BUFFER_WIDTH * sx];
	} else {
		if (_gbNotInView && (sy < 0 || sy >= SCREEN_HEIGHT + SCREEN_Y || sx < SCREEN_X || sx >= SCREEN_WIDTH + SCREEN_X))
			return;
		dst = &gpBuffer[sx + BUFFER_WIDTH * sy];
	}

	if (dst < gpBufEnd && dst > gpBufStart)
		*dst = gbPixelCol;
}
#endif

/**
 * @brief Draw a line on the back buffer
 * @param x0 Back buffer coordinate
 * @param y0 Back buffer coordinate
 * @param x1 Back buffer coordinate
 * @param y1 Back buffer coordinate
 * @param col Color index from current palette
 */
void AutomapDrawLine(int x0, int y0, int x1, int y1, BYTE col)
{
	int di, ip, dx, dy, ax, ay, steps;
	float df, fp;

	dx = x1 - x0;
	dy = y1 - y0;
	ax = abs(dx);
	ay = abs(dy);
	if (ax > ay) {
		steps = ax;
		di = dx / ax;
		df = dy / (float)steps;
		ip = x0;
		fp = (float)y0;
		for ( ; steps >= 0; steps--, ip += di, fp += df) {
			AutomapDrawPixel(ip, (int)fp, col);
		}
	} else {
		steps = ay;
		di = dy / ay;
		df = dx / float(steps);
		fp = (float)x0;
		ip = y0;
		for ( ; steps >= 0; steps--, fp += df, ip += di) {
			AutomapDrawPixel((int)fp, ip, col);
		}
	}
}

DEVILUTION_END_NAMESPACE
