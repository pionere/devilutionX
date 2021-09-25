/**
 * @file dun_render.cpp
 *
 * Implementation of functionality for rendering the level tiles.
 */
#include "dun_render.hpp"

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** Specifies the draw masks used to render transparency of the right side of tiles. */
static uint32_t RightMask[TILE_HEIGHT] = {
	0xEAAAAAAA, 0xF5555555,
	0xFEAAAAAA, 0xFF555555,
	0xFFEAAAAA, 0xFFF55555,
	0xFFFEAAAA, 0xFFFF5555,
	0xFFFFEAAA, 0xFFFFF555,
	0xFFFFFEAA, 0xFFFFFF55,
	0xFFFFFFEA, 0xFFFFFFF5,
	0xFFFFFFFE, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF
};
/** Specifies the draw masks used to render transparency of the left side of tiles. */
static uint32_t LeftMask[TILE_HEIGHT] = {
	0xAAAAAAAB, 0x5555555F,
	0xAAAAAABF, 0x555555FF,
	0xAAAAABFF, 0x55555FFF,
	0xAAAABFFF, 0x5555FFFF,
	0xAAABFFFF, 0x555FFFFF,
	0xAABFFFFF, 0x55FFFFFF,
	0xABFFFFFF, 0x5FFFFFFF,
	0xBFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF
};
/** Specifies the draw masks used to render transparency of wall tiles. */
static uint32_t WallMask[TILE_HEIGHT] = {
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555
};

static uint32_t SolidMask[TILE_HEIGHT] = {
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF
};

static uint32_t RightFoliageMask[TILE_HEIGHT] = {
	0xFFFFFFFF, 0x3FFFFFFF,
	0x0FFFFFFF, 0x03FFFFFF,
	0x00FFFFFF, 0x003FFFFF,
	0x000FFFFF, 0x0003FFFF,
	0x0000FFFF, 0x00003FFF,
	0x00000FFF, 0x000003FF,
	0x000000FF, 0x0000003F,
	0x0000000F, 0x00000003,
	0x00000000, 0x00000003,
	0x0000000F, 0x0000003F,
	0x000000FF, 0x000003FF,
	0x00000FFF, 0x00003FFF,
	0x0000FFFF, 0x0003FFFF,
	0x000FFFFF, 0x003FFFFF,
	0x00FFFFFF, 0x03FFFFFF,
	0x0FFFFFFF, 0x3FFFFFFF,
};

static uint32_t LeftFoliageMask[TILE_HEIGHT] = {
	0xFFFFFFFF, 0xFFFFFFFC,
	0xFFFFFFF0, 0xFFFFFFC0,
	0xFFFFFF00, 0xFFFFFC00,
	0xFFFFF000, 0xFFFFC000,
	0xFFFF0000, 0xFFFC0000,
	0xFFF00000, 0xFFC00000,
	0xFF000000, 0xFC000000,
	0xF0000000, 0xC0000000,
	0x00000000, 0xC0000000,
	0xF0000000, 0xFC000000,
	0xFF000000, 0xFFC00000,
	0xFFF00000, 0xFFFC0000,
	0xFFFF0000, 0xFFFFC000,
	0xFFFFF000, 0xFFFFFC00,
	0xFFFFFF00, 0xFFFFFFC0,
	0xFFFFFFF0, 0xFFFFFFFC,
};

inline static void RenderLine(BYTE* dst, BYTE* src, int n, uint32_t mask, int light)
{
//#ifdef NO_OVERDRAW
//	if (dst >= gpBufStart && dst <= gpBufEnd)
//#endif
	{
		assert(n != 0);
		int i = ((sizeof(uint32_t) * CHAR_BIT) - n);
		BYTE* tbl;
		// Add the lower bits about we don't care.
		mask |= (1 << i) - 1;
		if (mask == 0xFFFFFFFF) {
			if (light == LIGHTMAX) {
				memset(dst, 0, n);
			} else if (light == 0) {
				memcpy(dst, src, n);
			} else {
				tbl = LightTrns[light];
				for (i = 0; i < n; i++) {
					dst[i] = tbl[src[i]];
				}
			}
		} else {
			// Clear the lower bits of the mask to avoid testing i < n in the loops.
			mask = (mask >> i) << i;
			if (light == LIGHTMAX) {
				for (i = 0; mask != 0; i++, mask <<= 1) {
					if (mask & 0x80000000) {
						dst[i] = 0;
					}
				}
			} else if (light == 0) {
				for (i = 0; mask != 0; i++, mask <<= 1) {
					if (mask & 0x80000000) {
						dst[i] = src[i];
					}
				}
			} else {
				tbl = LightTrns[light];
				for (i = 0; mask != 0; i++, mask <<= 1) {
					if (mask & 0x80000000) {
						dst[i] = tbl[src[i]];
					}
				}
			}
		}
	}

	//(*src) += n;
	//(*dst) += n;
}

