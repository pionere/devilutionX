/**
 * @file automap_render.h
 */
#ifndef __AUTOMAP_RENDER_H__
#define __AUTOMAP_RENDER_H__

#include "../types.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

void AutomapDrawPixel(int sx, int sy, BYTE col);
void AutomapDrawLine(int x0, int y0, int x1, int y1, BYTE col);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __AUTOMAP_RENDER_H__ */
