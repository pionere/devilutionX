/**
 * @file dun_render.cpp
 *
 * Implementation of functionality for rendering the level tiles.
 */
#include "dun_render.h"

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** Specifies the draw masks used to render transparency of the right side of tiles. */
static uint32_t RightMask[MICRO_HEIGHT] = {
	// clang-format off
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
	// clang-format on
};
#if ASSET_MPL != 1
static uint32_t UpperRightMask[MICRO_HEIGHT] = {
	// clang-format off
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xEAAAAAAA, 0xF5555555,
	0xFEAAAAAA, 0xFF555555,
	0xFFEAAAAA, 0xFFF55555,
	0xFFFEAAAA, 0xFFFF5555,
	0xFFFFEAAA, 0xFFFFF555,
	0xFFFFFEAA, 0xFFFFFF55,
	0xFFFFFFEA, 0xFFFFFFF5,
	0xFFFFFFFE, 0xFFFFFFFF,
	// clang-format on
};
#endif
/** Specifies the draw masks used to render transparency of the left side of tiles. */
static uint32_t LeftMask[MICRO_HEIGHT] = {
	// clang-format off
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
	// clang-format on
};
#if ASSET_MPL != 1
static uint32_t UpperLeftMask[MICRO_HEIGHT] = {
	// clang-format off
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAB, 0x5555555F,
	0xAAAAAABF, 0x555555FF,
	0xAAAAABFF, 0x55555FFF,
	0xAAAABFFF, 0x5555FFFF,
	0xAAABFFFF, 0x555FFFFF,
	0xAABFFFFF, 0x55FFFFFF,
	0xABFFFFFF, 0x5FFFFFFF,
	0xBFFFFFFF, 0xFFFFFFFF,
	// clang-format on
};
#endif
/** Specifies the draw masks used to render transparency of wall tiles. */
static uint32_t WallMask[MICRO_HEIGHT] = {
	// clang-format off
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
	// clang-format on
};

static uint32_t SolidMask[MICRO_HEIGHT] = {
	// clang-format off
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
	// clang-format on
};

