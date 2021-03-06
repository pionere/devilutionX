/**
 * @file dun_render.hpp
 *
 * Interface of functionality for rendering the level tiles.
 */
#ifndef __DUN_RENDER_H__
#define __DUN_RENDER_H__

#include "../types.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

void RenderTile(BYTE *pBuff, uint16_t levelCelBlock, _arch_draw_type adt);
void world_draw_black_tile(int sx, int sy);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DUN_RENDER_H__ */
