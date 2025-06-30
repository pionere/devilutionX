/**
 * @file drlp_l4.cpp
 *
 * Implementation of the hell level patching functionality.
 */
#include "all.h"
#include "engine/render/dun_render.h"

DEVILUTION_BEGIN_NAMESPACE

static BYTE* patchHellStairsCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	const CelMicro micros[] = {
/*  0 */{ 139 - 1, 1, -1 },              // merge subtiles (used to block subsequent calls)
/*  1 */{ 138 - 1, 0, -1 },
/*  2 */{ 137 - 1, 2, MET_TRANSPARENT },
/*  3 */{ 137 - 1, 0, MET_SQUARE },
/*  4 */{ 140 - 1, 0, MET_LTRAPEZOID },
/*  5 */{ 137 - 1, 1, MET_TRANSPARENT },
/*  6 */{ 140 - 1, 1, MET_TRANSPARENT },

/*  7 */{ 136 - 1, 1, -1 },
/*  8 */{ 126 - 1, 3, MET_TRANSPARENT },

/*  9 */{  98 - 1, 0, -1 },
/* 10 */{  86 - 1, 4, MET_SQUARE },
/* 11 */{  86 - 1, 2, MET_SQUARE },
/* 12 */{  95 - 1, 1, MET_RTRAPEZOID },

/* 13 */{  91 - 1, 0, MET_TRANSPARENT }, // eliminate pointless pixels
/* 14 */{  91 - 1, 1, MET_TRANSPARENT },
/* 15 */{ 110 - 1, 0, MET_TRANSPARENT },
/* 16 */{ 110 - 1, 1, MET_TRANSPARENT },
/* 17 */{ 112 - 1, 0, MET_TRANSPARENT },
/* 18 */{ 112 - 1, 1, MET_TRANSPARENT },
/* 19 */{ 113 - 1, 0, MET_TRANSPARENT },
/* 20 */{ 113 - 1, 1, MET_TRANSPARENT },
/* 21 */{ 130 - 1, 0, MET_TRANSPARENT },
/* 22 */{ 132 - 1, 1, MET_TRANSPARENT },
/* 23 */{ 133 - 1, 0, MET_TRANSPARENT },

/* 24 */{ 100 - 1, 2, MET_TRANSPARENT },
/* 25 */{ 135 - 1, 5, MET_TRANSPARENT },
/* 26 */{ 127 - 1, 9, MET_TRANSPARENT },
/* 27 */{ 134 - 1, 0, MET_TRANSPARENT },
/* 28 */{ 134 - 1, 1, MET_TRANSPARENT },
/* 29 */{ 134 - 1, 2, MET_TRANSPARENT },

/* 30 */{ 141 - 1, 5, MET_TRANSPARENT },
/* 31 */{ 141 - 1, 3, MET_TRANSPARENT },
/* 32 */{ 141 - 1, 1, MET_RTRIANGLE },
	};

	const uint16_t* pSubtiles = (const uint16_t*)minBuf;
	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_L4;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		// if (micro.subtileIndex < 0) {
		//	continue;
		// }
		unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
		if ((SwapLE16(pSubtiles[index]) & 0xFFF) == 0) {
			return celBuf; // frame is empty -> assume it is already done
		}
	}

	// TODO: check celLen
	// draw the micros to the back-buffer
	pMicrosCel = celBuf;
	constexpr BYTE TRANS_COLOR = 128;
	constexpr int DRAW_HEIGHT = 8;
	memset(&gpBuffer[0], TRANS_COLOR, DRAW_HEIGHT * BUFFER_WIDTH * MICRO_HEIGHT);

	unsigned xx = 0, yy = MICRO_HEIGHT - 1;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		if (micro.subtileIndex >= 0) {
			unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
			RenderMicro(&gpBuffer[xx + yy * BUFFER_WIDTH], SwapLE16(pSubtiles[index]), DMT_NONE);
		}
		yy += MICRO_HEIGHT;
		if (yy == (DRAW_HEIGHT + 1) * MICRO_HEIGHT - 1) {
			yy = MICRO_HEIGHT - 1;
			xx += MICRO_WIDTH;
		}
	}

	// mask 138[0]
	for (int i = 1; i < 2; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color == TRANS_COLOR)
					continue;
				if (x >= 15 || color < 80) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// move pixels to 137[0] from 139[1]
	for (int i = 3; i < 4; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (0 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (0 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 139[1]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// move pixels to 140[0] from 139[1]
	for (int i = 4; i < 5; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (0 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (0 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 139[1]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}

	// mask 137[1]
	for (int i = 5; i < 6; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color == TRANS_COLOR)
					continue;
				if (x >= 15 || color < 80) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// move pixels to 137[1] from 138[0]
	for (int i = 5; i < 6; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (1 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (1 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 138[0]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// move pixels to 140[1] from 138[0]
	for (int i = 6; i < 7; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (1 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (1 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 138[0]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// move pixels to 126[3] from 136[1]
	for (int i = 8; i < 9; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (7 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (7 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 136[1]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// move pixels to 86[4] from 98[0]
	for (int i = 10; i < 11; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (9 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (9 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 98[0]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// move pixels to 86[2] from 91[1]
	{ // 86[2]
		int i = 11;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		unsigned addr2 = 0 + MICRO_WIDTH * (14 / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (14 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 91[1]
		gpBuffer[addr +   0 + 31 * BUFFER_WIDTH] = 125;
		gpBuffer[addr2 +  0 + 15 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	// move pixels to 95[1] from 113[0]
	{ // 95[1]
		int i = 12;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		unsigned addr2 = 0 + MICRO_WIDTH * (19 / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (19 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 113[0]
		gpBuffer[addr +  30 +  0 * BUFFER_WIDTH] = 124;
		gpBuffer[addr +  31 +  0 * BUFFER_WIDTH] = 126;
		gpBuffer[addr2 + 30 + 16 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr2 + 31 + 16 * BUFFER_WIDTH] = TRANS_COLOR;
	}

	// eliminate pointless pixels of 91[0], 91[1], 110[0], 110[1], 112[0], 112[1], 113[0], 113[1], 130[0], 132[1], 133[0]
	for (int i = 13; i < 24; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color == 80) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// eliminate pointless pixels of 100[2]
	for (int i = 24; i < 25; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (y >= 16 - x / 2 && (color % 16) > 12) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// 141[5]: move pixels up and move pixels from 141[3]
	for (int i = 30; i < 31; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = addr + MICRO_HEIGHT / 2 * BUFFER_WIDTH;
				} else {
					addr2 = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 141[3]
				}
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
	}
	// 141[3]: move pixels up and move pixels from 141[1]
	for (int i = 31; i < 32; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = addr + MICRO_HEIGHT / 2 * BUFFER_WIDTH;
				} else {
					if ((y - MICRO_HEIGHT / 2) > x / 2) {
						gpBuffer[addr] = TRANS_COLOR;
						continue;
					}
					addr2 = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 141[1]
				}
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
	}
	// move pixels to 127[9] from 135[5]
	{ // 127[9]
		int i = 26;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		unsigned addr2 = 0 + MICRO_WIDTH * (25 / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (25 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 135[5]
		gpBuffer[addr + 31 + 26 * BUFFER_WIDTH] = 115;
		gpBuffer[addr + 27 + 27 * BUFFER_WIDTH] = 106;
		gpBuffer[addr + 28 + 27 * BUFFER_WIDTH] = 104;
		gpBuffer[addr + 29 + 27 * BUFFER_WIDTH] = 115;
		gpBuffer[addr + 30 + 27 * BUFFER_WIDTH] = 113;
		gpBuffer[addr + 31 + 27 * BUFFER_WIDTH] = 113;
		gpBuffer[addr2 + 31 + 26 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr2 + 27 + 27 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr2 + 28 + 27 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr2 + 29 + 27 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr2 + 30 + 27 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr2 + 31 + 27 * BUFFER_WIDTH] = TRANS_COLOR;
	}

	// fix bad artifacts
	{ // 137[1] (140[3])
		int i = 5;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  7 +  7 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 12 + 29 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 13 + 22 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // 140[1]
		int i = 6;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 14 +  0 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // 133[0] - restore eliminated pixel
		int i = 23;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 31 +  3 * BUFFER_WIDTH] = 80;
	}
	{ // move pixels to 100[2] from 110[1]
		int i = 24;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		unsigned addr2 = 0 + MICRO_WIDTH * (16 / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (16 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 110[1]
		gpBuffer[addr +   2 +  5 * BUFFER_WIDTH] = 106;
		gpBuffer[addr +   6 +  7 * BUFFER_WIDTH] = 120;
		gpBuffer[addr2 +  2 + 21 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr2 +  6 + 23 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // 134[0]
		int i = 27;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 29 +  0 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 30 +  0 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 31 +  0 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // 134[1]
		int i = 28;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  0 +  0 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  1 +  0 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  2 +  0 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  3 +  0 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // 134[2]
		int i = 29;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  0 + 31 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  1 + 31 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  2 + 31 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  3 + 31 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  4 + 31 * BUFFER_WIDTH] = TRANS_COLOR;
	}

	// create the new CEL file
	constexpr int newEntries = lengthof(micros);
	size_t maxCelSize = *celLen + newEntries * MICRO_WIDTH * MICRO_HEIGHT;
	BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
	memset(resCelBuf, 0, maxCelSize);

	CelFrameEntry entries[newEntries];
	xx = 0, yy = MICRO_HEIGHT - 1;
	int idx = 0;
	for (int i = 0; i < newEntries; i++) {
		const CelMicro &micro = micros[i];
		if (micro.res_encoding >= 0) {
			entries[idx].encoding = micro.res_encoding;
			unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
			entries[idx].frameRef = SwapLE16(pSubtiles[index]) & 0xFFF;
			entries[idx].frameSrc = &gpBuffer[xx + yy * BUFFER_WIDTH];
			idx++;
		}
		yy += MICRO_HEIGHT;
		if (yy == (DRAW_HEIGHT + 1) * MICRO_HEIGHT - 1) {
			yy = MICRO_HEIGHT - 1;
			xx += MICRO_WIDTH;
		}
	}

	*celLen = encodeCelMicros(entries, idx, resCelBuf, celBuf, TRANS_COLOR);

	mem_free_dbg(celBuf);

	return resCelBuf;
}

static BYTE* patchHellChaosCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	const CelMicro micros[] = {
/*  0 */{  54 - 1, 5, MET_RTRIANGLE },   // redraw subtiles 
/*  1 */{  54 - 1, 7, -1 },              // (used to block subsequent calls)
/*  2 */{  55 - 1, 4, MET_LTRIANGLE },
/*  3 */{ /*55*/ - 1, 6, -1 },
/*  4 */{  53 - 1, 2, MET_LTRAPEZOID },
/*  5 */{  53 - 1, 3, MET_RTRAPEZOID },
/*  6 */{  53 - 1, 4, MET_TRANSPARENT },
/*  7 */{  53 - 1, 5, MET_TRANSPARENT },
/*  8 */{ /*53*/ - 1, 6, -1 },
/*  9 */{ /*53*/ - 1, 7, -1 },

/* 10 */{  48 - 1, 4, MET_TRANSPARENT },
/* 11 */{  52 - 1, 4, MET_TRANSPARENT },
/* 12 */{  58 - 1, 4, MET_TRANSPARENT },
/* 13 */{  67 - 1, 4, MET_TRANSPARENT },

/* 14 */{  52 - 1, 6, MET_TRANSPARENT },
/* 15 */{  67 - 1, 6, MET_TRANSPARENT },
/* 16 */{  76 - 1, 6, MET_TRANSPARENT },

/* 17 */{  46 - 1, 4, MET_TRANSPARENT },
/* 18 */{  50 - 1, 4, MET_TRANSPARENT },
/* 19 */{  56 - 1, 4, MET_TRANSPARENT },
/* 20 */{  62 - 1, 4, MET_TRANSPARENT },
/* 21 */{  65 - 1, 4, MET_TRANSPARENT },
/* 22 */{  68 - 1, 4, MET_TRANSPARENT },
/* 23 */{  74 - 1, 4, MET_TRANSPARENT },

/* 24 */{  46 - 1, 5, MET_TRANSPARENT },
/* 25 */{  50 - 1, 5, MET_TRANSPARENT },
/* 26 */{  56 - 1, 5, MET_TRANSPARENT },
/* 27 */{  59 - 1, 5, MET_TRANSPARENT },
/* 28 */{  65 - 1, 5, MET_TRANSPARENT },
/* 29 */{  77 - 1, 5, MET_TRANSPARENT },

/* 30 */{  74 - 1, 6, MET_TRANSPARENT },
/* 31 */{ /*77*/ - 1, 6, -1 },

/* 32 */{ /*74*/ - 1, 7, -1 },
/* 33 */{  77 - 1, 7, MET_TRANSPARENT },

/* 34 */{  47 - 1, 5, MET_TRANSPARENT },
/* 35 */{  51 - 1, 5, MET_TRANSPARENT },
/* 36 */{  66 - 1, 5, MET_TRANSPARENT },

/* 37 */{  47 - 1, 7, MET_TRANSPARENT },
/* 38 */{  66 - 1, 7, MET_TRANSPARENT },
/* 39 */{  /*78*/ - 1, 7, -1 },
	};

	const uint16_t* pSubtiles = (const uint16_t*)minBuf;
	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_L4;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		if (micro.subtileIndex < 0) {
			continue;
		}
		unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
		if ((SwapLE16(pSubtiles[index]) & 0xFFF) == 0) {
			return celBuf; // frame is empty -> assume it is already done
		}
	}

	// TODO: check celLen
	// draw the micros to the back-buffer
	pMicrosCel = celBuf;
	constexpr BYTE TRANS_COLOR = 128;
	constexpr int DRAW_HEIGHT = 8;
	memset(&gpBuffer[0], TRANS_COLOR, DRAW_HEIGHT * BUFFER_WIDTH * MICRO_HEIGHT);

	unsigned xx = 0, yy = MICRO_HEIGHT - 1;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		if (micro.subtileIndex >= 0) {
			unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
			RenderMicro(&gpBuffer[xx + yy * BUFFER_WIDTH], SwapLE16(pSubtiles[index]), DMT_NONE);
		}
		yy += MICRO_HEIGHT;
		if (yy == (DRAW_HEIGHT + 1) * MICRO_HEIGHT - 1) {
			yy = MICRO_HEIGHT - 1;
			xx += MICRO_WIDTH;
		}
	}

	// mask 54[5]
	for (int i = 0; i < 1; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y < 1 + x / 2 || y > 31 - x / 2) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 55[4]
	for (int i = 2; i < 3; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y < 16 - x / 2 || y > 16 + x / 2) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 53[2]
	for (int i = 4; i < 5; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y > 16 + x / 2) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 53[3]
	for (int i = 5; i < 6; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y > 31 - x / 2) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 53[4]
	for (int i = 6; i < 7; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y < 16 - x / 2) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 53[5]
	for (int i = 7; i < 8; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y < 1 + x / 2) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}

	// redraw 48[4], 52[4], 58[4], 67[4] using 55[4]
	for (int i = 10; i < 14; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (2 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (2 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 55[4]
				BYTE color = gpBuffer[addr];
				BYTE color2 = gpBuffer[addr2];
				if (color != 0 && color < 32 /*&& color2 != TRANS_COLOR*/ || (color == TRANS_COLOR && color2 != TRANS_COLOR)) {
					gpBuffer[addr] = color2;
				}
			}
		}
	}
	// redraw 52[6], 67[6], 76[6] using '55[6]'
	for (int i = 14; i < 17; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != 0 && color < 32) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// redraw 46[4], 50[4], 56[4], 62[4], 65[4], 68[4], 74[4] using 53[4]
	for (int i = 17; i < 24; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (6 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (6 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 53[4]
				BYTE color = gpBuffer[addr];
				BYTE color2 = gpBuffer[addr2];
				if (color != 0 && color < 32 /*&& color2 != TRANS_COLOR*/ || (color == TRANS_COLOR && color2 != TRANS_COLOR)) {
					gpBuffer[addr] = color2;
				}
			}
		}
	}
	// redraw 46[5], 50[5], 56[5], 59[5], 65[5], 77[5] using 53[5]
	for (int i = 24; i < 30; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (7 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (7 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 53[5]
				BYTE color = gpBuffer[addr];
				BYTE color2 = gpBuffer[addr2];
				if (color != 0 && color < 32 /*&& color2 != TRANS_COLOR*/ || (color == TRANS_COLOR && color2 != TRANS_COLOR)) {
					if (color2 == TRANS_COLOR && x < 17) {
						continue; // preserve 'missing wall-pixel' of 46[5]
					}
					gpBuffer[addr] = color2;
				}
			}
		}
	}
	// redraw 74[6], 77[6] using '53[6]'
	for (int i = 30; i < 32; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != 0 && color < 32) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// redraw 74[7], 77[7] using '53[7]'
	for (int i = 32; i < 34; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != 0 && color < 32) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}

	// redraw 47[5], 51[5], 66[5] using 54[5]
	for (int i = 34; i < 37; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (0 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (0 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 54[5]
				BYTE color = gpBuffer[addr];
				BYTE color2 = gpBuffer[addr2];
				if (color != 0 && color < 32 /*&& color2 != TRANS_COLOR*/ || (color == TRANS_COLOR && color2 != TRANS_COLOR)) {
					gpBuffer[addr] = color2;
				}
			}
		}
	}
	// redraw 47[7], 66[7] (using 54[7])
	for (int i = 37; i < 40; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != 0 && color < 32) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	{ // fix 77[5]
		int i = 29;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 28 +  9 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 29 +  1 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 29 +  2 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 29 +  3 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 29 +  4 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 29 +  5 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 29 +  6 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 29 +  7 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 30 +  3 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 30 +  4 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 30 +  5 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 30 +  6 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 30 +  7 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 31 +  6 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 31 +  7 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 31 +  8 * BUFFER_WIDTH] = TRANS_COLOR;
	}

	// create the new CEL file
	constexpr int newEntries = lengthof(micros);
	size_t maxCelSize = *celLen + newEntries * MICRO_WIDTH * MICRO_HEIGHT;
	BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
	memset(resCelBuf, 0, maxCelSize);

	CelFrameEntry entries[newEntries];
	xx = 0, yy = MICRO_HEIGHT - 1;
	int idx = 0;
	for (int i = 0; i < newEntries; i++) {
		const CelMicro &micro = micros[i];
		if (micro.res_encoding >= 0) {
			entries[idx].encoding = micro.res_encoding;
			unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
			entries[idx].frameRef = SwapLE16(pSubtiles[index]) & 0xFFF;
			entries[idx].frameSrc = &gpBuffer[xx + yy * BUFFER_WIDTH];
			idx++;
		}
		yy += MICRO_HEIGHT;
		if (yy == (DRAW_HEIGHT + 1) * MICRO_HEIGHT - 1) {
			yy = MICRO_HEIGHT - 1;
			xx += MICRO_WIDTH;
		}
	}

	*celLen = encodeCelMicros(entries, idx, resCelBuf, celBuf, TRANS_COLOR);

	mem_free_dbg(celBuf);

	return resCelBuf;
}

static BYTE* patchHellFloorCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	const CelMicro micros[] = {
/*  0 */{ 129 - 1, 1, MET_RTRIANGLE }, // fix micros
/*  1 */{ 171 - 1, 1, MET_RTRIANGLE },
/*  2 */{ 172 - 1, 1, MET_RTRIANGLE },
/*  3 */{ 187 - 1, 1, MET_RTRIANGLE },
/*  4 */{ 200 - 1, 1, MET_RTRAPEZOID },
/*  5 */{ 200 - 1, 4, MET_TRANSPARENT },
/*  6 */{ 225 - 1, 0, MET_LTRIANGLE },
/*  7 */{ 227 - 1, 1, MET_RTRIANGLE },
/*  8 */{ 228 - 1, 3, MET_TRANSPARENT },
/*  9 */{ 228 - 1, 5, MET_TRANSPARENT },
/* 10 */{ /*265*/ - 1, 0, -1 },
/* 11 */{ 272 - 1, 0, MET_LTRIANGLE },
/* 12 */{ 277 - 1, 0, MET_LTRIANGLE },
/* 13 */{ 289 - 1, 1, MET_RTRIANGLE },
/* 14 */{ 302 - 1, 0, MET_LTRIANGLE },
/* 15 */{ 337 - 1, 0, MET_LTRIANGLE },
/* 16 */{ 371 - 1, 0, MET_LTRIANGLE },
/* 17 */{ 340 - 1, 0, MET_LTRIANGLE },
/* 18 */{ 373 - 1, 0, MET_LTRIANGLE },

/* 19 */{   1 - 1, 5, -1 },
/* 20 */{ 188 - 1, 5, MET_TRANSPARENT },

/* 21 */{   5 - 1, 6, MET_TRANSPARENT },
/* 22 */{   8 - 1, 7, MET_TRANSPARENT },
/* 23 */{  15 - 1, 6, MET_TRANSPARENT },
/* 24 */{  50 - 1, 4, MET_TRANSPARENT },
/* 25 */{  50 - 1, 7, MET_TRANSPARENT },
/* 26 */{ 180 - 1, 6, MET_TRANSPARENT },
/* 27 */{ 212 - 1, 6, MET_TRANSPARENT },
/* 28 */{ 251 - 1, 6, MET_TRANSPARENT },
/* 29 */{   1 - 1, 7, MET_TRANSPARENT },
/* 30 */{  11 - 1, 7, MET_TRANSPARENT },
/* 31 */{ 196 - 1, 7, MET_TRANSPARENT },
/* 32 */{ 204 - 1, 7, MET_TRANSPARENT },
/* 33 */{ 208 - 1, 7, MET_TRANSPARENT },
/* 34 */{ 254 - 1, 7, MET_TRANSPARENT },

/* 35 */{   4 - 1, 1, -1 },
/* 36 */{ 155 - 1, 1, -1 },
/* 37 */{ 152 - 1, 1, MET_RTRIANGLE },

/* 38 */{ 238 - 1, 0, MET_LTRIANGLE },
/* 39 */{ 238 - 1, 1, MET_RTRIANGLE },
	};
	const uint16_t* pSubtiles = (const uint16_t*)minBuf;
	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_L4;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		if (micro.subtileIndex < 0) {
			continue;
		}
		unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
		if ((SwapLE16(pSubtiles[index]) & 0xFFF) == 0) {
			return celBuf; // frame is empty -> assume it is already done
		}
	}

	// TODO: check celLen
	// draw the micros to the back-buffer
	pMicrosCel = celBuf;
	constexpr BYTE TRANS_COLOR = 128;
	constexpr int DRAW_HEIGHT = 8;
	memset(&gpBuffer[0], TRANS_COLOR, DRAW_HEIGHT * BUFFER_WIDTH * MICRO_HEIGHT);

	unsigned xx = 0, yy = MICRO_HEIGHT - 1;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		if (micro.subtileIndex >= 0) {
			unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
			RenderMicro(&gpBuffer[xx + yy * BUFFER_WIDTH], SwapLE16(pSubtiles[index]), DMT_NONE);
		}
		yy += MICRO_HEIGHT;
		if (yy == (DRAW_HEIGHT + 1) * MICRO_HEIGHT - 1) {
			yy = MICRO_HEIGHT - 1;
			xx += MICRO_WIDTH;
		}
	}

	{ // 129[1] - adjust after reuse
		int i = 0;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 0 + 25 * BUFFER_WIDTH] = 40;
		gpBuffer[addr + 0 + 26 * BUFFER_WIDTH] = 40;
		gpBuffer[addr + 0 + 27 * BUFFER_WIDTH] = 42;
		gpBuffer[addr + 0 + 28 * BUFFER_WIDTH] = 40;
	}
	{ // 171[1] - adjust shadow after reuse
		int i = 1;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 29 + 15 * BUFFER_WIDTH] = 42;
	}
	{ // 187[1] - fix glitch
		int i = 3;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 11 + 26 * BUFFER_WIDTH] = 40;
	}

	{ // 200[1] - fix glitch(?)
		int i = 4;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 0 + 11 * BUFFER_WIDTH] = 115;
		gpBuffer[addr + 1 + 10 * BUFFER_WIDTH] = 115;
		gpBuffer[addr + 1 + 11 * BUFFER_WIDTH] = 117;
		gpBuffer[addr + 1 + 12 * BUFFER_WIDTH] = 115;

		gpBuffer[addr + 0 + 14 * BUFFER_WIDTH] = 118;
		gpBuffer[addr + 0 + 15 * BUFFER_WIDTH] = 120;
		gpBuffer[addr + 0 + 16 * BUFFER_WIDTH] = 117;
		gpBuffer[addr + 0 + 17 * BUFFER_WIDTH] = 116;

		gpBuffer[addr + 0 + 26 * BUFFER_WIDTH] = 43;
		gpBuffer[addr + 1 + 26 * BUFFER_WIDTH] = 44;
		gpBuffer[addr + 1 + 25 * BUFFER_WIDTH] = 41;
	}
	{ // 200[4] - adjust after reuse
		int i = 5;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 14 +  0 * BUFFER_WIDTH] = 121;
		gpBuffer[addr + 15 +  0 * BUFFER_WIDTH] = 120;
		gpBuffer[addr + 16 +  0 * BUFFER_WIDTH] = 120;
		gpBuffer[addr + 17 +  0 * BUFFER_WIDTH] = 116;

		gpBuffer[addr + 21 +  0 * BUFFER_WIDTH] = 113;
		gpBuffer[addr + 22 +  0 * BUFFER_WIDTH] = 114;
		gpBuffer[addr + 23 +  0 * BUFFER_WIDTH] = 115;
		gpBuffer[addr + 24 +  0 * BUFFER_WIDTH] = 116;
		gpBuffer[addr + 25 +  0 * BUFFER_WIDTH] = 115;

		gpBuffer[addr + 15 +  1 * BUFFER_WIDTH] = 89;
		gpBuffer[addr + 16 +  1 * BUFFER_WIDTH] = 120;
		gpBuffer[addr + 17 +  1 * BUFFER_WIDTH] = 116;
		gpBuffer[addr + 16 +  2 * BUFFER_WIDTH] = 119;
	}
	{ // 225[0] - adjust after reuse
		int i = 6;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 30 +  5 * BUFFER_WIDTH] = 40;
		gpBuffer[addr + 31 +  5 * BUFFER_WIDTH] = 39;
		gpBuffer[addr + 31 +  8 * BUFFER_WIDTH] = 40;
		gpBuffer[addr + 31 + 15 * BUFFER_WIDTH] = 39;
	}
	{ // 227[1] - fix shadow
		int i = 7;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  8 + 10 * BUFFER_WIDTH] = 45;
		gpBuffer[addr +  8 + 11 * BUFFER_WIDTH] = 111;
		gpBuffer[addr +  8 + 12 * BUFFER_WIDTH] = 111;
		gpBuffer[addr +  8 + 13 * BUFFER_WIDTH] = 126;
	}
	{ // 228[3] - fix 'shadow'
		int i = 8;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  8 + 10 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  6 + 19 * BUFFER_WIDTH] = 108;
		gpBuffer[addr +  6 + 20 * BUFFER_WIDTH] = 107;
		gpBuffer[addr +  6 + 23 * BUFFER_WIDTH] = 125;
		gpBuffer[addr +  6 + 25 * BUFFER_WIDTH] = 109;
		gpBuffer[addr +  6 + 26 * BUFFER_WIDTH] = 125;
		gpBuffer[addr +  6 + 27 * BUFFER_WIDTH] = 109;
		gpBuffer[addr +  6 + 28 * BUFFER_WIDTH] = 125;
		gpBuffer[addr +  6 + 29 * BUFFER_WIDTH] = 107;
	}
	{ // 228[5] - fix shadow after reuse + fix glare
		int i = 9;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 15 + 24 * BUFFER_WIDTH] = 126;

		gpBuffer[addr +  0 + 24 * BUFFER_WIDTH] = 116;
	}
	{ // 272[0] - adjust after reuse
		int i = 11;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 31 + 15 * BUFFER_WIDTH] = 43;
		gpBuffer[addr + 30 +  3 * BUFFER_WIDTH] = 40;
		gpBuffer[addr + 31 +  3 * BUFFER_WIDTH] = 43;
	}
	{ // 289[1] - adjust after reuse
		int i = 13;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  0 +  2 * BUFFER_WIDTH] = 40;
		gpBuffer[addr +  1 +  2 * BUFFER_WIDTH] = 42;
		gpBuffer[addr +  0 + 25 * BUFFER_WIDTH] = 53;
		gpBuffer[addr +  0 + 21 * BUFFER_WIDTH] = 42;
		gpBuffer[addr +  0 + 22 * BUFFER_WIDTH] = 40;
	}
	{ // 302[0] - adjust after reuse
		int i = 14;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 31 +  5 * BUFFER_WIDTH] = 39;
		gpBuffer[addr + 31 + 12 * BUFFER_WIDTH] = 41;
		gpBuffer[addr + 31 + 13 * BUFFER_WIDTH] = 39;
	}
	// 337[0], 371[0] - adjust after reuse
	for (int i = 15; i < 17; i++) {
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 31 +  7 * BUFFER_WIDTH] = 38;
		gpBuffer[addr + 31 +  8 * BUFFER_WIDTH] = 39;
		gpBuffer[addr + 31 +  9 * BUFFER_WIDTH] = 40;

		gpBuffer[addr + 30 + 25 * BUFFER_WIDTH] = 42;
		gpBuffer[addr + 31 + 25 * BUFFER_WIDTH] = 40;

		gpBuffer[addr + 29 + 28 * BUFFER_WIDTH] = 39;
		gpBuffer[addr + 30 + 28 * BUFFER_WIDTH] = 41;
		gpBuffer[addr + 31 + 28 * BUFFER_WIDTH] = 42;
	}
	// 340[0], 373[0] - adjust after reuse
	for (int i = 17; i < 19; i++) {
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 31 + 10 * BUFFER_WIDTH] = 41;
		gpBuffer[addr + 29 + 10 * BUFFER_WIDTH] = 39;

		gpBuffer[addr + 30 + 11 * BUFFER_WIDTH] = 40;
		gpBuffer[addr + 31 + 12 * BUFFER_WIDTH] = 40;

		gpBuffer[addr + 31 + 20 * BUFFER_WIDTH] = 39;
		gpBuffer[addr + 30 + 21 * BUFFER_WIDTH] = 41;
		gpBuffer[addr + 31 + 21 * BUFFER_WIDTH] = 38;

		gpBuffer[addr + 30 + 28 * BUFFER_WIDTH] = 42;
		gpBuffer[addr + 30 + 29 * BUFFER_WIDTH] = 40;
		gpBuffer[addr + 31 + 29 * BUFFER_WIDTH] = 39;
		gpBuffer[addr + 30 + 30 * BUFFER_WIDTH] = 39;
		gpBuffer[addr + 31 + 30 * BUFFER_WIDTH] = 38;
	}

	// 188[5] - adjust after reuse/fix using 1[5]
	for (int i = 20; i < 21; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (19 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (19 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 1[5]
				if (y < 6 && y < x + 3) {
					gpBuffer[addr] = gpBuffer[addr2];
				}
			}
		}
	}
	{ // 5[6] - adjust after reuse + sync
		int i = 21;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 31 + 30 * BUFFER_WIDTH] = TRANS_COLOR;

		// gpBuffer[addr + 17 + 19 * BUFFER_WIDTH] = 102;
		// gpBuffer[addr + 16 + 19 * BUFFER_WIDTH] = 98;
		gpBuffer[addr + 20 + 20 * BUFFER_WIDTH] = 103;
		gpBuffer[addr + 15 + 20 * BUFFER_WIDTH] = 96;
		gpBuffer[addr + 14 + 21 * BUFFER_WIDTH] = 96;
		gpBuffer[addr + 13 + 22 * BUFFER_WIDTH] = 112;
		gpBuffer[addr + 11 + 25 * BUFFER_WIDTH] = 113;
		gpBuffer[addr + 10 + 27 * BUFFER_WIDTH] = 115;

		gpBuffer[addr + 12 + 23 * BUFFER_WIDTH] = 112;
		gpBuffer[addr + 10 + 26 * BUFFER_WIDTH] = 114;
		gpBuffer[addr +  9 + 29 * BUFFER_WIDTH] = 109;
		gpBuffer[addr +  9 + 30 * BUFFER_WIDTH] = 110;
	}
	{ // 8[7] - adjust after reuse
		int i = 22;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 0 + 30 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // 15[6] - sync
		int i = 23;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 31 + 29 * BUFFER_WIDTH] = TRANS_COLOR;
		// gpBuffer[addr + 17 + 19 * BUFFER_WIDTH] = 102;
		// gpBuffer[addr + 16 + 19 * BUFFER_WIDTH] = 98;
		gpBuffer[addr + 12 + 23 * BUFFER_WIDTH] = 112;
	}
	{ // 50[4] - sync
		int i = 24;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 12 +  3 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 13 +  4 * BUFFER_WIDTH] = TRANS_COLOR;

		gpBuffer[addr + 14 +  6 * BUFFER_WIDTH] = 126;
		gpBuffer[addr + 15 +  7 * BUFFER_WIDTH] = 111;
		gpBuffer[addr + 15 +  8 * BUFFER_WIDTH] = 126;
	}
	{ // 50[7] - adjust after reuse
		int i = 25;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 0 + 31 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // 180[6] - sync
		int i = 26;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 31 + 30 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 28 + 28 * BUFFER_WIDTH] = 105;
		gpBuffer[addr + 27 + 27 * BUFFER_WIDTH] = 105;
		gpBuffer[addr + 26 + 26 * BUFFER_WIDTH] = 105;
		gpBuffer[addr + 25 + 25 * BUFFER_WIDTH] = 106;
		gpBuffer[addr + 24 + 24 * BUFFER_WIDTH] = 106;
		gpBuffer[addr + 23 + 23 * BUFFER_WIDTH] = 105;
		gpBuffer[addr + 22 + 22 * BUFFER_WIDTH] = 104;
		gpBuffer[addr + 21 + 21 * BUFFER_WIDTH] = 103;

		gpBuffer[addr + 17 + 19 * BUFFER_WIDTH] = TRANS_COLOR; // 118;
		gpBuffer[addr + 16 + 19 * BUFFER_WIDTH] = TRANS_COLOR; // 114;
		gpBuffer[addr + 20 + 20 * BUFFER_WIDTH] = 103;
		gpBuffer[addr + 15 + 20 * BUFFER_WIDTH] = 96;
		gpBuffer[addr + 16 + 20 * BUFFER_WIDTH] = 82;
		gpBuffer[addr + 14 + 21 * BUFFER_WIDTH] = 96;
		gpBuffer[addr + 15 + 21 * BUFFER_WIDTH] = 114;
		gpBuffer[addr + 14 + 22 * BUFFER_WIDTH] = 114;
		gpBuffer[addr + 13 + 22 * BUFFER_WIDTH] = 97;
		gpBuffer[addr + 11 + 25 * BUFFER_WIDTH] = 112;
		gpBuffer[addr + 11 + 26 * BUFFER_WIDTH] = 113;
		gpBuffer[addr + 18 + 24 * BUFFER_WIDTH] = 82;
		gpBuffer[addr + 16 + 27 * BUFFER_WIDTH] = 83;
		gpBuffer[addr + 19 + 20 * BUFFER_WIDTH] = 120;

		gpBuffer[addr + 12 + 23 * BUFFER_WIDTH] = 114;
		gpBuffer[addr + 10 + 26 * BUFFER_WIDTH] = 114;
		gpBuffer[addr +  9 + 29 * BUFFER_WIDTH] = 124;
		gpBuffer[addr +  9 + 30 * BUFFER_WIDTH] = 110;
	}
	{ // 212[6] - sync
		int i = 27;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		// gpBuffer[addr + 17 + 19 * BUFFER_WIDTH] = 102;
		// gpBuffer[addr + 16 + 19 * BUFFER_WIDTH] = 98;
		gpBuffer[addr + 20 + 20 * BUFFER_WIDTH] = 103;
		gpBuffer[addr + 15 + 20 * BUFFER_WIDTH] = 96;
		gpBuffer[addr + 14 + 21 * BUFFER_WIDTH] = 96;
		gpBuffer[addr + 13 + 22 * BUFFER_WIDTH] = 112;
		gpBuffer[addr + 11 + 25 * BUFFER_WIDTH] = 113;

		gpBuffer[addr + 12 + 23 * BUFFER_WIDTH] = 112;
		gpBuffer[addr + 10 + 26 * BUFFER_WIDTH] = 114;
		gpBuffer[addr +  9 + 29 * BUFFER_WIDTH] = 59;
		gpBuffer[addr +  9 + 30 * BUFFER_WIDTH] = 110;

		gpBuffer[addr + 11 + 27 * BUFFER_WIDTH] = 118;
		gpBuffer[addr + 10 + 27 * BUFFER_WIDTH] = 117;
		gpBuffer[addr + 10 + 28 * BUFFER_WIDTH] = 120;
		gpBuffer[addr + 13 + 25 * BUFFER_WIDTH] = 116;
		gpBuffer[addr + 15 + 22 * BUFFER_WIDTH] = 96;
	}
	{ // 251[6] - sync
		int i = 28;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		// gpBuffer[addr + 16 + 19 * BUFFER_WIDTH] = 98;
		gpBuffer[addr + 15 + 20 * BUFFER_WIDTH] = 96;
		gpBuffer[addr + 14 + 21 * BUFFER_WIDTH] = 96;
		gpBuffer[addr + 13 + 22 * BUFFER_WIDTH] = 112;
		gpBuffer[addr + 11 + 25 * BUFFER_WIDTH] = 113;
		gpBuffer[addr + 10 + 27 * BUFFER_WIDTH] = 117;

		gpBuffer[addr + 12 + 23 * BUFFER_WIDTH] = 112;
		gpBuffer[addr + 10 + 26 * BUFFER_WIDTH] = 114;
		gpBuffer[addr +  9 + 29 * BUFFER_WIDTH] = 59;
		gpBuffer[addr +  9 + 30 * BUFFER_WIDTH] = 110;

		gpBuffer[addr + 10 + 28 * BUFFER_WIDTH] = 104;
	}
	// 1[7], 11[7], 196[7], 204[7], 208[7], 254[7] - sync
	for (int i = 29; i < 35; i++) {
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 14 + 19 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 15 + 19 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 11 + 20 * BUFFER_WIDTH] = 108;
	}

	// fix shadow on 227[1] using 4[1]
	for (int i = 7; i < 8; i++) {
		for (int x = 7; x < 13; x++) {
			for (int y = 22; y < 28; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (35 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (35 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 4[1]
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
	}
	// fix shadow on 152[1] using 155[1]
	for (int i = 37; i < 38; i++) {
		for (int x = 7; x < 13; x++) {
			for (int y = 22; y < 28; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (36 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (36 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 155[1]
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
	}
	// fix shadow on 238[0] to make it more usable
	for (int i = 38; i < 39; i++) {
		for (int x = 26; x < MICRO_WIDTH; x++) {
			for (int y = 26; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR && color != 0 && (color % 16) < 12) {
					gpBuffer[addr] = (color & ~15) | (12 + (color % 16) / 4);
				}
			}
		}
	}
	// fix shadow on 238[1] to make it more usable
	for (int i = 39; i < 40; i++) {
		for (int x = 0; x < 8; x++) {
			for (int y = 21; y < 29; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR && color != 0 && (color % 16) < 12) {
					gpBuffer[addr] = (color & ~15) | (12 + (color % 16) / 4);
				}
			}
		}
	}

	// create the new CEL file
	constexpr int newEntries = lengthof(micros);
	size_t maxCelSize = *celLen + newEntries * MICRO_WIDTH * MICRO_HEIGHT;
	BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
	memset(resCelBuf, 0, maxCelSize);

	CelFrameEntry entries[newEntries];
	xx = 0, yy = MICRO_HEIGHT - 1;
	int idx = 0;
	for (int i = 0; i < newEntries; i++) {
		const CelMicro &micro = micros[i];
		if (micro.res_encoding >= 0) {
			entries[idx].encoding = micro.res_encoding;
			unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
			entries[idx].frameRef = SwapLE16(pSubtiles[index]) & 0xFFF;
			entries[idx].frameSrc = &gpBuffer[xx + yy * BUFFER_WIDTH];
			idx++;
		}
		yy += MICRO_HEIGHT;
		if (yy == (DRAW_HEIGHT + 1) * MICRO_HEIGHT - 1) {
			yy = MICRO_HEIGHT - 1;
			xx += MICRO_WIDTH;
		}
	}

	*celLen = encodeCelMicros(entries, idx, resCelBuf, celBuf, TRANS_COLOR);

	mem_free_dbg(celBuf);

	return resCelBuf;
}

static BYTE* patchHellWall1Cel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	const CelMicro micros[] = {
/*  0 */{   8 - 1, 0, -1 }, // mask walls leading to north east
/*  1 */{   8 - 1, 1, -1 },
/*  2 */{   8 - 1, 2, -1 },
/*  3 */{  21 - 1, 3, -1 },
/*  4 */{   8 - 1, 4, -1 },
/*  5 */{   8 - 1, 5, -1 },

/*  6 */{  21 - 1, 0, MET_LTRAPEZOID },
/*  7 */{  21 - 1, 2, MET_SQUARE },
/*  8 */{  21 - 1, 4, MET_TRANSPARENT },

/*  9 */{   1 - 1, 1, MET_TRANSPARENT },
/* 10 */{   1 - 1, 3, MET_TRANSPARENT },
/* 11 */{   1 - 1, 5, MET_TRANSPARENT },
/* 12 */{   1 - 1, 7, MET_TRANSPARENT },
/* 13 */{  11 - 1, 1, MET_TRANSPARENT },
/* 14 */{  11 - 1, 3, MET_TRANSPARENT },
/* 15 */{  11 - 1, 5, MET_TRANSPARENT },
/* 16 */{  11 - 1, 7, MET_TRANSPARENT },
/* 17 */{ 167 - 1, 1, MET_TRANSPARENT },
/* 18 */{ 167 - 1, 3, MET_TRANSPARENT },
/* 19 */{ 167 - 1, 5, MET_TRANSPARENT },
/* 20 */{ /*167*/ - 1, 7, -1 },
/* 21 */{ /*188*/ - 1, 1, -1 },
/* 22 */{ /*188*/ - 1, 3, -1 },
/* 23 */{ 188 - 1, 5, MET_TRANSPARENT },
/* 24 */{ /*188*/ - 1, 7, -1 },
/* 25 */{ /*196*/ - 1, 1, -1 },
/* 26 */{ /*196*/ - 1, 3, -1 },
/* 27 */{ 196 - 1, 5, MET_TRANSPARENT },
/* 28 */{ 196 - 1, 7, MET_TRANSPARENT },
/* 29 */{ 204 - 1, 1, MET_TRANSPARENT },
/* 30 */{ /*204*/ - 1, 3, -1 },
/* 31 */{ 204 - 1, 5, MET_TRANSPARENT },
/* 32 */{ 204 - 1, 7, MET_TRANSPARENT },
/* 33 */{ 208 - 1, 1, MET_TRANSPARENT },
/* 34 */{ 208 - 1, 3, MET_TRANSPARENT },
/* 35 */{ 208 - 1, 5, MET_TRANSPARENT },
/* 36 */{ 208 - 1, 7, MET_TRANSPARENT },
/* 37 */{ /*251*/ - 1, 1, -1 },
/* 38 */{ 251 - 1, 3, MET_TRANSPARENT },
/* 39 */{ 251 - 1, 5, MET_TRANSPARENT },
/* 40 */{ 251 - 1, 7, MET_TRANSPARENT },
/* 41 */{ /*254*/ - 1, 1, -1 },
/* 42 */{ /*254*/ - 1, 3, -1 },
/* 43 */{ 254 - 1, 5, MET_TRANSPARENT },
/* 44 */{ 254 - 1, 7, MET_TRANSPARENT },

/* 45 */{   3 - 1, 0, MET_TRANSPARENT },
/* 46 */{   3 - 1, 2, MET_TRANSPARENT },
/* 47 */{ /*190*/ - 1, 0, -1 },
/* 48 */{ 190 - 1, 2, MET_TRANSPARENT },
/* 49 */{ 198 - 1, 0, MET_TRANSPARENT },
/* 50 */{ /*198*/ - 1, 2, -1 },
/* 51 */{ 206 - 1, 0, MET_TRANSPARENT },
/* 52 */{ 206 - 1, 2, MET_TRANSPARENT },
/* 53 */{ 210 - 1, 0, MET_TRANSPARENT },
/* 54 */{ /*210*/ - 1, 2, -1 },

/* 55 */{  23 - 1, 4, MET_TRANSPARENT },
/* 56 */{ 190 - 1, 4, MET_TRANSPARENT },
/* 57 */{ 198 - 1, 4, MET_TRANSPARENT },
/* 58 */{ 206 - 1, 4, MET_TRANSPARENT },
/* 59 */{ 210 - 1, 4, MET_TRANSPARENT },
/* 60 */{  48 - 1, 4, MET_TRANSPARENT },
/* 61 */{  58 - 1, 4, MET_TRANSPARENT },

/* 62 */{   6 - 1, 5, MET_TRANSPARENT },
/* 63 */{  51 - 1, 5, MET_TRANSPARENT },
/* 64 */{ 193 - 1, 5, MET_TRANSPARENT },
/* 65 */{ 201 - 1, 5, MET_TRANSPARENT },
/* 66 */{ 217 - 1, 5, MET_TRANSPARENT },

/* 67 */{ /*244*/ - 1, 0, -1 },
/* 68 */{ 244 - 1, 2, MET_TRANSPARENT },
/* 69 */{ 244 - 1, 4, MET_TRANSPARENT },

/* 70 */{ /*246*/ - 1, 1, -1 },
/* 71 */{ 246 - 1, 3, MET_TRANSPARENT },
/* 72 */{ 246 - 1, 5, MET_TRANSPARENT },

/* 73 */{ 257 - 1, 5, MET_TRANSPARENT },
	};
	const uint16_t* pSubtiles = (const uint16_t*)minBuf;
	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_L4;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		if (micro.subtileIndex < 0) {
			continue;
		}
		unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
		if ((SwapLE16(pSubtiles[index]) & 0xFFF) == 0) {
			return celBuf; // frame is empty -> assume it is already done
		}
	}

	// TODO: check celLen
	// draw the micros to the back-buffer
	pMicrosCel = celBuf;
	constexpr BYTE TRANS_COLOR = 128;
	constexpr int DRAW_HEIGHT = 8;
	memset(&gpBuffer[0], TRANS_COLOR, DRAW_HEIGHT * BUFFER_WIDTH * MICRO_HEIGHT);

	unsigned xx = 0, yy = MICRO_HEIGHT - 1;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		if (micro.subtileIndex >= 0) {
			unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
			RenderMicro(&gpBuffer[xx + yy * BUFFER_WIDTH], SwapLE16(pSubtiles[index]), DMT_NONE);
		}
		yy += MICRO_HEIGHT;
		if (yy == (DRAW_HEIGHT + 1) * MICRO_HEIGHT - 1) {
			yy = MICRO_HEIGHT - 1;
			xx += MICRO_WIDTH;
		}
	}

	// mask 11[1], 167[1] using 1[1]
	for (int i = 13; i < 21; i += 4) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (9 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (9 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 1[1]
				if (gpBuffer[addr2] == TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	{ // mask 1[3]
		int i = 10;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		// gpBuffer[addr + 17 + 20 * BUFFER_WIDTH] = TRANS_COLOR;
		// gpBuffer[addr + 17 + 21 * BUFFER_WIDTH] = TRANS_COLOR;
		// gpBuffer[addr + 17 + 22 * BUFFER_WIDTH] = TRANS_COLOR;

		gpBuffer[addr + 17 + 31 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // mask 208[3]
		int i = 34;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 17 +  8 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 17 +  9 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 17 + 10 * BUFFER_WIDTH] = TRANS_COLOR;

		// gpBuffer[addr + 17 + 20 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 17 + 21 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 17 + 22 * BUFFER_WIDTH] = TRANS_COLOR;

		gpBuffer[addr + 17 + 31 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	// mask 167[3]
	for (int i = 18; i < 19; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (x < 17) {
					continue;
				}
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}
	// mask 11[3] using 21[3]
	for (int i = 14; i < 15; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (3 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (3 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 21[3]
				if (x < 17) {
					continue;
				}
				if (gpBuffer[addr2] != TRANS_COLOR) {
					continue;
				}
				BYTE color = gpBuffer[addr];
				if (color != 63 && color != 78 && color != 79 && color != 94 && color != 95 && (color < 107 || color > 111) && color != 126 && color != 127) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 251[3]
	for (int i = 38; i < 39; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (x < 17) {
					continue;
				}
				if (y < x - 18) {
					continue;
				}
				if (y == 14 || y == 15) {
					continue; // keep pixels continuous
				}
				BYTE color = gpBuffer[addr];
				if (color != 63 && color != 78 && color != 79 && color != 94 && color != 95 && (color < 107 || color > 111) && color != 126 && color != 127) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 1[5], 167[5], 188[5], 196[5], 204[5], 208[5]
	for (int i = 11; i < 37; i += 4) {
		if (i == 19) { // 167[5]
			for (int x = 17; x < MICRO_WIDTH; x++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
					if (y > 21 - x) {
						gpBuffer[addr] = TRANS_COLOR;
					}
				}
			}
		} else if (i != 15) { // 1[5], 188[5], 196[5], 204[5], 208[5]
			unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
			gpBuffer[addr + 20 +  2 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[addr + 19 +  3 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[addr + 18 +  4 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[addr + 17 +  5 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[addr + 17 +  6 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[addr + 17 +  7 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[addr + 17 +  8 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[addr + 17 +  9 * BUFFER_WIDTH] = TRANS_COLOR;
			if (i != 35) { // 208[5]
				gpBuffer[addr + 17 + 10 * BUFFER_WIDTH] = TRANS_COLOR;
				gpBuffer[addr + 17 + 11 * BUFFER_WIDTH] = TRANS_COLOR;
				gpBuffer[addr + 17 + 12 * BUFFER_WIDTH] = TRANS_COLOR;
			}
			if (i == 31) { // 204[5]
				gpBuffer[addr + 17 + 13 * BUFFER_WIDTH] = TRANS_COLOR;
				gpBuffer[addr + 17 + 14 * BUFFER_WIDTH] = TRANS_COLOR;
			}
		}
	}
	// mask 11[7] using 1[7]
	for (int i = 16; i < 17; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (12 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (12 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 1[7]
				if (x >= 20 || y < 14) {
					continue; // preserve the tusk
				}
				if (x >= 9 && y >= 26) {
					continue; // preserve the shadow
				}
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
	}
	// mask 3[0], 198[0], 206[0], 210[0] using 1[1] and 167[3]
	for (int i = 45; i < 55; i += 2) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * (18 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (18 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 167[3]
				} else {
					addr2 = x + MICRO_WIDTH * (9 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (9 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 1[1]
				}
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
		{ // remove shadow using '167[3]'
			unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
			gpBuffer[addr + 17 + 0 * BUFFER_WIDTH] = 72;
			gpBuffer[addr + 17 + 1 * BUFFER_WIDTH] = 72;
			gpBuffer[addr + 17 + 2 * BUFFER_WIDTH] = 71;
			gpBuffer[addr + 17 + 3 * BUFFER_WIDTH] = 70;
			gpBuffer[addr + 17 + 4 * BUFFER_WIDTH] = 121;
			gpBuffer[addr + 17 + 5 * BUFFER_WIDTH] = 85;
			gpBuffer[addr + 17 + 6 * BUFFER_WIDTH] = 123;
			gpBuffer[addr + 18 + 0 * BUFFER_WIDTH] = 72;
			gpBuffer[addr + 18 + 1 * BUFFER_WIDTH] = 72;
			gpBuffer[addr + 18 + 2 * BUFFER_WIDTH] = 70;
			gpBuffer[addr + 18 + 3 * BUFFER_WIDTH] = 71;
			gpBuffer[addr + 18 + 4 * BUFFER_WIDTH] = 120;
			gpBuffer[addr + 18 + 5 * BUFFER_WIDTH] = 86;
			gpBuffer[addr + 18 + 6 * BUFFER_WIDTH] = 119;
			gpBuffer[addr + 19 + 0 * BUFFER_WIDTH] = 72;
			gpBuffer[addr + 19 + 1 * BUFFER_WIDTH] = 69;
			gpBuffer[addr + 19 + 2 * BUFFER_WIDTH] = 71;
			gpBuffer[addr + 19 + 3 * BUFFER_WIDTH] = 72;
			gpBuffer[addr + 19 + 4 * BUFFER_WIDTH] = 70;
			gpBuffer[addr + 19 + 5 * BUFFER_WIDTH] = 104;
			gpBuffer[addr + 19 + 6 * BUFFER_WIDTH] = 104;
		}
	}
	// mask 3[2], 190[2], 206[2]
	for (int i = 46; i < 55; i += 2) {
		for (int x = 0; x < 17; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
		{ // remove shadow using '167[3]'
			unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
			gpBuffer[addr + 17 + 21 * BUFFER_WIDTH] = 76;
			gpBuffer[addr + 17 + 22 * BUFFER_WIDTH] = 74;
			gpBuffer[addr + 17 + 23 * BUFFER_WIDTH] = 117;
			gpBuffer[addr + 17 + 24 * BUFFER_WIDTH] = 105;
			gpBuffer[addr + 17 + 25 * BUFFER_WIDTH] = 104;
			gpBuffer[addr + 17 + 26 * BUFFER_WIDTH] = 105;
			gpBuffer[addr + 17 + 27 * BUFFER_WIDTH] = 63;
			gpBuffer[addr + 17 + 28 * BUFFER_WIDTH] = 71;
			gpBuffer[addr + 17 + 29 * BUFFER_WIDTH] = 121;
			gpBuffer[addr + 17 + 30 * BUFFER_WIDTH] = 90;
			gpBuffer[addr + 17 + 31 * BUFFER_WIDTH] = 60;
			gpBuffer[addr + 18 + 21 * BUFFER_WIDTH] = 77;
			gpBuffer[addr + 18 + 22 * BUFFER_WIDTH] = 120;
			gpBuffer[addr + 18 + 23 * BUFFER_WIDTH] = 114;
			gpBuffer[addr + 18 + 24 * BUFFER_WIDTH] = 120;
			gpBuffer[addr + 18 + 25 * BUFFER_WIDTH] = 103;
			gpBuffer[addr + 18 + 26 * BUFFER_WIDTH] = 125;
			gpBuffer[addr + 18 + 27 * BUFFER_WIDTH] = 74;
			gpBuffer[addr + 18 + 28 * BUFFER_WIDTH] = 68;
			gpBuffer[addr + 18 + 29 * BUFFER_WIDTH] = 76;
			gpBuffer[addr + 18 + 30 * BUFFER_WIDTH] = 73;
			gpBuffer[addr + 18 + 31 * BUFFER_WIDTH] = 72;
			gpBuffer[addr + 19 + 21 * BUFFER_WIDTH] = 73;
			gpBuffer[addr + 19 + 22 * BUFFER_WIDTH] = 98;
			gpBuffer[addr + 19 + 23 * BUFFER_WIDTH] = 102;
			gpBuffer[addr + 19 + 24 * BUFFER_WIDTH] = 104;
			gpBuffer[addr + 19 + 25 * BUFFER_WIDTH] = 108;
			gpBuffer[addr + 19 + 26 * BUFFER_WIDTH] = 78;
			gpBuffer[addr + 19 + 27 * BUFFER_WIDTH] = 73;
			gpBuffer[addr + 19 + 28 * BUFFER_WIDTH] = 91;
			gpBuffer[addr + 19 + 29 * BUFFER_WIDTH] = 71;
			gpBuffer[addr + 19 + 30 * BUFFER_WIDTH] = 71;
			gpBuffer[addr + 19 + 31 * BUFFER_WIDTH] = 71;
		}
	}
	// mask 23[4], 190[4], 198[4], 206[4] + 48[4], 58[4] using 1[7]
	for (int i = 55; i < 62; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * (12 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (12 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 1[7]
					if (gpBuffer[addr2] != TRANS_COLOR) {
						gpBuffer[addr] = TRANS_COLOR;
					}
				} else {
					if (x < 17 || (y < 38 - x)) {
						gpBuffer[addr] = TRANS_COLOR;
					}
				}
			}
		}
	}
	// fix 11[1], 246[1] using 8[1]
	for (int i = 13; i < 73; i += 57) {
		for (int x = 0; x < 3; x++) {
			for (int y = 0; y < 3; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (1 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (1 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 8[1]
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
	}
	// fix 11[3], 246[3] using 21[3]
	for (int i = 14; i < 73; i += 57) {
		for (int x = 0; x < 5; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (3 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (3 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 21[3]
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
	}
	// fix 11[5], 246[5] using 8[5]
	for (int i = 15; i < 73; i += 57) {
		for (int x = 0; x < 5; x++) {
			for (int y = 24; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (5 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (5 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 8[5]
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
	}
	// remove shadow from 254[5] using 8[5]
	for (int i = 43; i < 44; i++) {
		for (int x = 0; x < 5; x++) {
			for (int y = 24; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (5 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (5 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 8[5]
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
	}
	// fix 21[0], 244[0] using 8[0]
	for (int i = 6; i < 70; i += 61) {
		for (int x = 28; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < 3; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (0 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (0 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 8[0]
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
	}
	// fix 21[2], 244[2] using 8[2]
	for (int i = 7; i < 70; i += 61) {
		for (int x = 27; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (2 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (2 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 8[2]
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
	}
	// fix 21[4], 244[4] using 8[4]
	for (int i = 8; i < 70; i += 61) {
		for (int x = 27; x < MICRO_WIDTH; x++) {
			for (int y = 24; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (4 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (4 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 8[4]
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
	}

	// fix 11[3] - erase inconsistent shadow
	for (int i = 14; i < 15; i++) {
		for (int x = 18; x < 25; x++) {
			for (int y = 0; y < 10; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y < 2 * x - 31) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 30 + 20 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 31 + 19 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 17 + 23 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 18 + 23 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 17 + 24 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 18 + 24 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 19 + 24 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 20 + 24 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 20 + 25 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 21 + 25 * BUFFER_WIDTH] = TRANS_COLOR;

		gpBuffer[addr + 14 + 12 * BUFFER_WIDTH] = 63;
		gpBuffer[addr + 16 + 11 * BUFFER_WIDTH] = 63;
	}
	// fix 11[5], 254[5] - erase inconsistent shadow
	for (int i = 15; i < 68; i += 28) {
		for (int x = 18; x < 24; x++) {
			for (int y = 26; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y < 28) {
					continue; // keep pixels continuous
				}
				if (y > 47 - x && y > x + 5) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		// gpBuffer[addr + 20 + 26 * BUFFER_WIDTH] = TRANS_COLOR; - keep pixels continuous
		// gpBuffer[addr + 20 + 27 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 24 + 30 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 24 + 31 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	// fix 251[3]
	for (int i = 38; i < 39; i++) {
		for (int x = 18; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < 10; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				// erase non-symmetric tusk
				if (y < 3 * x / 2 - 39) {
					gpBuffer[addr] = TRANS_COLOR;
				}
				// erase inconsistent shadow
				if (y == 0) {
					continue; // keep pixels continuous
				}
				if (y < 2 * x - 31 && y > x - 19) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 17 +  1 * BUFFER_WIDTH] = TRANS_COLOR;
		// gpBuffer[addr + 19 +  0 * BUFFER_WIDTH] = TRANS_COLOR; - keep pixels continuous

		gpBuffer[addr + 31 +  3 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 31 +  4 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 31 +  5 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 31 +  6 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 31 +  7 * BUFFER_WIDTH] = 116;
		gpBuffer[addr + 31 + 12 * BUFFER_WIDTH] = TRANS_COLOR;

		gpBuffer[addr + 30 + 20 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 31 + 19 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 17 + 23 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 18 + 23 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 17 + 24 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 18 + 24 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 19 + 24 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 20 + 24 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 20 + 25 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 21 + 25 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	// fix 251[5] - erase non-symmetric tusk
	for (int i = 39; i < 40; i++) {
		for (int x = 25; x < MICRO_WIDTH; x++) {
			for (int y = 22; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y < 2 * x - 21) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	{ // 1[3] - add shadow
		int i = 10;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 19 +  7 * BUFFER_WIDTH] = 126;
		gpBuffer[addr + 18 +  8 * BUFFER_WIDTH] = 105;
		gpBuffer[addr + 19 +  8 * BUFFER_WIDTH] = 126;
		gpBuffer[addr + 18 +  9 * BUFFER_WIDTH] = 108;
		gpBuffer[addr + 19 +  9 * BUFFER_WIDTH] = 126;
		gpBuffer[addr + 18 + 10 * BUFFER_WIDTH] = 107;
		gpBuffer[addr + 19 + 10 * BUFFER_WIDTH] = 111;
		gpBuffer[addr + 18 + 11 * BUFFER_WIDTH] = 111;
		gpBuffer[addr + 19 + 11 * BUFFER_WIDTH] = 95;
		gpBuffer[addr + 18 + 12 * BUFFER_WIDTH] = 63;
		gpBuffer[addr + 19 + 12 * BUFFER_WIDTH] = 95;
		gpBuffer[addr + 18 + 13 * BUFFER_WIDTH] = 63;
		gpBuffer[addr + 19 + 13 * BUFFER_WIDTH] = 63;
		gpBuffer[addr + 18 + 14 * BUFFER_WIDTH] = 63;
		gpBuffer[addr + 19 + 14 * BUFFER_WIDTH] = 63;
		gpBuffer[addr + 18 + 15 * BUFFER_WIDTH] = 63;
		gpBuffer[addr + 19 + 15 * BUFFER_WIDTH] = 78;
		gpBuffer[addr + 18 + 16 * BUFFER_WIDTH] = 78;
		gpBuffer[addr + 19 + 16 * BUFFER_WIDTH] = 78;
		gpBuffer[addr + 17 + 17 * BUFFER_WIDTH] = 63;
		gpBuffer[addr + 18 + 17 * BUFFER_WIDTH] = 78;
		gpBuffer[addr + 19 + 17 * BUFFER_WIDTH] = 78;
		gpBuffer[addr + 17 + 18 * BUFFER_WIDTH] = 63;
		gpBuffer[addr + 18 + 18 * BUFFER_WIDTH] = 63;
		gpBuffer[addr + 19 + 18 * BUFFER_WIDTH] = 78;
		gpBuffer[addr + 17 + 19 * BUFFER_WIDTH] = 78;
		gpBuffer[addr + 18 + 19 * BUFFER_WIDTH] = 78;
		gpBuffer[addr + 19 + 19 * BUFFER_WIDTH] = 78;
		gpBuffer[addr + 17 + 20 * BUFFER_WIDTH] = 63;
		gpBuffer[addr + 18 + 20 * BUFFER_WIDTH] = 78;
		gpBuffer[addr + 19 + 20 * BUFFER_WIDTH] = 78;
		gpBuffer[addr + 17 + 21 * BUFFER_WIDTH] = 126;
		gpBuffer[addr + 18 + 21 * BUFFER_WIDTH] = 63;
		gpBuffer[addr + 19 + 21 * BUFFER_WIDTH] = 74;
		gpBuffer[addr + 17 + 22 * BUFFER_WIDTH] = 126;
		gpBuffer[addr + 18 + 22 * BUFFER_WIDTH] = 126;
		gpBuffer[addr + 19 + 22 * BUFFER_WIDTH] = 105;
		gpBuffer[addr + 17 + 23 * BUFFER_WIDTH] = 109;
		gpBuffer[addr + 18 + 23 * BUFFER_WIDTH] = 105;
		// keep pixels continuous
		gpBuffer[addr + 17 +  7 * BUFFER_WIDTH] = 117;
		gpBuffer[addr + 18 +  7 * BUFFER_WIDTH] = 114;
		gpBuffer[addr + 17 +  8 * BUFFER_WIDTH] = 105;
		gpBuffer[addr + 17 +  9 * BUFFER_WIDTH] = 104;
		gpBuffer[addr + 17 + 10 * BUFFER_WIDTH] = 105;
		gpBuffer[addr + 17 + 11 * BUFFER_WIDTH] = 63;
		gpBuffer[addr + 17 + 12 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 17 + 13 * BUFFER_WIDTH] = 121;
		gpBuffer[addr + 17 + 14 * BUFFER_WIDTH] = 90;
		gpBuffer[addr + 17 + 15 * BUFFER_WIDTH] = 60;
		gpBuffer[addr + 17 + 16 * BUFFER_WIDTH] = 72;
	}
	{ // 208[3] - add shadow
		int i = 34;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 17 + 12 * BUFFER_WIDTH] = 77;
		gpBuffer[addr + 17 + 13 * BUFFER_WIDTH] = 105;
		gpBuffer[addr + 17 + 14 * BUFFER_WIDTH] = 60;
		gpBuffer[addr + 17 + 15 * BUFFER_WIDTH] = 76;
		gpBuffer[addr + 17 + 16 * BUFFER_WIDTH] = 78;
		gpBuffer[addr + 17 + 17 * BUFFER_WIDTH] = 63;
		gpBuffer[addr + 17 + 18 * BUFFER_WIDTH] = 78;
		gpBuffer[addr + 17 + 19 * BUFFER_WIDTH] = 78;
		gpBuffer[addr + 17 + 20 * BUFFER_WIDTH] = 72;
		gpBuffer[addr + 18 + 16 * BUFFER_WIDTH] = 78;
		gpBuffer[addr + 18 + 17 * BUFFER_WIDTH] = 76;
		gpBuffer[addr + 18 + 18 * BUFFER_WIDTH] = 60;
		gpBuffer[addr + 18 + 19 * BUFFER_WIDTH] = 109;
	}
	{ // 190[4] - fix connection
		int i = 56;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 18 +  5 * BUFFER_WIDTH] = 96;
		gpBuffer[addr + 17 +  4 * BUFFER_WIDTH] = 97;
		gpBuffer[addr + 18 +  4 * BUFFER_WIDTH] = 113;
		gpBuffer[addr + 19 +  4 * BUFFER_WIDTH] = 112;
		gpBuffer[addr + 19 +  5 * BUFFER_WIDTH] = 96;
		gpBuffer[addr + 20 +  5 * BUFFER_WIDTH] = 81;
		gpBuffer[addr + 19 +  6 * BUFFER_WIDTH] = 81;
		gpBuffer[addr + 20 +  6 * BUFFER_WIDTH] = 114;
		gpBuffer[addr + 20 +  7 * BUFFER_WIDTH] = 113;
		gpBuffer[addr + 20 +  8 * BUFFER_WIDTH] = 113;
		gpBuffer[addr + 21 +  9 * BUFFER_WIDTH] = 113;
		gpBuffer[addr + 22 + 11 * BUFFER_WIDTH] = 99;
		gpBuffer[addr + 22 + 12 * BUFFER_WIDTH] = 101;

		gpBuffer[addr + 13 +  3 * BUFFER_WIDTH] = 105;
		gpBuffer[addr + 14 +  3 * BUFFER_WIDTH] = 103;
	}
	{ // 198[4] - fix connection
		int i = 57;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 20 +  8 * BUFFER_WIDTH] = 114;
		gpBuffer[addr + 22 + 10 * BUFFER_WIDTH] = 117;
		gpBuffer[addr + 22 + 11 * BUFFER_WIDTH] = 83;
		gpBuffer[addr + 22 + 12 * BUFFER_WIDTH] = 101;
		gpBuffer[addr + 23 + 11 * BUFFER_WIDTH] = 120;
		gpBuffer[addr + 23 + 12 * BUFFER_WIDTH] = 122;

		gpBuffer[addr + 13 +  3 * BUFFER_WIDTH] = 105;
		gpBuffer[addr + 14 +  3 * BUFFER_WIDTH] = 103;
		gpBuffer[addr + 15 +  3 * BUFFER_WIDTH] = 100;
		gpBuffer[addr + 16 +  3 * BUFFER_WIDTH] = 98;
		gpBuffer[addr + 14 +  2 * BUFFER_WIDTH] = 104;
	}
	{ // 206[4] - fix connection
		int i = 58;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 17 +  1 * BUFFER_WIDTH] = 118;
		gpBuffer[addr + 17 +  2 * BUFFER_WIDTH] = 85;
		gpBuffer[addr + 16 +  2 * BUFFER_WIDTH] = 100;
		gpBuffer[addr + 17 +  3 * BUFFER_WIDTH] = 84;
		gpBuffer[addr + 17 +  4 * BUFFER_WIDTH] = 81;
		gpBuffer[addr + 18 +  4 * BUFFER_WIDTH] = 82;
		gpBuffer[addr + 19 +  5 * BUFFER_WIDTH] = 81;
		gpBuffer[addr + 20 +  6 * BUFFER_WIDTH] = 113;
		gpBuffer[addr + 21 +  8 * BUFFER_WIDTH] = 112;
		gpBuffer[addr + 23 +  8 * BUFFER_WIDTH] = 113;
		gpBuffer[addr + 21 +  9 * BUFFER_WIDTH] = 112;
		gpBuffer[addr + 22 +  9 * BUFFER_WIDTH] = 112;
		gpBuffer[addr + 23 +  9 * BUFFER_WIDTH] = 112;
		gpBuffer[addr + 24 +  9 * BUFFER_WIDTH] = 115;
		gpBuffer[addr + 23 + 10 * BUFFER_WIDTH] = 114;
		gpBuffer[addr + 22 + 10 * BUFFER_WIDTH] = 112;
		gpBuffer[addr + 22 + 11 * BUFFER_WIDTH] = 98;
		gpBuffer[addr + 22 + 12 * BUFFER_WIDTH] = 101;

		gpBuffer[addr + 13 +  3 * BUFFER_WIDTH] = 105;
		gpBuffer[addr + 14 +  3 * BUFFER_WIDTH] = 101;
		gpBuffer[addr + 15 +  3 * BUFFER_WIDTH] = 100;
		gpBuffer[addr + 16 +  3 * BUFFER_WIDTH] = 100;
	}
	{ // 210[4] - fix connection
		int i = 59;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 16 +  2 * BUFFER_WIDTH] = 85;
		gpBuffer[addr + 17 +  3 * BUFFER_WIDTH] = 85;
		gpBuffer[addr + 17 +  4 * BUFFER_WIDTH] = 82;
		gpBuffer[addr + 18 +  2 * BUFFER_WIDTH] = 86;
		gpBuffer[addr + 18 +  3 * BUFFER_WIDTH] = 84;
		gpBuffer[addr + 18 +  4 * BUFFER_WIDTH] = 83;
		gpBuffer[addr + 18 +  5 * BUFFER_WIDTH] = 82;
		gpBuffer[addr + 19 +  2 * BUFFER_WIDTH] = 85;
		gpBuffer[addr + 19 +  3 * BUFFER_WIDTH] = 83;
		gpBuffer[addr + 19 +  6 * BUFFER_WIDTH] = 113;
		gpBuffer[addr + 20 +  7 * BUFFER_WIDTH] = 81;
		gpBuffer[addr + 22 + 10 * BUFFER_WIDTH] = 112;
		gpBuffer[addr + 22 + 11 * BUFFER_WIDTH] = 99;
		gpBuffer[addr + 22 + 12 * BUFFER_WIDTH] = 101;

		gpBuffer[addr + 13 +  3 * BUFFER_WIDTH] = 105;
		gpBuffer[addr + 14 +  3 * BUFFER_WIDTH] = 101;
		gpBuffer[addr + 15 +  3 * BUFFER_WIDTH] = 100;
		gpBuffer[addr + 16 +  3 * BUFFER_WIDTH] = 100;
	}
	// 48[4], 58[4] - fix connection
	for (int i = 60; i < 62; i++) {
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 13 +  3 * BUFFER_WIDTH] = 105;
		gpBuffer[addr + 14 +  3 * BUFFER_WIDTH] = 103;
		gpBuffer[addr + 15 +  3 * BUFFER_WIDTH] = 99;

		gpBuffer[addr + 14 +  2 * BUFFER_WIDTH] = 105;
		gpBuffer[addr + 15 +  2 * BUFFER_WIDTH] = 101;
		gpBuffer[addr + 16 +  2 * BUFFER_WIDTH] = 98;
	}
	{ // 23[4] - fix connection
		int i = 55;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 13 +  3 * BUFFER_WIDTH] = 105;
		gpBuffer[addr + 14 +  3 * BUFFER_WIDTH] = 103;
		gpBuffer[addr + 15 +  3 * BUFFER_WIDTH] =  99;
		gpBuffer[addr + 15 +  2 * BUFFER_WIDTH] = 101;
		gpBuffer[addr + 16 +  2 * BUFFER_WIDTH] =  98;
	}
	{ // 1[7] - fix connection
		int i = 12;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 21 + 28 * BUFFER_WIDTH] = 115;
		gpBuffer[addr + 21 + 29 * BUFFER_WIDTH] = 101;
		gpBuffer[addr + 22 + 29 * BUFFER_WIDTH] = 105;
		gpBuffer[addr + 21 + 30 * BUFFER_WIDTH] = 102;
		gpBuffer[addr + 22 + 30 * BUFFER_WIDTH] = 108;
		gpBuffer[addr + 21 + 31 * BUFFER_WIDTH] = 106;
	}
	{ // 196[7] - fix connection
		int i = 28;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 21 + 28 * BUFFER_WIDTH] = 115;
		gpBuffer[addr + 21 + 29 * BUFFER_WIDTH] = 101;
		gpBuffer[addr + 22 + 29 * BUFFER_WIDTH] = 105;
		gpBuffer[addr + 21 + 30 * BUFFER_WIDTH] = 102;
		gpBuffer[addr + 22 + 30 * BUFFER_WIDTH] = 108;
		gpBuffer[addr + 21 + 31 * BUFFER_WIDTH] = 106;
	}
	{ // 204[7] - fix connection
		int i = 32;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 21 + 28 * BUFFER_WIDTH] = 115;
		gpBuffer[addr + 21 + 29 * BUFFER_WIDTH] = 101;
		gpBuffer[addr + 22 + 29 * BUFFER_WIDTH] = 105;
		gpBuffer[addr + 21 + 30 * BUFFER_WIDTH] = 102;
		gpBuffer[addr + 22 + 30 * BUFFER_WIDTH] = 108;
		gpBuffer[addr + 21 + 31 * BUFFER_WIDTH] = 106;
	}
	{ // 208[7] - fix connection
		int i = 36;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 21 + 28 * BUFFER_WIDTH] = 115;
		gpBuffer[addr + 21 + 29 * BUFFER_WIDTH] = 101;
		gpBuffer[addr + 22 + 29 * BUFFER_WIDTH] = 105;
		gpBuffer[addr + 21 + 30 * BUFFER_WIDTH] = 102;
		gpBuffer[addr + 22 + 30 * BUFFER_WIDTH] = 108;
		gpBuffer[addr + 21 + 31 * BUFFER_WIDTH] = 106;
	}
	{ // 251[7] - fix connection
		int i = 44;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 21 + 28 * BUFFER_WIDTH] = 115;
		gpBuffer[addr + 21 + 29 * BUFFER_WIDTH] = 101;
		gpBuffer[addr + 22 + 29 * BUFFER_WIDTH] = 105;
		gpBuffer[addr + 21 + 30 * BUFFER_WIDTH] = 102;
		gpBuffer[addr + 22 + 30 * BUFFER_WIDTH] = 108;
		gpBuffer[addr + 21 + 31 * BUFFER_WIDTH] = 106;
	}
	// 6[5], 51[5], 193[5], 201[5], 217[5] - fix connection
	for (int i = 62; i < 67; i++) {
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  9 + 12 * BUFFER_WIDTH] = 106;
	}
	{ // 193[5] - fix connection+
		int i = 64;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 13 +  5 * BUFFER_WIDTH] = 96;
		gpBuffer[addr + 14 +  4 * BUFFER_WIDTH] = 112;
		gpBuffer[addr + 16 +  3 * BUFFER_WIDTH] = 98;
		gpBuffer[addr + 17 +  3 * BUFFER_WIDTH] = 100;
		gpBuffer[addr + 18 +  3 * BUFFER_WIDTH] = 104;
	}

	// create the new CEL file
	constexpr int newEntries = lengthof(micros);
	size_t maxCelSize = *celLen + newEntries * MICRO_WIDTH * MICRO_HEIGHT;
	BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
	memset(resCelBuf, 0, maxCelSize);

	CelFrameEntry entries[newEntries];
	xx = 0, yy = MICRO_HEIGHT - 1;
	int idx = 0;
	for (int i = 0; i < newEntries; i++) {
		const CelMicro &micro = micros[i];
		if (micro.res_encoding >= 0) {
			entries[idx].encoding = micro.res_encoding;
			unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
			entries[idx].frameRef = SwapLE16(pSubtiles[index]) & 0xFFF;
			entries[idx].frameSrc = &gpBuffer[xx + yy * BUFFER_WIDTH];
			idx++;
		}
		yy += MICRO_HEIGHT;
		if (yy == (DRAW_HEIGHT + 1) * MICRO_HEIGHT - 1) {
			yy = MICRO_HEIGHT - 1;
			xx += MICRO_WIDTH;
		}
	}

	*celLen = encodeCelMicros(entries, idx, resCelBuf, celBuf, TRANS_COLOR);

	mem_free_dbg(celBuf);

	return resCelBuf;
};

static BYTE* patchHellWall2Cel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	const CelMicro micros[] = {
/*  0 */{   8 - 1, 0, -1 }, // mask walls leading to north west
/*  1 */{   8 - 1, 1, -1 },
/*  2 */{   8 - 1, 2, -1 },
/*  3 */{  21 - 1, 3, -1 },
/*  4 */{   8 - 1, 4, -1 },
/*  5 */{   5 - 1, 6, -1 },

/*  6 */{   5 - 1, 0, MET_TRANSPARENT },
/*  7 */{   5 - 1, 2, MET_TRANSPARENT },
/*  8 */{   5 - 1, 4, MET_TRANSPARENT },
/*  9 */{  15 - 1, 0, MET_TRANSPARENT },
/* 10 */{  15 - 1, 2, MET_TRANSPARENT },
/* 11 */{  15 - 1, 4, MET_TRANSPARENT },
/* 12 */{  38 - 1, 0, MET_TRANSPARENT },
/* 13 */{ /*38*/ - 1, 2, -1 },
/* 14 */{ /*38*/ - 1, 4, -1 },
/* 15 */{ 180 - 1, 0, MET_TRANSPARENT },
/* 16 */{ 180 - 1, 2, MET_TRANSPARENT },
/* 17 */{ 180 - 1, 4, MET_TRANSPARENT },
/* 18 */{ 192 - 1, 0, MET_TRANSPARENT },
/* 19 */{ /*192*/ - 1, 2, -1 },
/* 20 */{ 192 - 1, 4, MET_TRANSPARENT },
/* 21 */{ 200 - 1, 0, MET_TRANSPARENT },
/* 22 */{ 200 - 1, 2, MET_TRANSPARENT },
/* 23 */{ 200 - 1, 4, MET_TRANSPARENT },
/* 24 */{ 212 - 1, 0, MET_TRANSPARENT },
/* 25 */{ /*212*/ - 1, 2, -1 },
/* 26 */{ 212 - 1, 4, MET_TRANSPARENT },
/* 27 */{ 216 - 1, 0, MET_TRANSPARENT },
/* 28 */{ /*216*/ - 1, 2, -1 },
/* 29 */{ 216 - 1, 4, MET_TRANSPARENT },
/* 30 */{ /*251*/ - 1, 0, -1 },
/* 31 */{ 251 - 1, 2, MET_TRANSPARENT },
/* 32 */{ 251 - 1, 4, MET_TRANSPARENT },

/* 33 */{   6 - 1, 1, MET_TRANSPARENT },
/* 34 */{   6 - 1, 3, MET_TRANSPARENT },
/* 35 */{ /*193*/ - 1, 1, -1 },
/* 36 */{ /*193*/ - 1, 3, -1 },
/* 37 */{ /*201*/ - 1, 1, -1 },
/* 38 */{ /*201*/ - 1, 3, -1 },
/* 39 */{ 213 - 1, 1, MET_TRANSPARENT },
/* 40 */{ /*213*/ - 1, 3, -1 },
/* 41 */{ /*217*/ - 1, 1, -1 },
/* 42 */{ 217 - 1, 3, MET_TRANSPARENT },

/* 43 */{   6 - 1, 5, MET_TRANSPARENT },
/* 44 */{ 193 - 1, 5, MET_TRANSPARENT },
/* 45 */{ 201 - 1, 5, MET_TRANSPARENT },
/* 46 */{ 213 - 1, 5, MET_TRANSPARENT },
/* 47 */{ 217 - 1, 5, MET_TRANSPARENT },
/* 48 */{  51 - 1, 5, MET_TRANSPARENT },
	};
	const uint16_t* pSubtiles = (const uint16_t*)minBuf;
	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_L4;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		if (micro.subtileIndex < 0) {
			continue;
		}
		unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
		if ((SwapLE16(pSubtiles[index]) & 0xFFF) == 0) {
			return celBuf; // frame is empty -> assume it is already done
		}
	}

	// TODO: check celLen
	// draw the micros to the back-buffer
	pMicrosCel = celBuf;
	constexpr BYTE TRANS_COLOR = 128;
	constexpr int DRAW_HEIGHT = 8;
	memset(&gpBuffer[0], TRANS_COLOR, DRAW_HEIGHT * BUFFER_WIDTH * MICRO_HEIGHT);

	unsigned xx = 0, yy = MICRO_HEIGHT - 1;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		if (micro.subtileIndex >= 0) {
			unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
			RenderMicro(&gpBuffer[xx + yy * BUFFER_WIDTH], SwapLE16(pSubtiles[index]), DMT_NONE);
		}
		yy += MICRO_HEIGHT;
		if (yy == (DRAW_HEIGHT + 1) * MICRO_HEIGHT - 1) {
			yy = MICRO_HEIGHT - 1;
			xx += MICRO_WIDTH;
		}
	}

	// mask 15[0], 38[0], 180[0] using 5[0]
	for (int i = 9; i < 18; i += 3) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (6 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (6 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 5[0]
				if (gpBuffer[addr2] == TRANS_COLOR) {
					BYTE color = gpBuffer[addr];
					if (x < 11 && y < 9 && color > 100 && (color < 112 || color == 126)) {
						continue; // preserve the shadow
					}
					if ((i == 9 || i == 12) && x == 10 && y == 7) {
						continue; // keep pixels continuous
					}
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}

	// mask 5[2], 180[2], 200[2]
	for (int i = 7; i < 24; i += 3) {
		if (i == 7 || i == 16 || i == 22) {
			for (int x = 14; x < 16; x++) {
				for (int y = 0; y < 23; y++) {
					unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	{ // mask++ 180[2]
		int i = 22;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 15 + 23 * BUFFER_WIDTH] = TRANS_COLOR;
	}

	// mask 15[2] using 8[2]
	for (int i = 10; i < 11; i++) {
		for (int x = 0; x < 16; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (2 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (2 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 8[2]
				if (y > 29 && (x < 3 || x == 15)) {
					continue;
				}
				if (y == 29 && (x == 2 || x == 3)) {
					continue;
				}
				if (gpBuffer[addr2] == TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}

	// mask 251[2]
	for (int i = 31; i < 32; i++) {
		for (int x = 0; x < 16; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y > 29 && (x < 3 || x == 15)) {
					continue; // preserve 'extra' shadow
				}
				if (y == 29 && (x == 2 || x == 3)) {
					continue; // preserve 'extra' shadow
				}
				if (y < 2) {
					continue; // keep pixels continuous
				}
				if (y > 12 || y > 14 - x) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  4 + 10 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  3 + 11 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  2 + 12 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  0 + 12 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  1 + 12 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 12 +  2 * BUFFER_WIDTH] = 126;
		gpBuffer[addr + 13 +  0 * BUFFER_WIDTH] = 126;
	}

	// mask 5[4], 15[4], 192[4], 200[4], 212[4], 216[4] + 180[4]
	for (int i = 8; i < 30; i += 3) {
		if (i == 11) { // 15[4] using 8[4]
			for (int x = 0; x < 16; x++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
					unsigned addr2 = x + MICRO_WIDTH * (4 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (4 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 8[4]
					if (gpBuffer[addr2] == TRANS_COLOR) {
						gpBuffer[addr] = TRANS_COLOR;
					}
				}
			}
		} else if (i == 8 || i == 20 || i == 23 || i == 26 || i == 29) { // 5[4], 192[4], 200[4], 212[4], 216[4] + 180[4]
			/*for (int x = 11; x < 16; x++) {
				for (int y = 1; y < 13; y++) {
					unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
					if (y > x - 11) {
						if (i == 26 && y > 9) {
							continue; // 212[4]
						}
						gpBuffer[addr] = TRANS_COLOR;
					}
				}
			}*/
		} else if (i == 17) { // 180[4]
			/*for (int x = 11; x < 16; x++) {
				for (int y = 2; y < 18; y++) {
					unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
					if (y > 2 * x - 24) {
						gpBuffer[addr] = TRANS_COLOR;
					}
				}
			}

			unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
			gpBuffer[addr + 11 +  1 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[addr + 10 +  0 * BUFFER_WIDTH] = TRANS_COLOR;*/
			unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
			gpBuffer[addr + 10 +  0 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[addr + 11 +  2 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[addr + 12 +  3 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[addr + 13 +  4 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[addr + 13 +  5 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[addr + 14 +  7 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[addr + 15 + 17 * BUFFER_WIDTH] = TRANS_COLOR;
		}
	}

	// mask 6[1], 213[1] using 5[0] and 5[2]
	for (int i = 33; i < 43; i += 2) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * (7 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (7 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 5[2]
				} else {
					addr2 = x + MICRO_WIDTH * (6 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (6 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 5[0]
				}
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}

	// mask 6[3], 217[3]
	for (int i = 34; i < 43; i += 2) {
		for (int x = 16; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
		// fix shadow
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 15 + 23 * BUFFER_WIDTH] = 126;
		gpBuffer[addr + 15 + 24 * BUFFER_WIDTH] = 126;
		gpBuffer[addr + 15 + 25 * BUFFER_WIDTH] = 126;
		gpBuffer[addr + 15 + 26 * BUFFER_WIDTH] = 126;
		gpBuffer[addr + 14 + 24 * BUFFER_WIDTH] = 126;
		gpBuffer[addr + 14 + 25 * BUFFER_WIDTH] = 126;
		gpBuffer[addr + 14 + 26 * BUFFER_WIDTH] = 111;
	}

	// mask 6[5], 193[5], 201[5], 213[5], 217[5] + 51[5] using 180[4] and 5[6]
	for (int i = 43; i < 49; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				/*unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * (5 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (5 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 5[6]
					if (gpBuffer[addr2] != TRANS_COLOR) {
						gpBuffer[addr] = TRANS_COLOR;
					}
				} else {
					if (x > 15 || (y < x + 6)) {
						gpBuffer[addr] = TRANS_COLOR;
					}
				}*/
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * (5 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (5 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 5[6]
				} else {
					addr2 = x + MICRO_WIDTH * (17 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (17 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 180[4]
				}
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
		/*// fix shadow
		if (i != 48) {
			unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
			// gpBuffer[addr + 15 + 21 * BUFFER_WIDTH] = 125;
			gpBuffer[addr + 15 + 22 * BUFFER_WIDTH] = 111;
			gpBuffer[addr + 15 + 23 * BUFFER_WIDTH] = 111;
			gpBuffer[addr + 15 + 24 * BUFFER_WIDTH] = 126;
			gpBuffer[addr + 15 + 25 * BUFFER_WIDTH] = 126;
			gpBuffer[addr + 15 + 26 * BUFFER_WIDTH] = 63;
			gpBuffer[addr + 15 + 27 * BUFFER_WIDTH] = 63;
			gpBuffer[addr + 15 + 28 * BUFFER_WIDTH] = 125;
		}*/
	}

	{ // 180[0] - fix glitch(?) + sync
		int i = 15;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 14 +  0 * BUFFER_WIDTH] = 107;
		gpBuffer[addr + 14 +  1 * BUFFER_WIDTH] = 99;
	}
	{ // 212[0] - fix glitch(?)
		int i = 24;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 14 +  4 * BUFFER_WIDTH] = 100;
		gpBuffer[addr + 13 +  5 * BUFFER_WIDTH] = 116;
		gpBuffer[addr + 14 +  5 * BUFFER_WIDTH] = 119;
		gpBuffer[addr + 12 +  6 * BUFFER_WIDTH] = 98;
		gpBuffer[addr + 13 +  6 * BUFFER_WIDTH] = 118;
		gpBuffer[addr + 12 +  7 * BUFFER_WIDTH] = 116;
		gpBuffer[addr + 11 +  8 * BUFFER_WIDTH] = 115;
	}
	{ // 216[0] - fix glitch(?)
		int i = 27;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 14 +  5 * BUFFER_WIDTH] = 99;
	}
	{ // 213[1] - fix glitch
		int i = 39;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 13 + 19 * BUFFER_WIDTH] = 112;
		gpBuffer[addr + 12 + 20 * BUFFER_WIDTH] = 114;
		gpBuffer[addr + 11 + 22 * BUFFER_WIDTH] = 97;
		gpBuffer[addr + 10 + 23 * BUFFER_WIDTH] = 97;
		gpBuffer[addr +  9 + 24 * BUFFER_WIDTH] = 97;
		gpBuffer[addr +  8 + 25 * BUFFER_WIDTH] = 96;
		gpBuffer[addr +  7 + 26 * BUFFER_WIDTH] = 116;
	}

	// create the new CEL file
	constexpr int newEntries = lengthof(micros);
	size_t maxCelSize = *celLen + newEntries * MICRO_WIDTH * MICRO_HEIGHT;
	BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
	memset(resCelBuf, 0, maxCelSize);

	CelFrameEntry entries[newEntries];
	xx = 0, yy = MICRO_HEIGHT - 1;
	int idx = 0;
	for (int i = 0; i < newEntries; i++) {
		const CelMicro &micro = micros[i];
		if (micro.res_encoding >= 0) {
			entries[idx].encoding = micro.res_encoding;
			unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
			entries[idx].frameRef = SwapLE16(pSubtiles[index]) & 0xFFF;
			entries[idx].frameSrc = &gpBuffer[xx + yy * BUFFER_WIDTH];
			idx++;
		}
		yy += MICRO_HEIGHT;
		if (yy == (DRAW_HEIGHT + 1) * MICRO_HEIGHT - 1) {
			yy = MICRO_HEIGHT - 1;
			xx += MICRO_WIDTH;
		}
	}

	*celLen = encodeCelMicros(entries, idx, resCelBuf, celBuf, TRANS_COLOR);

	mem_free_dbg(celBuf);

	return resCelBuf;
};

BYTE* DRLP_L4_PatchCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	celBuf = patchHellChaosCel(minBuf, minLen, celBuf, celLen);
	if (celBuf == NULL) {
		return NULL;
	}
	celBuf = patchHellFloorCel(minBuf, minLen, celBuf, celLen);
	if (celBuf == NULL) {
		return NULL;
	}
	celBuf = patchHellStairsCel(minBuf, minLen, celBuf, celLen);
	if (celBuf == NULL) {
		return NULL;
	}
	celBuf = patchHellWall1Cel(minBuf, minLen, celBuf, celLen);
	if (celBuf == NULL) {
		return NULL;
	}
	celBuf = patchHellWall2Cel(minBuf, minLen, celBuf, celLen);
	return celBuf;
}

void DRLP_L4_PatchMin(BYTE* buf)
{
	uint16_t* pSubtiles = (uint16_t*)buf;
	constexpr int blockSize = BLOCK_SIZE_L4;

	// adjust frame types
	// - after patchHellFloorCel
	SetFrameType(171, 1, MET_RTRIANGLE);
	SetFrameType(172, 1, MET_RTRIANGLE);
	SetFrameType(187, 1, MET_RTRIANGLE);
	SetFrameType(277, 0, MET_LTRIANGLE);
	// - after patchHellWall1Cel
	SetFrameType(11, 1, MET_TRANSPARENT);
	// - after patchHellWall2Cel
	SetFrameType(15, 0, MET_TRANSPARENT);
	SetFrameType(257, 0, MET_TRANSPARENT);
	SetFrameType(38, 0, MET_TRANSPARENT);

	// patch stairs II.
	// - after patchHellStairsCel
	if (pSubtiles[MICRO_IDX(137 - 1, blockSize, 1)] != 0) {
		MoveMcr(140, 3, 137, 1);
		MoveMcr(140, 2, 137, 0);
		MoveMcr(140, 4, 137, 2);
		SetFrameType(140, 3, MET_TRANSPARENT);
		SetFrameType(140, 2, MET_SQUARE);
		// SetFrameType(140, 4, MET_TRANSPARENT);

		Blk2Mcr(139, 1);
		Blk2Mcr(136, 1);

		SetFrameType(140, 0, MET_LTRAPEZOID);
		SetFrameType(140, 1, MET_TRANSPARENT);

		Blk2Mcr(98, 0);
		SetFrameType(86, 4, MET_SQUARE);
		SetFrameType(425, 4, MET_SQUARE);
		SetFrameType(86, 2, MET_SQUARE);
		SetFrameType(425, 2, MET_SQUARE);
		SetFrameType(95, 1, MET_RTRAPEZOID);

		SetFrameType(91, 0, MET_TRANSPARENT);
		SetFrameType(430, 0, MET_TRANSPARENT);
		SetFrameType(91, 1, MET_TRANSPARENT);
		SetFrameType(110, 0, MET_TRANSPARENT);
		SetFrameType(110, 1, MET_TRANSPARENT);
		SetFrameType(112, 0, MET_TRANSPARENT);
		SetFrameType(451, 0, MET_TRANSPARENT);
		SetFrameType(112, 1, MET_TRANSPARENT);
		SetFrameType(113, 0, MET_TRANSPARENT);
		SetFrameType(113, 1, MET_TRANSPARENT);

		SetFrameType(130, 0, MET_TRANSPARENT);
		SetFrameType(132, 1, MET_TRANSPARENT);
		SetFrameType(133, 0, MET_TRANSPARENT);

		ReplaceMcr(111, 0, 131, 0);

		SetFrameType(141, 1, MET_RTRIANGLE);
		MoveMcr(148, 0, 141, 3);
		MoveMcr(148, 2, 141, 5);
		ReplaceMcr(148, 1, 142, 3);
		HideMcr(142, 3);
		MoveMcr(148, 3, 142, 5);
		MoveMcr(148, 5, 142, 7);
	}

	// useless pixels
	Blk2Mcr(111, 1);
	Blk2Mcr(132, 0);
	Blk2Mcr(172, 3);

	// create new shadow
	SetMcr(35, 0, 238, 0);
	SetMcr(35, 1, 238, 1);
	SetMcr(35, 4, 96, 6);
	SetMcr(35, 6, 7, 6);
	HideMcr(35, 5);
	HideMcr(35, 7);
	ReplaceMcr(26, 0, 238, 0);
	ReplaceMcr(26, 1, 238, 1);
	SetMcr(26, 4, 96, 6);
	SetMcr(26, 6, 16, 6);
	ReplaceMcr(37, 0, 238, 0);
	SetMcr(37, 1, 238, 1);
	HideMcr(37, 5);
	HideMcr(37, 7);
	SetMcr(37, 8, 179, 8);
	SetMcr(37, 10, 179, 10);
	ReplaceMcr(13, 0, 238, 0);
	ReplaceMcr(13, 1, 238, 1);

	// fix chaos
	// - mask 0
	SetFrameType(54, 5, MET_RTRIANGLE);
	SetFrameType(55, 4, MET_LTRIANGLE);
	SetFrameType(53, 2, MET_LTRAPEZOID);
	SetFrameType(53, 3, MET_RTRAPEZOID);

	SetFrameType(392, 2, MET_LTRAPEZOID);
	SetFrameType(392, 3, MET_RTRAPEZOID);
	SetFrameType(393, 5, MET_RTRIANGLE);
	SetFrameType(455, 9, MET_RTRIANGLE);
	SetFrameType(394, 4, MET_LTRIANGLE);
	SetFrameType(454, 8, MET_LTRIANGLE);

	// - reuse
	// ReplaceMcr(48, 2, 55, 2);
	ReplaceMcr(46, 2, 53, 2);
	ReplaceMcr(46, 3, 53, 3);
	ReplaceMcr(50, 2, 53, 2);
	ReplaceMcr(50, 3, 53, 3);
	ReplaceMcr(50, 6, 5, 6);
	// ReplaceMcr(51, 3, 54, 3);

	// ReplaceMcr(58, 2, 55, 2);
	ReplaceMcr(56, 2, 53, 2);
	ReplaceMcr(56, 3, 53, 3);
	// ReplaceMcr(57, 3, 54, 3);
	ReplaceMcr(57, 5, 51, 5);

	// ReplaceMcr(61, 2, 55, 2);
	ReplaceMcr(59, 2, 53, 2);
	ReplaceMcr(59, 3, 53, 3);
	// ReplaceMcr(60, 3, 54, 3);
	ReplaceMcr(60, 5, 51, 5);

	// ReplaceMcr(64, 2, 55, 2);
	ReplaceMcr(64, 4, 58, 4);
	ReplaceMcr(62, 2, 53, 2);
	ReplaceMcr(62, 3, 53, 3);
	ReplaceMcr(62, 5, 56, 5); // lost details
	// ReplaceMcr(63, 3, 54, 3);
	// ReplaceMcr(63, 5, 51, 5);

	ReplaceMcr(65, 2, 53, 2);
	ReplaceMcr(65, 3, 53, 3);

	// ReplaceMcr(70, 2, 55, 2);
	ReplaceMcr(70, 4, 48, 4);
	ReplaceMcr(68, 2, 53, 2);
	ReplaceMcr(68, 3, 53, 3);
	ReplaceMcr(68, 5, 65, 5); // lost details
	ReplaceMcr(68, 7, 11, 7); // lost details
	ReplaceMcr(68, 9, 11, 9);
	// ReplaceMcr(69, 5, 66, 5); // lost details
	// ReplaceMcr(69, 7, 66, 7); // lost details

	// ReplaceMcr(73, 4, 58, 4);
	// ReplaceMcr(73, 6, 64, 6);
	ReplaceMcr(71, 2, 53, 2);
	ReplaceMcr(71, 3, 53, 3);
	ReplaceMcr(71, 5, 59, 5);
	ReplaceMcr(71, 7, 1, 7);
	ReplaceMcr(71, 4, 65, 4);
	ReplaceMcr(71, 6, 65, 6);
	// ReplaceMcr(72, 3, 54, 3);
	// ReplaceMcr(72, 5, 51, 5);

	ReplaceMcr(76, 4, 67, 4);
	ReplaceMcr(74, 2, 53, 2);
	ReplaceMcr(74, 3, 53, 3);
	ReplaceMcr(74, 5, 59, 5);
	ReplaceMcr(74, 7, 1, 7);

	ReplaceMcr(77, 2, 53, 2);
	ReplaceMcr(77, 3, 53, 3);
	ReplaceMcr(77, 4, 62, 4);
	ReplaceMcr(77, 6, 5, 6);
	ReplaceMcr(78, 5, 66, 5); // lost details
	ReplaceMcr(78, 7, 9, 7);

	ReplaceMcr(79, 2, 53, 2);
	ReplaceMcr(79, 3, 53, 3);
	ReplaceMcr(79, 4, 62, 4);
	ReplaceMcr(79, 5, 59, 5);
	// - mask 1
	Blk2Mcr(53, 0);
	Blk2Mcr(53, 1);
	Blk2Mcr(53, 6);
	Blk2Mcr(53, 7);
	Blk2Mcr(54, 3);
	Blk2Mcr(54, 7);
	Blk2Mcr(55, 2);
	Blk2Mcr(55, 6);

	Blk2Mcr(48, 2);
	Blk2Mcr(58, 2);
	// Blk2Mcr(61, 2);
	Blk2Mcr(64, 2);
	Blk2Mcr(70, 2);
	Blk2Mcr(52, 2); Blk2Mcr(67, 2); Blk2Mcr(73, 2); Blk2Mcr(76, 2); Blk2Mcr(391, 2); Blk2Mcr(394, 2); Blk2Mcr(406, 2); /*Blk2Mcr(412, 2);*/ Blk2Mcr(415, 2); Blk2Mcr(454, 2);
	Blk2Mcr(51, 3);
	Blk2Mcr(57, 3);
	Blk2Mcr(60, 3);
	// Blk2Mcr(63, 3);
	Blk2Mcr(46, 0); Blk2Mcr(50, 0); Blk2Mcr(56, 0); Blk2Mcr(59, 0); Blk2Mcr(62, 0); Blk2Mcr(65, 0); Blk2Mcr(68, 0); Blk2Mcr(71, 0); Blk2Mcr(74, 0); Blk2Mcr(77, 0); Blk2Mcr(79, 0); // Blk2Mcr(456, 6);
	Blk2Mcr(46, 1); Blk2Mcr(50, 1); Blk2Mcr(56, 1); Blk2Mcr(59, 1); Blk2Mcr(62, 1); Blk2Mcr(65, 1); Blk2Mcr(68, 1); Blk2Mcr(71, 1); Blk2Mcr(74, 1); Blk2Mcr(77, 1); Blk2Mcr(79, 1); // Blk2Mcr(456, 7);

	Blk2Mcr(392, 6); // Blk2Mcr(453, 10);
	Blk2Mcr(392, 7); // Blk2Mcr(453, 11);

	Blk2Mcr(393, 7); Blk2Mcr(455, 11);
	Blk2Mcr(394, 6); Blk2Mcr(454, 6); Blk2Mcr(454, 10);

	Blk2Mcr(47, 3); Blk2Mcr(66, 3); /*Blk2Mcr(69, 3); */ Blk2Mcr(78, 3); Blk2Mcr(386, 3); Blk2Mcr(393, 3); Blk2Mcr(405, 3); /*Blk2Mcr(408, 3); Blk2Mcr(417, 3);*/ Blk2Mcr(455, 7);

	// reuse micros
	ReplaceMcr(109, 0, 158, 0);
	ReplaceMcr(99, 1, 157, 1);
	ReplaceMcr(86, 1, 156, 1);
	ReplaceMcr(82, 0, 141, 0);
	ReplaceMcr(289, 0, 141, 0);
	ReplaceMcr(124, 1, 154, 1);
	ReplaceMcr(202, 0, 161, 0);
	ReplaceMcr(307, 0, 161, 0); // lost details
	ReplaceMcr(311, 0, 154, 0);
	ReplaceMcr(205, 0, 160, 0); // lost details
	ReplaceMcr(205, 1, 160, 1); // lost details
	ReplaceMcr(189, 0, 157, 0); // lost details
	ReplaceMcr(189, 1, 157, 1); // lost details
	ReplaceMcr(209, 0, 157, 0); // lost details
	ReplaceMcr(209, 1, 157, 1); // lost details
	ReplaceMcr(197, 0, 153, 0); // lost details
	ReplaceMcr(197, 1, 153, 1); // lost details
	ReplaceMcr(241, 1, 153, 1); // lost details
	ReplaceMcr(302, 1, 153, 1); // lost details
	ReplaceMcr(164, 0, 2, 0); // lost details
	ReplaceMcr(164, 1, 2, 1); // lost details
	ReplaceMcr(225, 1, 2, 1);
	// ReplaceMcr(43, 0, 22, 0); // lost details
	ReplaceMcr(169, 0, 19, 0); // lost details
	ReplaceMcr(255, 0, 258, 0); // lost details
	ReplaceMcr(299, 0, 19, 0);
	ReplaceMcr(272, 1, 155, 1);
	ReplaceMcr(288, 1, 155, 1);
	ReplaceMcr(265, 0, 156, 0);
	// ReplaceMcr(281, 1, 159, 1);
	ReplaceMcr(280, 0, 4, 0);
	ReplaceMcr(129, 0, 4, 0);
	ReplaceMcr(340, 1, 4, 1);
	ReplaceMcr(373, 1, 4, 1);
	ReplaceMcr(337, 1, 17, 1);
	ReplaceMcr(371, 1, 17, 1);
	ReplaceMcr(145, 0, 156, 0);
	ReplaceMcr(329, 0, 156, 0);
	ReplaceMcr(364, 0, 156, 0);
	ReplaceMcr(332, 0, 158, 0);
	ReplaceMcr(366, 0, 158, 0);

	ReplaceMcr(194, 0, 147, 0);
	ReplaceMcr(194, 1, 147, 1);
	ReplaceMcr(214, 0, 154, 0);
	ReplaceMcr(214, 1, 154, 1);
	ReplaceMcr(242, 0, 154, 0);
	ReplaceMcr(242, 1, 154, 1);

	ReplaceMcr(232, 0, 17, 0);
	ReplaceMcr(235, 0, 4, 0);
	ReplaceMcr(239, 1, 158, 1);

	ReplaceMcr(10, 0, 19, 0); // lost details
	ReplaceMcr(10, 1, 19, 1); // lost details
	ReplaceMcr(16, 0, 19, 0); // lost details
	ReplaceMcr(16, 1, 19, 1); // lost details
	ReplaceMcr(253, 0, 19, 0); // lost details
	ReplaceMcr(253, 1, 19, 1); // lost details

	ReplaceMcr(7, 0, 19, 0); // lost details
	ReplaceMcr(7, 1, 19, 1); // lost details
	// ReplaceMcr(26, 0, 19, 0); // lost details
	// ReplaceMcr(26, 1, 19, 1); // lost details
	// ReplaceMcr(40, 0, 19, 0); // lost details
	// ReplaceMcr(40, 1, 19, 1); // lost details
	ReplaceMcr(179, 0, 19, 0); // lost details
	ReplaceMcr(179, 1, 19, 1); // lost details
	ReplaceMcr(218, 0, 19, 0); // lost details
	ReplaceMcr(218, 1, 19, 1); // lost details
	ReplaceMcr(230, 0, 19, 0); // lost details
	ReplaceMcr(230, 1, 19, 1); // lost details

	ReplaceMcr(119, 0, 2, 0);
	ReplaceMcr(119, 1, 2, 1);
	// ReplaceMcr(168, 0, 2, 0); // lost details
	// ReplaceMcr(168, 1, 2, 1); // lost details
	// ReplaceMcr(298, 0, 2, 0); // lost details
	// ReplaceMcr(298, 1, 2, 1); // lost details
	// ReplaceMcr(338, 0, 2, 0); // lost details
	// ReplaceMcr(338, 1, 2, 1); // lost details

	// ReplaceMcr(37, 0, 9, 0); // lost details
	ReplaceMcr(181, 0, 9, 0); // lost details
	ReplaceMcr(229, 0, 9, 0); // lost details
	ReplaceMcr(229, 1, 2, 1); // lost details

	ReplaceMcr(258, 0, 255, 0); // lost details

	ReplaceMcr(34, 1, 11, 1);
	ReplaceMcr(34, 3, 11, 3);
	ReplaceMcr(34, 5, 11, 5);
	ReplaceMcr(34, 7, 11, 7);
	ReplaceMcr(36, 1, 11, 1);
	ReplaceMcr(36, 3, 11, 3);
	ReplaceMcr(36, 5, 11, 5);
	ReplaceMcr(36, 7, 11, 7);
	ReplaceMcr(65, 7, 11, 7);
	// ReplaceMcr(68, 9, 11, 9);
	ReplaceMcr(251, 1, 11, 1);
	ReplaceMcr(254, 1, 11, 1);
	ReplaceMcr(254, 3, 11, 3);

	ReplaceMcr(38, 7, 1, 7);

	ReplaceMcr(23, 0, 3, 0);
	ReplaceMcr(33, 0, 3, 0);
	ReplaceMcr(190, 0, 3, 0);
	ReplaceMcr(247, 0, 3, 0);
	ReplaceMcr(23, 1, 3, 1);
	ReplaceMcr(33, 1, 3, 1);
	ReplaceMcr(3, 4, 23, 4);

	ReplaceMcr(190, 8, 3, 8); // lost details
	ReplaceMcr(48, 6, 3, 6);

	ReplaceMcr(198, 2, 3, 2); // lost details
	ReplaceMcr(210, 2, 3, 2); // lost details
	ReplaceMcr(253, 4, 242, 4); // lost details

	ReplaceMcr(193, 9, 6, 9);

	ReplaceMcr(47, 7, 2, 7);
	ReplaceMcr(189, 7, 2, 7);
	ReplaceMcr(201, 7, 6, 7);
	ReplaceMcr(258, 7, 9, 7);
	ReplaceMcr(66, 7, 12, 7);
	ReplaceMcr(252, 7, 12, 7);
	ReplaceMcr(255, 7, 12, 7);

	ReplaceMcr(9, 5, 2, 5);
	ReplaceMcr(189, 5, 2, 5);
	ReplaceMcr(258, 5, 2, 5);
	ReplaceMcr(255, 5, 12, 5);

	ReplaceMcr(241, 5, 252, 5);

	ReplaceMcr(201, 3, 6, 3);
	ReplaceMcr(213, 3, 6, 3);
	ReplaceMcr(193, 3, 217, 3);

	ReplaceMcr(25, 1, 6, 1);
	ReplaceMcr(31, 1, 6, 1);
	ReplaceMcr(193, 1, 6, 1);
	ReplaceMcr(201, 1, 6, 1);
	// ReplaceMcr(245, 1, 6, 1);

	ReplaceMcr(217, 1, 213, 1);

	ReplaceMcr(25, 0, 6, 0);
	ReplaceMcr(31, 0, 6, 0);
	ReplaceMcr(217, 0, 6, 0);
	// ReplaceMcr(245, 0, 6, 0); // lost details

	ReplaceMcr(21, 8, 260, 8);
	ReplaceMcr(27, 8, 260, 8);
	ReplaceMcr(30, 8, 260, 8);
	ReplaceMcr(56, 8, 260, 8);
	ReplaceMcr(59, 8, 260, 8);
	ReplaceMcr(248, 8, 244, 8);

	ReplaceMcr(192, 6, 5, 6); // fix glitch (connection)
	ReplaceMcr(216, 6, 5, 6);
	ReplaceMcr(38, 6, 15, 6);
	ReplaceMcr(65, 6, 15, 6);
	ReplaceMcr(71, 6, 15, 6);
	ReplaceMcr(74, 6, 8, 6);
	ReplaceMcr(200, 6, 180, 6);
	ReplaceMcr(67, 6, 16, 6);

	ReplaceMcr(248, 4, 21, 4);
	ReplaceMcr(36, 4, 32, 4);
	ReplaceMcr(34, 4, 15, 4);
	ReplaceMcr(38, 4, 15, 4);

	ReplaceMcr(36, 2, 21, 2);
	ReplaceMcr(204, 2, 188, 2);
	ReplaceMcr(192, 2, 5, 2);
	ReplaceMcr(212, 2, 5, 2);
	ReplaceMcr(216, 2, 5, 2);
	ReplaceMcr(34, 2, 15, 2);
	ReplaceMcr(38, 2, 15, 2);
	ReplaceMcr(163, 2, 1, 2); // lost details
	ReplaceMcr(196, 2, 1, 2);
	ReplaceMcr(208, 2, 1, 2);

	ReplaceMcr(11, 0, 8, 0);
	ReplaceMcr(244, 0, 8, 0);
	ReplaceMcr(254, 0, 8, 0);
	ReplaceMcr(27, 0, 21, 0);
	ReplaceMcr(30, 0, 21, 0);
	ReplaceMcr(32, 0, 21, 0);
	ReplaceMcr(45, 0, 21, 0);
	ReplaceMcr(246, 0, 21, 0);
	ReplaceMcr(248, 0, 21, 0);
	ReplaceMcr(36, 0, 21, 0); // fix glitch(?)
	ReplaceMcr(34, 0, 15, 0);
	ReplaceMcr(251, 0, 15, 0);
	ReplaceMcr(41, 0, 24, 0);

	ReplaceMcr(24, 9, 260, 9);
	ReplaceMcr(27, 9, 260, 9);
	ReplaceMcr(32, 9, 260, 9);
	ReplaceMcr(56, 9, 260, 9);
	ReplaceMcr(62, 9, 260, 9);
	ReplaceMcr(257, 9, 260, 9);
	ReplaceMcr(248, 9, 246, 9);

	ReplaceMcr(21, 7, 8, 7);
	ReplaceMcr(42, 7, 8, 7);
	ReplaceMcr(77, 7, 8, 7);
	ReplaceMcr(46, 7, 1, 7);
	ReplaceMcr(167, 7, 1, 7);
	ReplaceMcr(188, 7, 1, 7); // fix glitch (connection)

	ReplaceMcr(248, 5, 24, 5);

	ReplaceMcr(24, 3, 27, 3);
	ReplaceMcr(188, 3, 1, 3);
	ReplaceMcr(196, 3, 1, 3);
	ReplaceMcr(204, 3, 1, 3);
	ReplaceMcr(212, 3, 5, 3);
	ReplaceMcr(216, 3, 192, 3);
	ReplaceMcr(8, 3, 21, 3);
	ReplaceMcr(15, 3, 21, 3);
	ReplaceMcr(257, 3, 21, 3);
	ReplaceMcr(260, 3, 21, 3);

	ReplaceMcr(188, 1, 1, 1);
	ReplaceMcr(196, 1, 1, 1);
	ReplaceMcr(24, 1, 27, 1);
	ReplaceMcr(38, 1, 27, 1);
	ReplaceMcr(41, 1, 27, 1);
	ReplaceMcr(244, 1, 27, 1);
	ReplaceMcr(15, 1, 8, 1);
	ReplaceMcr(21, 1, 8, 1);
	ReplaceMcr(42, 1, 8, 1);
	ReplaceMcr(246, 1, 8, 1);
	ReplaceMcr(257, 1, 8, 1);
	ReplaceMcr(260, 1, 8, 1);

	// ReplaceMcr(194, 4, 7, 4); // lost details
	ReplaceMcr(7, 4, 96, 6); // lost details
	ReplaceMcr(194, 4, 96, 6); // lost details
	ReplaceMcr(16, 4, 96, 6); // lost details
	// ReplaceMcr(40, 4, 96, 6); // lost details
	ReplaceMcr(194, 6, 7, 6); // lost details

	// ReplaceMcr(175, 0, 147, 0); // after patchHellFloorCel

	// eliminate micros of unused subtiles
	// Blk2Mcr(240,  ...),
	// moved to other subtile
	// Blk2Mcr(137, 0);
	// Blk2Mcr(137, 1);
	// Blk2Mcr(137, 2);
	// reused for the new shadow
	// 35
	// Blk2Mcr(26, 0);
	// Blk2Mcr(26, 1);
	// Blk2Mcr(37, 0);
	// Blk2Mcr(13, 0);
	// Blk2Mcr(13, 1);
	Blk2Mcr(385, 0);
	Blk2Mcr(385, 1);
	Blk2Mcr(385, 2);
	Blk2Mcr(385, 3);
	Blk2Mcr(385, 7);
	Blk2Mcr(386, 0);
	Blk2Mcr(386, 1);
	Blk2Mcr(386, 7);
	Blk2Mcr(387, 0);
	Blk2Mcr(387, 1);
	Blk2Mcr(387, 2);
	Blk2Mcr(387, 6);
	Blk2Mcr(389, 0);
	Blk2Mcr(389, 1);
	Blk2Mcr(389, 2);
	Blk2Mcr(389, 3);
	Blk2Mcr(389, 6);
	Blk2Mcr(390, 0);
	Blk2Mcr(390, 1);
	Blk2Mcr(390, 3);
	Blk2Mcr(391, 0);
	Blk2Mcr(391, 1);
	Blk2Mcr(392, 0);
	Blk2Mcr(392, 1);
	Blk2Mcr(393, 0);
	Blk2Mcr(393, 1);
	Blk2Mcr(394, 0);
	Blk2Mcr(394, 1);
	Blk2Mcr(395, 0);
	Blk2Mcr(395, 1);
	Blk2Mcr(395, 2);
	Blk2Mcr(395, 3);
	Blk2Mcr(395, 8);
	Blk2Mcr(395, 9);
	Blk2Mcr(396, 0);
	Blk2Mcr(396, 1);
	Blk2Mcr(396, 3);
	Blk2Mcr(396, 5);
	Blk2Mcr(397, 0);
	Blk2Mcr(397, 1);
	Blk2Mcr(397, 2);
	Blk2Mcr(398, 0);
	Blk2Mcr(398, 1);
	Blk2Mcr(398, 2);
	Blk2Mcr(398, 3);
	Blk2Mcr(398, 8);
	Blk2Mcr(399, 0);
	Blk2Mcr(399, 1);
	Blk2Mcr(399, 3);
	Blk2Mcr(399, 5);
	Blk2Mcr(400, 0);
	Blk2Mcr(400, 1);
	Blk2Mcr(400, 2);
	Blk2Mcr(400, 4);
	Blk2Mcr(401, 0);
	Blk2Mcr(401, 1);
	Blk2Mcr(401, 2);
	Blk2Mcr(401, 3);
	Blk2Mcr(401, 5);
	Blk2Mcr(401, 9);
	Blk2Mcr(402, 0);
	Blk2Mcr(402, 1);
	Blk2Mcr(402, 3);
	Blk2Mcr(402, 5);
	Blk2Mcr(403, 0);
	Blk2Mcr(403, 1);
	Blk2Mcr(403, 2);
	Blk2Mcr(403, 4);
	Blk2Mcr(404, 0);
	Blk2Mcr(404, 1);
	Blk2Mcr(404, 2);
	Blk2Mcr(404, 3);
	Blk2Mcr(404, 6);
	Blk2Mcr(404, 7);
	Blk2Mcr(405, 0);
	Blk2Mcr(405, 1);
	Blk2Mcr(405, 7);
	Blk2Mcr(406, 0);
	Blk2Mcr(406, 1);
	Blk2Mcr(406, 6);
	Blk2Mcr(407, 0);
	Blk2Mcr(407, 1);
	Blk2Mcr(407, 2);
	Blk2Mcr(407, 3);
	Blk2Mcr(407, 5);
	Blk2Mcr(407, 7);
	Blk2Mcr(409, 0);
	Blk2Mcr(409, 1);
	Blk2Mcr(409, 2);
	Blk2Mcr(409, 4);
	Blk2Mcr(410, 0);
	Blk2Mcr(410, 1);
	Blk2Mcr(410, 2);
	Blk2Mcr(410, 3);
	Blk2Mcr(410, 4);
	Blk2Mcr(410, 5);
	Blk2Mcr(410, 6);
	Blk2Mcr(410, 7);
	Blk2Mcr(411, 0);
	Blk2Mcr(411, 1);
	Blk2Mcr(411, 3);
	Blk2Mcr(411, 5);
	Blk2Mcr(413, 0);
	Blk2Mcr(413, 1);
	Blk2Mcr(413, 2);
	Blk2Mcr(413, 3);
	Blk2Mcr(413, 5);
	Blk2Mcr(413, 6);
	Blk2Mcr(413, 7);
	Blk2Mcr(415, 0);
	Blk2Mcr(415, 1);
	Blk2Mcr(415, 4);
	Blk2Mcr(416, 0);
	Blk2Mcr(416, 1);
	Blk2Mcr(416, 2);
	Blk2Mcr(416, 3);
	Blk2Mcr(416, 4);
	Blk2Mcr(416, 6);
	Blk2Mcr(416, 7);
	Blk2Mcr(417, 0);
	Blk2Mcr(417, 1);
	Blk2Mcr(417, 3);
	Blk2Mcr(417, 5);
	Blk2Mcr(417, 7);
	Blk2Mcr(418, 0);
	Blk2Mcr(418, 1);
	Blk2Mcr(418, 2);
	Blk2Mcr(418, 3);
	Blk2Mcr(418, 4);
	Blk2Mcr(418, 5);
	Blk2Mcr(419, 0);
	Blk2Mcr(419, 1);
	Blk2Mcr(419, 3);
	Blk2Mcr(419, 5);
	Blk2Mcr(420, 0);
	Blk2Mcr(420, 1);
	Blk2Mcr(420, 2);
	Blk2Mcr(420, 4);
	Blk2Mcr(422, 0);
	Blk2Mcr(425, 1);
	Blk2Mcr(427, 0);
	Blk2Mcr(429, 0);
	Blk2Mcr(430, 1);
	Blk2Mcr(431, 0);
	Blk2Mcr(432, 0);
	Blk2Mcr(433, 1);
	Blk2Mcr(434, 1);
	Blk2Mcr(434, 5);
	Blk2Mcr(435, 0);
	Blk2Mcr(436, 0);
	Blk2Mcr(437, 0);
	Blk2Mcr(437, 3);
	Blk2Mcr(437, 4);
	Blk2Mcr(438, 0);
	Blk2Mcr(438, 1);
	Blk2Mcr(439, 0);
	Blk2Mcr(439, 4);
	Blk2Mcr(439, 6);
	Blk2Mcr(445, 0);
	Blk2Mcr(451, 1);
	Blk2Mcr(454, 9);
	Blk2Mcr(454, 11);
	Blk2Mcr(455, 8);
	Blk2Mcr(455, 10);

	Blk2Mcr(22, 0);
	Blk2Mcr(22, 5);
	Blk2Mcr(28, 0);
	Blk2Mcr(28, 1);
	Blk2Mcr(29, 0);
	Blk2Mcr(29, 1);
	Blk2Mcr(39, 0);
	Blk2Mcr(39, 1);
	Blk2Mcr(40, 0);
	Blk2Mcr(40, 1);
	Blk2Mcr(40, 4);
	Blk2Mcr(43, 0);
	Blk2Mcr(43, 5);
	Blk2Mcr(44, 0);
	Blk2Mcr(44, 1);
	Blk2Mcr(61, 2);
	Blk2Mcr(61, 4);
	Blk2Mcr(63, 3);
	Blk2Mcr(63, 5);
	Blk2Mcr(72, 3);
	Blk2Mcr(72, 5);
	Blk2Mcr(80, 3);
	Blk2Mcr(80, 5);
	Blk2Mcr(81, 2);
	Blk2Mcr(81, 4);
	// Blk2Mcr(148, 1); - reused to fix graphical glitch
	Blk2Mcr(173, 1);
	Blk2Mcr(178, 1);
	Blk2Mcr(178, 3);
	Blk2Mcr(178, 5);
	Blk2Mcr(240, 0);
	Blk2Mcr(240, 1);
	Blk2Mcr(245, 0);
	Blk2Mcr(245, 1);
	Blk2Mcr(249, 0);
	Blk2Mcr(249, 1);
	Blk2Mcr(249, 7);
	Blk2Mcr(250, 0);
	Blk2Mcr(250, 1);
	Blk2Mcr(250, 6);
	Blk2Mcr(259, 1);

	const int unusedSubtiles[] = {
		14, 20, 69, 73, 75, 84, 85, 87, 89, 102, 103, 104, 105, 108, 115, 116, 117, 121, 122, 123, 125, 128, 138, 143, 144, 165, 166, 168, 170, 174, 175, 182, 191, 195, 199, 203, 207, 211, 215, 219, 222, 223, 224, 231, 234, 236, 237, 243, 256, 261, 267, 270, 276, 279, 282, 286, 291, 298, 303, 304, 308, 310, 312, 314, 316, 317, 331, 338, 352, 388, 408, 412, 414, 421, 423, 424, 426, 428, 440, 441, 442, 443, 444, 446, 447, 448, 449, 450, 452, 453, 456
	};

	for (int n = 0; n < lengthof(unusedSubtiles); n++) {
		for (int i = 0; i < blockSize; i++) {
			Blk2Mcr(unusedSubtiles[n], i);
		}
	}
}

void DRLP_L4_PatchTil(BYTE* buf)
{
	uint16_t* pTiles = (uint16_t*)buf;

	// TODO: tile 66, 57 vs shadow?
	//       fix inconsistent shadow on 24, 27, 38, 41, 244[2 3], 254[1] (+180, 200 vs. default shadow?) in patchHellWall1Cel?

	// patch stairs III.
	pTiles[(45 - 1) * 4 + 0] = SwapLE16(17 - 1);  // 137
	pTiles[(45 - 1) * 4 + 1] = SwapLE16(18 - 1);  // 138

	// fix shadow
	pTiles[(36 - 1) * 4 + 3] = SwapLE16(155 - 1); // 105
	pTiles[(72 - 1) * 4 + 0] = SwapLE16(17 - 1);  // 224
	pTiles[(55 - 1) * 4 + 2] = SwapLE16(154 - 1); // 175
	// fix graphical glitch (on explosion)
	pTiles[(44 - 1) * 4 + 2] = SwapLE16(148 - 1); // (136)

	// create the new shadows
	pTiles[(61 - 1) * 4 + 0] = SwapLE16(5 - 1); // copy from tile 2
	pTiles[(61 - 1) * 4 + 1] = SwapLE16(6 - 1);
	pTiles[(61 - 1) * 4 + 2] = SwapLE16(35 - 1);
	pTiles[(61 - 1) * 4 + 3] = SwapLE16(239 - 1);
	pTiles[(62 - 1) * 4 + 0] = SwapLE16(5 - 1); // copy from tile 2
	pTiles[(62 - 1) * 4 + 1] = SwapLE16(6 - 1);
	pTiles[(62 - 1) * 4 + 2] = SwapLE16(7 - 1);
	pTiles[(62 - 1) * 4 + 3] = SwapLE16(176 - 1);
	pTiles[(76 - 1) * 4 + 0] = SwapLE16(41 - 1); // copy from tile 15
	pTiles[(76 - 1) * 4 + 1] = SwapLE16(31 - 1);
	pTiles[(76 - 1) * 4 + 2] = SwapLE16(13 - 1);
	pTiles[(76 - 1) * 4 + 3] = SwapLE16(239 - 1);
	pTiles[(129 - 1) * 4 + 0] = SwapLE16(41 - 1); // copy from tile 15
	pTiles[(129 - 1) * 4 + 1] = SwapLE16(31 - 1);
	pTiles[(129 - 1) * 4 + 2] = SwapLE16(10 - 1);
	pTiles[(129 - 1) * 4 + 3] = SwapLE16(176 - 1);
	pTiles[(130 - 1) * 4 + 0] = SwapLE16(177 - 1); // copy from tile 56
	pTiles[(130 - 1) * 4 + 1] = SwapLE16(31 - 1);
	pTiles[(130 - 1) * 4 + 2] = SwapLE16(37 - 1);
	pTiles[(130 - 1) * 4 + 3] = SwapLE16(239 - 1);
	pTiles[(131 - 1) * 4 + 0] = SwapLE16(177 - 1); // copy from tile 56
	pTiles[(131 - 1) * 4 + 1] = SwapLE16(31 - 1);
	pTiles[(131 - 1) * 4 + 2] = SwapLE16(179 - 1);
	pTiles[(131 - 1) * 4 + 3] = SwapLE16(176 - 1);
	pTiles[(132 - 1) * 4 + 0] = SwapLE16(24 - 1); // copy from tile 8
	pTiles[(132 - 1) * 4 + 1] = SwapLE16(25 - 1);
	pTiles[(132 - 1) * 4 + 2] = SwapLE16(13 - 1);
	pTiles[(132 - 1) * 4 + 3] = SwapLE16(239 - 1);
	pTiles[(133 - 1) * 4 + 0] = SwapLE16(24 - 1); // copy from tile 8
	pTiles[(133 - 1) * 4 + 1] = SwapLE16(25 - 1);
	pTiles[(133 - 1) * 4 + 2] = SwapLE16(10 - 1);
	pTiles[(133 - 1) * 4 + 3] = SwapLE16(176 - 1);
	pTiles[(134 - 1) * 4 + 0] = SwapLE16(38 - 1); // copy from tile 14
	pTiles[(134 - 1) * 4 + 1] = SwapLE16(31 - 1);
	pTiles[(134 - 1) * 4 + 2] = SwapLE16(26 - 1);
	pTiles[(134 - 1) * 4 + 3] = SwapLE16(239 - 1);
	pTiles[(135 - 1) * 4 + 0] = SwapLE16(38 - 1); // copy from tile 14
	pTiles[(135 - 1) * 4 + 1] = SwapLE16(31 - 1);
	pTiles[(135 - 1) * 4 + 2] = SwapLE16(16 - 1);
	pTiles[(135 - 1) * 4 + 3] = SwapLE16(176 - 1);
	// separate subtiles for the automap
	// pTiles[(44 - 1) * 4 + 2] = SwapLE16(136 - 1);
	pTiles[(136 - 1) * 4 + 0] = SwapLE16(149 - 1);
	pTiles[(136 - 1) * 4 + 1] = SwapLE16(153 - 1);
	pTiles[(136 - 1) * 4 + 2] = SwapLE16(97 - 1);
	pTiles[(136 - 1) * 4 + 3] = SwapLE16(136 - 1);
	// use common subtiles
	pTiles[( 4 - 1) * 4 + 2] = SwapLE16(10 - 1);  // 13
	pTiles[(81 - 1) * 4 + 2] = SwapLE16(10 - 1);
	pTiles[( 6 - 1) * 4 + 3] = SwapLE16(4 - 1);   // 20
	pTiles[( 8 - 1) * 4 + 2] = SwapLE16(10 - 1);  // 26
	pTiles[(15 - 1) * 4 + 2] = SwapLE16(10 - 1);
	pTiles[( 7 - 1) * 4 + 1] = SwapLE16(9 - 1);   // 22
	pTiles[( 9 - 1) * 4 + 2] = SwapLE16(23 - 1);  // 29
	pTiles[(10 - 1) * 4 + 2] = SwapLE16(23 - 1);
	pTiles[(12 - 1) * 4 + 1] = SwapLE16(12 - 1);  // 35
	pTiles[(14 - 1) * 4 + 2] = SwapLE16(16 - 1);  // 40
	pTiles[(16 - 1) * 4 + 1] = SwapLE16(9 - 1);   // 43
	pTiles[(16 - 1) * 4 + 2] = SwapLE16(33 - 1);  // 44
	pTiles[(22 - 1) * 4 + 2] = SwapLE16(58 - 1);  // 61
	pTiles[(23 - 1) * 4 + 1] = SwapLE16(57 - 1);  // 63
	pTiles[(25 - 1) * 4 + 1] = SwapLE16(66 - 1);  // 69
	pTiles[(26 - 1) * 4 + 1] = SwapLE16(60 - 1);  // 72
	pTiles[(26 - 1) * 4 + 2] = SwapLE16(67 - 1);  // 73
	pTiles[(27 - 1) * 4 + 1] = SwapLE16(60 - 1);  // 75
	pTiles[(32 - 1) * 4 + 1] = SwapLE16(157 - 1); // 87
	pTiles[(32 - 1) * 4 + 3] = SwapLE16(158 - 1); // 89
	pTiles[(36 - 1) * 4 + 0] = SwapLE16(149 - 1); // 102
	pTiles[(36 - 1) * 4 + 1] = SwapLE16(153 - 1); // 103
	pTiles[(36 - 1) * 4 + 2] = SwapLE16(154 - 1); // 104
	pTiles[(37 - 1) * 4 + 2] = SwapLE16(147 - 1); // 108
	pTiles[(39 - 1) * 4 + 1] = SwapLE16(157 - 1); // 115
	pTiles[(39 - 1) * 4 + 2] = SwapLE16(147 - 1); // 116
	pTiles[(39 - 1) * 4 + 3] = SwapLE16(158 - 1); // 117
	pTiles[(40 - 1) * 4 + 3] = SwapLE16(4 - 1);   // 121
	pTiles[(41 - 1) * 4 + 0] = SwapLE16(149 - 1); // 122
	pTiles[(41 - 1) * 4 + 1] = SwapLE16(153 - 1); // 123
	pTiles[(41 - 1) * 4 + 3] = SwapLE16(155 - 1); // 125
	pTiles[(42 - 1) * 4 + 2] = SwapLE16(19 - 1);  // 128
	pTiles[(46 - 1) * 4 + 2] = SwapLE16(19 - 1);  // 143
	pTiles[(46 - 1) * 4 + 3] = SwapLE16(4 - 1);   // 144
	pTiles[(47 - 1) * 4 + 3] = SwapLE16(158 - 1); // 148
	pTiles[(54 - 1) * 4 + 2] = SwapLE16(161 - 1); // 173
	pTiles[(54 - 1) * 4 + 3] = SwapLE16(162 - 1); // 174
	pTiles[(57 - 1) * 4 + 2] = SwapLE16(19 - 1);  // 182
	pTiles[(71 - 1) * 4 + 2] = SwapLE16(19 - 1);  // 222
	pTiles[(71 - 1) * 4 + 3] = SwapLE16(4 - 1);   // 223
	pTiles[(73 - 1) * 4 + 3] = SwapLE16(4 - 1);   // 231
	pTiles[(74 - 1) * 4 + 2] = SwapLE16(19 - 1);  // 234
	pTiles[(75 - 1) * 4 + 0] = SwapLE16(156 - 1); // 236
	pTiles[(75 - 1) * 4 + 1] = SwapLE16(157 - 1); // 237
	pTiles[(77 - 1) * 4 + 1] = SwapLE16(31 - 1);  // 245
	pTiles[(79 - 1) * 4 + 2] = SwapLE16(23 - 1);  // 250
	pTiles[(84 - 1) * 4 + 0] = SwapLE16(159 - 1); // 261
	pTiles[(85 - 1) * 4 + 2] = SwapLE16(147 - 1); // 267
	pTiles[(86 - 1) * 4 + 1] = SwapLE16(153 - 1); // 270
	pTiles[(87 - 1) * 4 + 3] = SwapLE16(4 - 1);   // 276
	pTiles[(88 - 1) * 4 + 2] = SwapLE16(19 - 1);  // 279
	pTiles[(89 - 1) * 4 + 1] = SwapLE16(160 - 1); // 282
	pTiles[(90 - 1) * 4 + 1] = SwapLE16(153 - 1); // 286
	pTiles[(91 - 1) * 4 + 2] = SwapLE16(19 - 1);  // 291
	pTiles[(94 - 1) * 4 + 2] = SwapLE16(154 - 1); // 303
	pTiles[(94 - 1) * 4 + 3] = SwapLE16(155 - 1); // 304
	pTiles[(98 - 1) * 4 + 0] = SwapLE16(149 - 1); // 317
	pTiles[(107 - 1) * 4 + 0] = SwapLE16(149 - 1);
	pTiles[(101 - 1) * 4 + 2] = SwapLE16(147 - 1); // 331
	pTiles[(110 - 1) * 4 + 2] = SwapLE16(147 - 1);
	pTiles[(106 - 1) * 4 + 3] = SwapLE16(4 - 1); // 352
	pTiles[(115 - 1) * 4 + 3] = SwapLE16(4 - 1);

	// use common subtiles instead of minor alterations
	pTiles[( 9 - 1) * 4 + 1] = SwapLE16(25 - 1);  // 28
	pTiles[(11 - 1) * 4 + 1] = SwapLE16(25 - 1);
	pTiles[(13 - 1) * 4 + 1] = SwapLE16(12 - 1);  // 37
	pTiles[(14 - 1) * 4 + 1] = SwapLE16(31 - 1);  // 39
	pTiles[(15 - 1) * 4 + 1] = SwapLE16(31 - 1);
	pTiles[(29 - 1) * 4 + 1] = SwapLE16(60 - 1);  // 80
	pTiles[(29 - 1) * 4 + 2] = SwapLE16(70 - 1);  // 81
	pTiles[(31 - 1) * 4 + 2] = SwapLE16(19 - 1);  // 84
	pTiles[(31 - 1) * 4 + 3] = SwapLE16(4 - 1);   // 85
	pTiles[(52 - 1) * 4 + 2] = SwapLE16(33 - 1);  // 165
	pTiles[(52 - 1) * 4 + 3] = SwapLE16(4 - 1);   // 166
	pTiles[(53 - 1) * 4 + 1] = SwapLE16(119 - 1); // 168
	pTiles[(53 - 1) * 4 + 3] = SwapLE16(4 - 1);   // 170
	pTiles[(56 - 1) * 4 + 1] = SwapLE16(31 - 1);  // 178
	pTiles[(63 - 1) * 4 + 3] = SwapLE16(158 - 1); // 191
	pTiles[(64 - 1) * 4 + 3] = SwapLE16(158 - 1); // 195
	pTiles[(65 - 1) * 4 + 3] = SwapLE16(155 - 1); // 199
	pTiles[(66 - 1) * 4 + 3] = SwapLE16(162 - 1); // 203
	pTiles[(67 - 1) * 4 + 3] = SwapLE16(162 - 1); // 207
	pTiles[(68 - 1) * 4 + 3] = SwapLE16(158 - 1); // 211
	pTiles[(69 - 1) * 4 + 3] = SwapLE16(155 - 1); // 215
	pTiles[(70 - 1) * 4 + 3] = SwapLE16(4 - 1);   // 219
	// pTiles[(76 - 1) * 4 + 3] = SwapLE16(155 - 1); // 243
	pTiles[(77 - 1) * 4 + 3] = SwapLE16(155 - 1); // 243
	pTiles[(78 - 1) * 4 + 3] = SwapLE16(155 - 1);
	pTiles[(79 - 1) * 4 + 1] = SwapLE16(25 - 1);  // 249
	pTiles[( 4 - 1) * 4 + 3] = SwapLE16(4 - 1);   // 14
	pTiles[(81 - 1) * 4 + 3] = SwapLE16(4 - 1);   // 256
	pTiles[(82 - 1) * 4 + 3] = SwapLE16(4 - 1);   // 259
	pTiles[(83 - 1) * 4 + 3] = SwapLE16(4 - 1);
	pTiles[(93 - 1) * 4 + 1] = SwapLE16(119 - 1);  // 298
	pTiles[(95 - 1) * 4 + 3] = SwapLE16(162 - 1);  // 308
	pTiles[(97 - 1) * 4 + 1] = SwapLE16(153 - 1);  // 314
	pTiles[(97 - 1) * 4 + 3] = SwapLE16(155 - 1);  // 316
	pTiles[(96 - 1) * 4 + 1] = SwapLE16(153 - 1);  // 310
	pTiles[(96 - 1) * 4 + 3] = SwapLE16(155 - 1);  // 312
	pTiles[(103 - 1) * 4 + 1] = SwapLE16(119 - 1); // 338
	pTiles[(112 - 1) * 4 + 1] = SwapLE16(119 - 1);

	// eliminate subtiles of unused tiles
	const int unusedTiles[] = {
		20, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 137
	};
	constexpr int blankSubtile = 14;
	for (int n = 0; n < lengthof(unusedTiles); n++) {
		int tileId = unusedTiles[n];
		pTiles[(tileId - 1) * 4 + 0] = SwapLE16(blankSubtile - 1);
		pTiles[(tileId - 1) * 4 + 1] = SwapLE16(blankSubtile - 1);
		pTiles[(tileId - 1) * 4 + 2] = SwapLE16(blankSubtile - 1);
		pTiles[(tileId - 1) * 4 + 3] = SwapLE16(blankSubtile - 1);
	}
}

DEVILUTION_END_NAMESPACE