#if defined(__clang__) || defined(__GNUC__)
__attribute__((no_sanitize("shift-base")))
#endif
/**
 * @brief Blit current world CEL to the given buffer
 * @param pBuff Output buffer
 * @param levelCelBlock block/tile to draw
 *   the current MIN block of the level CEL file, as used during rendering of the level tiles.
 *      frameNum  := block & 0x0FFF
 *      frameType := block & 0x7000 >> 12
 * @param adt the type of arches to render.
 */
void RenderTile(BYTE *pBuff, uint16_t levelCelBlock, _arch_draw_type adt)
{
	int i, j, light;
	char v, tile;
	BYTE *src, *dst;
	uint32_t m, *mask, *pFrameTable;

	dst = pBuff;
	pFrameTable = (uint32_t *)pDungeonCels;

	src = &pDungeonCels[SwapLE32(pFrameTable[levelCelBlock & 0xFFF])];
	tile = (levelCelBlock & 0x7000) >> 12;

	mask = &SolidMask[TILE_HEIGHT - 1];

	if (gbCelTransparencyActive) {
		if (adt == RADT_NONE) {
			mask = &WallMask[TILE_HEIGHT - 1];
		} else if (adt == RADT_LEFT) {
			if (tile != RT_LTRIANGLE) {
				if (pieceFlags[level_piece_id] & PFLAG_TRANS_MASK_LEFT) {
					mask = &LeftMask[TILE_HEIGHT - 1];
				}
			}
		} else {
			// assert(adt == RADT_RIGHT);
			if (tile != RT_RTRIANGLE) {
				if (pieceFlags[level_piece_id] & PFLAG_TRANS_MASK_RIGHT) {
					mask = &RightMask[TILE_HEIGHT - 1];
				}
			}
		}
	} else if (adt != RADT_NONE && gbCelFoliageActive) {
		if (tile != RT_TRANSPARENT) {
			return;
		}
		if (adt == RADT_LEFT) {
			mask = &LeftFoliageMask[TILE_HEIGHT - 1];
		} else {
			// assert(adt == RADT_RIGHT);
			mask = &RightFoliageMask[TILE_HEIGHT - 1];
		}
	}

#ifdef _DEBUG
	if (GetAsyncKeyState(DVL_VK_MENU)) {
		mask = &SolidMask[TILE_HEIGHT - 1];
	}
#endif

	light = light_table_index;
	static_assert(TILE_HEIGHT - 2 < TILE_WIDTH / 2, "Line with negative or zero width.");
	static_assert(TILE_WIDTH / 2 <= sizeof(*mask) * CHAR_BIT, "Mask is too small to cover the tile.");
	switch (tile) {
	case RT_SQUARE:
		for (i = TILE_HEIGHT; i != 0; i--, dst -= BUFFER_WIDTH + TILE_WIDTH / 2, mask--) {
			RenderLine(dst, src, TILE_WIDTH / 2, *mask, light);
			src += TILE_WIDTH / 2;
			dst += TILE_WIDTH / 2;
		}
		break;
	case RT_TRANSPARENT:
		for (i = TILE_HEIGHT; i != 0; i--, dst -= BUFFER_WIDTH + TILE_WIDTH / 2, mask--) {
			m = *mask;
			static_assert(TILE_WIDTH / 2 <= sizeof(m) * CHAR_BIT, "Undefined left-shift behavior.");
			for (j = TILE_WIDTH / 2; j != 0; j -= v, m <<= v) {
				v = *src++;
				if (v > 0) {
					RenderLine(dst, src, v, m, light);
					src += v;
					dst += v;
				} else {
					v = -v;
					dst += v;
				}
			}
		}
		break;
	case RT_LTRIANGLE:
		for (i = TILE_HEIGHT - 2; i >= 0; i -= 2, dst -= BUFFER_WIDTH + TILE_WIDTH / 2, mask--) {
			src += i & 2;
			dst += i;
			RenderLine(dst, src, TILE_WIDTH / 2 - i, *mask, light);
			src += TILE_WIDTH / 2 - i;
			dst += TILE_WIDTH / 2 - i;
		}
		for (i = 2; i != TILE_WIDTH / 2; i += 2, dst -= BUFFER_WIDTH + TILE_WIDTH / 2, mask--) {
			src += i & 2;
			dst += i;
			RenderLine(dst, src, TILE_WIDTH / 2 - i, *mask, light);
			src += TILE_WIDTH / 2 - i;
			dst += TILE_WIDTH / 2 - i;
		}
		break;
	case RT_RTRIANGLE:
		for (i = TILE_HEIGHT - 2; i >= 0; i -= 2, dst -= BUFFER_WIDTH + TILE_WIDTH / 2, mask--) {
			RenderLine(dst, src, TILE_WIDTH / 2 - i, *mask, light);
			src += TILE_WIDTH / 2 - i;
			dst += TILE_WIDTH / 2 - i;
			src += i & 2;
			dst += i;
		}
		for (i = 2; i != TILE_HEIGHT; i += 2, dst -= BUFFER_WIDTH + TILE_WIDTH / 2, mask--) {
			RenderLine(dst, src, TILE_WIDTH / 2 - i, *mask, light);
			src += TILE_WIDTH / 2 - i;
			dst += TILE_WIDTH / 2 - i;
			src += i & 2;
			dst += i;
		}
		break;
	case RT_LTRAPEZOID:
		for (i = TILE_HEIGHT - 2; i >= 0; i -= 2, dst -= BUFFER_WIDTH + TILE_WIDTH / 2, mask--) {
			src += i & 2;
			dst += i;
			RenderLine(dst, src, TILE_WIDTH / 2 - i, *mask, light);
			src += TILE_WIDTH / 2 - i;
			dst += TILE_WIDTH / 2 - i;
		}
		for (i = TILE_HEIGHT / 2; i != 0; i--, dst -= BUFFER_WIDTH + TILE_WIDTH / 2, mask--) {
			RenderLine(dst, src, TILE_WIDTH / 2, *mask, light);
			src += TILE_WIDTH / 2;
			dst += TILE_WIDTH / 2;
		}
		break;
	case RT_RTRAPEZOID:
		for (i = TILE_HEIGHT - 2; i >= 0; i -= 2, dst -= BUFFER_WIDTH + TILE_WIDTH / 2, mask--) {
			RenderLine(dst, src, TILE_WIDTH / 2 - i, *mask, light);
			src += TILE_WIDTH / 2 - i;
			dst += TILE_WIDTH / 2 - i;
			src += i & 2;
			dst += i;
		}
		for (i = TILE_HEIGHT / 2; i != 0; i--, dst -= BUFFER_WIDTH + TILE_WIDTH / 2, mask--) {
			RenderLine(dst, src, TILE_WIDTH / 2, *mask, light);
			src += TILE_WIDTH / 2;
			dst += TILE_WIDTH / 2;
		}
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

/**
 * @brief Render a black tile
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 */
void world_draw_black_tile(int sx, int sy)
{
	int i;
	BYTE *dst;

	if (sx < SCREEN_X - (TILE_WIDTH - 4) || sx >= SCREEN_X + SCREEN_WIDTH)
		return;

	if (sy <= SCREEN_Y || sy >= SCREEN_Y + VIEWPORT_HEIGHT + TILE_HEIGHT)
		return;

	dst = &gpBuffer[sx + BUFFER_WIDTH * sy] + TILE_WIDTH / 2 - 2;

	for (i = 1; i <= TILE_HEIGHT / 2; i++, dst -= BUFFER_WIDTH + 2) {
//#ifdef NO_OVERDRAW
//		if (dst < gpBufEnd)
//#endif
			memset(dst, 0, 4 * i);
	}
	dst += 4;
	for (i = TILE_HEIGHT / 2 - 1; i >= 1; i--, dst -= BUFFER_WIDTH - 2) {
//#ifdef NO_OVERDRAW
//		if (dst < gpBufEnd)
//#endif
			memset(dst, 0, 4 * i);
	}
}

DEVILUTION_END_NAMESPACE