static uint32_t RightFoliageMask[MICRO_HEIGHT] = {
	// clang-format off
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
	// clang-format on
};
#if ASSET_MPL != 1
static uint32_t TopRightFoliageMask[MICRO_HEIGHT] = {
	// clang-format off
	0xFFFFFFFF, 0x3FFFFFFF,
	0x0FFFFFFF, 0x03FFFFFF,
	0x00FFFFFF, 0x003FFFFF,
	0x000FFFFF, 0x0003FFFF,
	0x0000FFFF, 0x00003FFF,
	0x00000FFF, 0x000003FF,
	0x000000FF, 0x0000003F,
	0x0000000F, 0x00000003,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	// clang-format on
};
static uint32_t UpperTopRightFoliageMask[MICRO_HEIGHT] = {
	// clang-format off
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0x3FFFFFFF,
	0x0FFFFFFF, 0x03FFFFFF,
	0x00FFFFFF, 0x003FFFFF,
	0x000FFFFF, 0x0003FFFF,
	0x0000FFFF, 0x00003FFF,
	0x00000FFF, 0x000003FF,
	0x000000FF, 0x0000003F,
	0x0000000F, 0x00000003,
	// clang-format on
};
static uint32_t BottomRightFoliageMask[MICRO_HEIGHT] = {
	// clang-format off
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000003,
	0x0000000F, 0x0000003F,
	0x000000FF, 0x000003FF,
	0x00000FFF, 0x00003FFF,
	0x0000FFFF, 0x0003FFFF,
	0x000FFFFF, 0x003FFFFF,
	0x00FFFFFF, 0x03FFFFFF,
	0x0FFFFFFF, 0x3FFFFFFF,
	// clang-format on
};
static uint32_t LowerBottomRightFoliageMask[MICRO_HEIGHT] = {
	// clang-format off
	0x00000000, 0x00000003,
	0x0000000F, 0x0000003F,
	0x000000FF, 0x000003FF,
	0x00000FFF, 0x00003FFF,
	0x0000FFFF, 0x0003FFFF,
	0x000FFFFF, 0x003FFFFF,
	0x00FFFFFF, 0x03FFFFFF,
	0x0FFFFFFF, 0x3FFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	// clang-format on
};
#endif
static uint32_t LeftFoliageMask[MICRO_HEIGHT] = {
	// clang-format off
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
	// clang-format on
};
#if ASSET_MPL != 1
static uint32_t TopLeftFoliageMask[MICRO_HEIGHT] = {
	// clang-format off
	0xFFFFFFFF, 0xFFFFFFFC,
	0xFFFFFFF0, 0xFFFFFFC0,
	0xFFFFFF00, 0xFFFFFC00,
	0xFFFFF000, 0xFFFFC000,
	0xFFFF0000, 0xFFFC0000,
	0xFFF00000, 0xFFC00000,
	0xFF000000, 0xFC000000,
	0xF0000000, 0xC0000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	// clang-format on
};
static uint32_t UpperTopLeftFoliageMask[MICRO_HEIGHT] = {
	// clang-format off
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFC,
	0xFFFFFFF0, 0xFFFFFFC0,
	0xFFFFFF00, 0xFFFFFC00,
	0xFFFFF000, 0xFFFFC000,
	0xFFFF0000, 0xFFFC0000,
	0xFFF00000, 0xFFC00000,
	0xFF000000, 0xFC000000,
	0xF0000000, 0xC0000000,
	// clang-format on
};
static uint32_t BottomLeftFoliageMask[MICRO_HEIGHT] = {
	// clang-format off
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0x00000000,
	0x00000000, 0xC0000000,
	0xF0000000, 0xFC000000,
	0xFF000000, 0xFFC00000,
	0xFFF00000, 0xFFFC0000,
	0xFFFF0000, 0xFFFFC000,
	0xFFFFF000, 0xFFFFFC00,
	0xFFFFFF00, 0xFFFFFFC0,
	0xFFFFFFF0, 0xFFFFFFFC,
	// clang-format on
};
static uint32_t LowerBottomLeftFoliageMask[MICRO_HEIGHT] = {
	// clang-format off
	0x00000000, 0xC0000000,
	0xF0000000, 0xFC000000,
	0xFF000000, 0xFFC00000,
	0xFFF00000, 0xFFFC0000,
	0xFFFF0000, 0xFFFFC000,
	0xFFFFF000, 0xFFFFFC00,
	0xFFFFFF00, 0xFFFFFFC0,
	0xFFFFFFF0, 0xFFFFFFFC,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	// clang-format on
};
#endif

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
			tbl = ColorTrns[light];
			for (i = 0; i < n; i++) {
				dst[i] = tbl[src[i]];
			}
		} else {
			// Clear the lower bits of the mask to avoid testing i < n in the loops.
			mask = (mask >> i) << i;
			tbl = ColorTrns[light];
			for (i = 0; mask != 0; i++, mask <<= 1) {
				if (mask & 0x80000000) {
					dst[i] = tbl[src[i]];
				}
			}
		}
	}

	//(*src) += n;
	//(*dst) += n;
}

/**
 * @brief Blit a micro CEL to the given buffer
 * @param pBuff Output buffer
 * @param levelCelBlock block/tile to draw
 *   the current MIN block of the level CEL file, as used during rendering of the level tiles.
 *      frameNum  := block & 0x0FFF
 *      frameType := block >> 12
 * @param maskType the type of mask to apply.
 */
