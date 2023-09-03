/**
 * @file dun_render.h
 *
 * Interface of functionality for rendering the level tiles.
 */
#ifndef __DUN_RENDER_H__
#define __DUN_RENDER_H__

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#define MICRO_WIDTH  32
#define MICRO_HEIGHT 32

#ifdef __cplusplus
extern "C" {
#endif

void RenderMicro(BYTE* pBuff, uint16_t levelCelBlock, int maskType);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DUN_RENDER_H__ */
