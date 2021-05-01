/**
 * @file render.h
 *
 * Interface of functionality for rendering the level tiles.
 */
#ifndef __RENDER_H__
#define __RENDER_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

void RenderTile(BYTE *pBuff, uint16_t levelCelBlock, _arch_draw_type adt);
void world_draw_black_tile(int sx, int sy);
void trans_rect(int sx, int sy, int width, int height);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __RENDER_H__ */