void RenderMicro(BYTE* pBuff, uint16_t levelCelBlock, int maskType)
{
	int i, j, light;
	int8_t v;
	BYTE *src, *dst, encoding;
	uint32_t m, *mask, *pFrameTable;

	dst = pBuff;
	pFrameTable = (uint32_t*)pMicrosCel;

#if ASSET_MPL == 1
	src = &pMicrosCel[SwapLE32(pFrameTable[levelCelBlock & 0xFFF])];
	encoding = levelCelBlock >> 12;
#else
	src = &pMicrosCel[SwapLE32(pFrameTable[levelCelBlock])];
	encoding = *src;
	src++;
#endif

	switch (maskType) {
	case DMT_NONE:
		mask = &SolidMask[MICRO_HEIGHT - 1];
		break;
	case DMT_TWALL:
		mask = &WallMask[MICRO_HEIGHT - 1];
		break;
	case DMT_LTFLOOR:
		mask = &LeftMask[MICRO_HEIGHT - 1];
		break;
	case DMT_RTFLOOR:
		mask = &RightMask[MICRO_HEIGHT - 1];
		break;
	case DMT_LFLOOR:
		mask = &LeftFoliageMask[MICRO_HEIGHT - 1];
		break;
	case DMT_RFLOOR:
		mask = &RightFoliageMask[MICRO_HEIGHT - 1];
		break;
#if ASSET_MPL != 1
	case DMT_FLOOR_UP_TOP_LEFT:
		mask = &UpperTopLeftFoliageMask[MICRO_HEIGHT - 1];
		break;
	case DMT_FLOOR_TOP_LEFT:
		mask = &TopLeftFoliageMask[MICRO_HEIGHT - 1];
		break;
	case DMT_FLOOR_LOW_BOTTOM_LEFT:
		mask = &LowerBottomLeftFoliageMask[MICRO_HEIGHT - 1];
		break;
	case DMT_FLOOR_BOTTOM_LEFT:
		mask = &BottomLeftFoliageMask[MICRO_HEIGHT - 1];
		break;
	case DMT_FLOOR_UP_TOP_RIGHT:
		mask = &UpperTopRightFoliageMask[MICRO_HEIGHT - 1];
		break;
	case DMT_FLOOR_TOP_RIGHT:
		mask = &TopRightFoliageMask[MICRO_HEIGHT - 1];
		break;
	case DMT_FLOOR_LOW_BOTTOM_RIGHT:
		mask = &LowerBottomRightFoliageMask[MICRO_HEIGHT - 1];
		break;
	case DMT_FLOOR_BOTTOM_RIGHT:
		mask = &BottomRightFoliageMask[MICRO_HEIGHT - 1];
		break;
	case DMT_FLOOR_TRN_UP_LEFT:
		mask = &UpperLeftMask[MICRO_HEIGHT - 1];
		break;
	case DMT_FLOOR_TRN_UP_RIGHT:
		mask = &UpperRightMask[MICRO_HEIGHT - 1];
		break;
#if ASSET_MPL > 2
	case DMT_EMPTY:
		return;
#endif /* ASSET_MPL > 2 */
#endif /* ASSET_MPL != 1 */
	default:
		ASSUME_UNREACHABLE;
		mask = &SolidMask[MICRO_HEIGHT - 1];
		break;
	}

#if DEBUG_MODE
	if (SDL_GetModState() & KMOD_ALT) {
		mask = &SolidMask[MICRO_HEIGHT - 1];
	}
#endif

	light = light_trn_index;
	static_assert(MICRO_HEIGHT - 2 < MICRO_WIDTH, "Line with negative or zero width.");
	static_assert(MICRO_WIDTH <= sizeof(*mask) * CHAR_BIT, "Mask is too small to cover the tile.");
	switch (encoding) {
	case MET_SQUARE:
		for (i = MICRO_HEIGHT; i != 0; i--, dst -= BUFFER_WIDTH + MICRO_WIDTH, mask--) {
			RenderLine(dst, src, MICRO_WIDTH, *mask, light);
			src += MICRO_WIDTH;
			dst += MICRO_WIDTH;
		}
		break;
	case MET_TRANSPARENT:
		for (i = MICRO_HEIGHT; i != 0; i--, dst -= BUFFER_WIDTH + MICRO_WIDTH, mask--) {
			m = *mask;
			static_assert(MICRO_WIDTH <= sizeof(m) * CHAR_BIT, "Undefined left-shift behavior.");
			for (j = MICRO_WIDTH; j != 0; j -= v, m <<= v) {
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
	case MET_LTRIANGLE:
		for (i = MICRO_HEIGHT - 2; i >= 0; i -= 2, dst -= BUFFER_WIDTH + MICRO_WIDTH, mask--) {
			src += i & 2;
			dst += i;
			RenderLine(dst, src, MICRO_WIDTH - i, *mask, light);
			src += MICRO_WIDTH - i;
			dst += MICRO_WIDTH - i;
		}
		for (i = 2; i != MICRO_HEIGHT; i += 2, dst -= BUFFER_WIDTH + MICRO_WIDTH, mask--) {
			src += i & 2;
			dst += i;
			RenderLine(dst, src, MICRO_WIDTH - i, *mask, light);
			src += MICRO_WIDTH - i;
			dst += MICRO_WIDTH - i;
		}
		break;
	case MET_RTRIANGLE:
		for (i = MICRO_HEIGHT - 2; i >= 0; i -= 2, dst -= BUFFER_WIDTH + MICRO_WIDTH, mask--) {
			RenderLine(dst, src, MICRO_WIDTH - i, *mask, light);
			src += MICRO_WIDTH - i;
			dst += MICRO_WIDTH - i;
			src += i & 2;
			dst += i;
		}
		for (i = 2; i != MICRO_HEIGHT; i += 2, dst -= BUFFER_WIDTH + MICRO_WIDTH, mask--) {
			RenderLine(dst, src, MICRO_WIDTH - i, *mask, light);
			src += MICRO_WIDTH - i;
			dst += MICRO_WIDTH - i;
			src += i & 2;
			dst += i;
		}
		break;
	case MET_LTRAPEZOID:
		for (i = MICRO_HEIGHT - 2; i >= 0; i -= 2, dst -= BUFFER_WIDTH + MICRO_WIDTH, mask--) {
			src += i & 2;
			dst += i;
			RenderLine(dst, src, MICRO_WIDTH - i, *mask, light);
			src += MICRO_WIDTH - i;
			dst += MICRO_WIDTH - i;
		}
		for (i = MICRO_HEIGHT / 2; i != 0; i--, dst -= BUFFER_WIDTH + MICRO_WIDTH, mask--) {
			RenderLine(dst, src, MICRO_WIDTH, *mask, light);
			src += MICRO_WIDTH;
			dst += MICRO_WIDTH;
		}
		break;
	case MET_RTRAPEZOID:
		for (i = MICRO_HEIGHT - 2; i >= 0; i -= 2, dst -= BUFFER_WIDTH + MICRO_WIDTH, mask--) {
			RenderLine(dst, src, MICRO_WIDTH - i, *mask, light);
			src += MICRO_WIDTH - i;
			dst += MICRO_WIDTH - i;
			src += i & 2;
			dst += i;
		}
		for (i = MICRO_HEIGHT / 2; i != 0; i--, dst -= BUFFER_WIDTH + MICRO_WIDTH, mask--) {
			RenderLine(dst, src, MICRO_WIDTH, *mask, light);
			src += MICRO_WIDTH;
			dst += MICRO_WIDTH;
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
	BYTE* dst;

	if (sx <= SCREEN_X - TILE_WIDTH || sx >= SCREEN_X + SCREEN_WIDTH)
		return;

	if (sy < SCREEN_Y || sy >= SCREEN_Y + SCREEN_HEIGHT + TILE_HEIGHT)
		return;

	static_assert(TILE_WIDTH / TILE_HEIGHT == 2, "world_draw_black_tile relies on fix width/height ratio of the floor-tile.");
	dst = &gpBuffer[BUFFERXY(sx, sy)] + TILE_WIDTH / 2 - 2;

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
