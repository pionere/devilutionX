/**
 * @file raw_render.cpp
 *
 * Basic drawing routines.
 */
#include "raw_render.h"

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#if DEBUG_MODE
/** flip - if y < x */
bool _gbRotateMap;
#endif

/**
 * @brief Set the value of a single pixel in the back buffer, DOES NOT checks bounds
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param col Color index from current palette
 */
void DrawPixel(int sx, int sy, BYTE col)
{
	BYTE* dst;

	//assert(gpBuffer != NULL);

	//if (sy < SCREEN_Y || sy >= SCREEN_HEIGHT + SCREEN_Y || sx < SCREEN_X || sx >= SCREEN_WIDTH + SCREEN_X)
	//	return;

	dst = &gpBuffer[BUFFERXY(sx, sy)];
	//if (dst < gpBufEnd && dst > gpBufStart)
		*dst = col;
}

#if DEBUG_MODE
/**
 * @brief Set the value of a single pixel in the back buffer, checks bounds
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param col Color index from current palette
 */
void DrawPixelSafe(int sx, int sy, BYTE col)
{
	BYTE* dst;

	assert(gpBuffer != NULL);

	if (_gbRotateMap) {
		if (sx < 0 || sx >= SCREEN_HEIGHT + SCREEN_Y || sy < SCREEN_X || sy >= SCREEN_WIDTH + SCREEN_X)
			return;
		dst = &gpBuffer[BUFFERXY(sy, sx)];
	} else {
		if (sy < 0 || sy >= SCREEN_HEIGHT + SCREEN_Y || sx < SCREEN_X || sx >= SCREEN_WIDTH + SCREEN_X)
			return;
		dst = &gpBuffer[BUFFERXY(sx, sy)];
	}

	if (dst < gpBufEnd && dst >= gpBufStart)
		*dst = col;
}
#endif

/**
 * @brief Draw a line on the back buffer from [x0; y0] to [x1; y1] inclusive on both ends
 * @param x0 Back buffer coordinate
 * @param y0 Back buffer coordinate
 * @param x1 Back buffer coordinate
 * @param y1 Back buffer coordinate
 * @param col Color index from current palette
 */
void DrawLine(int x0, int y0, int x1, int y1, BYTE col)
{
	int d, xyinc, dx, dy, tmp;
	BYTE* dst;

	dx = x1 - x0;
	dy = y1 - y0;
	if (abs(dx) >= abs(dy)) {
		if (dx == 0)
			return; // this should never happen, and if this is the case, we might want to draw at least once?
		// alway proceed from lower to higher x
		if (dx < 0) {
			tmp = x0;
			x0 = x1;
			x1 = tmp;
			tmp = y0;
			y0 = y1;
			y1 = tmp;
			dx = -dx;
			dy = -dy;
		}
		// find out step size and direction on the y coordinate
		xyinc = BUFFER_WIDTH;
		if (dy < 0) {
			dy = -dy;
			xyinc = -xyinc;
		}
		// multiply by 2 so we round up
		//dy *= 2;
		d = 0;
		// draw to the final position as well
		dx++;
		dy++;
		x1++;
		// initialize the buffer-pointer
		dst = &gpBuffer[BUFFERXY(x0, y0)];
		while (true) {
			*dst = col; // DrawPixel
			d += dy;
			if (d >= dx) {
				d -= /*2 **/ dx; // multiply by 2 to support rounding
				dst += xyinc;
			}
			x0++;
			dst++;
			if (x0 == x1)
				return;
		}
	} else {
		// alway proceed from lower to higher y
		if (dy < 0) {
			tmp = y0;
			y0 = y1;
			y1 = tmp;
			tmp = x0;
			x0 = x1;
			x1 = tmp;
			dy = -dy;
			dx = -dx;
		}
		// find out step size and direction on the x coordinate
		if (dx >= 0) {
			xyinc = 1;
		} else {
			dx = -dx;
			xyinc = -1;
		}
		// multiply by 2 so we round up
		//dx *= 2;
		d = 0;
		// draw to the final position as well
		dx++;
		dy++;
		y1++;
		// initialize the buffer-pointer
		dst = &gpBuffer[BUFFERXY(x0, y0)];
		while (true) {
			*dst = col; // DrawPixel
			d += dx;
			if (d >= dy) {
				d -= /*2 **/ dy; // multiply by 2 to support rounding
				dst += xyinc;
			}
			y0++;
			dst += BUFFER_WIDTH;
			if (y0 == y1)
				return;
		}
	}
}

/**
 * @brief Draw a half-transparent, black rectangle using stippled-transparency
 * @param sx left side of the rectangle (back buffer coordinate)
 * @param sy top of the rectangle (back buffer coordinate)
 * @param width Rectangle width
 * @param height Rectangle height
 */
/*void DrawRectTrans(int sx, int sy, int width, int height)
{
	int row, col;
	BYTE* pix = &gpBuffer[BUFFERXY(sx, sy)];
	uint32_t mask;

	for (row = height; row > 0; row--) {
		mask = 0x00FF00FF << ((row & 1) * 8);
		col = width;
		while (col >= 4) {
			col -= 4;
			*(uint32_t*)pix &= mask;
			pix += 4;
		}
		while (--col >= 0) {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			*pix &= ((BYTE*)&mask)[3];
			mask <<= 8;
#else
			*pix &= mask;
			mask >>= 8;
#endif
			pix++;
		}
		pix += BUFFER_WIDTH - width;
	}
}*/
/**
 * @brief Draw a half-transparent, black rectangle using stippled-transparency
 * @param sx left side of the rectangle (back buffer coordinate)
 * @param sy top of the rectangle (back buffer coordinate)
 * @param width Rectangle width
 * @param height Rectangle height
 * @param color the color of the rectangle
 */
void DrawRectTrans(int sx, int sy, int width, int height, BYTE color)
{
	int y, x;
	BYTE* pix = &gpBuffer[BUFFERXY(sx, sy)];

	for (y = 0; y < height; y++) {
		for (x = width; x > 0; x -= 2, pix += 2) {
			*pix = color;
		}
		pix += BUFFER_WIDTH - (width - x + ((y & 1) ? -1 : 1));
	}
}

DEVILUTION_END_NAMESPACE
