/**
 * @file dun_render.hpp
 *
 * Interface of functionality for rendering the level tiles.
 */
#ifndef __DUN_RENDER_H__
#define __DUN_RENDER_H__

#include "../types.h"

DEVILUTION_BEGIN_NAMESPACE

#define MICRO_WIDTH  32
#define MICRO_HEIGHT 32

#ifdef __cplusplus
extern "C" {
#endif

void RenderMicro(BYTE* pBuff, uint16_t levelCelBlock, int maskType);
void world_draw_black_tile(int sx, int sy);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DUN_RENDER_H__ */
