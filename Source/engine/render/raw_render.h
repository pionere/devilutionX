/**
 * @file raw_render.h
 */
#ifndef __RAW_RENDER_H__
#define __RAW_RENDER_H__

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Set the value of a single pixel in the back buffer, DOES NOT checks bounds
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param col Color index from current palette
 */
void DrawPixel(int sx, int sy, BYTE col);
/**
 * @brief Draw a line on the back buffer
 * @param x0 Back buffer coordinate
 * @param y0 Back buffer coordinate
 * @param x1 Back buffer coordinate
 * @param y1 Back buffer coordinate
 * @param col Color index from current palette
 */
void DrawLine(int x0, int y0, int x1, int y1, BYTE col);
/**
 * @brief Draw a half-transparent rectangle using stippled-transparency
 * @param sx left side of the rectangle (back buffer coordinate)
 * @param sy top of the rectangle (back buffer coordinate)
 * @param width Rectangle width
 * @param height Rectangle height
 * @param color the color of the rectangle
 */
void DrawRectTrans(int sx, int sy, int width, int height, BYTE col);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __RAW_RENDER_H__ */
