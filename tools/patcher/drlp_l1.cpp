/**
 * @file drlp_l1.cpp
 *
 * Implementation of the cathedral (and crypt) level patching functionality.
 */
#include "all.h"
#include "engine/render/cel_render.h"
#include "engine/render/dun_render.h"

DEVILUTION_BEGIN_NAMESPACE

BYTE* DRLP_L1_PatchDoors(BYTE* celBuf, size_t* celLen)
{
	constexpr int FRAME_WIDTH = 64;
	constexpr int FRAME_HEIGHT = 160;

	constexpr BYTE TRANS_COLOR = 128;
	constexpr BYTE SUB_HEADER_SIZE = 10;

	DWORD* srcHeaderCursor = (DWORD*)celBuf;
	int srcCelEntries = SwapLE32(srcHeaderCursor[0]);
	srcHeaderCursor++;

	// create the new CEL file
	size_t maxCelSize = *celLen + 2 * *celLen;
	BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
	memset(resCelBuf, 0, maxCelSize);

	DWORD* dstHeaderCursor = (DWORD*)resCelBuf;
	*dstHeaderCursor = SwapLE32(srcCelEntries);
	dstHeaderCursor++;

	BYTE* dstDataCursor = resCelBuf + 4 * (srcCelEntries + 2);
	for (int i = 0; i < srcCelEntries; i++) {
		// draw the frame to the back-buffer
		memset(&gpBuffer[0], TRANS_COLOR, FRAME_HEIGHT * BUFFER_WIDTH);
		CelClippedDraw(0, FRAME_HEIGHT - 1, celBuf, i + 1, FRAME_WIDTH);

		if (i == 0) {
			// add missing pixels after DRLP_L1_PatchSpec
			gpBuffer[29 + 81 * BUFFER_WIDTH] = 47;
			gpBuffer[30 + 80 * BUFFER_WIDTH] = 110;
			gpBuffer[31 + 79 * BUFFER_WIDTH] = 47;
		}
		if (i == 1) {
			// add missing pixels after DRLP_L1_PatchSpec
			gpBuffer[31 + 79 * BUFFER_WIDTH] = 46;
			gpBuffer[32 + 79 * BUFFER_WIDTH] = 47;
			gpBuffer[33 + 80 * BUFFER_WIDTH] = 47;
			gpBuffer[34 + 81 * BUFFER_WIDTH] = 47;
			// move the door-handle to the right
			if (gpBuffer[17 + 112 * BUFFER_WIDTH] == 42) {
				// copy the door-handle to the right
				for (int y = 108; y < 119; y++) {
					for (int x = 16; x < 24; x++) {
						BYTE color = gpBuffer[x + y * BUFFER_WIDTH];
						if (color == 47) {
							gpBuffer[x + 21 + (y + 7) * BUFFER_WIDTH] = 0;
						}
					}
				}
				gpBuffer[22 + 21 + (116 + 7) * BUFFER_WIDTH] = 0;
				gpBuffer[18 + 21 + (116 + 7) * BUFFER_WIDTH] = 0;
				gpBuffer[18 + 21 + (117 + 7) * BUFFER_WIDTH] = 0;
				gpBuffer[17 + 21 + (111 + 7) * BUFFER_WIDTH] = 0;
				// remove the original door-handle
				for (int y = 108; y < 120; y++) {
					for (int x = 16; x < 24; x++) {
						gpBuffer[x + y * BUFFER_WIDTH] = gpBuffer[x + 8 + (y + 7) * BUFFER_WIDTH];
					}
				}
			}
		}
		if (i == 2) {
			// add missing pixels after DRLP_L1_PatchSpec
			gpBuffer[29 + 81 * BUFFER_WIDTH] = 47;
			gpBuffer[30 + 80 * BUFFER_WIDTH] = 46;
			gpBuffer[31 + 79 * BUFFER_WIDTH] = 63;
			// add pixels for better outline
			for (int y = 72; y < 99; y++) {
				for (int x = 18; x < 42; x++) {
					if (y == 72 && (x < 23 || x > 32)) {
						continue;
					}
					if (y == 73 && (x < 19 || x > 35)) {
						continue;
					}
					if (y == 74 && x > 38) {
						continue;
					}
					if (y == 75 && x > 40) {
						continue;
					}
					if (gpBuffer[x + y * BUFFER_WIDTH] != TRANS_COLOR) {
						continue;
					}
					gpBuffer[x + y * BUFFER_WIDTH] = 47;
				}
			}
		}
		if (i == 3) {
			// add missing pixels after DRLP_L1_PatchSpec
			gpBuffer[31 + 79 * BUFFER_WIDTH] = 46;
			gpBuffer[32 + 79 * BUFFER_WIDTH] = 46;
			gpBuffer[33 + 80 * BUFFER_WIDTH] = 46;
			gpBuffer[34 + 81 * BUFFER_WIDTH] = 46;
			// add pixels for better outline
			for (int y = 72; y < 102; y++) {
				for (int x = 23; x < 47; x++) {
					if (y == 72 && (x < 32 || x > 41)) {
						continue;
					}
					if (y == 73 && (x < 29 || x > 45)) {
						continue;
					}
					if (y == 74 && x < 26) {
						continue;
					}
					if (y == 75 && x < 24) {
						continue;
					}
					if (gpBuffer[x + y * BUFFER_WIDTH] != TRANS_COLOR) {
						continue;
					}
					gpBuffer[x + y * BUFFER_WIDTH] = 47;
				}
			}
		}

		dstHeaderCursor[0] = SwapLE32((DWORD)((size_t)dstDataCursor - (size_t)resCelBuf));
		dstHeaderCursor++;

		dstDataCursor = EncodeFrame(dstDataCursor, FRAME_WIDTH, FRAME_HEIGHT, SUB_HEADER_SIZE, TRANS_COLOR);

		// skip the original frame
		srcHeaderCursor++;
	}
	// add file-size
	*celLen = (size_t)dstDataCursor - (size_t)resCelBuf;
	dstHeaderCursor[0] = SwapLE32((DWORD)(*celLen));

	return resCelBuf;
}

BYTE* DRLP_L1_PatchSpec(BYTE* sCelBuf, size_t* sCelLen)
{
	constexpr BYTE TRANS_COLOR = 128;
	constexpr BYTE SUB_HEADER_SIZE = 10;
	constexpr int FRAME_WIDTH = 64;
	constexpr int FRAME_HEIGHT = 160;

	DWORD* srcHeaderCursor = (DWORD*)sCelBuf;
	int srcCelEntries = SwapLE32(srcHeaderCursor[0]);
	srcHeaderCursor++;

	// create the new CEL file
	size_t maxCelSize = *sCelLen;
	BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
	memset(resCelBuf, 0, maxCelSize);

	DWORD* dstHeaderCursor = (DWORD*)resCelBuf;
	*dstHeaderCursor = SwapLE32(srcCelEntries);
	dstHeaderCursor++;

	BYTE* dstDataCursor = resCelBuf + 4 * (srcCelEntries + 2);

	for (int i = 0; i < srcCelEntries; i++) {
		// draw the frame to the back-buffer
		memset(&gpBuffer[0], TRANS_COLOR, FRAME_HEIGHT * BUFFER_WIDTH);
		CelClippedDraw(0, FRAME_HEIGHT - 1, sCelBuf, i + 1, FRAME_WIDTH);

		// eliminate unnecessary pixels on top
		for (int y = 0; y < 47; y++) {
			for (int x = 0; x < FRAME_WIDTH; x++) {
				gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
			}
		}
		if (i == 7 - 1) {
			for (int y = 71; y < 82; y++) {
				for (int x = 28; x < 44; x++) {
					BYTE color = gpBuffer[x + y * BUFFER_WIDTH];
					if (color == 14 || color == 29 || color == 30 || color == 46 || color == 47 || y > 112 - x) {
						gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
					}
				}
			}
			gpBuffer[38 + 74 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[35 + 77 * BUFFER_WIDTH] = TRANS_COLOR;

			gpBuffer[28 + 81 * BUFFER_WIDTH] = 22;
			gpBuffer[29 + 80 * BUFFER_WIDTH] = 10;
			gpBuffer[30 + 79 * BUFFER_WIDTH] = 10;
			gpBuffer[31 + 78 * BUFFER_WIDTH] = 23;
		}
		if (i == 8 - 1) {
			for (int y = 71; y < 82; y++) {
				for (int x = 19; x < 35; x++) {
					if (x == 34 && y == 71) {
						continue;
					}
					BYTE color = gpBuffer[x + y * BUFFER_WIDTH];
					if (color == 14 || color == 29 || color == 30 || color == 46 || color == 47) {
						gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
					}
				}
			}
			// gpBuffer[19 + 70 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[32 + 78 * BUFFER_WIDTH] = 26;
			gpBuffer[33 + 79 * BUFFER_WIDTH] = 27;
			gpBuffer[34 + 80 * BUFFER_WIDTH] = 28;
		}
		// eliminate pixels of the unused frames
		if (i == 3 - 1 || i == 6 - 1) {
			for (int y = 0; y < FRAME_HEIGHT; y++) {
				for (int x = 0; x < FRAME_WIDTH; x++) {
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
		}

		// write to the new SCEL file
		dstHeaderCursor[0] = SwapLE32((DWORD)((size_t)dstDataCursor - (size_t)resCelBuf));
		dstHeaderCursor++;

		dstDataCursor = EncodeFrame(dstDataCursor, FRAME_WIDTH, FRAME_HEIGHT, SUB_HEADER_SIZE, TRANS_COLOR);

		// skip the original frame
		srcHeaderCursor++;
	}

	// add file-size
	*sCelLen = (size_t)dstDataCursor - (size_t)resCelBuf;
	dstHeaderCursor[0] = SwapLE32((DWORD)(*sCelLen));

	return resCelBuf;
}

static BYTE* patchCathedralFloorCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	const CelMicro micros[] = {
/*  0 */{ 137 - 1, 5, MET_SQUARE },     // change type
/*  1 */{ 286 - 1, 1, MET_RTRIANGLE },  // change type
/*  2 */{ 408 - 1, 0, MET_LTRIANGLE },  // change type
/*  3 */{ 248 - 1, 0, MET_LTRIANGLE },  // change type

/*  4 */{ 392 - 1, 0, MET_TRANSPARENT }, // mask door
/*  5 */{ 392 - 1, 2, MET_TRANSPARENT },
/*  6 */{ 394 - 1, 1, MET_TRANSPARENT },
/*  7 */{ 394 - 1, 3, MET_TRANSPARENT },

/*  8 */{ 108 - 1, 1, -1 },              // used to block subsequent calls
/*  9 */{ 106 - 1, 0, MET_TRANSPARENT },
/* 10 */{ 109 - 1, 0, MET_TRANSPARENT },
/* 11 */{ 106 - 1, 1, MET_TRANSPARENT },

/* 12 */{ 178 - 1, 2, MET_TRANSPARENT },

/* 13 */{ 152 - 1, 5, MET_TRANSPARENT }, // blocks subsequent calls

/* 14 */{ 23 - 1, 0, -1 },
/* 15 */{ 270 - 1, 0, MET_TRANSPARENT },

/* 16 */{ 407 - 1, 0, MET_TRANSPARENT }, // mask door

/* 17 */{ 156 - 1, 0, MET_LTRIANGLE },
/* 18 */{ 160 - 1, 1, MET_RTRIANGLE },

/* 19 */{ 152 - 1, 1, MET_TRANSPARENT },
/* 20 */{ 159 - 1, 0, MET_LTRIANGLE },

/* 21 */{ 163 - 1, 1, MET_RTRIANGLE },

/* 22 */{ 137 - 1, 0, MET_TRANSPARENT },

/* 23 */{ 176 - 1, 1, MET_TRANSPARENT },
/* 24 */{ 171 - 1, 1, MET_RTRIANGLE },

/* 25 */{ 153 - 1, 0, MET_LTRIANGLE },

/* 26 */{ 231 - 1, 4, -1 },
/* 27 */{ 417 - 1, 4, MET_TRANSPARENT },
/* 28 */{ 418 - 1, 4, MET_TRANSPARENT },
/* 29 */{ 417 - 1, 2, MET_SQUARE },
	};

	const uint16_t* pSubtiles = (const uint16_t*)minBuf;
	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_L1;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		// if (micro.subtileIndex < 0) {
		// 	continue;
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
	constexpr int DRAW_HEIGHT = 4;
	memset(&gpBuffer[0], TRANS_COLOR, DRAW_HEIGHT * BUFFER_WIDTH * MICRO_HEIGHT);

	unsigned xx = 0, yy = MICRO_HEIGHT - 1;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		// if (micro.subtileIndex >= 0) {
			unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
			RenderMicro(&gpBuffer[xx + yy * BUFFER_WIDTH], SwapLE16(pSubtiles[index]), DMT_NONE);
		// }
		yy += MICRO_HEIGHT;
		if (yy == (DRAW_HEIGHT + 1) * MICRO_HEIGHT - 1) {
			yy = MICRO_HEIGHT - 1;
			xx += MICRO_WIDTH;
		}
	}
	// mask 392[0]
	for (int i = 4; i < 5; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (x < 17 && (x != 16 || y != 24)) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 392[2]
	for (int i = 5; i < 6; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (x < 14 || (x == 14 && y > 1) || (x == 15 && y > 2) || (x == 16 && y > 4)) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 394[1]
	// mask 394[3]
	for (int i = 6; i < 8; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (x > 15 && (i == 6 || (x > 18 || y > 9 || (x == 17 && y > 5) || (x == 18 && y != 0)))) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// copy 108[1] to 106[0] and 109[0]
	for (int i = 8; i < 9; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR && color != 46) {
					if (y < MICRO_HEIGHT / 2) {
						addr = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 106[0]
					} else {
						addr = x + MICRO_WIDTH * ((i + 2) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 2) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 109[0]
					}
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// mask 106[0]
	// mask 109[0]
	// mask 106[1]
	for (int i = 9; i < 12; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color == 46 && (y > 8 || i != 11) && (x > 22 || y < 22 || i != 10)) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// move pixels of 152[5] down to enable reuse as 153[6]
	for (int i = 13; i < 14; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR) {
					gpBuffer[addr + (MICRO_HEIGHT / 2) * BUFFER_WIDTH] = color;
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// remove shadow from 270[0] using 23[0]
	for (int i = 15; i < 16; i++) {
		for (int x = 22; x < 29; x++) {
			for (int y = 5; y < 12; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (x == 28 && y == 11) {
					continue;
				}
				unsigned addr2 = x + MICRO_WIDTH * ((i - 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 1) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 23[0]
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
	}
	// mask 407[0]
	for (int i = 16; i < 17; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (x < 17) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 137[0]
	for (int i = 22; i < 23; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (x > 16) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// add missing pixels after DRLP_L1_PatchSpec to 417[4] using 231[4]
	for (int i = 27; i < 28; i++) {
		for (int x = 0; x < 12; x++) {
			for (int y = 23; y < 31; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * ((i - 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 1) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 231[4]
				if (y < 29 - x) {
					continue;
				}
				if (x > 1 && y < 30 - x) {
					continue;
				}
				if (x == 6 && y == 24) {
					continue;
				}
				if (y == 23 && (x == 7 || x == 8)) {
					continue;
				}
				if (gpBuffer[addr] != TRANS_COLOR) {
					continue;
				}
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}

		// fix glitch
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  5 + 28 * BUFFER_WIDTH] = 29;
		gpBuffer[addr +  6 + 28 * BUFFER_WIDTH] = 28;
		gpBuffer[addr +  7 + 28 * BUFFER_WIDTH] = 27;
		gpBuffer[addr +  3 + 29 * BUFFER_WIDTH] = 47;
		gpBuffer[addr +  4 + 29 * BUFFER_WIDTH] = 47;
		gpBuffer[addr +  5 + 29 * BUFFER_WIDTH] = 46;
		gpBuffer[addr +  6 + 29 * BUFFER_WIDTH] = 27;
		gpBuffer[addr + 15 + 27 * BUFFER_WIDTH] = 6;

		// fix shadow
		gpBuffer[addr +  9 + 26 * BUFFER_WIDTH] = 28;
		gpBuffer[addr + 10 + 26 * BUFFER_WIDTH] = 26;
		gpBuffer[addr + 12 + 26 * BUFFER_WIDTH] = 11;
		gpBuffer[addr + 13 + 26 * BUFFER_WIDTH] = 12;
		gpBuffer[addr + 14 + 26 * BUFFER_WIDTH] = 28;

		gpBuffer[addr +  7 + 29 * BUFFER_WIDTH] = 26;
		gpBuffer[addr +  8 + 29 * BUFFER_WIDTH] = 42;
		gpBuffer[addr +  9 + 29 * BUFFER_WIDTH] = 26;
		gpBuffer[addr +  5 + 30 * BUFFER_WIDTH] = 28;
		gpBuffer[addr +  6 + 30 * BUFFER_WIDTH] = 26;
		gpBuffer[addr +  7 + 30 * BUFFER_WIDTH] = 27;
		gpBuffer[addr +  8 + 30 * BUFFER_WIDTH] = 27;
		gpBuffer[addr +  9 + 30 * BUFFER_WIDTH] = 26;
		gpBuffer[addr + 10 + 30 * BUFFER_WIDTH] = 27;
		gpBuffer[addr + 11 + 30 * BUFFER_WIDTH] = 26;
		gpBuffer[addr + 12 + 30 * BUFFER_WIDTH] = 26;
		gpBuffer[addr +  4 + 31 * BUFFER_WIDTH] = 28;
		gpBuffer[addr +  5 + 31 * BUFFER_WIDTH] = 26;
		gpBuffer[addr +  6 + 31 * BUFFER_WIDTH] = 26;
		gpBuffer[addr +  7 + 31 * BUFFER_WIDTH] = 12;
		gpBuffer[addr +  8 + 31 * BUFFER_WIDTH] = 26;
		gpBuffer[addr +  9 + 31 * BUFFER_WIDTH] = 26;
		gpBuffer[addr + 10 + 31 * BUFFER_WIDTH] = 44;
		gpBuffer[addr + 11 + 31 * BUFFER_WIDTH] = 26;
		gpBuffer[addr + 12 + 31 * BUFFER_WIDTH] = 26;
		gpBuffer[addr + 13 + 31 * BUFFER_WIDTH] = 12;
		gpBuffer[addr + 14 + 31 * BUFFER_WIDTH] = 26;
	}
	{ // 418[4] - add missing pixels after DRLP_L1_PatchSpec
		int i = 28;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 31 + 15 * BUFFER_WIDTH] = 46;
		gpBuffer[addr + 30 + 16 * BUFFER_WIDTH] = 30;
		gpBuffer[addr + 29 + 17 * BUFFER_WIDTH] = 30;

		// fix shadow
		gpBuffer[addr + 31 + 23 * BUFFER_WIDTH] = 28;
		gpBuffer[addr + 30 + 24 * BUFFER_WIDTH] = 29;
		gpBuffer[addr + 31 + 24 * BUFFER_WIDTH] = 27;
		gpBuffer[addr + 30 + 25 * BUFFER_WIDTH] = 28;
		gpBuffer[addr + 31 + 25 * BUFFER_WIDTH] = 28;
		gpBuffer[addr + 30 + 26 * BUFFER_WIDTH] = 27;
		gpBuffer[addr + 31 + 26 * BUFFER_WIDTH] = 27;
		gpBuffer[addr + 28 + 27 * BUFFER_WIDTH] = 29;
		gpBuffer[addr + 29 + 27 * BUFFER_WIDTH] = 27;
		gpBuffer[addr + 30 + 27 * BUFFER_WIDTH] = 28;
		gpBuffer[addr + 27 + 28 * BUFFER_WIDTH] = 29;
		gpBuffer[addr + 28 + 28 * BUFFER_WIDTH] = 27;
		gpBuffer[addr + 29 + 28 * BUFFER_WIDTH] = 27;
		gpBuffer[addr + 27 + 29 * BUFFER_WIDTH] = 28;
		gpBuffer[addr + 28 + 29 * BUFFER_WIDTH] = 28;
		gpBuffer[addr + 26 + 30 * BUFFER_WIDTH] = 28;
		gpBuffer[addr + 27 + 30 * BUFFER_WIDTH] = 26;
		gpBuffer[addr + 28 + 30 * BUFFER_WIDTH] = 27;
		gpBuffer[addr + 29 + 30 * BUFFER_WIDTH] = 28;
	}
	{ // 417[2] - fix shadow
		int i = 29;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  0 +  9 * BUFFER_WIDTH] = 28;
		gpBuffer[addr +  0 +  8 * BUFFER_WIDTH] = 27;
		gpBuffer[addr +  1 +  8 * BUFFER_WIDTH] = 26;
		gpBuffer[addr +  2 +  8 * BUFFER_WIDTH] = 26;

		gpBuffer[addr +  0 +  7 * BUFFER_WIDTH] = 26;
		gpBuffer[addr +  1 +  7 * BUFFER_WIDTH] = 26;
		gpBuffer[addr +  2 +  7 * BUFFER_WIDTH] = 12;
		gpBuffer[addr +  4 +  7 * BUFFER_WIDTH] = 27;

		gpBuffer[addr +  0 +  6 * BUFFER_WIDTH] = 28;
		gpBuffer[addr +  1 +  6 * BUFFER_WIDTH] = 27;
		gpBuffer[addr +  2 +  6 * BUFFER_WIDTH] = 25;
		gpBuffer[addr +  3 +  6 * BUFFER_WIDTH] = 25;
		gpBuffer[addr +  4 +  6 * BUFFER_WIDTH] = 11;

		gpBuffer[addr +  0 +  5 * BUFFER_WIDTH] = 27;
		gpBuffer[addr +  1 +  5 * BUFFER_WIDTH] = 26;
		gpBuffer[addr +  2 +  5 * BUFFER_WIDTH] = 12;
		gpBuffer[addr +  3 +  5 * BUFFER_WIDTH] = 10;
		gpBuffer[addr +  4 +  5 * BUFFER_WIDTH] = 10;
		gpBuffer[addr +  5 +  5 * BUFFER_WIDTH] = 26;

		gpBuffer[addr +  1 +  4 * BUFFER_WIDTH] = 26;
		gpBuffer[addr +  2 +  4 * BUFFER_WIDTH] = 11;
		gpBuffer[addr +  3 +  4 * BUFFER_WIDTH] = 10;
		gpBuffer[addr +  4 +  4 * BUFFER_WIDTH] = 11;
		gpBuffer[addr +  5 +  4 * BUFFER_WIDTH] = 10;
		gpBuffer[addr +  6 +  4 * BUFFER_WIDTH] = 25;
		gpBuffer[addr +  7 +  4 * BUFFER_WIDTH] = 27;

		gpBuffer[addr +  1 +  3 * BUFFER_WIDTH] = 28;
		gpBuffer[addr +  2 +  3 * BUFFER_WIDTH] = 26;
		gpBuffer[addr +  3 +  3 * BUFFER_WIDTH] = 27;
		gpBuffer[addr +  4 +  3 * BUFFER_WIDTH] = 27;
		gpBuffer[addr +  5 +  3 * BUFFER_WIDTH] = 28;
		gpBuffer[addr +  6 +  3 * BUFFER_WIDTH] = 27;
		gpBuffer[addr +  7 +  3 * BUFFER_WIDTH] = 26;
		gpBuffer[addr +  8 +  3 * BUFFER_WIDTH] = 26;

		gpBuffer[addr +  2 +  2 * BUFFER_WIDTH] = 28;
		gpBuffer[addr +  3 +  2 * BUFFER_WIDTH] = 27;
		gpBuffer[addr +  4 +  2 * BUFFER_WIDTH] = 26;
		gpBuffer[addr +  5 +  2 * BUFFER_WIDTH] = 26;
		gpBuffer[addr +  6 +  2 * BUFFER_WIDTH] = 26;
		gpBuffer[addr +  7 +  2 * BUFFER_WIDTH] = 26;
		gpBuffer[addr +  8 +  2 * BUFFER_WIDTH] = 26;

		gpBuffer[addr +  3 +  1 * BUFFER_WIDTH] = 27;
		gpBuffer[addr +  4 +  1 * BUFFER_WIDTH] = 27;
		gpBuffer[addr +  5 +  1 * BUFFER_WIDTH] = 26;
		gpBuffer[addr +  6 +  1 * BUFFER_WIDTH] = 26;
		gpBuffer[addr +  7 +  1 * BUFFER_WIDTH] = 28;
		gpBuffer[addr +  8 +  1 * BUFFER_WIDTH] = 29;
		gpBuffer[addr +  9 +  1 * BUFFER_WIDTH] = 28;
		gpBuffer[addr + 10 +  1 * BUFFER_WIDTH] = 26;

		gpBuffer[addr +  4 +  0 * BUFFER_WIDTH] = 28;
		gpBuffer[addr +  5 +  0 * BUFFER_WIDTH] = 28;
		gpBuffer[addr +  6 +  0 * BUFFER_WIDTH] = 28;
		gpBuffer[addr +  7 +  0 * BUFFER_WIDTH] = 26;
		gpBuffer[addr +  8 +  0 * BUFFER_WIDTH] = 26;
		gpBuffer[addr +  9 +  0 * BUFFER_WIDTH] = 12;
		gpBuffer[addr + 10 +  0 * BUFFER_WIDTH] = 26;
		gpBuffer[addr + 11 +  0 * BUFFER_WIDTH] = 26;
	}

	// fix artifacts
	/*{ // 392[0]
		int i = 4;
		unsigned addr = 16 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (24 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = 42;
	}
	{ // 392[2]
		int i = 5;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 14 +  0 * BUFFER_WIDTH] = 46;
		gpBuffer[addr + 14 +  1 * BUFFER_WIDTH] = 46;
		gpBuffer[addr + 15 +  1 * BUFFER_WIDTH] = 46;
		gpBuffer[addr + 15 +  2 * BUFFER_WIDTH] = 29;
		gpBuffer[addr + 16 +  2 * BUFFER_WIDTH] = 44;
		gpBuffer[addr + 16 +  3 * BUFFER_WIDTH] = 29;
		gpBuffer[addr + 16 +  4 * BUFFER_WIDTH] = 46;
		gpBuffer[addr + 17 + 28 * BUFFER_WIDTH] = 106;
	}*/
	{ // 178[2]
		int i = 12;
		unsigned addr = 14 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 1 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 0 + 0 * BUFFER_WIDTH] = 10; // 14:1
		gpBuffer[addr + 1 + 0 * BUFFER_WIDTH] = 20; // 15:1
		gpBuffer[addr + 1 + 1 * BUFFER_WIDTH] = 10; // 15:2
		gpBuffer[addr + 2 + 1 * BUFFER_WIDTH] = 10; // 16:2
		gpBuffer[addr + 2 + 2 * BUFFER_WIDTH] = 10; // 16:3
		gpBuffer[addr + 3 + 2 * BUFFER_WIDTH] = 10; // 17:3
		gpBuffer[addr + 3 + 0 * BUFFER_WIDTH] = 20; // 17:1
		gpBuffer[addr + 4 + 0 * BUFFER_WIDTH] = 10; // 18:1
		gpBuffer[addr + 0 + 3 * BUFFER_WIDTH] = TRANS_COLOR; // 14:4
		gpBuffer[addr + 1 + 4 * BUFFER_WIDTH] = TRANS_COLOR; // 15:5
		gpBuffer[addr + 2 + 5 * BUFFER_WIDTH] = TRANS_COLOR; // 16:6
	}
	{ // 407[0]
		int i = 16;
		unsigned addr = 31 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (26 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = 57;
		gpBuffer[addr + BUFFER_WIDTH] = 89;
	}
	{ // 156[0]
		int i = 17;
		unsigned addr = 30 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (31 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = 41;
	}
	{ // 160[1]
		int i = 18;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 0 + 1 * BUFFER_WIDTH] = 41;
		gpBuffer[addr + 0 + 2 * BUFFER_WIDTH] = 117;
		gpBuffer[addr + 1 + 2 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 1 + 3 * BUFFER_WIDTH] = 116;
		gpBuffer[addr + 2 + 3 * BUFFER_WIDTH] = 41;
		gpBuffer[addr + 3 + 4 * BUFFER_WIDTH] = 119;
		gpBuffer[addr + 4 + 4 * BUFFER_WIDTH] = 43;
	}
	{ // 152[1]
		int i = 19;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 11 + 6 * BUFFER_WIDTH] = 47;
		gpBuffer[addr + 13 + 7 * BUFFER_WIDTH] = 47;
		gpBuffer[addr + 15 + 8 * BUFFER_WIDTH] = 47;
		gpBuffer[addr + 17 + 9 * BUFFER_WIDTH] = 47;
		gpBuffer[addr + 19 + 10 * BUFFER_WIDTH] = 47;
		gpBuffer[addr + 20 + 13 * BUFFER_WIDTH] = 47;
		gpBuffer[addr + 21 + 13 * BUFFER_WIDTH] = 47;
		gpBuffer[addr + 21 + 11 * BUFFER_WIDTH] = 43;
		gpBuffer[addr + 22 + 16 * BUFFER_WIDTH] = 47;
		gpBuffer[addr + 23 + 15 * BUFFER_WIDTH] = 43;
		gpBuffer[addr + 23 + 16 * BUFFER_WIDTH] = 47;
		gpBuffer[addr + 25 + 17 * BUFFER_WIDTH] = 120;
		gpBuffer[addr + 25 + 19 * BUFFER_WIDTH] = 47;
	}
	{ // 159[0]
		int i = 20;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 20 + 26 * BUFFER_WIDTH] = 47;
		gpBuffer[addr + 21 + 26 * BUFFER_WIDTH] = 119;
	}
	{ // 163[1]
		int i = 21;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 0 + 26 * BUFFER_WIDTH] = 110;
	}
	{ // 176[1] - fix connection
		int i = 23;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  8 +  5 * BUFFER_WIDTH] = 20;
		gpBuffer[addr +  9 +  5 * BUFFER_WIDTH] = 20;
		gpBuffer[addr + 10 +  6 * BUFFER_WIDTH] = 19;
		gpBuffer[addr + 11 +  6 * BUFFER_WIDTH] = 5;
		gpBuffer[addr + 12 +  7 * BUFFER_WIDTH] = 2;
		gpBuffer[addr + 13 +  7 * BUFFER_WIDTH] = 4;
		gpBuffer[addr + 14 +  8 * BUFFER_WIDTH] = 17;
		gpBuffer[addr + 15 +  8 * BUFFER_WIDTH] = 6;
		gpBuffer[addr + 16 +  9 * BUFFER_WIDTH] = 3;
		gpBuffer[addr + 17 +  9 * BUFFER_WIDTH] = 41;
	}	
	{ // 171[1]
		int i = 24;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 17 +  9 * BUFFER_WIDTH] = 20;
		gpBuffer[addr + 17 + 10 * BUFFER_WIDTH] = 21;
		gpBuffer[addr + 18 + 10 * BUFFER_WIDTH] = 22;
		gpBuffer[addr + 18 + 11 * BUFFER_WIDTH] = 21;
		gpBuffer[addr + 19 + 11 * BUFFER_WIDTH] = 21;
		gpBuffer[addr + 20 + 11 * BUFFER_WIDTH] = 21;
		gpBuffer[addr + 21 + 11 * BUFFER_WIDTH] = 22;
		gpBuffer[addr + 16 + 12 * BUFFER_WIDTH] = 5;
		gpBuffer[addr + 17 + 12 * BUFFER_WIDTH] = 20;
		gpBuffer[addr + 19 + 12 * BUFFER_WIDTH] = 21;
		gpBuffer[addr + 20 + 12 * BUFFER_WIDTH] = 22;
		gpBuffer[addr + 21 + 12 * BUFFER_WIDTH] = 22;
		gpBuffer[addr + 22 + 12 * BUFFER_WIDTH] = 21;
		gpBuffer[addr + 23 + 12 * BUFFER_WIDTH] = 21;
		gpBuffer[addr + 16 + 13 * BUFFER_WIDTH] = 20;
		gpBuffer[addr + 20 + 13 * BUFFER_WIDTH] = 22;
		gpBuffer[addr + 21 + 13 * BUFFER_WIDTH] = 22;
	}
	{ // 153[0]
		int i = 25;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 27 +  4 * BUFFER_WIDTH] = 43;
		gpBuffer[addr + 27 +  6 * BUFFER_WIDTH] = 46;
	}

	// create the new CEL file
	size_t maxCelSize = *celLen + lengthof(micros) * MICRO_WIDTH * MICRO_HEIGHT;
	BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
	memset(resCelBuf, 0, maxCelSize);

	CelFrameEntry entries[lengthof(micros)];
	xx = 0, yy = MICRO_HEIGHT - 1;
	int idx = 0;
	for (int i = 0; i < lengthof(micros); i++) {
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

static BYTE shadowColorCathedral(BYTE color)
{
	// assert(color < 128);
	if (color == 0) {
		return 0;
	}
	switch (color % 16) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		return (color & ~15) + 13;
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
		return (color & ~15) + 14;
	case 11:
	case 12:
	case 13:
		return (color & ~15) + 15;
	}
	return 0;
}
static BYTE* fixCathedralShadows(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	const CelMicro micros[] = {
		// add shadow of the grate
/*  0 */{ 306 - 1, 1, -1 },             // used to block subsequent calls
/*  1 */{ 304 - 1, 0, -1 },
/*  2 */{ 304 - 1, 1, -1 },
/*  3 */{ 57 - 1, 0, MET_TRANSPARENT },
/*  4 */{ 53 - 1, 2, MET_TRANSPARENT },
/*  5 */{ 53 - 1, 0, MET_TRANSPARENT },
		// floor source
/*  6 */{  23 - 1, 0, -1 },
/*  7 */{  23 - 1, 1, -1 },
/*  8 */{   2 - 1, 0, -1 },
/*  9 */{   2 - 1, 1, -1 },
/* 10 */{   7 - 1, 0, -1 },
/* 11 */{   7 - 1, 1, -1 },
/* 12 */{   4 - 1, 0, -1 },
/* 13 */{   4 - 1, 1, -1 },
		// grate source
/* 14 */{  53 - 1, 1, -1 },
/* 15 */{  47 - 1, 1, -1 },
/* 16 */{  48 - 1, 0, -1 },
/* 17 */{  48 - 1, 1, -1 },
		// base shadows
/* 18 */{ 296 - 1, 0, MET_LTRIANGLE },
/* 19 */{ 296 - 1, 1, MET_RTRIANGLE },
/* 20 */{ 297 - 1, 0, MET_LTRIANGLE },
/* 21 */{ 297 - 1, 1, MET_RTRIANGLE },
/* 22 */{ 313 - 1, 0, MET_LTRIANGLE },
/* 23 */{ 313 - 1, 1, MET_RTRIANGLE },
/* 24 */{ 299 - 1, 0, MET_LTRIANGLE },
/* 25 */{ 299 - 1, 1, MET_RTRIANGLE },
/* 26 */{ 301 - 1, 0, MET_LTRIANGLE },
/* 27 */{ 301 - 1, 1, MET_RTRIANGLE },
/* 28 */{ 302 - 1, 0, MET_LTRIANGLE },
/* 29 */{ 302 - 1, 1, MET_RTRIANGLE },
/* 30 */{ 307 - 1, 0, MET_LTRIANGLE },
/* 31 */{ 307 - 1, 1, MET_RTRIANGLE },
/* 32 */{ 308 - 1, 0, MET_LTRIANGLE },
/* 33 */{ 308 - 1, 1, MET_RTRIANGLE },
/* 34 */{ 328 - 1, 0, MET_LTRIANGLE },
/* 35 */{ 328 - 1, 1, MET_RTRIANGLE },
		// complex shadows
/* 36 */{ 310 - 1, 0, MET_LTRIANGLE },
/* 37 */{ 310 - 1, 1, MET_RTRIANGLE },
/* 38 */{ 320 - 1, 0, MET_LTRAPEZOID },
/* 39 */{ 320 - 1, 1, MET_RTRIANGLE },
/* 40 */{ 321 - 1, 0, MET_LTRIANGLE },
/* 41 */{ 321 - 1, 1, MET_RTRIANGLE },
/* 42 */{ -1, 0, -1 },
/* 43 */{ 322 - 1, 1, MET_RTRAPEZOID },
/* 44 */{ 323 - 1, 0, MET_LTRIANGLE },
/* 45 */{ 323 - 1, 1, MET_RTRAPEZOID },
/* 46 */{ 324 - 1, 0, MET_TRANSPARENT },
/* 47 */{ 324 - 1, 1, MET_TRANSPARENT },
/* 48 */{ 325 - 1, 0, MET_LTRIANGLE },
/* 49 */{ 325 - 1, 1, MET_RTRIANGLE },

/* 50 */{ 298 - 1, 0, -1 },
/* 51 */{ -1, 1, -1 },
/* 52 */{ 304 - 1, 1, MET_TRANSPARENT },
/* 53 */{ 334 - 1, 0, MET_TRANSPARENT },
/* 54 */{ 334 - 1, 1, MET_RTRIANGLE },
/* 55 */{ 334 - 1, 2, MET_TRANSPARENT },

		// special shadows for the banner setpiece
/* 56 */{ 112 - 1, 0, -1 },
/* 57 */{ 112 - 1, 1, -1 },
/* 58 */{ 336 - 1, 0, MET_TRANSPARENT },
/* 59 */{ 336 - 1, 1, MET_RTRIANGLE },
/* 60 */{ 118 - 1, 1, -1 },
/* 61 */{ 338 - 1, 1, MET_TRANSPARENT },

/* 62 */{ 330 - 1, 0, MET_LTRIANGLE },
	};

	const uint16_t* pSubtiles = (const uint16_t*)minBuf;
	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_L1;
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
	constexpr int DRAW_HEIGHT = 4;
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

	// copy 306[1] to 53[2] and 53[0]
	for (int i = 0; i < 1; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color == 0 || color == 15 || color == 43 || color == 44 || color == 45 || color == 46 || color == 47 ||  color == 109 ||  color == 110 || color == 127) {
					if (y < MICRO_HEIGHT / 2) {
						addr = x + MICRO_WIDTH * ((i + 4) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 4) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 53[2]
					} else {
						addr = x + MICRO_WIDTH * ((i + 5) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 5) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 53[0]
					}
					if (gpBuffer[addr] == TRANS_COLOR) {
						gpBuffer[addr] = color;
					}
				}
			}
		}
	}
	// copy 304[0] to 53[2]
	for (int i = 1; i < 2; i++) {
		for (int x = 23; x < MICRO_WIDTH; x++) {
			for (int y = 25; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color == 0 || color == 15 || color == 43 || color == 44 || color == 45 || color == 46 || color == 47 ||  color == 109 ||  color == 110 || color == 127) {
					addr = x + MICRO_WIDTH * ((i + 3) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 3) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 53[2]
					if (gpBuffer[addr] == TRANS_COLOR) {
						gpBuffer[addr] = color;
					}
				}
			}
		}
	}
	// copy 304[1] to 57[0]
	for (int i = 2; i < 3; i++) {
		for (int x = 0; x < 7; x++) {
			for (int y = 19; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color == 0 || color == 15 || color == 43 || color == 44 || color == 45 || color == 46 || color == 47 ||  color == 109 ||  color == 110 || color == 127) {
					addr = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 57[0]
					if (gpBuffer[addr] == TRANS_COLOR) {
						gpBuffer[addr] = color;
					}
				}
			}
		}
	}
	// fix artifacts I.
	{ // 57[0]
		int i = 3;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  5 +  4 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  6 +  4 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  1 +  9 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  1 + 10 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  2 + 10 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  1 +  5 * BUFFER_WIDTH] = 46;
		gpBuffer[addr +  3 +  4 * BUFFER_WIDTH] = 42;
		gpBuffer[addr + 10 +  8 * BUFFER_WIDTH] = 47;
		gpBuffer[addr + 11 +  8 * BUFFER_WIDTH] = 45;
		gpBuffer[addr + 12 +  8 * BUFFER_WIDTH] = 43;
		gpBuffer[addr + 10 +  9 * BUFFER_WIDTH] = 46;
		gpBuffer[addr + 11 +  9 * BUFFER_WIDTH] = 44;
	}
	{ // 53[2]
		int i = 4;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 15 + 22 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 15 + 23 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 15 + 24 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 15 + 25 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  9 + 27 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  8 + 29 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  9 + 29 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 10 + 27 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 25 + 28 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 28 + 26 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 19 + 28 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 24 + 30 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 19 + 31 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 24 + 31 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 25 + 31 * BUFFER_WIDTH] = TRANS_COLOR;

		gpBuffer[addr + 19 + 16 * BUFFER_WIDTH] = 46;
		gpBuffer[addr + 18 + 17 * BUFFER_WIDTH] = 44;
		gpBuffer[addr + 19 + 17 * BUFFER_WIDTH] = 46;
		gpBuffer[addr + 20 + 17 * BUFFER_WIDTH] = 43;
		gpBuffer[addr + 18 + 18 * BUFFER_WIDTH] = 45;
		gpBuffer[addr + 19 + 18 * BUFFER_WIDTH] = 46;
		gpBuffer[addr + 20 + 18 * BUFFER_WIDTH] = 45;
		gpBuffer[addr + 19 + 19 * BUFFER_WIDTH] = 45;
		gpBuffer[addr + 20 + 19 * BUFFER_WIDTH] = 46;
		gpBuffer[addr + 20 + 20 * BUFFER_WIDTH] = 45;
		gpBuffer[addr + 20 + 21 * BUFFER_WIDTH] = 44;
		gpBuffer[addr + 20 + 22 * BUFFER_WIDTH] = 43;

		gpBuffer[addr +  8 + 30 * BUFFER_WIDTH] = 44;
		gpBuffer[addr + 19 + 23 * BUFFER_WIDTH] = 44;
		gpBuffer[addr + 20 + 23 * BUFFER_WIDTH] = 45;
		gpBuffer[addr + 16 + 25 * BUFFER_WIDTH] = 45;
		gpBuffer[addr + 17 + 25 * BUFFER_WIDTH] = 46;
		gpBuffer[addr + 18 + 24 * BUFFER_WIDTH] = 45;
		gpBuffer[addr + 19 + 24 * BUFFER_WIDTH] = 45;
		gpBuffer[addr + 27 + 31 * BUFFER_WIDTH] = 45;
		gpBuffer[addr + 28 + 27 * BUFFER_WIDTH] = 44;
		gpBuffer[addr + 29 + 28 * BUFFER_WIDTH] = 45;
		gpBuffer[addr + 24 + 20 * BUFFER_WIDTH] = 46;
		gpBuffer[addr + 24 + 19 * BUFFER_WIDTH] = 46;
		gpBuffer[addr + 25 + 19 * BUFFER_WIDTH] = 45;
		gpBuffer[addr + 26 + 19 * BUFFER_WIDTH] = 45;
		gpBuffer[addr + 25 + 18 * BUFFER_WIDTH] = 43;
		gpBuffer[addr + 26 + 18 * BUFFER_WIDTH] = 45;
		gpBuffer[addr + 27 + 18 * BUFFER_WIDTH] = 46;
		gpBuffer[addr + 28 + 18 * BUFFER_WIDTH] = 44;
		gpBuffer[addr + 27 + 17 * BUFFER_WIDTH] = 43;
		gpBuffer[addr + 28 + 17 * BUFFER_WIDTH] = 45;
		gpBuffer[addr + 29 + 17 * BUFFER_WIDTH] = 46;
		gpBuffer[addr + 28 + 16 * BUFFER_WIDTH] = 43;
		gpBuffer[addr + 29 + 16 * BUFFER_WIDTH] = 45;
	}
	{ // 53[0]
		int i = 5;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 15 +  0 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 19 +  0 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 20 +  0 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 17 +  2 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  9 +  8 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 10 +  8 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  8 +  9 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  9 +  9 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  8 + 10 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 29 +  0 * BUFFER_WIDTH] = 47;
		gpBuffer[addr + 27 +  1 * BUFFER_WIDTH] = 46;
		gpBuffer[addr + 25 +  2 * BUFFER_WIDTH] = 46;
		gpBuffer[addr + 19 +  3 * BUFFER_WIDTH] = 46;
		gpBuffer[addr + 17 +  4 * BUFFER_WIDTH] = 45;
		gpBuffer[addr + 11 +  9 * BUFFER_WIDTH] = 47;
		gpBuffer[addr +  8 + 10 * BUFFER_WIDTH] = 45;
		gpBuffer[addr +  8 + 11 * BUFFER_WIDTH] = 45;
		gpBuffer[addr +  8 + 12 * BUFFER_WIDTH] = 44;
		gpBuffer[addr +  9 + 12 * BUFFER_WIDTH] = 43;
	}
	// reduce 313[1] using 7[1]
	for (int i = 23; i < 24; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y < (x + 1) / 2 + 18) {
					gpBuffer[addr] = gpBuffer[x + MICRO_WIDTH * ((i - 12) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 12) % DRAW_HEIGHT)) * BUFFER_WIDTH]; // 7[1]
				}
			}
		}
	}
	// draw 298[0] using 48[0] and 297[0]
	/*for (int i = 50; i < 51; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[x + MICRO_WIDTH * ((i - 30) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 30) % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != 0 && color != 45 && color != 46 && color != 47 && color != 109 && color != 110 && color != 111 && color != 127) {
					color = gpBuffer[x + MICRO_WIDTH * ((i - 16) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 16) % DRAW_HEIGHT)) * BUFFER_WIDTH];
				}
				gpBuffer[addr] = color;
			}
		}
	}*/
	// draw 304[1] using 47[1] and 296[1]
	for (int i = 52; i < 53; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[x + MICRO_WIDTH * ((i - 33) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 33) % DRAW_HEIGHT)) * BUFFER_WIDTH]; // 296[1]
				if (x <= 16 || (color != 0 && color != 45 && color != 46 && color != 47 && color != 109 && color != 110 && color != 111 && color != 127)) {
					color = gpBuffer[x + MICRO_WIDTH * ((i - 37) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 37) % DRAW_HEIGHT)) * BUFFER_WIDTH]; // 47[1]
				}
				gpBuffer[addr] = color;
			}
		}
	}
	// draw 334[0] using 53[0] and 301[0]
	for (int i = 53; i < 54; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * ((i - 48) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 48) % DRAW_HEIGHT)) * BUFFER_WIDTH]; // 53[0]
				if (x > 7 && color != TRANS_COLOR) {
					if ((x >= 12 && x <= 14) || (x >= 21 && x <= 23)) {
						color = shadowColorCathedral(color);
					} else if (y > x / 2 + 1) {
						BYTE color2 = gpBuffer[x + MICRO_WIDTH * ((i - 27) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 27) % DRAW_HEIGHT)) * BUFFER_WIDTH]; // 301[0]
						if (color2 != TRANS_COLOR) {
							color = color2;
						} else {
							color = shadowColorCathedral(color);
						}
					}
				}
				gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH] = color;
			}
		}
	}
	// draw 334[1] using 53[1] and 301[1]
	for (int i = 54; i < 55; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr;
				if (y > (x + 1) / 2 + 17) {
					addr = x + MICRO_WIDTH * ((i - 27) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 27) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 301[1]
				} else {
					addr = x + MICRO_WIDTH * ((i - 40) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 40) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 53[1]
				}
				gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH] = gpBuffer[addr];
			}
		}
	}
	// draw 334[2] using 53[2]
	for (int i = 55; i < 56; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * ((i - 51) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 51) % DRAW_HEIGHT)) * BUFFER_WIDTH]; // 53[2]
				if (x > 8 && (color == 0 || color == 12 || color == 27 || color == 29 || color == 30 || (color >= 42 && color <= 45) || color == 47 || color == 74 || color == 75 || color == 104 || color == 118 || color == 119 || color == 123) && (y > x - 10)) {
					color = shadowColorCathedral(color);
				} else if (x <= 8 || (x < 18 && y <= 26)) {
					color = TRANS_COLOR;
				}
				gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH] = color;
			}
		}
	}
	// draw 336[0] using 112[0]
	for (int i = 58; i < 59; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * ((i - 2) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 2) % DRAW_HEIGHT)) * BUFFER_WIDTH]; // 112[0]
				if (color != TRANS_COLOR && y > 33 - x / 2) {
					color = shadowColorCathedral(color);
				}
				gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH] = color;
			}
		}
	}
	// draw 336[1] using 112[1]
	for (int i = 59; i < 60; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * ((i - 2) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 2) % DRAW_HEIGHT)) * BUFFER_WIDTH]; // 112[1]
				if (color != TRANS_COLOR && y > 17 - x / 2) {
					color = shadowColorCathedral(color);
				}
				gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH] = color;
			}
		}
	}
	// draw 338[1] using 118[1] and 324[1]
	for (int i = 61; i < 62; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr;
				if (x < 16) {
					addr = x + MICRO_WIDTH * ((i - 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 1) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 118[1]
				} else {
					addr = x + MICRO_WIDTH * ((i - 14) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 14) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 324[1]
				}
				gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH] = gpBuffer[addr];
			}
		}
	}
	// reduce 330[0] using 7[0]
	for (int i = 62; i < 63; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y < (x + 1) / 2 + 2) {
					gpBuffer[addr] = gpBuffer[x + MICRO_WIDTH * ((i - 52) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 52) % DRAW_HEIGHT)) * BUFFER_WIDTH]; // 7[0]
				}
			}
		}
	}
	// fix artifacts II.
	{ // 334[2]
		int i = 55;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 12 + 29 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 13 + 30 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 14 + 22 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 15 + 22 * BUFFER_WIDTH] = 0;
	}

	// create the new CEL file
	size_t maxCelSize = *celLen + lengthof(micros) * MICRO_WIDTH * MICRO_HEIGHT;
	BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
	memset(resCelBuf, 0, maxCelSize);

	CelFrameEntry entries[lengthof(micros)];
	xx = 0, yy = MICRO_HEIGHT - 1;
	int idx = 0;
	for (int i = 0; i < lengthof(micros); i++) {
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

BYTE* DRLP_L1_PatchCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	// TODO: fix connection of tile 83 and tile 25 in Skng1.dun

	celBuf = patchCathedralFloorCel(minBuf, minLen, celBuf, celLen);
	if (celBuf == NULL) {
		return NULL;
	}
	celBuf = fixCathedralShadows(minBuf, minLen, celBuf, celLen);
	return celBuf;
}

void DRLP_L1_PatchMin(BYTE* buf)
{
	uint16_t* pSubtiles = (uint16_t*)buf;
	constexpr int blockSize = BLOCK_SIZE_L1;
	// use micros created by patchCathedralFloorCel
	if (pSubtiles[MICRO_IDX(108 - 1, blockSize, 1)] != 0) {
		Blk2Mcr(108, 1);
		SetFrameType(137, 5, MET_SQUARE);
		SetFrameType(286, 1, MET_RTRIANGLE);
		SetFrameType(408, 0, MET_LTRIANGLE);
		SetFrameType(248, 0, MET_LTRIANGLE);

		SetFrameType(392, 0, MET_TRANSPARENT);
		SetFrameType(396, 0, MET_TRANSPARENT);
		SetFrameType(397, 0, MET_TRANSPARENT);
		SetFrameType(399, 0, MET_TRANSPARENT);
		SetFrameType(401, 0, MET_TRANSPARENT);
		SetFrameType(403, 0, MET_TRANSPARENT);
		SetFrameType(409, 0, MET_TRANSPARENT);
		SetFrameType(411, 0, MET_TRANSPARENT);
		SetFrameType(392, 2, MET_TRANSPARENT);
		SetFrameType(396, 2, MET_TRANSPARENT);
		SetFrameType(397, 2, MET_TRANSPARENT);
		SetFrameType(399, 2, MET_TRANSPARENT);
		SetFrameType(401, 2, MET_TRANSPARENT);
		SetFrameType(403, 2, MET_TRANSPARENT);
		SetFrameType(407, 2, MET_TRANSPARENT);
		SetFrameType(409, 2, MET_TRANSPARENT);
		SetFrameType(411, 2, MET_TRANSPARENT);
		SetFrameType(394, 1, MET_TRANSPARENT);
		SetFrameType(396, 1, MET_TRANSPARENT);
		SetFrameType(398, 1, MET_TRANSPARENT);
		SetFrameType(400, 1, MET_TRANSPARENT);
		SetFrameType(404, 1, MET_TRANSPARENT);
		SetFrameType(406, 1, MET_TRANSPARENT);
		SetFrameType(410, 1, MET_TRANSPARENT);
		SetFrameType(412, 1, MET_TRANSPARENT);
		SetFrameType(394, 3, MET_TRANSPARENT);
		SetFrameType(396, 3, MET_TRANSPARENT);
		SetFrameType(398, 3, MET_TRANSPARENT);
		SetFrameType(400, 3, MET_TRANSPARENT);
		SetFrameType(404, 3, MET_TRANSPARENT);
		SetFrameType(406, 3, MET_TRANSPARENT);
		SetFrameType(410, 3, MET_TRANSPARENT);
		SetFrameType(412, 3, MET_TRANSPARENT);

		SetFrameType(407, 0, MET_TRANSPARENT);
		SetFrameType(171, 1, MET_RTRIANGLE);
		SetFrameType(106, 0, MET_TRANSPARENT);
		SetFrameType(109, 0, MET_TRANSPARENT);
		SetFrameType(106, 1, MET_TRANSPARENT);

		MoveMcr(153, 6, 152, 5);
		ReplaceMcr(160, 0, 23, 0);
	}
	// use micros created by fixCathedralShadows
	Blk2Mcr(306, 1);
	ReplaceMcr(298, 0, 297, 0);
	ReplaceMcr(298, 1, 48, 1);
	SetMcr(298, 3, 48, 3);
	SetMcr(298, 5, 48, 5);
	SetMcr(298, 7, 48, 7);
	ReplaceMcr(304, 0, 8, 0);
	SetFrameType(304, 1, MET_TRANSPARENT);
	SetMcr(304, 3, 47, 3);
	SetMcr(304, 5, 47, 5);
	SetMcr(304, 7, 64, 7);
	SetFrameType(334, 2, MET_TRANSPARENT);
	SetFrameType(334, 0, MET_TRANSPARENT);
	SetMcr(334, 4, 53, 4);
	SetMcr(334, 6, 3, 6);

	SetMcr(339, 0, 9, 0);
	SetMcr(339, 1, 322, 1);
	SetMcr(339, 2, 9, 2);
	SetMcr(339, 3, 322, 3);
	ReplaceMcr(339, 4, 9, 4);
	ReplaceMcr(339, 5, 9, 5);
	SetMcr(339, 6, 9, 6);
	SetMcr(339, 7, 9, 7);

	ReplaceMcr(340, 0, 8, 0);
	SetMcr(340, 1, 322, 1);
	SetMcr(340, 2, 14, 2);
	SetMcr(340, 3, 322, 3);
	ReplaceMcr(340, 4, 14, 4);
	ReplaceMcr(340, 5, 5, 5);
	ReplaceMcr(340, 6, 14, 6);
	SetMcr(340, 7, 5, 7);

	SetMcr(342, 0, 8, 0);
	SetMcr(342, 1, 322, 1);
	SetMcr(342, 2, 24, 2);
	SetMcr(342, 3, 322, 3);
	ReplaceMcr(342, 4, 24, 4);
	SetMcr(342, 5, 24, 5);
	SetMcr(342, 6, 1, 6);
	SetMcr(342, 7, 24, 7);

	ReplaceMcr(343, 0, 65, 0);
	ReplaceMcr(343, 1, 322, 1);
	SetMcr(343, 2, 65, 2);
	ReplaceMcr(343, 3, 322, 3);
	ReplaceMcr(343, 4, 65, 4);
	ReplaceMcr(343, 5, 5, 5);
	SetMcr(343, 6, 65, 6);
	ReplaceMcr(343, 7, 9, 7);

	SetMcr(344, 0, 299, 0);
	ReplaceMcr(344, 1, 302, 1);
	ReplaceMcr(330, 1, 7, 1);
	// - special subtiles for the banner setpiece
	SetFrameType(336, 0, MET_TRANSPARENT);
	SetFrameType(336, 1, MET_RTRIANGLE);
	SetMcr(336, 2, 112, 2);
	Blk2Mcr(336, 3);
	SetMcr(336, 4, 112, 4);
	HideMcr(336, 5);
	HideMcr(336, 7);
	ReplaceMcr(337, 0, 110, 0);
	ReplaceMcr(337, 1, 324, 1);
	SetMcr(337, 2, 110, 2);
	ReplaceMcr(337, 3, 110, 3);
	SetMcr(337, 4, 110, 4);
	SetMcr(337, 5, 110, 5);
	HideMcr(337, 7);
	ReplaceMcr(338, 0, 118, 0);
	SetFrameType(338, 1, MET_TRANSPARENT);
	SetMcr(338, 2, 118, 2);
	ReplaceMcr(338, 3, 118, 3);
	SetMcr(338, 4, 118, 4);
	SetMcr(338, 5, 118, 5);
	HideMcr(338, 7);
	// - special subtile for the vile setmap
	ReplaceMcr(335, 0, 8, 0);
	ReplaceMcr(335, 1, 10, 1);
	ReplaceMcr(335, 2, 29, 2);
	ReplaceMcr(335, 4, 29, 4);
	SetMcr(335, 6, 29, 6);
	SetMcr(335, 3, 29, 3);
	SetMcr(335, 5, 29, 5);
	SetMcr(335, 7, 29, 7);
	// subtile to make the inner tile at the entrance non-walkable II.
	Blk2Mcr(425, 0);
	ReplaceMcr(425, 1, 299, 1);
	// subtile for the separate pillar tile
	ReplaceMcr(61, 0, 8, 0);
	SetMcr(61, 1, 8, 1);
	ReplaceMcr(61, 2, 8, 2);
	SetMcr(61, 3, 8, 3);
	ReplaceMcr(61, 4, 8, 4);
	SetMcr(61, 5, 8, 5);
	SetMcr(61, 6, 8, 6);
	SetMcr(61, 7, 8, 7);
	// pointless door micros (re-drawn by dSpecial or the object)
	// - vertical doors	
	ReplaceMcr(392, 4, 231, 4);
	ReplaceMcr(407, 4, 231, 4);
	Blk2Mcr(214, 6);
	Blk2Mcr(214, 4);
	Blk2Mcr(214, 2);
	ReplaceMcr(214, 0, 408, 0);
	ReplaceMcr(214, 1, 408, 1);
	ReplaceMcr(213, 0, 408, 0);
	SetMcr(213, 1, 408, 1);
	// ReplaceMcr(212, 0, 407, 0);
	// ReplaceMcr(212, 2, 392, 2);
	// ReplaceMcr(212, 4, 231, 4);
	Blk2Mcr(408, 2);
	Blk2Mcr(408, 4);
	ReplaceMcr(44, 0, 7, 0);
	ReplaceMcr(44, 1, 7, 1);
	Blk2Mcr(44, 2);
	Blk2Mcr(44, 4);
	HideMcr(44, 6);
	ReplaceMcr(43, 0, 7, 0);
	ReplaceMcr(43, 1, 7, 1);
	Blk2Mcr(43, 2);
	Blk2Mcr(43, 4);
	HideMcr(43, 6);
	ReplaceMcr(393, 0, 7, 0);
	ReplaceMcr(393, 1, 7, 1);
	Blk2Mcr(393, 2);
	Blk2Mcr(393, 4);
	// ReplaceMcr(43, 0, 392, 0);
	// ReplaceMcr(43, 2, 392, 2);
	// ReplaceMcr(43, 4, 231, 4);
	// HideMcr(51, 6);
	// Blk2Mcr(51, 4);
	// Blk2Mcr(51, 2);
	// ReplaceMcr(51, 0, 7, 0);
	// ReplaceMcr(51, 1, 7, 1);
	// - horizontal doors
	ReplaceMcr(394, 5, 5, 5);
	Blk2Mcr(395, 3);
	Blk2Mcr(395, 5);
	ReplaceMcr(395, 1, 11, 1);
	ReplaceMcr(395, 0, 11, 0);
	ReplaceMcr(46, 0, 11, 0);
	ReplaceMcr(46, 1, 11, 1);
	Blk2Mcr(46, 3);
	Blk2Mcr(46, 5);
	HideMcr(46, 7);
	ReplaceMcr(45, 0, 11, 0);
	ReplaceMcr(45, 1, 11, 1);
	Blk2Mcr(45, 3);
	Blk2Mcr(45, 5);
	HideMcr(45, 7);
	// ReplaceMcr(45, 1, 394, 1); // lost details
	// ReplaceMcr(45, 3, 394, 3); // lost details
	// ReplaceMcr(45, 5, 5, 5);
	ReplaceMcr(72, 1, 394, 1);
	ReplaceMcr(72, 3, 394, 3);
	ReplaceMcr(72, 5, 5, 5);
	// useless black micros
	Blk2Mcr(107, 0);
	Blk2Mcr(107, 1);
	Blk2Mcr(109, 1);
	Blk2Mcr(137, 1);
	Blk2Mcr(138, 0);
	Blk2Mcr(138, 1);
	Blk2Mcr(140, 1);
	// pointless pixels
	// Blk2Mcr(241, 0);
	// // Blk2Mcr(250, 3);
	Blk2Mcr(148, 4);
	Blk2Mcr(190, 3);
	Blk2Mcr(190, 5);
	Blk2Mcr(247, 2);
	Blk2Mcr(247, 6);
	Blk2Mcr(426, 0);
	HideMcr(427, 1);
	// Blk2Mcr(428, 0);
	// Blk2Mcr(428, 1);
	// - pwater column
	ReplaceMcr(171, 6, 37, 6);
	ReplaceMcr(171, 7, 37, 7);
	Blk2Mcr(171, 4);
	Blk2Mcr(171, 5);
	ReplaceMcr(171, 3, 176, 3); // lost details
	// fix graphical glitch
	// ReplaceMcr(15, 1, 6, 1);
	ReplaceMcr(134, 1, 6, 1);
	ReplaceMcr(65, 7, 9, 7);
	ReplaceMcr(66, 7, 9, 7);
	// ReplaceMcr(68, 7, 9, 7);
	// ReplaceMcr(69, 7, 9, 7);
	ReplaceMcr(179, 7, 28, 7);
	ReplaceMcr(247, 3, 5, 3);
	ReplaceMcr(258, 1, 8, 1);
	// reuse subtiles
	ReplaceMcr(139, 6, 3, 6);
	ReplaceMcr(206, 6, 3, 6);
	ReplaceMcr(208, 6, 3, 6);
	// ReplaceMcr(214, 6, 3, 6);
	ReplaceMcr(228, 6, 3, 6);
	ReplaceMcr(232, 6, 3, 6);
	ReplaceMcr(236, 6, 3, 6);
	ReplaceMcr(240, 6, 3, 6);
	ReplaceMcr(257, 6, 3, 6);
	ReplaceMcr(261, 6, 3, 6);
	ReplaceMcr(269, 6, 3, 6);
	ReplaceMcr(320, 6, 3, 6);
	ReplaceMcr(358, 6, 3, 6);
	ReplaceMcr(362, 6, 3, 6);
	ReplaceMcr(366, 6, 3, 6);
	ReplaceMcr(32, 4, 39, 4);
	// ReplaceMcr(439, 4, 39, 4);
	ReplaceMcr(228, 4, 232, 4); // lost details
	ReplaceMcr(206, 4, 3, 4);   // lost details
	ReplaceMcr(208, 4, 3, 4);   // lost details
	ReplaceMcr(240, 4, 3, 4);   // lost details
	ReplaceMcr(257, 4, 3, 4);
	ReplaceMcr(261, 4, 3, 4);   // lost details
	ReplaceMcr(320, 4, 3, 4);   // lost details
	ReplaceMcr(261, 2, 240, 2); // lost details
	ReplaceMcr(208, 2, 3, 2);   // lost details
	// ReplaceMcr(63, 0, 53, 0);
	// ReplaceMcr(49, 0, 3, 0);
	ReplaceMcr(206, 0, 3, 0);
	ReplaceMcr(208, 0, 3, 0); // lost details
	ReplaceMcr(240, 0, 3, 0);
	// ReplaceMcr(63, 1, 53, 1);
	// ReplaceMcr(58, 1, 53, 1);
	ReplaceMcr(206, 1, 3, 1);
	// ReplaceMcr(15, 7, 6, 7); // lost details
	// ReplaceMcr(56, 7, 6, 7); // lost details
	// ReplaceMcr(60, 7, 6, 7); // lost details
	ReplaceMcr(127, 7, 6, 7);
	ReplaceMcr(134, 7, 6, 7); // lost details
	ReplaceMcr(138, 7, 6, 7);
	ReplaceMcr(198, 7, 6, 7);
	ReplaceMcr(202, 7, 6, 7);
	// ReplaceMcr(204, 7, 6, 7);
	ReplaceMcr(230, 7, 6, 7);
	ReplaceMcr(234, 7, 6, 7);
	ReplaceMcr(238, 7, 6, 7);
	// ReplaceMcr(242, 7, 6, 7);
	ReplaceMcr(244, 7, 6, 7);
	ReplaceMcr(246, 7, 6, 7);
	// ReplaceMcr(251, 7, 6, 7);
	ReplaceMcr(323, 7, 6, 7);
	ReplaceMcr(333, 7, 6, 7);
	ReplaceMcr(365, 7, 6, 7);
	ReplaceMcr(369, 7, 6, 7);
	ReplaceMcr(373, 7, 6, 7);

	// ReplaceMcr(15, 5, 6, 5);
	// ReplaceMcr(46, 5, 6, 5);
	// ReplaceMcr(56, 5, 6, 5);
	ReplaceMcr(127, 5, 6, 5);
	ReplaceMcr(134, 5, 6, 5);
	ReplaceMcr(198, 5, 6, 5);
	ReplaceMcr(202, 5, 6, 5);
	// ReplaceMcr(204, 5, 6, 5);
	ReplaceMcr(230, 5, 6, 5); // lost details
	ReplaceMcr(234, 5, 6, 5); // lost details
	// ReplaceMcr(242, 5, 6, 5);
	ReplaceMcr(244, 5, 6, 5);
	ReplaceMcr(246, 5, 6, 5);
	// ReplaceMcr(251, 5, 6, 5);
	ReplaceMcr(323, 5, 6, 5);
	ReplaceMcr(333, 5, 6, 5);
	// ReplaceMcr(416, 5, 6, 5);
	ReplaceMcr(6, 3, 15, 3);
	// ReplaceMcr(204, 3, 15, 3);
	// ReplaceMcr(242, 3, 15, 3);
	ReplaceMcr(244, 3, 15, 3); // lost details
	ReplaceMcr(246, 3, 15, 3); // lost details
	// ReplaceMcr(251, 3, 15, 3);
	// ReplaceMcr(416, 3, 15, 3);
	// ReplaceMcr(15, 1, 6, 1);
	ReplaceMcr(134, 1, 6, 1);
	ReplaceMcr(198, 1, 6, 1);
	ReplaceMcr(202, 1, 6, 1);
	ReplaceMcr(323, 1, 6, 1);
	// ReplaceMcr(416, 1, 6, 1);
	// ReplaceMcr(15, 0, 6, 0);

	ReplaceMcr(249, 1, 11, 1);
	ReplaceMcr(325, 1, 11, 1);
	// ReplaceMcr(344, 1, 11, 1);
	// ReplaceMcr(402, 1, 11, 1);
	ReplaceMcr(308, 0, 11, 0);
	ReplaceMcr(308, 1, 11, 1);

	// ReplaceMcr(180, 6, 8, 6);
	ReplaceMcr(178, 6, 14, 6);
	ReplaceMcr(10, 6, 1, 6);
	ReplaceMcr(13, 6, 1, 6);
	ReplaceMcr(16, 6, 1, 6);
	ReplaceMcr(21, 6, 1, 6);
	ReplaceMcr(24, 6, 1, 6);
	ReplaceMcr(41, 6, 1, 6);
	// ReplaceMcr(54, 6, 1, 6);
	ReplaceMcr(57, 6, 1, 6);
	// ReplaceMcr(70, 6, 1, 6);
	ReplaceMcr(73, 6, 1, 6);
	ReplaceMcr(137, 6, 1, 6);
	ReplaceMcr(176, 6, 1, 6);
	ReplaceMcr(205, 6, 1, 6);
	ReplaceMcr(207, 6, 1, 6);
	ReplaceMcr(209, 6, 1, 6);
	// ReplaceMcr(212, 6, 1, 6);
	ReplaceMcr(227, 6, 1, 6);
	ReplaceMcr(231, 6, 1, 6);
	ReplaceMcr(235, 6, 1, 6);
	ReplaceMcr(239, 6, 1, 6);
	ReplaceMcr(254, 6, 1, 6);
	ReplaceMcr(256, 6, 1, 6);
	ReplaceMcr(258, 6, 1, 6);
	ReplaceMcr(260, 6, 1, 6);
	ReplaceMcr(319, 6, 1, 6);
	ReplaceMcr(356, 6, 1, 6);
	ReplaceMcr(360, 6, 1, 6);
	ReplaceMcr(364, 6, 1, 6);
	ReplaceMcr(392, 6, 1, 6);
	ReplaceMcr(407, 6, 1, 6);
	ReplaceMcr(417, 6, 1, 6);

	ReplaceMcr(16, 4, 10, 4);
	ReplaceMcr(209, 4, 10, 4);
	ReplaceMcr(37, 4, 34, 4);
	ReplaceMcr(42, 4, 34, 4);
	ReplaceMcr(30, 4, 38, 4);
	// ReplaceMcr(437, 4, 38, 4);
	ReplaceMcr(62, 4, 52, 4);
	// ReplaceMcr(171, 4, 28, 4);
	// ReplaceMcr(180, 4, 28, 4);
	ReplaceMcr(133, 4, 14, 4);
	ReplaceMcr(176, 4, 167, 4);
	ReplaceMcr(13, 4, 1, 4);
	ReplaceMcr(205, 4, 1, 4);
	ReplaceMcr(207, 4, 1, 4);
	ReplaceMcr(239, 4, 1, 4);
	ReplaceMcr(256, 4, 1, 4);
	ReplaceMcr(260, 4, 1, 4);
	ReplaceMcr(319, 4, 1, 4);
	ReplaceMcr(356, 4, 1, 4);
	ReplaceMcr(227, 4, 231, 4);
	ReplaceMcr(235, 4, 231, 4);
	ReplaceMcr(13, 2, 1, 2);
	ReplaceMcr(207, 2, 1, 2);
	ReplaceMcr(256, 2, 1, 2);
	ReplaceMcr(319, 2, 1, 2);
	// ReplaceMcr(179, 2, 28, 2);
	ReplaceMcr(16, 2, 10, 2);
	ReplaceMcr(254, 2, 10, 2);
	// ReplaceMcr(426, 0, 23, 0);
	ReplaceMcr(10, 0, 8, 0); // TODO: 203 would be better?
	ReplaceMcr(14, 0, 8, 0);
	ReplaceMcr(16, 0, 8, 0);
	ReplaceMcr(17, 0, 8, 0);
	ReplaceMcr(21, 0, 8, 0);
	ReplaceMcr(24, 0, 8, 0);
	ReplaceMcr(25, 0, 8, 0);
	// ReplaceMcr(55, 0, 8, 0);
	ReplaceMcr(59, 0, 8, 0);
	// ReplaceMcr(70, 0, 8, 0);
	ReplaceMcr(73, 0, 8, 0);
	ReplaceMcr(178, 0, 8, 0);
	ReplaceMcr(203, 0, 8, 0);
	ReplaceMcr(5, 0, 8, 0); // TODO: triangle begin?
	ReplaceMcr(22, 0, 8, 0);
	// ReplaceMcr(45, 0, 8, 0);
	ReplaceMcr(64, 0, 8, 0);
	ReplaceMcr(201, 0, 8, 0);
	ReplaceMcr(229, 0, 8, 0);
	ReplaceMcr(233, 0, 8, 0);
	ReplaceMcr(237, 0, 8, 0);
	ReplaceMcr(243, 0, 8, 0);
	ReplaceMcr(245, 0, 8, 0);
	ReplaceMcr(247, 0, 8, 0);
	ReplaceMcr(322, 0, 8, 0);
	ReplaceMcr(324, 0, 8, 0);
	ReplaceMcr(394, 0, 8, 0);
	ReplaceMcr(420, 0, 8, 0); // TODO: triangle end?

	ReplaceMcr(52, 0, 57, 0);
	ReplaceMcr(66, 0, 57, 0);
	// ReplaceMcr(67, 0, 57, 0);
	ReplaceMcr(13, 0, 1, 0);
	ReplaceMcr(319, 0, 1, 0);

	// ReplaceMcr(168, 0, 167, 0);
	// ReplaceMcr(168, 1, 167, 1);
	// ReplaceMcr(168, 3, 167, 2); // lost details
	// ReplaceMcr(191, 0, 167, 0); // lost details
	// ReplaceMcr(191, 1, 167, 1); // lost details
	ReplaceMcr(175, 1, 167, 1); // lost details
	ReplaceMcr(177, 0, 167, 0); // lost details
	ReplaceMcr(177, 1, 167, 1); // lost details
	ReplaceMcr(177, 2, 167, 2); // lost details
	// ReplaceMcr(177, 4, 168, 4); // lost details
	ReplaceMcr(175, 4, 177, 4);
	// ReplaceMcr(191, 2, 177, 2); // lost details
	// ReplaceMcr(191, 4, 177, 4);

	// ReplaceMcr(181, 0, 169, 0);

	ReplaceMcr(170, 0, 169, 0);
	ReplaceMcr(170, 1, 169, 1);
	ReplaceMcr(170, 3, 169, 3); // lost details
	// ReplaceMcr(178, 1, 172, 1); // lost details
	// ReplaceMcr(172, 0, 190, 0);
	ReplaceMcr(176, 2, 190, 2);
	ReplaceMcr(176, 0, 190, 0); // lost details

	ReplaceMcr(176, 7, 13, 7);
	// ReplaceMcr(171, 7, 8, 7);
	ReplaceMcr(10, 7, 9, 7);
	ReplaceMcr(20, 7, 9, 7);
	ReplaceMcr(364, 7, 9, 7);
	ReplaceMcr(14, 7, 5, 7);
	ReplaceMcr(17, 7, 5, 7);
	ReplaceMcr(22, 7, 5, 7);
	ReplaceMcr(42, 7, 5, 7);
	// ReplaceMcr(55, 7, 5, 7);
	ReplaceMcr(59, 7, 5, 7);
	ReplaceMcr(133, 7, 5, 7);
	ReplaceMcr(178, 7, 5, 7);
	ReplaceMcr(197, 7, 5, 7);
	ReplaceMcr(201, 7, 5, 7);
	ReplaceMcr(203, 7, 5, 7);
	ReplaceMcr(229, 7, 5, 7);
	ReplaceMcr(233, 7, 5, 7);
	ReplaceMcr(237, 7, 5, 7);
	ReplaceMcr(241, 7, 5, 7);
	ReplaceMcr(243, 7, 5, 7);
	ReplaceMcr(245, 7, 5, 7);
	ReplaceMcr(247, 7, 5, 7);
	ReplaceMcr(248, 7, 5, 7);
	ReplaceMcr(322, 7, 5, 7);
	ReplaceMcr(324, 7, 5, 7);
	ReplaceMcr(368, 7, 5, 7);
	ReplaceMcr(372, 7, 5, 7);
	ReplaceMcr(394, 7, 5, 7);
	ReplaceMcr(420, 7, 5, 7);

	ReplaceMcr(37, 5, 30, 5);
	ReplaceMcr(41, 5, 30, 5);
	ReplaceMcr(42, 5, 34, 5);
	ReplaceMcr(59, 5, 47, 5);
	ReplaceMcr(64, 5, 47, 5);
	ReplaceMcr(178, 5, 169, 5);

	ReplaceMcr(17, 5, 10, 5);
	ReplaceMcr(22, 5, 10, 5);
	ReplaceMcr(66, 5, 10, 5);
	// ReplaceMcr(68, 5, 10, 5);
	ReplaceMcr(248, 5, 10, 5);
	ReplaceMcr(324, 5, 10, 5);
	// ReplaceMcr(180, 5, 29, 5);
	// ReplaceMcr(179, 5, 171, 5);
	ReplaceMcr(14, 5, 5, 5); // TODO: 243 would be better?
	// ReplaceMcr(45, 5, 5, 5);
	// ReplaceMcr(50, 5, 5, 5);
	// ReplaceMcr(55, 5, 5, 5);
	ReplaceMcr(65, 5, 5, 5);
	// ReplaceMcr(67, 5, 5, 5);
	// ReplaceMcr(69, 5, 5, 5);
	// ReplaceMcr(70, 5, 5, 5);
	// ReplaceMcr(72, 5, 5, 5);
	ReplaceMcr(133, 5, 5, 5);
	ReplaceMcr(197, 5, 5, 5);
	ReplaceMcr(201, 5, 5, 5);
	ReplaceMcr(203, 5, 5, 5);
	ReplaceMcr(229, 5, 5, 5);
	ReplaceMcr(233, 5, 5, 5);
	ReplaceMcr(237, 5, 5, 5);
	ReplaceMcr(241, 5, 5, 5);
	ReplaceMcr(243, 5, 5, 5);
	ReplaceMcr(245, 5, 5, 5);
	ReplaceMcr(322, 5, 5, 5);
	ReplaceMcr(52, 3, 47, 3);
	ReplaceMcr(59, 3, 47, 3);
	ReplaceMcr(64, 3, 47, 3);
	ReplaceMcr(73, 3, 47, 3);
	ReplaceMcr(17, 3, 10, 3);
	ReplaceMcr(20, 3, 10, 3);
	ReplaceMcr(22, 3, 10, 3);
	// ReplaceMcr(68, 3, 10, 3);
	ReplaceMcr(324, 3, 10, 3);
	ReplaceMcr(57, 3, 13, 3);
	// ReplaceMcr(180, 3, 29, 3);
	ReplaceMcr(9, 3, 5, 3);
	ReplaceMcr(14, 3, 5, 3);
	ReplaceMcr(24, 3, 5, 3);
	ReplaceMcr(65, 3, 5, 3);
	ReplaceMcr(133, 3, 5, 3); // lost details
	ReplaceMcr(245, 3, 5, 3); // lost details
	ReplaceMcr(52, 1, 47, 1);
	ReplaceMcr(59, 1, 47, 1);
	ReplaceMcr(64, 1, 47, 1);
	ReplaceMcr(73, 1, 47, 1);
	ReplaceMcr(17, 1, 10, 1);
	ReplaceMcr(20, 1, 10, 1);
	// ReplaceMcr(68, 1, 10, 1);
	ReplaceMcr(9, 1, 5, 1);
	ReplaceMcr(14, 1, 5, 1);
	ReplaceMcr(24, 1, 5, 1);
	ReplaceMcr(65, 1, 5, 1);
	ReplaceMcr(243, 1, 5, 1); // lost details
	ReplaceMcr(247, 1, 5, 1);
	ReplaceMcr(13, 1, 176, 1); // lost details
	ReplaceMcr(16, 1, 176, 1); // lost details
	// ReplaceMcr(54, 1, 176, 1); // lost details
	ReplaceMcr(57, 1, 176, 1);  // lost details
	ReplaceMcr(190, 1, 176, 1); // lost details
	// ReplaceMcr(397, 1, 176, 1); // lost details
	ReplaceMcr(25, 1, 8, 1);
	ReplaceMcr(110, 1, 8, 1); // lost details
	ReplaceMcr(1, 1, 8, 1);   // lost details TODO: triangle begin?
	ReplaceMcr(21, 1, 8, 1);  // lost details
	// ReplaceMcr(43, 1, 8, 1);  // lost details
	ReplaceMcr(62, 1, 8, 1);  // lost details
	ReplaceMcr(205, 1, 8, 1); // lost details
	ReplaceMcr(207, 1, 8, 1); // lost details
	// ReplaceMcr(212, 1, 8, 1); // lost details 
	ReplaceMcr(407, 1, 8, 1); // lost details 
	ReplaceMcr(227, 1, 8, 1); // lost details
	ReplaceMcr(231, 1, 8, 1); // lost details
	ReplaceMcr(235, 1, 8, 1); // lost details
	ReplaceMcr(239, 1, 8, 1); // lost details
	ReplaceMcr(254, 1, 8, 1); // lost details
	ReplaceMcr(256, 1, 8, 1); // lost details
	ReplaceMcr(260, 1, 8, 1); // lost details
	// ReplaceMcr(266, 1, 8, 1); // lost details
	ReplaceMcr(319, 1, 8, 1); // lost details
	ReplaceMcr(392, 1, 8, 1); // lost details
	// ReplaceMcr(413, 1, 8, 1); // lost details
	ReplaceMcr(417, 1, 8, 1); // lost details
	// ReplaceMcr(429, 1, 8, 1); // lost details TODO: triangle end?

	// ReplaceMcr(122, 0, 23, 0);
	// ReplaceMcr(122, 1, 23, 1);
	// ReplaceMcr(124, 0, 23, 0);
	ReplaceMcr(141, 0, 23, 0);
	// ReplaceMcr(220, 0, 23, 0);
	// ReplaceMcr(220, 1, 23, 1);
	// ReplaceMcr(293, 0, 23, 0); // lost details
	// ReplaceMcr(300, 0, 23, 0);
	// ReplaceMcr(309, 0, 23, 0);
	// ReplaceMcr(329, 0, 23, 0);
	// ReplaceMcr(329, 1, 23, 1);
	// ReplaceMcr(312, 0, 23, 0);
	// ReplaceMcr(275, 0, 23, 0);
	// ReplaceMcr(275, 1, 23, 1);
	// ReplaceMcr(282, 0, 23, 0);
	// ReplaceMcr(282, 1, 23, 1);
	ReplaceMcr(296, 0, 23, 0);
	// ReplaceMcr(303, 0, 23, 0);
	// ReplaceMcr(303, 1, 296, 1);
	ReplaceMcr(307, 0, 23, 0);
	ReplaceMcr(315, 1, 23, 1);
	// ReplaceMcr(326, 0, 23, 0);
	// ReplaceMcr(327, 0, 23, 0);
	// ReplaceMcr(111, 0, 2, 0);
	// ReplaceMcr(119, 0, 2, 0);
	// ReplaceMcr(119, 1, 2, 1);
	// ReplaceMcr(213, 0, 2, 0); // lost details
	// ReplaceMcr(271, 0, 2, 0);
	// ReplaceMcr(276, 0, 2, 0);
	// ReplaceMcr(279, 0, 2, 0);
	// ReplaceMcr(285, 0, 2, 0);
	// ReplaceMcr(290, 0, 2, 0);
	// ReplaceMcr(316, 0, 2, 0);
	// ReplaceMcr(316, 1, 2, 1);
	ReplaceMcr(12, 0, 7, 0);
	ReplaceMcr(12, 1, 7, 1);
	// ReplaceMcr(18, 0, 7, 0);
	// ReplaceMcr(18, 1, 7, 1);
	// ReplaceMcr(71, 0, 7, 0);
	// ReplaceMcr(71, 1, 7, 1);
	// ReplaceMcr(341, 0, 7, 0);
	// ReplaceMcr(341, 1, 7, 1);
	// ReplaceMcr(405, 0, 7, 0);
	// ReplaceMcr(405, 1, 7, 1);
	// ReplaceMcr(116, 1, 7, 1);
	// ReplaceMcr(120, 0, 7, 0);
	// ReplaceMcr(120, 1, 7, 1);
	// ReplaceMcr(125, 0, 7, 0);
	// ReplaceMcr(125, 1, 7, 1);
	ReplaceMcr(157, 1, 7, 1);
	ReplaceMcr(211, 0, 7, 0);
	ReplaceMcr(222, 0, 7, 0); // lost details
	ReplaceMcr(226, 0, 7, 0);
	ReplaceMcr(259, 0, 7, 0);
	// ReplaceMcr(272, 1, 7, 1);
	// ReplaceMcr(273, 0, 7, 0);
	// ReplaceMcr(273, 1, 7, 1);
	// ReplaceMcr(291, 1, 7, 1);
	ReplaceMcr(452, 0, 7, 0);
	ReplaceMcr(145, 1, 147, 1);
	// ReplaceMcr(19, 0, 4, 0);
	// ReplaceMcr(19, 1, 4, 1);
	// ReplaceMcr(113, 0, 4, 0);
	// ReplaceMcr(113, 1, 4, 1);
	// ReplaceMcr(117, 0, 4, 0);
	// ReplaceMcr(117, 1, 4, 1);
	// ReplaceMcr(121, 0, 4, 0);
	// ReplaceMcr(121, 1, 4, 1);
	// ReplaceMcr(158, 1, 4, 1);
	// ReplaceMcr(161, 0, 4, 0);
	// ReplaceMcr(200, 0, 4, 0);
	// ReplaceMcr(200, 1, 4, 1);
	ReplaceMcr(215, 1, 4, 1);
	// ReplaceMcr(277, 1, 4, 1);
	// ReplaceMcr(295, 0, 4, 0);
	// ReplaceMcr(318, 1, 4, 1);
	// ReplaceMcr(419, 0, 4, 0);
	ReplaceMcr(321, 0, 301, 0);
	ReplaceMcr(321, 1, 301, 1);
	// ReplaceMcr(305, 0, 298, 0);
	// ReplaceMcr(332, 1, 306, 1);

	// ReplaceMcr(168, 2, 167, 2); // lost details

	// ReplaceMcr(400, 6, 1, 6);
	// ReplaceMcr(406, 6, 1, 6);
	// ReplaceMcr(410, 6, 1, 6);

	// eliminate micros of unused subtiles
	// Blk2Mcr(39, 311 ...),
	Blk2Mcr(15, 0);
	Blk2Mcr(15, 1);
	Blk2Mcr(15, 5);
	Blk2Mcr(15, 7);
	Blk2Mcr(49, 0);
	Blk2Mcr(50, 0);
	Blk2Mcr(50, 1);
	Blk2Mcr(50, 2);
	Blk2Mcr(50, 3);
	Blk2Mcr(50, 4);
	Blk2Mcr(50, 5);
	Blk2Mcr(51, 0);
	Blk2Mcr(51, 1);
	Blk2Mcr(51, 2);
	Blk2Mcr(51, 4);
	Blk2Mcr(54, 0);
	Blk2Mcr(54, 1);
	Blk2Mcr(54, 2);
	Blk2Mcr(54, 4);
	Blk2Mcr(54, 6);
	Blk2Mcr(55, 0);
	Blk2Mcr(55, 1);
	Blk2Mcr(55, 3);
	Blk2Mcr(55, 5);
	Blk2Mcr(55, 7);
	Blk2Mcr(56, 0);
	Blk2Mcr(56, 1);
	Blk2Mcr(56, 3);
	Blk2Mcr(56, 5);
	Blk2Mcr(56, 7);
	Blk2Mcr(58, 1);
	Blk2Mcr(60, 7);
	// Blk2Mcr(61, 0);
	// Blk2Mcr(61, 2);
	// Blk2Mcr(61, 4);
	Blk2Mcr(63, 0);
	Blk2Mcr(63, 1);
	Blk2Mcr(67, 0);
	Blk2Mcr(67, 1);
	Blk2Mcr(67, 3);
	Blk2Mcr(67, 5);
	Blk2Mcr(68, 0);
	Blk2Mcr(68, 1);
	Blk2Mcr(68, 2);
	Blk2Mcr(68, 3);
	Blk2Mcr(68, 4);
	Blk2Mcr(68, 5);
	Blk2Mcr(68, 7);
	Blk2Mcr(69, 0);
	Blk2Mcr(69, 1);
	Blk2Mcr(69, 2);
	Blk2Mcr(69, 3);
	Blk2Mcr(69, 4);
	Blk2Mcr(69, 5);
	Blk2Mcr(69, 7);
	Blk2Mcr(70, 0);
	Blk2Mcr(70, 1);
	Blk2Mcr(70, 3);
	Blk2Mcr(70, 5);
	Blk2Mcr(70, 6);
	Blk2Mcr(204, 3);
	Blk2Mcr(204, 5);
	Blk2Mcr(204, 7);
	Blk2Mcr(242, 3);
	Blk2Mcr(242, 5);
	Blk2Mcr(242, 7);
	Blk2Mcr(354, 0);
	Blk2Mcr(354, 2);
	Blk2Mcr(354, 4);
	Blk2Mcr(355, 1);
	Blk2Mcr(355, 3);
	Blk2Mcr(355, 5);
	Blk2Mcr(411, 1);
	Blk2Mcr(411, 3);
	Blk2Mcr(411, 5);
	Blk2Mcr(412, 0);
	Blk2Mcr(412, 2);
	Blk2Mcr(412, 4);

	Blk2Mcr(124, 0);
	Blk2Mcr(111, 0);
	Blk2Mcr(116, 1);
	Blk2Mcr(158, 1);
	Blk2Mcr(161, 0);
	Blk2Mcr(271, 0);
	Blk2Mcr(272, 1);
	Blk2Mcr(277, 1);
	Blk2Mcr(285, 0);
	Blk2Mcr(290, 0);
	Blk2Mcr(291, 1);
	Blk2Mcr(293, 0);
	Blk2Mcr(295, 0);
	Blk2Mcr(300, 0);
	Blk2Mcr(309, 0);
	Blk2Mcr(312, 0);
	Blk2Mcr(326, 0);
	Blk2Mcr(327, 0);
	Blk2Mcr(419, 0);

	Blk2Mcr(168, 0);
	Blk2Mcr(168, 1);
	Blk2Mcr(168, 2);
	Blk2Mcr(168, 4);
	Blk2Mcr(172, 0);
	Blk2Mcr(172, 1);
	Blk2Mcr(172, 2);
	Blk2Mcr(172, 3);
	Blk2Mcr(172, 4);
	Blk2Mcr(172, 5);
	Blk2Mcr(173, 0);
	Blk2Mcr(173, 1);
	Blk2Mcr(173, 3);
	Blk2Mcr(173, 5);
	Blk2Mcr(174, 0);
	Blk2Mcr(174, 1);
	Blk2Mcr(174, 2);
	Blk2Mcr(174, 4);
	Blk2Mcr(179, 1);
	Blk2Mcr(179, 2);
	Blk2Mcr(179, 3);
	Blk2Mcr(179, 5);
	Blk2Mcr(180, 3);
	Blk2Mcr(180, 4);
	Blk2Mcr(180, 5);
	Blk2Mcr(180, 6);
	Blk2Mcr(181, 0);
	Blk2Mcr(181, 1);
	Blk2Mcr(181, 3);
	Blk2Mcr(182, 0);
	Blk2Mcr(182, 1);
	Blk2Mcr(182, 2);
	Blk2Mcr(182, 4);
	Blk2Mcr(183, 0);
	Blk2Mcr(183, 1);
	Blk2Mcr(183, 2);
	Blk2Mcr(183, 4);
	Blk2Mcr(184, 0);
	Blk2Mcr(184, 2);
	Blk2Mcr(184, 4);
	Blk2Mcr(185, 0);
	Blk2Mcr(185, 1);
	Blk2Mcr(185, 2);
	Blk2Mcr(185, 4);
	Blk2Mcr(186, 1);
	Blk2Mcr(186, 3);
	Blk2Mcr(186, 5);
	Blk2Mcr(187, 0);
	Blk2Mcr(187, 1);
	Blk2Mcr(187, 3);
	Blk2Mcr(187, 5);
	Blk2Mcr(188, 0);
	Blk2Mcr(188, 1);
	Blk2Mcr(188, 3);
	Blk2Mcr(188, 5);
	Blk2Mcr(189, 0);
	Blk2Mcr(189, 1);
	Blk2Mcr(189, 3);
	Blk2Mcr(191, 0);
	Blk2Mcr(191, 1);
	Blk2Mcr(191, 2);
	Blk2Mcr(191, 4);
	Blk2Mcr(194, 1);
	Blk2Mcr(194, 3);
	Blk2Mcr(194, 5);
	Blk2Mcr(195, 0);
	Blk2Mcr(195, 1);
	Blk2Mcr(195, 3);
	Blk2Mcr(195, 5);
	Blk2Mcr(196, 0);
	Blk2Mcr(196, 2);
	Blk2Mcr(196, 4);
	Blk2Mcr(196, 5);
	// reused micros in fixCathedralShadows
	// Blk2Mcr(330, 0);
	// Blk2Mcr(330, 1);
	// Blk2Mcr(334, 0);
	// Blk2Mcr(334, 1);
	// Blk2Mcr(334, 2);
	// Blk2Mcr(335, 0);
	// Blk2Mcr(335, 1);
	// Blk2Mcr(335, 2);
	// Blk2Mcr(335, 4);
	// Blk2Mcr(336, 0);
	// Blk2Mcr(336, 1);
	// Blk2Mcr(336, 3);
	// Blk2Mcr(337, 0);
	// Blk2Mcr(337, 1);
	// Blk2Mcr(337, 3);
	// Blk2Mcr(338, 0);
	// Blk2Mcr(338, 1);
	// Blk2Mcr(338, 3);
	// Blk2Mcr(339, 4);
	// Blk2Mcr(339, 5);
	// Blk2Mcr(340, 0);
	// Blk2Mcr(340, 4);
	// Blk2Mcr(340, 5);
	// Blk2Mcr(340, 6);
	// Blk2Mcr(342, 4);
	// Blk2Mcr(343, 0);
	// Blk2Mcr(343, 1);
	// Blk2Mcr(343, 3);
	// Blk2Mcr(343, 4);
	// Blk2Mcr(343, 5);
	// Blk2Mcr(343, 7);
	// Blk2Mcr(344, 1);
	Blk2Mcr(345, 0);
	Blk2Mcr(345, 1);
	Blk2Mcr(345, 2);
	Blk2Mcr(345, 4);
	Blk2Mcr(354, 5);
	Blk2Mcr(355, 4);

	Blk2Mcr(402, 1);

	Blk2Mcr(279, 0);

	Blk2Mcr(251, 0);
	Blk2Mcr(251, 3);
	Blk2Mcr(251, 5);
	Blk2Mcr(251, 7);
	Blk2Mcr(252, 0);
	Blk2Mcr(252, 1);
	Blk2Mcr(252, 3);
	Blk2Mcr(252, 5);
	Blk2Mcr(252, 7);
	Blk2Mcr(266, 0);
	Blk2Mcr(266, 1);
	Blk2Mcr(266, 2);
	Blk2Mcr(266, 4);
	Blk2Mcr(266, 6);
	Blk2Mcr(269, 0);
	Blk2Mcr(269, 1);
	Blk2Mcr(269, 2);
	Blk2Mcr(269, 4);
	Blk2Mcr(274, 0);

	// Blk2Mcr(306, 1);
	Blk2Mcr(314, 0);
	Blk2Mcr(332, 1);
	Blk2Mcr(333, 0);
	Blk2Mcr(333, 1);

	Blk2Mcr(396, 4);
	Blk2Mcr(396, 5);
	Blk2Mcr(396, 6);
	Blk2Mcr(396, 7);
	Blk2Mcr(397, 1);
	Blk2Mcr(397, 4);
	Blk2Mcr(397, 6);
	Blk2Mcr(398, 0);
	Blk2Mcr(398, 5);
	Blk2Mcr(398, 7);
	Blk2Mcr(399, 4);
	Blk2Mcr(399, 6);
	Blk2Mcr(400, 0);
	Blk2Mcr(400, 5);
	Blk2Mcr(400, 6);
	Blk2Mcr(400, 7);
	Blk2Mcr(401, 1);
	Blk2Mcr(401, 3);
	Blk2Mcr(401, 4);
	Blk2Mcr(401, 5);
	Blk2Mcr(401, 6);
	Blk2Mcr(401, 7);
	Blk2Mcr(403, 1);
	Blk2Mcr(403, 3);
	Blk2Mcr(403, 4);
	Blk2Mcr(403, 5);
	Blk2Mcr(403, 6);
	Blk2Mcr(403, 7);
	Blk2Mcr(404, 0);
	Blk2Mcr(404, 5);
	Blk2Mcr(404, 6);
	Blk2Mcr(404, 7);
	Blk2Mcr(406, 5);
	Blk2Mcr(406, 6);
	Blk2Mcr(406, 7);
	Blk2Mcr(409, 4);
	Blk2Mcr(409, 5);
	Blk2Mcr(409, 6);
	Blk2Mcr(410, 4);
	Blk2Mcr(410, 5);
	Blk2Mcr(410, 6);
	Blk2Mcr(410, 7);
	Blk2Mcr(411, 4);
	Blk2Mcr(411, 6);
	Blk2Mcr(412, 5);
	Blk2Mcr(412, 7);
	Blk2Mcr(413, 0);
	Blk2Mcr(413, 1);
	Blk2Mcr(413, 2);
	Blk2Mcr(413, 4);
	Blk2Mcr(414, 0);
	Blk2Mcr(414, 1);
	Blk2Mcr(414, 2);
	Blk2Mcr(414, 4);
	Blk2Mcr(415, 0);
	Blk2Mcr(415, 1);
	Blk2Mcr(415, 3);
	Blk2Mcr(415, 5);
	Blk2Mcr(416, 0);
	Blk2Mcr(416, 1);
	Blk2Mcr(416, 3);
	Blk2Mcr(416, 5);
	Blk2Mcr(429, 0);
	Blk2Mcr(429, 1);
	Blk2Mcr(429, 4);
	Blk2Mcr(429, 6);
	Blk2Mcr(431, 0);
	Blk2Mcr(433, 0);
	Blk2Mcr(433, 1);
	Blk2Mcr(433, 3);
	Blk2Mcr(433, 7);
	Blk2Mcr(434, 0);
	Blk2Mcr(434, 1);

	Blk2Mcr(422, 0);
	Blk2Mcr(422, 1);
	Blk2Mcr(423, 0);
	Blk2Mcr(423, 1);
	Blk2Mcr(424, 0);
	Blk2Mcr(424, 1);
	Blk2Mcr(428, 0);
	Blk2Mcr(428, 1);
	Blk2Mcr(437, 0);
	Blk2Mcr(437, 1);
	Blk2Mcr(437, 4);
	Blk2Mcr(438, 0);
	Blk2Mcr(439, 1);
	Blk2Mcr(439, 4);
	Blk2Mcr(441, 0);
	Blk2Mcr(441, 1);
	Blk2Mcr(442, 0);
	Blk2Mcr(443, 1);
	Blk2Mcr(444, 0);
	Blk2Mcr(444, 1);
	Blk2Mcr(444, 4);
	Blk2Mcr(444, 5);
	Blk2Mcr(445, 0);
	Blk2Mcr(445, 1);
	Blk2Mcr(446, 1);
	Blk2Mcr(447, 0);
	Blk2Mcr(447, 1);
	Blk2Mcr(448, 0);
	Blk2Mcr(448, 1);
	Blk2Mcr(448, 5);
	Blk2Mcr(448, 6);
	Blk2Mcr(449, 0);
	Blk2Mcr(449, 1);
	Blk2Mcr(449, 4);
	Blk2Mcr(449, 5);
	Blk2Mcr(449, 7);
	const int unusedSubtiles[] = {
		18, 19, 71, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 113, 117, 119, 120, 121, 122, 125, 200, 212, 220, 250, 253, 267, 268, 273, 275, 276, 278, 280, 281, 282, 303, 305, 316, 318, 329, 331, 341, 405, 430, 432, 435, 436, 440
	};
	for (int n = 0; n < lengthof(unusedSubtiles); n++) {
		for (int i = 0; i < blockSize; i++) {
			Blk2Mcr(unusedSubtiles[n], i);
		}
	}
}

void DRLP_L1_PatchTil(BYTE* buf)
{
	uint16_t* pTiles = (uint16_t*)buf;
	// make the inner tile at the entrance non-walkable II.
	pTiles[(196 - 1) * 4 + 3] = SwapLE16(425 - 1);
	// fix shadow (use common subtiles)
	pTiles[(7 - 1) * 4 + 1] = SwapLE16(6 - 1);    // 15
	pTiles[(37 - 1) * 4 + 1] = SwapLE16(6 - 1);
	// use common subtiles
	pTiles[(9 - 1) * 4 + 2] = SwapLE16(7 - 1);    // 18
	pTiles[(9 - 1) * 4 + 3] = SwapLE16(4 - 1);    // 19
	pTiles[(21 - 1) * 4 + 2] = SwapLE16(32 - 1);  // 39
	pTiles[(23 - 1) * 4 + 2] = SwapLE16(32 - 1);
	pTiles[(27 - 1) * 4 + 2] = SwapLE16(3 - 1);   // 49
	pTiles[(43 - 1) * 4 + 2] = SwapLE16(3 - 1);
	pTiles[(32 - 1) * 4 + 2] = SwapLE16(53 - 1);  // 58
	pTiles[(37 - 1) * 4 + 2] = SwapLE16(53 - 1);
	pTiles[(38 - 1) * 4 + 2] = SwapLE16(53 - 1);
	pTiles[(33 - 1) * 4 + 1] = SwapLE16(48 - 1);  // 60
	pTiles[(35 - 1) * 4 + 2] = SwapLE16(53 - 1);  // 63
	pTiles[(58 - 1) * 4 + 1] = SwapLE16(2 - 1);   // 111
	pTiles[(60 - 1) * 4 + 1] = SwapLE16(2 - 1);   // 119
	pTiles[(61 - 1) * 4 + 0] = SwapLE16(23 - 1);  // 122
	pTiles[(62 - 1) * 4 + 0] = SwapLE16(23 - 1);  // 124
	pTiles[(73 - 1) * 4 + 0] = SwapLE16(23 - 1);
	pTiles[(74 - 1) * 4 + 0] = SwapLE16(23 - 1);
	pTiles[(75 - 1) * 4 + 0] = SwapLE16(23 - 1);
	pTiles[(77 - 1) * 4 + 0] = SwapLE16(23 - 1);
	// pTiles[(129 - 1) * 4 + 0] = SwapLE16(23 - 1);
	pTiles[(136 - 1) * 4 + 0] = SwapLE16(23 - 1);
	pTiles[(99 - 1) * 4 + 1] = SwapLE16(6 - 1);   // 204
	pTiles[(103 - 1) * 4 + 1] = SwapLE16(2 - 1);  // 213
	pTiles[(186 - 1) * 4 + 1] = SwapLE16(2 - 1);
	pTiles[(105 - 1) * 4 + 0] = SwapLE16(23 - 1); // 220
	pTiles[(114 - 1) * 4 + 1] = SwapLE16(6 - 1);  // 242
	pTiles[(117 - 1) * 4 + 1] = SwapLE16(6 - 1);
	// pTiles[(130 - 1) * 4 + 0] = SwapLE16(23 - 1); // 275
	pTiles[(133 - 1) * 4 + 0] = SwapLE16(23 - 1); // 282
	pTiles[(137 - 1) * 4 + 0] = SwapLE16(23 - 1); // 293
	pTiles[(128 - 1) * 4 + 1] = SwapLE16(2 - 1);  // 271
	pTiles[(134 - 1) * 4 + 1] = SwapLE16(2 - 1);  // 285
	pTiles[(136 - 1) * 4 + 1] = SwapLE16(2 - 1);  // 290
	// pTiles[(42 - 1) * 4 + 2] = SwapLE16(12 - 1);
	pTiles[(44 - 1) * 4 + 2] = SwapLE16(12 - 1);  // 71
	// pTiles[(159 - 1) * 4 + 2] = SwapLE16(12 - 1); // 341
	pTiles[(59 - 1) * 4 + 2] = SwapLE16(7 - 1);   // 116
	pTiles[(60 - 1) * 4 + 2] = SwapLE16(7 - 1);   // 120
	pTiles[(62 - 1) * 4 + 2] = SwapLE16(7 - 1);   // 125
	pTiles[(128 - 1) * 4 + 2] = SwapLE16(7 - 1);  // 272
	// pTiles[(129 - 1) * 4 + 2] = SwapLE16(7 - 1);  // 273
	pTiles[(136 - 1) * 4 + 2] = SwapLE16(7 - 1);  // 291
	pTiles[(58 - 1) * 4 + 3] = SwapLE16(4 - 1);   // 113
	pTiles[(59 - 1) * 4 + 3] = SwapLE16(4 - 1);   // 117
	pTiles[(60 - 1) * 4 + 3] = SwapLE16(4 - 1);   // 121
	pTiles[(74 - 1) * 4 + 3] = SwapLE16(4 - 1);   // 158
	pTiles[(76 - 1) * 4 + 3] = SwapLE16(4 - 1);   // 161
	pTiles[(97 - 1) * 4 + 3] = SwapLE16(4 - 1);   // 200
	// pTiles[(130 - 1) * 4 + 3] = SwapLE16(4 - 1);  // 277
	pTiles[(137 - 1) * 4 + 3] = SwapLE16(4 - 1);  // 295
	pTiles[(193 - 1) * 4 + 3] = SwapLE16(4 - 1);  // 419
	pTiles[(196 - 1) * 4 + 2] = SwapLE16(36 - 1); // 428
	// use common subtiles instead of minor alterations
	// pTiles[(93 - 1) * 4 + 2] = SwapLE16(177 - 1); // 191
	// simplified door subtiles
	pTiles[(25 - 1) * 4 + 0] = SwapLE16(392 - 1);  // (43)
	pTiles[(26 - 1) * 4 + 0] = SwapLE16(394 - 1);  // (45)
	pTiles[(103 - 1) * 4 + 0] = SwapLE16(407 - 1); // 212
	pTiles[(186 - 1) * 4 + 2] = SwapLE16(213 - 1); // - to make 213 'accessible'
	pTiles[(175 - 1) * 4 + 2] = SwapLE16(43 - 1);  // - to make 43 'accessible'
	pTiles[(176 - 1) * 4 + 1] = SwapLE16(45 - 1);  // - to make 45 'accessible'
	// create separate pillar tile
	pTiles[(28 - 1) * 4 + 0] = SwapLE16(61 - 1);
	pTiles[(28 - 1) * 4 + 1] = SwapLE16(2 - 1);
	pTiles[(28 - 1) * 4 + 2] = SwapLE16(7 - 1);
	pTiles[(28 - 1) * 4 + 3] = SwapLE16(4 - 1);
	// create the new shadows
	// - use the shadows created by fixCathedralShadows
	pTiles[(131 - 1) * 4 + 0] = SwapLE16(23 - 1);
	pTiles[(131 - 1) * 4 + 1] = SwapLE16(2 - 1);
	pTiles[(131 - 1) * 4 + 2] = SwapLE16(301 - 1);
	pTiles[(131 - 1) * 4 + 3] = SwapLE16(302 - 1);
	pTiles[(132 - 1) * 4 + 0] = SwapLE16(23 - 1);
	pTiles[(132 - 1) * 4 + 1] = SwapLE16(2 - 1);
	pTiles[(132 - 1) * 4 + 2] = SwapLE16(310 - 1);
	pTiles[(132 - 1) * 4 + 3] = SwapLE16(344 - 1);
	pTiles[(126 - 1) * 4 + 0] = SwapLE16(296 - 1);
	pTiles[(126 - 1) * 4 + 1] = SwapLE16(297 - 1);
	pTiles[(126 - 1) * 4 + 2] = SwapLE16(310 - 1);
	pTiles[(126 - 1) * 4 + 3] = SwapLE16(344 - 1);
	// pTiles[(139 - 1) * 4 + 0] = SwapLE16(296 - 1);
	// pTiles[(139 - 1) * 4 + 1] = SwapLE16(297 - 1);
	pTiles[(139 - 1) * 4 + 2] = SwapLE16(328 - 1);
	// pTiles[(139 - 1) * 4 + 3] = SwapLE16(299 - 1);
	pTiles[(140 - 1) * 4 + 0] = SwapLE16(23 - 1);
	// pTiles[(140 - 1) * 4 + 1] = SwapLE16(2 - 1);
	// pTiles[(140 - 1) * 4 + 2] = SwapLE16(301 - 1);
	pTiles[(140 - 1) * 4 + 3] = SwapLE16(330 - 1);
	pTiles[(141 - 1) * 4 + 0] = SwapLE16(23 - 1);
	pTiles[(141 - 1) * 4 + 1] = SwapLE16(2 - 1);
	pTiles[(141 - 1) * 4 + 2] = SwapLE16(310 - 1);
	pTiles[(141 - 1) * 4 + 3] = SwapLE16(299 - 1);
	pTiles[(127 - 1) * 4 + 0] = SwapLE16(296 - 1);
	pTiles[(127 - 1) * 4 + 1] = SwapLE16(297 - 1);
	pTiles[(127 - 1) * 4 + 2] = SwapLE16(310 - 1);
	pTiles[(127 - 1) * 4 + 3] = SwapLE16(299 - 1);
	// pTiles[(142 - 1) * 4 + 0] = SwapLE16(307 - 1);
	// pTiles[(142 - 1) * 4 + 1] = SwapLE16(308 - 1);
	// pTiles[(142 - 1) * 4 + 2] = SwapLE16(7 - 1);
	// pTiles[(142 - 1) * 4 + 3] = SwapLE16(4 - 1);
	pTiles[(143 - 1) * 4 + 0] = SwapLE16(23 - 1);
	// pTiles[(143 - 1) * 4 + 1] = SwapLE16(2 - 1);
	pTiles[(143 - 1) * 4 + 2] = SwapLE16(313 - 1);
	pTiles[(143 - 1) * 4 + 3] = SwapLE16(330 - 1);
	pTiles[(144 - 1) * 4 + 0] = SwapLE16(315 - 1);
	// pTiles[(144 - 1) * 4 + 1] = SwapLE16(2 - 1);
	pTiles[(144 - 1) * 4 + 2] = SwapLE16(317 - 1);
	pTiles[(144 - 1) * 4 + 3] = SwapLE16(299 - 1);
	pTiles[(145 - 1) * 4 + 0] = SwapLE16(21 - 1);
	pTiles[(145 - 1) * 4 + 1] = SwapLE16(2 - 1);
	pTiles[(145 - 1) * 4 + 2] = SwapLE16(321 - 1);
	pTiles[(145 - 1) * 4 + 3] = SwapLE16(302 - 1);
	pTiles[(146 - 1) * 4 + 0] = SwapLE16(1 - 1);
	// pTiles[(146 - 1) * 4 + 1] = SwapLE16(2 - 1);
	// pTiles[(146 - 1) * 4 + 2] = SwapLE16(320 - 1);
	// pTiles[(146 - 1) * 4 + 3] = SwapLE16(302 - 1);
	pTiles[(147 - 1) * 4 + 0] = SwapLE16(13 - 1);
	// pTiles[(147 - 1) * 4 + 1] = SwapLE16(2 - 1);
	pTiles[(147 - 1) * 4 + 2] = SwapLE16(320 - 1);
	// pTiles[(147 - 1) * 4 + 3] = SwapLE16(302 - 1);
	// pTiles[(148 - 1) * 4 + 0] = SwapLE16(322 - 1);
	// pTiles[(148 - 1) * 4 + 1] = SwapLE16(323 - 1);
	pTiles[(148 - 1) * 4 + 2] = SwapLE16(328 - 1);
	// pTiles[(148 - 1) * 4 + 3] = SwapLE16(299 - 1);
	// pTiles[(149 - 1) * 4 + 0] = SwapLE16(324 - 1);
	// pTiles[(149 - 1) * 4 + 1] = SwapLE16(325 - 1);
	pTiles[(149 - 1) * 4 + 2] = SwapLE16(328 - 1);
	// pTiles[(149 - 1) * 4 + 3] = SwapLE16(299 - 1);
	pTiles[(150 - 1) * 4 + 0] = SwapLE16(5 - 1);
	pTiles[(150 - 1) * 4 + 1] = SwapLE16(6 - 1);
	pTiles[(150 - 1) * 4 + 2] = SwapLE16(328 - 1);
	pTiles[(150 - 1) * 4 + 3] = SwapLE16(299 - 1);
	pTiles[(151 - 1) * 4 + 0] = SwapLE16(22 - 1);
	pTiles[(151 - 1) * 4 + 1] = SwapLE16(11 - 1);
	// pTiles[(151 - 1) * 4 + 2] = SwapLE16(328 - 1);
	pTiles[(151 - 1) * 4 + 3] = SwapLE16(299 - 1);
	pTiles[(152 - 1) * 4 + 0] = SwapLE16(64 - 1);
	pTiles[(152 - 1) * 4 + 1] = SwapLE16(48 - 1);
	// pTiles[(152 - 1) * 4 + 2] = SwapLE16(328 - 1);
	pTiles[(152 - 1) * 4 + 3] = SwapLE16(299 - 1);
	pTiles[(153 - 1) * 4 + 0] = SwapLE16(304 - 1);
	pTiles[(153 - 1) * 4 + 1] = SwapLE16(298 - 1);
	pTiles[(153 - 1) * 4 + 2] = SwapLE16(328 - 1);
	pTiles[(153 - 1) * 4 + 3] = SwapLE16(299 - 1);
	pTiles[(154 - 1) * 4 + 0] = SwapLE16(14 - 1);
	pTiles[(154 - 1) * 4 + 1] = SwapLE16(6 - 1);
	pTiles[(154 - 1) * 4 + 2] = SwapLE16(328 - 1);
	pTiles[(154 - 1) * 4 + 3] = SwapLE16(299 - 1);
	pTiles[(155 - 1) * 4 + 0] = SwapLE16(340 - 1);
	pTiles[(155 - 1) * 4 + 1] = SwapLE16(323 - 1);
	pTiles[(155 - 1) * 4 + 2] = SwapLE16(328 - 1);
	pTiles[(155 - 1) * 4 + 3] = SwapLE16(299 - 1);
	pTiles[(156 - 1) * 4 + 0] = SwapLE16(394 - 1);
	pTiles[(156 - 1) * 4 + 1] = SwapLE16(46 - 1);
	pTiles[(156 - 1) * 4 + 2] = SwapLE16(328 - 1);
	// pTiles[(156 - 1) * 4 + 3] = SwapLE16(299 - 1);
	pTiles[(157 - 1) * 4 + 0] = SwapLE16(62 - 1);
	pTiles[(157 - 1) * 4 + 1] = SwapLE16(2 - 1);
	pTiles[(157 - 1) * 4 + 2] = SwapLE16(334 - 1);
	pTiles[(157 - 1) * 4 + 3] = SwapLE16(302 - 1);
	// pTiles[(158 - 1) * 4 + 0] = SwapLE16(339 - 1);
	pTiles[(158 - 1) * 4 + 1] = SwapLE16(323 - 1);
	// pTiles[(158 - 1) * 4 + 2] = SwapLE16(3 - 1);
	// pTiles[(158 - 1) * 4 + 3] = SwapLE16(4 - 1);
	pTiles[(159 - 1) * 4 + 0] = SwapLE16(23 - 1);
	pTiles[(159 - 1) * 4 + 1] = SwapLE16(2 - 1);
	pTiles[(159 - 1) * 4 + 2] = SwapLE16(328 - 1);
	pTiles[(159 - 1) * 4 + 3] = SwapLE16(299 - 1);
	// pTiles[(160 - 1) * 4 + 0] = SwapLE16(342 - 1);
	pTiles[(160 - 1) * 4 + 1] = SwapLE16(323 - 1);
	pTiles[(160 - 1) * 4 + 2] = SwapLE16(12 - 1);
	// pTiles[(160 - 1) * 4 + 3] = SwapLE16(4 - 1);
	// pTiles[(161 - 1) * 4 + 0] = SwapLE16(343 - 1);
	pTiles[(161 - 1) * 4 + 1] = SwapLE16(323 - 1);
	pTiles[(161 - 1) * 4 + 2] = SwapLE16(53 - 1); // 345
	// pTiles[(161 - 1) * 4 + 3] = SwapLE16(4 - 1);
	pTiles[(164 - 1) * 4 + 0] = SwapLE16(23 - 1);
	pTiles[(164 - 1) * 4 + 1] = SwapLE16(2 - 1);
	pTiles[(164 - 1) * 4 + 2] = SwapLE16(313 - 1);
	pTiles[(164 - 1) * 4 + 3] = SwapLE16(302 - 1);
	pTiles[(165 - 1) * 4 + 0] = SwapLE16(296 - 1);
	pTiles[(165 - 1) * 4 + 1] = SwapLE16(297 - 1);
	pTiles[(165 - 1) * 4 + 2] = SwapLE16(328 - 1);
	pTiles[(165 - 1) * 4 + 3] = SwapLE16(344 - 1);
	// - shadows for the banner setpiece
	pTiles[(56 - 1) * 4 + 0] = SwapLE16(1 - 1);
	pTiles[(56 - 1) * 4 + 1] = SwapLE16(2 - 1);
	pTiles[(56 - 1) * 4 + 2] = SwapLE16(3 - 1);
	pTiles[(56 - 1) * 4 + 3] = SwapLE16(126 - 1);
	pTiles[(55 - 1) * 4 + 0] = SwapLE16(1 - 1);
	pTiles[(55 - 1) * 4 + 1] = SwapLE16(123 - 1);
	pTiles[(55 - 1) * 4 + 2] = SwapLE16(3 - 1);
	// pTiles[(55 - 1) * 4 + 3] = SwapLE16(4 - 1);
	pTiles[(54 - 1) * 4 + 0] = SwapLE16(338 - 1);
	pTiles[(54 - 1) * 4 + 1] = SwapLE16(297 - 1);
	pTiles[(54 - 1) * 4 + 2] = SwapLE16(328 - 1);
	pTiles[(54 - 1) * 4 + 3] = SwapLE16(299 - 1);
	pTiles[(53 - 1) * 4 + 0] = SwapLE16(337 - 1);
	pTiles[(53 - 1) * 4 + 1] = SwapLE16(297 - 1);
	pTiles[(53 - 1) * 4 + 2] = SwapLE16(336 - 1);
	pTiles[(53 - 1) * 4 + 3] = SwapLE16(344 - 1);
	// - shadows for the vile setmap
	pTiles[(52 - 1) * 4 + 0] = SwapLE16(5 - 1);
	pTiles[(52 - 1) * 4 + 1] = SwapLE16(6 - 1);
	pTiles[(52 - 1) * 4 + 2] = SwapLE16(313 - 1);
	pTiles[(52 - 1) * 4 + 3] = SwapLE16(302 - 1);
	pTiles[(51 - 1) * 4 + 0] = SwapLE16(5 - 1);
	pTiles[(51 - 1) * 4 + 1] = SwapLE16(6 - 1);
	pTiles[(51 - 1) * 4 + 2] = SwapLE16(301 - 1);
	pTiles[(51 - 1) * 4 + 3] = SwapLE16(302 - 1);
	pTiles[(50 - 1) * 4 + 0] = SwapLE16(1 - 1);
	// pTiles[(50 - 1) * 4 + 1] = SwapLE16(2 - 1);
	pTiles[(50 - 1) * 4 + 2] = SwapLE16(320 - 1);
	pTiles[(50 - 1) * 4 + 3] = SwapLE16(330 - 1);
	pTiles[(49 - 1) * 4 + 0] = SwapLE16(335 - 1);
	pTiles[(49 - 1) * 4 + 1] = SwapLE16(308 - 1);
	pTiles[(49 - 1) * 4 + 2] = SwapLE16(7 - 1);
	pTiles[(49 - 1) * 4 + 3] = SwapLE16(4 - 1);
	pTiles[(48 - 1) * 4 + 0] = SwapLE16(21 - 1);
	pTiles[(48 - 1) * 4 + 1] = SwapLE16(2 - 1);
	pTiles[(48 - 1) * 4 + 2] = SwapLE16(321 - 1);
	pTiles[(48 - 1) * 4 + 3] = SwapLE16(330 - 1);
	pTiles[(47 - 1) * 4 + 0] = SwapLE16(5 - 1);
	pTiles[(47 - 1) * 4 + 1] = SwapLE16(6 - 1);
	pTiles[(47 - 1) * 4 + 2] = SwapLE16(301 - 1);
	pTiles[(47 - 1) * 4 + 3] = SwapLE16(330 - 1);
	pTiles[(46 - 1) * 4 + 0] = SwapLE16(14 - 1);
	pTiles[(46 - 1) * 4 + 1] = SwapLE16(6 - 1);
	pTiles[(46 - 1) * 4 + 2] = SwapLE16(301 - 1);
	pTiles[(46 - 1) * 4 + 3] = SwapLE16(302 - 1);
	// eliminate subtiles of unused tiles
	const int unusedTiles[] = {
		30, 31, 34,/* 38,*/ 39, 40, 41, 42,/*43, 44,*/ 45, 79, 82, 86, 87, 88, 89, 90, 91, 92, 93, 95, 96, 119, 120, 129, 130, 177, 178, 179, 180, 181, 182, 183, 184, 185, 187, 188, 189, 190, 191, 192, 195, 197, 198, 199, 200, 201, 202, 203, 204, 205
	};
	constexpr int blankSubtile = 74;
	for (int n = 0; n < lengthof(unusedTiles); n++) {
		int tileId = unusedTiles[n];
		pTiles[(tileId - 1) * 4 + 0] = SwapLE16(blankSubtile - 1);
		pTiles[(tileId - 1) * 4 + 1] = SwapLE16(blankSubtile - 1);
		pTiles[(tileId - 1) * 4 + 2] = SwapLE16(blankSubtile - 1);
		pTiles[(tileId - 1) * 4 + 3] = SwapLE16(blankSubtile - 1);
	}
}

#ifdef HELLFIRE
BYTE* DRLP_L5_PatchSpec(const BYTE* minBuf, size_t minLen, const BYTE* celBuf, size_t celLen, BYTE* sCelBuf, size_t* sCelLen)
{
	typedef struct {
		int subtileIndex0;
		int8_t microIndices0[5];
		int subtileIndex1;
		int8_t microIndices1[5];
	} SCelFrame;
	const SCelFrame frames[] = {
/*  0 */{ 206 - 1, {-1,-1, 4,-1,-1 }, /*204*/ - 1, {-1,-1, 4, 6,-1 } },
/*  1 */{     - 1, { 0 },             /*208*/ - 1, {-1,-1, 5, 7,-1 } },
/*  2 */{  31 - 1, { 0, 2, 4, 6, 8 },  29 - 1, {-1,-1, 4, 6,-1 } },
/*  3 */{ 274 - 1, { 0, 2, 4, 6, 8 }, 272 - 1, {-1,-1, 4, 6,-1 } },
/*  4 */{ 558 - 1, { 0, 2, 4, 6, 8 }, 557 - 1, {-1,-1, 4, 6,-1 } },
/*  5 */{ 299 - 1, { 0, 2, 4, 6, 8 }, 298 - 1, {-1,-1, 4, 6, 8 } },
/*  6 */{ 301 - 1, {-1,-1, 4, 6, 8 }, 300 - 1, {-1,-1, 4, 6,-1 } },
/*  7 */{ 333 - 1, { 0, 2, 4, 6, 8 }, 331 - 1, {-1,-1, 4, 6,-1 } },
/*  8 */{ 356 - 1, { 0, 2, 4, 6, 8 }, 355 - 1, {-1,-1, 4, 6,-1 } },
/*  9 */{ 404 - 1, { 0, 2, 4, 6, 8 },  29 - 1, {-1,-1, 4, 6,-1 } },
/* 10 */{ 415 - 1, { 0, 2, 4, 6, 8 }, 413 - 1, {-1,-1, 4, 6,-1 } },
/* 11 */{ 456 - 1, { 0, 2, 4, 6, 8 }, 454 - 1, {-1,-1, 4, 6,-1 } },

/* 12 */{  18 - 1, { 1, 3, 5, 7, 9 },  17 - 1, {-1,-1, 5, 7,-1 } },
/* 13 */{ 459 - 1, { 1, 3, 5, 7, 9 }, 458 - 1, {-1,-1, 5, 7,-1 } },
/* 14 */{ 352 - 1, { 1, 3, 5, 7, 9 }, 351 - 1, {-1,-1, 5, 7,-1 } },
/* 15 */{ 348 - 1, { 1, 3, 5, 7, 9 }, 347 - 1, {-1,-1, 5, 7,-1 } },
/* 16 */{ 406 - 1, { 1, 3, 5, 7, 9 },  17 - 1, {-1,-1, 5, 7,-1 } },
/* 17 */{ 444 - 1, { 1, 3, 5, 7, 9 },  17 - 1, {-1,-1, 5, 7,-1 } },
/* 18 */{ 471 - 1, { 1, 3, 5, 7, 9 }, 470 - 1, {-1,-1, 5, 7,-1 } },
/* 19 */{ 562 - 1, { 1, 3, 5, 7, 9 }, 561 - 1, {-1,-1, 5, 7,-1 } },
	};

	const uint16_t* pSubtiles = (const uint16_t*)minBuf;

	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_L5;
	for (int i = 0; i < lengthof(frames); i++) {
		const SCelFrame &frame = frames[i];
		// if (frame.subtileIndex0 < 0) {
		// 	continue;
		// }
		for (int n = 0; n < lengthof(frame.microIndices0) && frame.subtileIndex0 >= 0; n++) {
			if (frame.microIndices0[n] < 0) {
				continue;
			}
			unsigned index = MICRO_IDX(frame.subtileIndex0, blockSize, frame.microIndices0[n]);
			if ((SwapLE16(pSubtiles[index]) & 0xFFF) == 0) {
				return sCelBuf; // frame is empty -> assume it is already done
			}
		}
		if (frame.subtileIndex1 < 0) {
		 	continue;
		}
		for (int n = 0; n < lengthof(frame.microIndices1); n++) {
			if (frame.microIndices1[n] < 0) {
				continue;
			}
			unsigned index = MICRO_IDX(frame.subtileIndex1, blockSize, frame.microIndices1[n]);
			if ((SwapLE16(pSubtiles[index]) & 0xFFF) == 0) {
				return sCelBuf; // frame is empty -> assume it is already done
			}
		}
	}

	pMicrosCel = const_cast<BYTE*>(celBuf);

	constexpr BYTE TRANS_COLOR = 128;
	constexpr BYTE SUB_HEADER_SIZE = 10;
	constexpr int FRAME_WIDTH = 64;
	constexpr int FRAME_HEIGHT = 160;

	DWORD* srcHeaderCursor = (DWORD*)sCelBuf;
	int srcCelEntries = SwapLE32(srcHeaderCursor[0]);
	srcHeaderCursor++;

	if (srcCelEntries != 2) {
		return sCelBuf; // assume it is already done
	}

	// calculate the number of frames in the result
	int resCelEntries = std::max(srcCelEntries, lengthof(frames));

	// create the new CEL file
	size_t maxCelSize = resCelEntries * FRAME_WIDTH * FRAME_HEIGHT * 2;
	BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
	memset(resCelBuf, 0, maxCelSize);

	DWORD* dstHeaderCursor = (DWORD*)resCelBuf;
	*dstHeaderCursor = SwapLE32(resCelEntries);
	dstHeaderCursor++;

	BYTE* dstDataCursor = resCelBuf + 4 * (resCelEntries + 2);

	// int idx = 0;
	for (int i = 0; i < lengthof(frames); i++) {
		const SCelFrame &frame = frames[i];

		// draw the frame to the back-buffer
		memset(&gpBuffer[0], TRANS_COLOR, FRAME_HEIGHT * BUFFER_WIDTH);
		if (i < srcCelEntries) {
			CelClippedDraw(0, FRAME_HEIGHT - 1, sCelBuf, i + 1, FRAME_WIDTH);
		}

		if (i < 12 && i != 1) {
			for (int n = 0; n < lengthof(frame.microIndices0) && frame.subtileIndex0 >= 0; n++) {
				if (frame.microIndices0[n] < 0) {
					continue;
				}
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					memset(&gpBuffer[FRAME_WIDTH + y * BUFFER_WIDTH], TRANS_COLOR, MICRO_WIDTH);
				}
				unsigned index = MICRO_IDX(frame.subtileIndex0, blockSize, frame.microIndices0[n]);
				RenderMicro(&gpBuffer[FRAME_WIDTH + (MICRO_HEIGHT - 1) * BUFFER_WIDTH], SwapLE16(pSubtiles[index]), DMT_NONE);
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 0; x < MICRO_WIDTH; x++) {
						if (i == 0) { // 206[4]
							if (y > 14 - x / 2) {
								continue;
							}
						}
						if ((i == 2 || i == 3 || i == 4 || i == 5 || i == 7 || i == 8 || i == 10 || i == 11) && n == 0) { // 31[0] - mask floor with left column
							if (x > 23) {
								continue;
							}
							if (x == 23 && (y != 17 && y != 18)) {
								continue;
							}
							if (x == 22 && (y < 16 || y > 19)) {
								continue;
							}
							if (x == 21 && (y == 21 || y == 22)) {
								continue;
							}
						}
						if (i == 9 && n == 0) { // 404[0] - mask floor with left column
							if (x > 23) {
								continue;
							}
							if (x == 23 && y > 18) {
								continue;
							}
							if (x == 22 && y > 19) {
								continue;
							}
							if (x == 21 && (y == 21 || y == 22)) {
								continue;
							}
						}
						if (i == 6 && n == 2) { // 301[4] eliminate 'bad' pixels
							if (y > 12 - (x + 1) / 2) {
								continue;
							}
						}
						if (i == 6 && n == 3) { // 301[6] eliminate 'bad' pixels
							if (y > 44 - (x + 1) / 2) {
								continue;
							}
						}
						BYTE color = gpBuffer[FRAME_WIDTH + x + y * BUFFER_WIDTH];
						if (color != TRANS_COLOR) {
							unsigned addr = x + (FRAME_HEIGHT - MICRO_HEIGHT * (n + 1) + y) * BUFFER_WIDTH;
							if (gpBuffer[addr] == TRANS_COLOR) {
								gpBuffer[addr] = color;
							}
						}
					}
				}
			}
			for (int n = 0; n < lengthof(frame.microIndices1) && frame.subtileIndex1 >= 0; n++) {
				if (frame.microIndices1[n] < 0) {
					continue;
				}
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					memset(&gpBuffer[FRAME_WIDTH + y * BUFFER_WIDTH], TRANS_COLOR, MICRO_WIDTH);
				}
				unsigned index = MICRO_IDX(frame.subtileIndex1, blockSize, frame.microIndices1[n]);
				RenderMicro(&gpBuffer[FRAME_WIDTH + (MICRO_HEIGHT - 1) * BUFFER_WIDTH], SwapLE16(pSubtiles[index]), DMT_NONE);
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 0; x < MICRO_WIDTH; x++) {
						/*if (i == 0 && n == 2) { // 204[4]
							if (y > 14 - x / 2) {
								continue;
							}
						}*/

						if ((i == 2 || i == 3 || i == 4 || i == 7 || i == 8 || i == 9 || i == 11) && n == 2) { // 29[4]
							if (y > 16) {
								continue;
							}
							if (y > 25 - x) {
								continue;
							}
							/*if (x > 18 && y > 24 - x) {
								continue;
							}*/
						}
						if ((i == 2 || i == 3 || i == 4 || i == 7 || i == 8 || i == 9 || i == 11) && n == 3) { // 29[6]
							if (y > 57 - x) {
								continue;
							}
							if (x > 27) {
								continue;
							}
							if (y < x / 2 - 1) {
								continue;
							}
							if (y < 5 - x / 2) {
								continue;
							}
						}
						if (i == 10 && n == 2) { // 413[4]
							if (y > 10 - x) {
								continue;
							}
						}
						if (i == 10 && n == 3) { // 413[6]
							if (x > 26) {
								continue;
							}
							if (y > 45 - x) {
								continue;
							}
							if ((x < 19 || x > 21) && y > 44 - x) {
								continue;
							}
							if ((x < 16 || x > 22) && y > 43 - x) {
								continue;
							}
							if (x < 14 && y > 42 - x) {
								continue;
							}
							if (y < x / 2 - 1) {
								continue;
							}
							if (y < 5 - x / 2) {
								continue;
							}
						}
						if (i == 6 && n == 2) { // 331[4]
							if (y > 12 - (x + 1) / 2) {
								continue;
							}
						}
						if (i == 6 && n == 3) { // 331[6]
							if (x == 27 && y == 31) {
								continue;
							}
							if (x > 27) {
								continue;
							}
							if (y < x / 2 - 1) {
								continue;
							}
							if (y < 5 - x / 2) {
								continue;
							}
						}

						BYTE color = gpBuffer[FRAME_WIDTH + x + y * BUFFER_WIDTH];
						if ((i == 3 || i == 4) && n == 0 && ((color < 16 && color != 0) /*|| (x == 23 && y == 18)*/)) {
							continue; // remove bright lava-pixels from 274[0], 558[0]
						}
						if (color != TRANS_COLOR) {
							unsigned addr = MICRO_WIDTH + x + (FRAME_HEIGHT - MICRO_HEIGHT * (n + 1) + y - 16) * BUFFER_WIDTH;
							if (gpBuffer[addr] == TRANS_COLOR) {
								gpBuffer[addr] = color;
							}
						}
					}
				}
			}
		}

		if (i >= 12 || i == 1) {
			for (int n = 0; n < lengthof(frame.microIndices0) && frame.subtileIndex0 >= 0; n++) {
				if (frame.microIndices0[n] < 0) {
					continue;
				}
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					memset(&gpBuffer[FRAME_WIDTH + y * BUFFER_WIDTH], TRANS_COLOR, MICRO_WIDTH);
				}
				unsigned index = MICRO_IDX(frame.subtileIndex0, blockSize, frame.microIndices0[n]);
				RenderMicro(&gpBuffer[FRAME_WIDTH + (MICRO_HEIGHT - 1) * BUFFER_WIDTH], SwapLE16(pSubtiles[index]), DMT_NONE);
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 0; x < MICRO_WIDTH; x++) {
						if (i != 18 && i != 17 && i != 1 && n == 0) { // 18[1]
							if (x < 8) {
								continue;
							}
							if (x == 8 && (y != 18 && y != 19)) {
								continue;
							}
							if (x == 9 && (y < 17 || y > 20)) {
								continue;
							}
							if (x == 10 && (y == 22 || y == 23)) {
								continue;
							}
						}

						if (i == 17 && n == 0) { // 444[1] -- remove additional pixels due to brocken column
							if (x < 10) {
								continue;
							}
							if (x == 10 && (y < 17 || y > 19)) {
								continue;
							}
							if (x == 11 && (y < 16 || y == 22 || y == 23 || y == 26)) {
								continue;
							}
							if (x == 12 && y < 15) {
								continue;
							}
							if (x == 13 && y < 13) {
								continue;
							}
							if (x == 14 && y < 11) {
								continue;
							}
							if (x == 15 && y < 10) {
								continue;
							}
						}

						if (i == 18 && n == 0) { // 471[1]
							if (x < 5) {
								continue;
							}
							if (x == 5 && (y < 14 || y > 17)) {
								continue;
							}
							if (x == 6 && (y < 11 || y > 18)) {
								continue;
							}
							if (x == 7 && y > 20) {
								continue;
							}
							if (x == 8 && y > 22) {
								continue;
							}
							if (x == 9 && y > 23) {
								continue;
							}
						}

						if (i == 17 && n == 1) { // 444[3] - remove pixels from the left side of the arch
							if (x < 2) {
								continue;
							}
						}
						BYTE color = gpBuffer[FRAME_WIDTH + x + y * BUFFER_WIDTH];
						if (i == 19 && n == 0 && ((color < 16 && color != 0) /*|| (x == 8 && y == 18)*/)) {
							continue; // remove bright lava-pixels from 562[1]
						}
						if (color != TRANS_COLOR) {
							unsigned addr = MICRO_WIDTH + x + (FRAME_HEIGHT - MICRO_HEIGHT * (n + 1) + y) * BUFFER_WIDTH;
							if (gpBuffer[addr] == TRANS_COLOR) {
								gpBuffer[addr] = color;
							}
						}
					}
				}
			}
			for (int n = 0; n < lengthof(frame.microIndices1) && frame.subtileIndex1 >= 0; n++) {
				if (frame.microIndices1[n] < 0) {
					continue;
				}
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					memset(&gpBuffer[FRAME_WIDTH + y * BUFFER_WIDTH], TRANS_COLOR, MICRO_WIDTH);
				}
				unsigned index = MICRO_IDX(frame.subtileIndex1, blockSize, frame.microIndices1[n]);
				RenderMicro(&gpBuffer[FRAME_WIDTH + (MICRO_HEIGHT - 1) * BUFFER_WIDTH], SwapLE16(pSubtiles[index]), DMT_NONE);
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 0; x < MICRO_WIDTH; x++) {
						if (i != 1 && n == 2) { // 17[5]
							if (y > 16) {
								continue;
							}
							if (y > x - 5) {
								continue;
							}
						}
						if (i != 1 && n == 3) { // 17[7]
							if (y > x + 27) {
								continue;
							}
							if (x < 4) {
								continue;
							}
							if (y < x / 2 - 10) {
								continue;
							}
							if (y < 14 - x / 2) {
								continue;
							}
						}

						BYTE color = gpBuffer[FRAME_WIDTH + x + y * BUFFER_WIDTH];
						if (color != TRANS_COLOR) {
							unsigned addr = x + (FRAME_HEIGHT - MICRO_HEIGHT * (n + 1) + y - 16) * BUFFER_WIDTH;
							if (gpBuffer[addr] == TRANS_COLOR) {
								gpBuffer[addr] = color;
							}
						}
					}
				}
			}
		}

		if (i == 0) {
			gpBuffer[31 + 63 * BUFFER_WIDTH] = 44;
			gpBuffer[22 + 155 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[59 + 45 * BUFFER_WIDTH] = 44;
			gpBuffer[58 + 46 * BUFFER_WIDTH] = 42;
			gpBuffer[58 + 47 * BUFFER_WIDTH] = 41;
		}

		if (i == 6) {
			gpBuffer[10 + 32 * BUFFER_WIDTH] = 41;
			gpBuffer[12 + 31 * BUFFER_WIDTH] = 41;
			gpBuffer[14 + 30 * BUFFER_WIDTH] = 40;
			gpBuffer[16 + 29 * BUFFER_WIDTH] = 41;
			gpBuffer[18 + 28 * BUFFER_WIDTH] = 41;
			gpBuffer[20 + 27 * BUFFER_WIDTH] = 41;
			gpBuffer[22 + 26 * BUFFER_WIDTH] = 41;
			gpBuffer[24 + 25 * BUFFER_WIDTH] = 40;
			gpBuffer[26 + 24 * BUFFER_WIDTH] = 41;
			gpBuffer[28 + 23 * BUFFER_WIDTH] = 40;
			gpBuffer[30 + 22 * BUFFER_WIDTH] = 41;
			gpBuffer[32 + 21 * BUFFER_WIDTH] = 41;
			gpBuffer[34 + 20 * BUFFER_WIDTH] = 42;
		}

		// write to the new SCEL file
		dstHeaderCursor[0] = SwapLE32((DWORD)((size_t)dstDataCursor - (size_t)resCelBuf));
		dstHeaderCursor++;

		dstDataCursor = EncodeFrame(dstDataCursor, FRAME_WIDTH, FRAME_HEIGHT, SUB_HEADER_SIZE, TRANS_COLOR);

		// skip the original frame
		srcHeaderCursor++;
		// idx++;
	}

	// copy the remaining content
	/*while (idx < srcCelEntries) {
		dstHeaderCursor[0] = SwapLE32((DWORD)((size_t)dstDataCursor - (size_t)resCelBuf));
		dstHeaderCursor++;
		DWORD len = srcHeaderCursor[1] - srcHeaderCursor[0];
		memcpy(dstDataCursor, sCelBuf + srcHeaderCursor[0], len);
		dstDataCursor += len;
		srcHeaderCursor++;

		idx++;
	}*/

	// add file-size
	*sCelLen = (size_t)dstDataCursor - (size_t)resCelBuf;
	dstHeaderCursor[0] = SwapLE32((DWORD)(*sCelLen));

	return resCelBuf;
}

static BYTE* patchCryptFloorCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	const CelMicro micros[] = {
/*  0 */{ 159 - 1, 3, MET_SQUARE },
/*  1 */{ 336 - 1, 0, MET_LTRIANGLE },
/*  2 */{ 409 - 1, 0, MET_LTRIANGLE },
/*  3 */{ 481 - 1, 1, MET_RTRIANGLE },
/*  4 */{ 492 - 1, 0, MET_LTRIANGLE },
/*  5 */{ 519 - 1, 0, MET_LTRIANGLE },
/*  6 */{ 595 - 1, 1, MET_RTRIANGLE },
/*  7 */{ 368 - 1, 1, MET_RTRIANGLE },
/*  8 */{ 162 - 1, 2, MET_TRANSPARENT },
/*  9 */{  63 - 1, 4, MET_SQUARE },
/* 10 */{ 450 - 1, 0, MET_TRANSPARENT },

/* 11 */{ 206 - 1, 0, MET_LTRIANGLE },   // mask doors
/* 12 */{ 204 - 1, 0, MET_TRANSPARENT },
/* 13 */{ 204 - 1, 2, MET_TRANSPARENT },
/* 14 */{ 204 - 1, 4, MET_TRANSPARENT },
/* 15 */{ 209 - 1, 1, MET_RTRIANGLE },
/* 16 */{ 208 - 1, 1, MET_TRANSPARENT },
/* 17 */{ 208 - 1, 3, MET_TRANSPARENT },
/* 18 */{ 208 - 1, 5, MET_TRANSPARENT },

/* 19 */{ 290 - 1, 4, MET_TRANSPARENT },
/* 20 */{ 292 - 1, 0, MET_TRANSPARENT },
/* 21 */{ 292 - 1, 2, MET_TRANSPARENT },
/* 22 */{ 292 - 1, 4, MET_TRANSPARENT },
/* 23 */{ 294 - 1, 4, MET_TRANSPARENT },
/* 24 */{ 296 - 1, 4, MET_TRANSPARENT },
/* 25 */{ 296 - 1, 6, MET_TRANSPARENT },

/* 26 */{ 274 - 1, 0, MET_LTRIANGLE }, // with the new special cels
/* 27 */{ 299 - 1, 0, MET_LTRIANGLE },
/* 28 */{ 404 - 1, 0, MET_LTRIANGLE },
/* 29 */{ 415 - 1, 0, MET_LTRIANGLE },
/* 30 */{ 456 - 1, 0, MET_LTRIANGLE },
/* 31 */{  18 - 1, 1, MET_RTRIANGLE },
/*  *///{ 277 - 1, 1, MET_RTRIANGLE }, -- altered in fixCryptShadows
/* 32 */{ 444 - 1, 1, MET_RTRIANGLE },
/* 33 */{ 471 - 1, 1, MET_RTRIANGLE },

/* 34 */{  29 - 1, 4, MET_TRANSPARENT },
/* 35 */{ 272 - 1, 4, MET_TRANSPARENT },
/* 36 */{ 454 - 1, 4, MET_TRANSPARENT },
/* 37 */{ 557 - 1, 4, MET_TRANSPARENT },
/* 38 */{ 559 - 1, 4, MET_TRANSPARENT },
/* 39 */{ 413 - 1, 4, MET_TRANSPARENT },
/* 40 */{ 300 - 1, 4, MET_TRANSPARENT },
/* 41 */{  33 - 1, 5, MET_TRANSPARENT },
/* 42 */{ 347 - 1, 5, MET_TRANSPARENT },
/* 43 */{ 357 - 1, 5, MET_TRANSPARENT },
/* 44 */{ 400 - 1, 5, MET_TRANSPARENT },
/* 45 */{ 458 - 1, 5, MET_TRANSPARENT },
/* 46 */{ 563 - 1, 5, MET_TRANSPARENT },
/* 47 */{ 276 - 1, 5, MET_TRANSPARENT },

/* 48 */{ 258 - 1, 6, MET_TRANSPARENT }, // fix micros after reuse
/* 49 */{ 256 - 1, 6, MET_TRANSPARENT },
	};

	const uint16_t* pSubtiles = (const uint16_t*)minBuf;

	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_L5;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
		if ((SwapLE16(pSubtiles[index]) & 0xFFF) == 0) {
			mem_free_dbg(celBuf);
			app_warn("Invalid (empty) crypt on subtile (%d).", micro.subtileIndex + 1);
			return NULL;
		}
	}

	// TODO: check celLen
	// draw the micros to the back-buffer
	pMicrosCel = celBuf;
	constexpr BYTE TRANS_COLOR = 128;
	constexpr int DRAW_HEIGHT = 3;
	memset(&gpBuffer[0], TRANS_COLOR, DRAW_HEIGHT * BUFFER_WIDTH * MICRO_HEIGHT);

	unsigned xx = 0, yy = MICRO_HEIGHT - 1;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
		RenderMicro(&gpBuffer[xx + yy * BUFFER_WIDTH], SwapLE16(pSubtiles[index]), DMT_NONE);
		yy += MICRO_HEIGHT;
		if (yy == (DRAW_HEIGHT + 1) * MICRO_HEIGHT - 1) {
			yy = MICRO_HEIGHT - 1;
			xx += MICRO_WIDTH;
		}
	}

	{ // 336[0] - add missing pixels
		int i = 1;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 30 + 1 * BUFFER_WIDTH] = 46;
		gpBuffer[addr + 31 + 1 * BUFFER_WIDTH] = 76;
	}
	{ // 519[0] - add missing pixels
		int i = 5;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 0 + 16 * BUFFER_WIDTH] = 43;
	}
	{ // 368[1] - make the edge of the oil(?) smoother
		int i = 7;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 0 + 7 * BUFFER_WIDTH] = 43;
		gpBuffer[addr + 0 + 9 * BUFFER_WIDTH] = 41;
	}
	{ // 162[2] - adjust the shadow to enable 'fix crack in the chair'
		int i = 8;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 31 + 13 * BUFFER_WIDTH] = 41;
		gpBuffer[addr + 31 + 14 * BUFFER_WIDTH] = 36;
		gpBuffer[addr + 31 + 18 * BUFFER_WIDTH] = 36;
		gpBuffer[addr + 30 + 19 * BUFFER_WIDTH] = 35;
		gpBuffer[addr + 31 + 20 * BUFFER_WIDTH] = 45;
		gpBuffer[addr + 31 + 21 * BUFFER_WIDTH] = 63;
		gpBuffer[addr + 31 + 22 * BUFFER_WIDTH] = 40;
		gpBuffer[addr + 31 + 29 * BUFFER_WIDTH] = 36;
	}
	{ // 63[4] - adjust pixels to make the connections better
		int i = 9;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 0 + 19 * BUFFER_WIDTH] = 91;
		gpBuffer[addr + 0 + 20 * BUFFER_WIDTH] = 93;
	}
	// 450[0] - add missing pixels
	for (int i = 10; i < 11; i++) {
	for (int y = 13; y < 16; y++) {
		for (int x = 2; x < 8; x++) {
			if (y > 14 - (x - 2) / 2) {
				BYTE color = 43;
				if (y == 14 - (x - 2) / 2) {
					if ((x & 1) == 0) {
						color = 44;
					} else if (x == 7) {
						color = 77;
					}
				}
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr] = color; // 450[0]
				// gpBuffer[3 * MICRO_WIDTH + xx + (MICRO_HEIGHT * 1 + yy) * BUFFER_WIDTH] = color; // 450[0]
			}
		}
	}
	}

	// mask doors 204[0]
	for (int i = 12; i < 13; i++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 0; x < 16; x++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (x < 14 || (x == 14 && y > 4) || (x == 15 && y > 14 && y < 23)) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask doors 204[2]
	for (int i = 13; i < 14; i++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 0; x < 14; x++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (x < 12 || (x == 12 && y > 12) || (x == 13 && y > 26)) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask doors 204[4]
	for (int i = 14; i < 15; i++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 0; x < 26; x++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (x > 10 && y > 14 - x / 2) {
					continue;
				}
				if (x == 10 && y > 9 && y < 17) {
					continue;
				}
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}
	// mask doors 208[1]
	for (int i = 16; i < 17; i++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 17; x < MICRO_WIDTH; x++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (x > 17 || (x == 17 && y > 5 && y < 23)) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask doors 208[3]
	for (int i = 17; i < 18; i++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 18; x < MICRO_WIDTH; x++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (x > 19 || (x == 19 && y > 0) || (x == 18 && y > 17)) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask doors 208[5]
	for (int i = 18; i < 19; i++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 6; x < MICRO_WIDTH; x++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (x < 20 && y > x / 2 - 1) {
					continue;
				}
				if (x == 20 && y > 9 && y < 17) {
					continue;
				}
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}
	// mask 'doors' 290[4]
	for (int i = 19; i < 20; i++) {
		for (int y = 0; y < 13; y++) {
			for (int x = 0; x < MICRO_WIDTH; x++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y > 13 - (x + 1) / 2) {
					continue;
				}
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}
	// mask 'doors' 294[4], 296[4]
	for (int i = 23; i < 25; i++) {
		for (int y = 0; y < 13; y++) {
			for (int x = 0; x < MICRO_WIDTH; x++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y > 12 - (x + 1) / 2) {
					continue;
				}
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}
	// mask 'doors' 296[6]
	for (int i = 25; i < 26; i++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 0; x < MICRO_WIDTH; x++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y > 44 - (x + 1) / 2) {
					continue;
				}
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}
	// mask 'doors' 292[0]
	for (int i = 20; i < 21; i++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 0; x < 24; x++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (x == 23 && (y < 17 || y > 18)) {
					continue;
				}
				if (x == 22 && (y < 16 || y > 19)) {
					continue;
				}
				if (x == 21 && (y == 21 || y == 22)) {
					continue;
				}
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}
	// mask 'doors' 292[2]
	for (int i = 21; i < 22; i++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 0; x < 26; x++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (x == 25 && y > 7) {
					continue;
				}
				if (x == 24 && y > 13) {
					continue;
				}
				if (x == 23 && y > 20) {
					continue;
				}
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}
	// mask 'doors' 292[4]
	for (int i = 22; i < 23; i++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 0; x < 30; x++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (x == 29 && (y < 14 || y > 17)) {
					continue;
				}
				if (x == 28 && (y < 13 || y > 18)) {
					continue;
				}
				if (x == 27 && ((y > 1 && y < 13) || y > 18)) {
					continue;
				}
				if (x == 26 && ((y > 2 && y < 10) || y > 30)) {
					continue;
				}
				if (x == 25 && (y > 3 && y < 6)) {
					continue;
				}
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}
	// mask the new special cels 29[4], 272[4], 454[4], 557[4], 559[4]
	for (int i = 34; i < 39; i++) {
		for (int y = 0; y < 17; y++) {
			for (int x = 0; x < MICRO_WIDTH; x++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y > 25 - x) {
					continue;
				}
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}
	// mask the new special cels 413[4]
	for (int i = 39; i < 40; i++) {
		for (int y = 0; y < 9; y++) {
			for (int x = 0; x < 11; x++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y > 10 - x) {
					continue;
				}
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}
	// mask the new special cels 300[4]
	for (int i = 40; i < 41; i++) {
		for (int y = 0; y < 14; y++) {
			for (int x = 0; x < MICRO_WIDTH; x++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (x > 3 && y > 12 - ((x + 1) / 2)) {
					continue;
				}
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}
	// mask the new special cels 33[5]
	for (int i = 41; i < 48; i++) {
		for (int y = 0; y < 17; y++) {
			for (int x = 0; x < MICRO_WIDTH; x++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y > x - 5) {
					continue;
				}
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}

	{ // 206[0] - fix bad artifact
		int i = 11;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 26 + 4 * BUFFER_WIDTH] = 45;
	}
	{ // 559[4] - fix bad artifact after masking
		int i = 38;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 15 + 11 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 14 + 12 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 13 + 13 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 12 + 14 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 11 + 15 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 10 + 16 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // 563[5] - fix bad artifact after masking
		int i = 46;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 17 + 13 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 17 + 14 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 18 + 14 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 18 + 15 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 19 + 15 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 19 + 16 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 20 + 16 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // 258[6] - fix micros after reuse
		int i = 48;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  3 + 3 * BUFFER_WIDTH] = 42;
		gpBuffer[addr +  4 + 3 * BUFFER_WIDTH] = 42;
		gpBuffer[addr +  6 + 2 * BUFFER_WIDTH] = 42;
		gpBuffer[addr +  8 + 1 * BUFFER_WIDTH] = 45;
		gpBuffer[addr + 10 + 0 * BUFFER_WIDTH] = 44;
	}
	{ // 256[6] - fix micros after reuse
		int i = 49;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 0 + 5 * BUFFER_WIDTH] = 41;
		gpBuffer[addr + 1 + 5 * BUFFER_WIDTH] = 41;
		gpBuffer[addr + 2 + 4 * BUFFER_WIDTH] = 42;
		gpBuffer[addr + 3 + 4 * BUFFER_WIDTH] = 41;
		gpBuffer[addr + 3 + 0 * BUFFER_WIDTH] = 45;
		gpBuffer[addr + 3 + 1 * BUFFER_WIDTH] = 45;
		gpBuffer[addr + 3 + 2 * BUFFER_WIDTH] = 45;
	}

	// create the new CEL file
	size_t maxCelSize = *celLen + lengthof(micros) * MICRO_WIDTH * MICRO_HEIGHT;
	BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
	memset(resCelBuf, 0, maxCelSize);

	CelFrameEntry entries[lengthof(micros)];
	xx = 0, yy = MICRO_HEIGHT - 1;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		entries[i].encoding = micro.res_encoding;
		unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
		entries[i].frameRef = SwapLE16(pSubtiles[index]) & 0xFFF;
		entries[i].frameSrc = &gpBuffer[xx + yy * BUFFER_WIDTH];
		yy += MICRO_HEIGHT;
		if (yy == (DRAW_HEIGHT + 1) * MICRO_HEIGHT - 1) {
			yy = MICRO_HEIGHT - 1;
			xx += MICRO_WIDTH;
		}
	}

	*celLen = encodeCelMicros(entries, lengthof(entries), resCelBuf, celBuf, TRANS_COLOR);

	mem_free_dbg(celBuf);

	return resCelBuf;
}

static BYTE* maskCryptBlacks(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	const CelMicro micros[] = {
		{ 126 - 1, 1 },
		{ 129 - 1, 0 },
		{ 129 - 1, 1 },
		{ 131 - 1, 0 },
		{ 131 - 1, 1 },
		{ 132 - 1, 0 },
		{ 133 - 1, 0 },
		{ 133 - 1, 1 },
		{ 134 - 1, 3 },
		{ 135 - 1, 0 },
		{ 135 - 1, 1 },
		{ 142 - 1, 0 },
//		{ 146 - 1, 0 },
//		{ 149 - 1, 4 },
//		{ 150 - 1, 6 },
//		{ 151 - 1, 2 },
		{ 151 - 1, 4 },
		{ 151 - 1, 5 },
		{ 152 - 1, 7 },
		{ 153 - 1, 2 },
		{ 153 - 1, 4 },
		{ 159 - 1, 1 },
	};

	const uint16_t* pSubtiles = (const uint16_t*)minBuf;

	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_L5;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
		if ((SwapLE16(pSubtiles[index]) & 0xFFF) == 0) {
			mem_free_dbg(celBuf);
			app_warn("Invalid (empty) crypt on subtile (%d).", micro.subtileIndex + 1);
			return NULL;
		}
	}

	// TODO: check celLen
	// draw the micros to the back-buffer
	pMicrosCel = celBuf;
	constexpr BYTE TRANS_COLOR = 128;
	constexpr int DRAW_HEIGHT = 4;
	memset(&gpBuffer[0], TRANS_COLOR, DRAW_HEIGHT * BUFFER_WIDTH * MICRO_HEIGHT);

	unsigned xx = 0, yy = MICRO_HEIGHT - 1;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
		RenderMicro(&gpBuffer[xx + yy * BUFFER_WIDTH], SwapLE16(pSubtiles[index]), DMT_NONE);
		yy += MICRO_HEIGHT;
		if (yy == (DRAW_HEIGHT + 1) * MICRO_HEIGHT - 1) {
			yy = MICRO_HEIGHT - 1;
			xx += MICRO_WIDTH;
		}
	}

	// mask the black pixels
	xx = 0, yy = 0;
	for (int i = 0; i < lengthof(micros); i++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 0; x < MICRO_WIDTH; x++) {
				BYTE* pixel = &gpBuffer[xx + x + (yy + y) * BUFFER_WIDTH];
				if (*pixel == 79) {
					if (i == 0 && x < 9) { // 126[1]
						continue;
					}
					if (i == 1 && y < 10) { // 129[0]
						continue;
					}
					if (i == 6 && y < 10) { // 133[0]
						continue;
					}
					if (i == 7 && y == 0) { // 133[1]
						continue;
					}
					if (i == 12 && (y < MICRO_HEIGHT - (x - 4) / 2)) { // 151[4]
						continue;
					}
					if (i == 13 && ((x < 6 && y < 18) || (x >= 6 && y < 18 - (x - 6) / 2))) { // 151[5]
						continue;
					}
					if (i == 14 && (y < 16 - (x - 3) / 2)) { // 152[7]
						continue;
					}
					if (i == 16 && y < 21) { // 153[4]
						continue;
					}
					if (i == 17 && x < 10 && y < 3) { // 159[1]
						continue;
					}
					*pixel = TRANS_COLOR;
				}
			}
		}
		yy += MICRO_HEIGHT;
		if (yy == DRAW_HEIGHT * MICRO_HEIGHT) {
			yy = 0;
			xx += MICRO_WIDTH;
		}
	}

	// create the new CEL file
	size_t maxCelSize = *celLen + lengthof(micros) * MICRO_WIDTH * MICRO_HEIGHT;
	BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
	memset(resCelBuf, 0, maxCelSize);

	CelFrameEntry entries[lengthof(micros)];
	xx = 0, yy = MICRO_HEIGHT - 1;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		entries[i].encoding = MET_TRANSPARENT;
		unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
		entries[i].frameRef = SwapLE16(pSubtiles[index]) & 0xFFF;
		entries[i].frameSrc = &gpBuffer[xx + yy * BUFFER_WIDTH];
		yy += MICRO_HEIGHT;
		if (yy == (DRAW_HEIGHT + 1) * MICRO_HEIGHT - 1) {
			yy = MICRO_HEIGHT - 1;
			xx += MICRO_WIDTH;
		}
	}

	*celLen = encodeCelMicros(entries, lengthof(entries), resCelBuf, celBuf, TRANS_COLOR);

	mem_free_dbg(celBuf);

	return resCelBuf;
}

static BYTE* fixCryptShadows(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	const CelMicro micros[] = {
/*  0 */{ 626 - 1, 0, MET_LTRIANGLE },
/*  1 */{ 626 - 1, 1, MET_RTRIANGLE },
/*  2 */{ /*627*/ - 1, 0, -1 },
/*  3 */{ 638 - 1, 1, MET_RTRIANGLE },
/*  4 */{ 639 - 1, 0, MET_LTRIANGLE },
/*  5 */{ 639 - 1, 1, MET_RTRIANGLE },
/*  6 */{ /*631*/ - 1, 0, -1 },
/*  7 */{ 634 - 1, 0, MET_LTRIANGLE },
/*  8 */{ 634 - 1, 1, MET_RTRIANGLE },

/*  9 */{ 277 - 1, 1, MET_RTRIANGLE },
/* 10 */{ 303 - 1, 1, MET_RTRIANGLE },

/* 11 */{ 620 - 1, 0, MET_RTRIANGLE },
/* 12 */{ 621 - 1, 1, MET_SQUARE },
/* 13 */{ 625 - 1, 0, MET_RTRIANGLE },
/* 14 */{ 624 - 1, 0, MET_TRANSPARENT },
/* 15 */{ 15 - 1, 1, -1 },
/* 16 */{ 15 - 1, 2, -1 },
/* 17 */{ 89 - 1, 1, -1 },
/* 18 */{ 89 - 1, 2, -1 },

/* 19 */{ 619 - 1, 1, MET_LTRAPEZOID },
	};

	const uint16_t* pSubtiles = (const uint16_t*)minBuf;

	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_L5;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		if (micro.subtileIndex < 0) {
			continue;
		}
		unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
		if ((SwapLE16(pSubtiles[index]) & 0xFFF) == 0) {
			mem_free_dbg(celBuf);
			app_warn("Invalid (empty) crypt on subtile (%d).", micro.subtileIndex + 1);
			return NULL;
		}
	}

	// TODO: check celLen
	// draw the micros to the back-buffer
	pMicrosCel = celBuf;
	constexpr BYTE TRANS_COLOR = 128;
	constexpr BYTE SHADOW_COLOR = 0;
	constexpr BYTE DRAW_HEIGHT = 3;
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

	// extend the shadows to NE: 626[0], 626[1]
	for (int i = 0; i < 2; i++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 0; x < MICRO_WIDTH; x++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color == TRANS_COLOR) {
					continue;
				}
				if (i == 0 && color != 79 && y <= (x / 2) + 13 - MICRO_HEIGHT / 2) { // 626[0]
					continue;
				}
				if (i == 1 && color != 79 && y <= (x / 2) + 13) { // 626[1]
					continue;
				}
				//if (i == 2 && color != 79 && y <= (x / 2) + 13 - MICRO_HEIGHT / 2) { // 627[0]
				//	continue;
				//}
				gpBuffer[addr] = SHADOW_COLOR;
			}
		}
	}
	// extend the shadows to NW: 638[0], 639[0], 639[1]
	for (int i = 3; i < 6; i++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 0; x < MICRO_WIDTH; x++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color == TRANS_COLOR) {
					continue;
				}
				if (i == 3 && color != 79 && y <= 13 - (x / 2)) { // 638[1]
					continue;
				}
				if (i == 4 && color != 79 && (x > 19 || y > 23) && (x != 16 || y != 24)) { // 639[0]
					continue;
				}
				if (i == 5 && color != 79 && x <= 7) { // 639[1]
					continue;
				}
				gpBuffer[addr] = SHADOW_COLOR;
			}
		}
	}
	// extend the shadows to NW: 631[1]
	/*for (int i = 6; i < 7; i++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 0; x < MICRO_WIDTH; x++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color == TRANS_COLOR) {
					continue;
				}
				if (i == 6 && color != 79 && y <= (x / 2) + 15) { // 631[1]
					continue;
				}
				gpBuffer[addr] = SHADOW_COLOR;
			}
		}
	}*/
	// extend the shadows to NE: 634[0], 634[1]
	for (int i = 7; i < 9; i++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 0; x < MICRO_WIDTH; x++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color == TRANS_COLOR) {
					continue;
				}
				if (i == 7 && color != 79
				&& (y <= (x / 2) - 3 || (x >= 20 && y >= 14 && color >= 59 && color <= 95 && (color >= 77 || color <= 63)))) { // 634[0]
					continue;
				}
				if (i == 8 && color != 79
				&& (y <= (x / 2) + 13 || (x <= 8 && y >= 12 && color >= 62 && color <= 95 && (color >= 80 || color <= 63)))) { // 634[1]
					continue;
				}
				gpBuffer[addr] = SHADOW_COLOR;
			}
		}
	}
	// use consistent lava + shadow micro II. 277[1]
	for (int i = 9; i < 10; i++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 0; x < MICRO_WIDTH; x++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color == TRANS_COLOR) {
					continue;
				}
				BYTE pixelSrc = gpBuffer[x + MICRO_WIDTH * (10 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (10 % DRAW_HEIGHT)) * BUFFER_WIDTH]; // 303[1]
				if (pixelSrc == TRANS_COLOR) {
					continue;
				}
				if (x > 11) {
					continue;
				}
				if (x == 11 && (y < 9 || (y >= 17 && y <= 20))) {
					continue;
				}
				if (x == 10 && (y == 18 || y == 19)) {
					continue;
				}
				gpBuffer[addr] = pixelSrc;
			}
		}
	}
	// draw the new micros
	for (int i = 11; i < 15; i++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 0; x < MICRO_WIDTH; x++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE srcPixel = gpBuffer[x + MICRO_WIDTH * ((i + 4) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 4) % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (i == 11) { // 260[0]
					// wall/floor in shadow
					if (x <= 1) {
						if (y >= 4 * x) {
							srcPixel = SHADOW_COLOR;
						}
					} else if (x <= 3) {
						if (y > 6 + (x - 1) / 2) {
							srcPixel = SHADOW_COLOR;
						}
					} else if (x <= 5) {
						if (y >= 7 + 4 * (x - 3)) {
							srcPixel = SHADOW_COLOR;
						}
					} else {
						if (y > 14 + (x - 1) / 2) {
							srcPixel = SHADOW_COLOR;
						}
					}
				}
				if (i == 13) { // 625[0]
					// grate/floor in shadow
					if (x <= 1 && y >= 7 * x) {
						srcPixel = SHADOW_COLOR;
					}
					if (x > 1 && y > 14 + (x - 1) / 2) {
						srcPixel = SHADOW_COLOR;
					}
				}
				if (i == 12 || i == 14) { // 621[1], 624[0]
					// wall/grate in shadow
					if (y >= 7 * (x - 27) && srcPixel != TRANS_COLOR) {
						srcPixel = SHADOW_COLOR;
					}
				}
				gpBuffer[addr] = srcPixel;
			}
		}
	}
	// create shadow micro - 619[1]
	for (int i = 19; i < 20; i++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 0; x < MICRO_WIDTH; x++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr] = SHADOW_COLOR;
			}
		}
	}

	// fix bad artifacts
	{ // 634[0]
		int i = 7;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 22 + 20 * BUFFER_WIDTH] = TRANS_COLOR;
	}

	// create the new CEL file
	size_t maxCelSize = *celLen + lengthof(micros) * MICRO_WIDTH * MICRO_HEIGHT;
	BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
	memset(resCelBuf, 0, maxCelSize);

	CelFrameEntry entries[lengthof(micros)];
	xx = 0, yy = MICRO_HEIGHT - 1;
	int idx = 0;
	for (int i = 0; i < lengthof(micros); i++) {
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

BYTE* DRLP_L5_PatchCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	celBuf = patchCryptFloorCel(minBuf, minLen, celBuf, celLen);
	if (celBuf == NULL) {
		return NULL;
	}
	celBuf = maskCryptBlacks(minBuf, minLen, celBuf, celLen);
	if (celBuf == NULL) {
		return NULL;
	}
	celBuf = fixCryptShadows(minBuf, minLen, celBuf, celLen);
	return celBuf;
}

void DRLP_L5_PatchMin(BYTE* buf)
{
	uint16_t* pSubtiles = (uint16_t*)buf;
	constexpr int blockSize = BLOCK_SIZE_L5;
	// adjust the frame types
	// - after patchCryptFloorCel
	SetFrameType(159, 3, MET_SQUARE);
	SetFrameType(185, 3, MET_SQUARE);
	// SetFrameType(159, 1, MET_RTRAPEZOID);
	SetFrameType(336, 0, MET_LTRIANGLE);
	SetFrameType(409, 0, MET_LTRIANGLE);
	SetFrameType(481, 1, MET_RTRIANGLE);
	SetFrameType(492, 0, MET_LTRIANGLE);
	SetFrameType(519, 0, MET_LTRIANGLE);
	SetFrameType(595, 1, MET_RTRIANGLE);
	SetFrameType(206, 0, MET_LTRIANGLE);
	SetFrameType(204, 0, MET_TRANSPARENT);
	// SetFrameType(212, 0, MET_TRANSPARENT);
	// SetFrameType(215, 0, MET_TRANSPARENT);
	// SetFrameType(220, 0, MET_TRANSPARENT);
	// SetFrameType(224, 0, MET_TRANSPARENT);
	// SetFrameType(226, 0, MET_TRANSPARENT);
	// SetFrameType(232, 0, MET_TRANSPARENT);
	SetFrameType(204, 2, MET_TRANSPARENT);
	// SetFrameType(212, 2, MET_TRANSPARENT);
	// SetFrameType(215, 2, MET_TRANSPARENT);
	// SetFrameType(220, 2, MET_TRANSPARENT);
	// SetFrameType(224, 2, MET_TRANSPARENT);
	// SetFrameType(226, 2, MET_TRANSPARENT);
	// SetFrameType(232, 2, MET_TRANSPARENT);
	SetFrameType(204, 4, MET_TRANSPARENT);
	// SetFrameType(212, 4, MET_TRANSPARENT);
	// SetFrameType(215, 4, MET_TRANSPARENT);
	// SetFrameType(220, 4, MET_TRANSPARENT);
	// SetFrameType(224, 4, MET_TRANSPARENT);
	// SetFrameType(226, 4, MET_TRANSPARENT);
	// SetFrameType(232, 4, MET_TRANSPARENT);
	SetFrameType(209, 1, MET_RTRIANGLE);
	SetFrameType(208, 1, MET_TRANSPARENT);
	// SetFrameType(212, 1, MET_TRANSPARENT);
	// SetFrameType(218, 1, MET_TRANSPARENT);
	// SetFrameType(222, 1, MET_TRANSPARENT);
	// SetFrameType(228, 1, MET_TRANSPARENT);
	// SetFrameType(230, 1, MET_TRANSPARENT);
	// SetFrameType(234, 1, MET_TRANSPARENT);
	SetFrameType(208, 3, MET_TRANSPARENT);
	// SetFrameType(212, 3, MET_TRANSPARENT);
	// SetFrameType(218, 3, MET_TRANSPARENT);
	// SetFrameType(222, 3, MET_TRANSPARENT);
	// SetFrameType(228, 3, MET_TRANSPARENT);
	// SetFrameType(230, 3, MET_TRANSPARENT);
	// SetFrameType(234, 3, MET_TRANSPARENT);
	SetFrameType(208, 5, MET_TRANSPARENT);
	// SetFrameType(212, 5, MET_TRANSPARENT);
	// SetFrameType(218, 5, MET_TRANSPARENT);
	// SetFrameType(222, 5, MET_TRANSPARENT);
	// SetFrameType(228, 5, MET_TRANSPARENT);
	// SetFrameType(230, 5, MET_TRANSPARENT);
	// SetFrameType(234, 5, MET_TRANSPARENT);
	// -- with the new special cels
	SetFrameType(274, 0, MET_LTRIANGLE);
	SetFrameType(299, 0, MET_LTRIANGLE);
	SetFrameType(404, 0, MET_LTRIANGLE);
	SetFrameType(415, 0, MET_LTRIANGLE);
	SetFrameType(456, 0, MET_LTRIANGLE);
	SetFrameType(18, 1, MET_RTRIANGLE);
	SetFrameType(463, 1, MET_RTRIANGLE);
	SetFrameType(277, 1, MET_RTRIANGLE);
	SetFrameType(444, 1, MET_RTRIANGLE);
	SetFrameType(471, 1, MET_RTRIANGLE);
	SetFrameType(290, 4, MET_TRANSPARENT);
	SetFrameType(292, 0, MET_TRANSPARENT);
	SetFrameType(292, 2, MET_TRANSPARENT);
	SetFrameType(292, 4, MET_TRANSPARENT);
	SetFrameType(294, 4, MET_TRANSPARENT);
	SetFrameType(296, 4, MET_TRANSPARENT);
	SetFrameType(296, 6, MET_TRANSPARENT);
	// - after maskCryptBlacks
	SetFrameType(126, 1, MET_TRANSPARENT);
	SetFrameType(129, 0, MET_TRANSPARENT);
	SetFrameType(129, 1, MET_TRANSPARENT);
	SetFrameType(131, 0, MET_TRANSPARENT);
	SetFrameType(131, 1, MET_TRANSPARENT);
	SetFrameType(132, 0, MET_TRANSPARENT);
	SetFrameType(133, 0, MET_TRANSPARENT);
	SetFrameType(133, 1, MET_TRANSPARENT);
	SetFrameType(134, 3, MET_TRANSPARENT);
	SetFrameType(135, 0, MET_TRANSPARENT);
	SetFrameType(135, 1, MET_TRANSPARENT);
	SetFrameType(142, 0, MET_TRANSPARENT);
//	SetFrameType(146, 0, MET_TRANSPARENT);
//	SetFrameType(149, 4, MET_TRANSPARENT);
//	SetFrameType(150, 6, MET_TRANSPARENT);
	SetFrameType(151, 2, MET_TRANSPARENT);
	SetFrameType(151, 4, MET_TRANSPARENT);
	SetFrameType(151, 5, MET_TRANSPARENT);
	SetFrameType(152, 7, MET_TRANSPARENT);
	SetFrameType(153, 2, MET_TRANSPARENT);
	SetFrameType(153, 4, MET_TRANSPARENT);
	SetFrameType(159, 1, MET_TRANSPARENT);
//	SetFrameType(178, 2, MET_TRANSPARENT);
	// - after fixCryptShadows
	// SetFrameType(630, 0, MET_TRANSPARENT);
	SetFrameType(620, 0, MET_RTRIANGLE);
	SetFrameType(621, 1, MET_SQUARE);
	SetFrameType(625, 0, MET_RTRIANGLE);
	SetFrameType(624, 0, MET_TRANSPARENT);
	SetFrameType(619, 1, MET_LTRAPEZOID);
	// prepare subtiles after fixCryptShadows
	ReplaceMcr(3, 0, 619, 1);
	ReplaceMcr(3, 1, 620, 0);
	ReplaceMcr(3, 2, 621, 1);
	Blk2Mcr(3, 4);
	ReplaceMcr(3, 6, 15, 6);
	// SetMcr(242, 0, 630, 0);
	SetMcr(242, 0, 626, 0);
	ReplaceMcr(242, 1, 626, 1);
	// SetMcr(242, 2, 31, 2);
	Blk2Mcr(242, 4);
	// ReplaceMcr(242, 6, 89, 6);
	// SetMcr(242, 8, 89, 8);
	Blk2Mcr(242, 6); // - with the new special cels
	HideMcr(242, 8);
	ReplaceMcr(178, 0, 619, 1);
	ReplaceMcr(178, 1, 625, 0);
	SetMcr(178, 2, 624, 0);
	Blk2Mcr(178, 4);
	ReplaceMcr(178, 6, 89, 6);
	ReplaceMcr(178, 8, 89, 8);
	ReplaceMcr(238, 0, 634, 0);
	ReplaceMcr(238, 1, 634, 1);
	Blk2Mcr(238, 4);
	// SetMcr(238, 6, 89, 6);
	// SetMcr(238, 8, 89, 8);
	HideMcr(238, 6); // - with the new special cels
	HideMcr(238, 8);
	// pointless door micros (re-drawn by dSpecial or the object)
	ReplaceMcr(77, 0, 206, 0);
	ReplaceMcr(77, 1, 206, 1);
	Blk2Mcr(77, 2);
	Blk2Mcr(77, 4);
	Blk2Mcr(77, 6);
	Blk2Mcr(77, 8);
	ReplaceMcr(76, 0, 206, 0);
	ReplaceMcr(76, 1, 206, 1);
	// ReplaceMcr(75, 0, 204, 0);
	// ReplaceMcr(75, 1, 204, 1);
	// ReplaceMcr(75, 2, 204, 2);
	// ReplaceMcr(75, 4, 204, 4);
	// ReplaceMcr(91, 0, 204, 0);
	// ReplaceMcr(91, 2, 204, 2);
	// ReplaceMcr(91, 4, 204, 4);
	ReplaceMcr(99, 0, 204, 0);
	ReplaceMcr(99, 2, 204, 2);
	ReplaceMcr(99, 4, 204, 4);
	ReplaceMcr(99, 6, 119, 6);
	ReplaceMcr(113, 0, 204, 0);
	ReplaceMcr(113, 2, 204, 2);
	ReplaceMcr(113, 4, 204, 4);
	ReplaceMcr(113, 6, 119, 6);
	ReplaceMcr(115, 0, 204, 0);
	ReplaceMcr(115, 2, 204, 2);
	ReplaceMcr(115, 4, 204, 4);
	ReplaceMcr(115, 6, 119, 6);
	ReplaceMcr(204, 6, 119, 6);
	ReplaceMcr(80, 0, 209, 0);
	ReplaceMcr(80, 1, 209, 1);
	Blk2Mcr(80, 3);
	Blk2Mcr(80, 5);
	Blk2Mcr(80, 7);
	Blk2Mcr(80, 9);
	ReplaceMcr(79, 0, 209, 0);
	ReplaceMcr(79, 1, 209, 1);
	Blk2Mcr(79, 3);
	Blk2Mcr(79, 5);
	Blk2Mcr(79, 7);
	Blk2Mcr(79, 9);
	// ReplaceMcr(79, 0, 208, 0);
	// ReplaceMcr(79, 1, 208, 1);
	// ReplaceMcr(79, 3, 208, 3);
	// ReplaceMcr(79, 5, 208, 5);
	ReplaceMcr(93, 1, 208, 1);
	ReplaceMcr(93, 3, 208, 3);
	ReplaceMcr(93, 5, 208, 5);
	ReplaceMcr(111, 1, 208, 1);
	ReplaceMcr(111, 3, 208, 3);
	ReplaceMcr(111, 5, 208, 5);
	ReplaceMcr(117, 1, 208, 1);
	ReplaceMcr(117, 3, 208, 3);
	ReplaceMcr(117, 5, 208, 5);
	ReplaceMcr(119, 1, 208, 1);
	ReplaceMcr(119, 3, 208, 3);
	ReplaceMcr(119, 5, 208, 5);
	Blk2Mcr(206, 2);
	Blk2Mcr(206, 4);
	Blk2Mcr(206, 6);
	Blk2Mcr(206, 8);
	Blk2Mcr(209, 3);
	Blk2Mcr(209, 5);
	Blk2Mcr(209, 7);
	Blk2Mcr(209, 9);
	// Blk2Mcr(213, 6);
	// Blk2Mcr(213, 8);
	// Blk2Mcr(216, 6);
	// Blk2Mcr(216, 8);
	// useless black micros
	Blk2Mcr(130, 0);
	Blk2Mcr(130, 1);
	Blk2Mcr(132, 1);
	Blk2Mcr(134, 0);
	Blk2Mcr(134, 1);
	Blk2Mcr(149, 0);
	Blk2Mcr(149, 1);
	Blk2Mcr(149, 2);
	Blk2Mcr(150, 0);
	Blk2Mcr(150, 1);
	Blk2Mcr(150, 2);
	Blk2Mcr(150, 4);
	Blk2Mcr(151, 0);
	Blk2Mcr(151, 1);
	Blk2Mcr(151, 3);
	Blk2Mcr(152, 0);
	Blk2Mcr(152, 1);
	Blk2Mcr(152, 3);
	Blk2Mcr(152, 5);
	Blk2Mcr(153, 0);
	Blk2Mcr(153, 1);
	// fix bad artifact
	Blk2Mcr(156, 2);
	// useless black micros
	// Blk2Mcr(172, 0);
	// Blk2Mcr(172, 1);
	// Blk2Mcr(172, 2);
	Blk2Mcr(173, 0);
	Blk2Mcr(173, 1);
	// Blk2Mcr(174, 0);
	// Blk2Mcr(174, 1);
	// Blk2Mcr(174, 2);
	// Blk2Mcr(174, 4);
	Blk2Mcr(175, 0);
	Blk2Mcr(175, 1);
	// Blk2Mcr(176, 0);
	// Blk2Mcr(176, 1);
	// Blk2Mcr(176, 3);
	// Blk2Mcr(177, 0);
	// Blk2Mcr(177, 1);
	// Blk2Mcr(177, 3);
	// Blk2Mcr(177, 5);
	// Blk2Mcr(178, 0);
	// Blk2Mcr(178, 1);
	Blk2Mcr(179, 0);
	Blk2Mcr(179, 1);
	// fix 'bad' artifact
	// Blk2Mcr(398, 5);
	// fix graphical glitch
	ReplaceMcr(21, 1, 55, 1);
	ReplaceMcr(25, 0, 33, 0);
	// ReplaceMcr(22, 0, 2, 0);
	// ReplaceMcr(22, 1, 2, 1);
	ReplaceMcr(336, 1, 4, 1);
	ReplaceMcr(339, 0, 33, 0);
	// ReplaceMcr(391, 1, 335, 1); - the whole right side is replaced
	ReplaceMcr(393, 0, 33, 0);
	ReplaceMcr(421, 0, 399, 0);
	ReplaceMcr(421, 2, 399, 2);
	// - fix crack in the chair
	ReplaceMcr(162, 5, 154, 5);
	ReplaceMcr(162, 3, 154, 3);
	// - use consistent lava + shadow micro I.
	ReplaceMcr(277, 0, 303, 0);
	ReplaceMcr(562, 0, 303, 0);
	// ReplaceMcr(564, 0, 303, 0);
	ReplaceMcr(635, 0, 308, 0);
	// - extend shadow to make more usable (after fixCryptShadows)
	// ReplaceMcr(627, 0, 626, 0);
	// SetMcr(627, 1, 626, 1);
	// prepare new subtiles for the shadows
	ReplaceMcr(623, 0, 631, 0);
	ReplaceMcr(623, 1, 638, 1);
	ReplaceMcr(636, 0, 626, 0);
	ReplaceMcr(636, 1, 638, 1);
	// fix automap of the entrance I.
	Blk2Mcr(148, 0);
	Blk2Mcr(148, 1);
	// with the new special cels
	ReplaceMcr(31, 0, 4, 0);
	// ReplaceMcr(468, 0, 4, 0);
	ReplaceMcr(333, 0, 31, 0); // lost details
	ReplaceMcr(345, 0, 31, 0); // lost details
	ReplaceMcr(356, 0, 7, 0);
	ReplaceMcr(445, 0, 7, 0); // lost details
	ReplaceMcr(404, 0, 474, 0); // lost details
	// TODO: 274[0], 277[1] ?
	Blk2Mcr(31, 2);
	// Blk2Mcr(468, 2);
	Blk2Mcr(274, 2);
	Blk2Mcr(299, 2);
	Blk2Mcr(333, 2);
	Blk2Mcr(345, 2);
	Blk2Mcr(356, 2);
	Blk2Mcr(404, 2);
	Blk2Mcr(415, 2);
	Blk2Mcr(445, 2);
	Blk2Mcr(456, 2);
	ReplaceMcr(331, 4, 29, 4);
	ReplaceMcr(343, 4, 29, 4);
	ReplaceMcr(355, 4, 29, 4);
	ReplaceMcr(363, 4, 29, 4);
	ReplaceMcr(443, 4, 29, 4);
	Blk2Mcr(31, 4);
	// Blk2Mcr(468, 4);
	Blk2Mcr(274, 4);
	Blk2Mcr(298, 4);
	Blk2Mcr(299, 4);
	Blk2Mcr(301, 4);
	Blk2Mcr(333, 4);
	Blk2Mcr(345, 4);
	Blk2Mcr(356, 4);
	Blk2Mcr(404, 4);
	Blk2Mcr(415, 4);
	Blk2Mcr(445, 4);
	Blk2Mcr(456, 4);
	Blk2Mcr(31, 6);
	// Blk2Mcr(468, 6);
	Blk2Mcr(274, 6);
	Blk2Mcr(298, 6);
	Blk2Mcr(299, 6);
	ReplaceMcr(300, 6, 119, 6); // lost details
	ReplaceMcr(294, 6, 119, 6); // lost details
	ReplaceMcr(454, 6, 252, 6); // lost details
	ReplaceMcr(413, 6, 25, 6); // lost details
	ReplaceMcr(331, 6, 25, 6); // lost details
	ReplaceMcr(343, 6, 25, 6); // lost details
	ReplaceMcr(355, 6, 25, 6); // lost details
	ReplaceMcr(363, 6, 25, 6); // lost details
	ReplaceMcr(557, 6, 25, 6); // lost details
	ReplaceMcr(559, 6, 25, 6); // lost details
	ReplaceMcr(290, 6, 296, 6);
	ReplaceMcr(292, 6, 296, 6);
	Blk2Mcr(301, 6);
	Blk2Mcr(333, 6);
	Blk2Mcr(345, 6);
	Blk2Mcr(356, 6);
	Blk2Mcr(404, 6);
	Blk2Mcr(415, 6);
	Blk2Mcr(445, 6);
	Blk2Mcr(456, 6);
	Blk2Mcr(31, 8);
	// Blk2Mcr(468, 8);
	Blk2Mcr(274, 8);
	Blk2Mcr(290, 8);
	Blk2Mcr(292, 8);
	Blk2Mcr(296, 8);
	Blk2Mcr(298, 8);
	Blk2Mcr(299, 8);
	Blk2Mcr(301, 8);
	Blk2Mcr(333, 8);
	Blk2Mcr(345, 8);
	Blk2Mcr(356, 8);
	Blk2Mcr(404, 8);
	Blk2Mcr(415, 8);
	Blk2Mcr(445, 8);
	Blk2Mcr(456, 8);

	ReplaceMcr(348, 1, 18, 1);
	ReplaceMcr(352, 1, 18, 1);
	ReplaceMcr(358, 1, 18, 1);
	ReplaceMcr(406, 1, 18, 1);
	ReplaceMcr(459, 1, 18, 1);
	Blk2Mcr(18, 3);
	Blk2Mcr(277, 3);
	Blk2Mcr(332, 3);
	Blk2Mcr(348, 3);
	Blk2Mcr(352, 3);
	Blk2Mcr(358, 3);
	Blk2Mcr(406, 3);
	Blk2Mcr(444, 3); // lost details
	Blk2Mcr(459, 3);
	Blk2Mcr(463, 3);
	Blk2Mcr(471, 3);
	Blk2Mcr(562, 3);
	ReplaceMcr(470, 5, 276, 5);
	Blk2Mcr(18, 5);
	Blk2Mcr(277, 5);
	Blk2Mcr(332, 5);
	Blk2Mcr(348, 5);
	Blk2Mcr(352, 5);
	Blk2Mcr(358, 5);
	Blk2Mcr(406, 5);
	Blk2Mcr(444, 5);
	Blk2Mcr(459, 5);
	Blk2Mcr(463, 5);
	Blk2Mcr(471, 5);
	Blk2Mcr(562, 5);
	ReplaceMcr(347, 7, 13, 7); // lost details
	ReplaceMcr(276, 7, 13, 7); // lost details
	ReplaceMcr(458, 7, 13, 7); // lost details
	ReplaceMcr(561, 7, 13, 7); // lost details
	ReplaceMcr(563, 7, 13, 7); // lost details
	Blk2Mcr(18, 7);
	Blk2Mcr(277, 7);
	Blk2Mcr(332, 7);
	Blk2Mcr(348, 7);
	Blk2Mcr(352, 7);
	Blk2Mcr(358, 7);
	Blk2Mcr(406, 7);
	Blk2Mcr(444, 7);
	Blk2Mcr(459, 7);
	Blk2Mcr(463, 7);
	Blk2Mcr(471, 7);
	Blk2Mcr(562, 7);
	Blk2Mcr(18, 9);
	Blk2Mcr(277, 9);
	Blk2Mcr(332, 9);
	Blk2Mcr(348, 9);
	Blk2Mcr(352, 9);
	Blk2Mcr(358, 9);
	Blk2Mcr(406, 9);
	Blk2Mcr(444, 9);
	Blk2Mcr(459, 9);
	Blk2Mcr(463, 9);
	Blk2Mcr(471, 9);
	Blk2Mcr(562, 9);
	// subtile for the separate pillar tile
	// - 91 == 9
	ReplaceMcr(91, 0, 33, 0);
	ReplaceMcr(91, 1, 55, 1);
	ReplaceMcr(91, 2, 29, 2);
	SetMcr(91, 3, 21, 3);
	ReplaceMcr(91, 4, 25, 4);
	SetMcr(91, 5, 21, 5);
	ReplaceMcr(91, 6, 25, 6);
	SetMcr(91, 7, 21, 7);
	ReplaceMcr(91, 8, 9, 8);
	ReplaceMcr(91, 9, 9, 9);
	// reuse subtiles
	ReplaceMcr(631, 1, 626, 1);
	ReplaceMcr(149, 4, 1, 4);
	ReplaceMcr(150, 6, 15, 6);
	ReplaceMcr(324, 7, 6, 7);
	ReplaceMcr(432, 7, 6, 7);
	ReplaceMcr(440, 7, 6, 7);
	// ReplaceMcr(26, 9, 6, 9);
	// ReplaceMcr(340, 9, 6, 9);
	ReplaceMcr(394, 9, 6, 9);
	ReplaceMcr(451, 9, 6, 9);
	// ReplaceMcr(14, 7, 6, 7); // lost shine
	// ReplaceMcr(26, 7, 6, 7); // lost shine
	// ReplaceMcr(80, 7, 6, 7);
	ReplaceMcr(451, 7, 6, 7); // lost shine
	// ReplaceMcr(340, 7, 6, 7); // lost shine
	ReplaceMcr(364, 7, 6, 7); // lost crack
	ReplaceMcr(394, 7, 6, 7); // lost shine
	// ReplaceMcr(554, 7, 269, 7);
	ReplaceMcr(608, 7, 6, 7);   // lost details
	ReplaceMcr(616, 7, 6, 7);   // lost details
	ReplaceMcr(269, 5, 554, 5); // lost details
	ReplaceMcr(556, 5, 554, 5);
	ReplaceMcr(440, 5, 432, 5); // lost details
	// ReplaceMcr(14, 5, 6, 5); // lost details
	// ReplaceMcr(26, 5, 6, 5); // lost details
	ReplaceMcr(451, 5, 6, 5); // lost details
	// ReplaceMcr(80, 5, 6, 5); // lost details
	ReplaceMcr(324, 5, 432, 5); // lost details
	// ReplaceMcr(340, 5, 432, 5); // lost details
	ReplaceMcr(364, 5, 432, 5); // lost details
	ReplaceMcr(380, 5, 432, 5); // lost details
	ReplaceMcr(394, 5, 432, 5); // lost details
	ReplaceMcr(6, 3, 14, 3);    // lost details
	// ReplaceMcr(26, 3, 14, 3); // lost details
	// ReplaceMcr(80, 3, 14, 3); // lost details
	ReplaceMcr(269, 3, 14, 3); // lost details
	ReplaceMcr(414, 3, 14, 3); // lost details
	ReplaceMcr(451, 3, 14, 3); // lost details
	// ReplaceMcr(554, 3, 14, 3); // lost details
	ReplaceMcr(556, 3, 14, 3); // lost details
	// ? ReplaceMcr(608, 3, 103, 3); // lost details
	ReplaceMcr(324, 3, 380, 3); // lost details
	// ReplaceMcr(340, 3, 380, 3); // lost details
	ReplaceMcr(364, 3, 380, 3); // lost details
	ReplaceMcr(432, 3, 380, 3); // lost details
	ReplaceMcr(440, 3, 380, 3); // lost details
	ReplaceMcr(6, 0, 14, 0);
	// ReplaceMcr(26, 0, 14, 0);
	ReplaceMcr(269, 0, 14, 0);
	// ReplaceMcr(554, 0, 14, 0); // lost details
	// ReplaceMcr(340, 0, 324, 0); // lost details
	ReplaceMcr(364, 0, 324, 0); // lost details
	ReplaceMcr(451, 0, 324, 0); // lost details
	// ReplaceMcr(14, 1, 6, 1);
	// ReplaceMcr(26, 1, 6, 1);
	// ReplaceMcr(80, 1, 6, 1);
	ReplaceMcr(269, 1, 6, 1);
	ReplaceMcr(380, 1, 6, 1);
	ReplaceMcr(451, 1, 6, 1);
	// ReplaceMcr(554, 1, 6, 1);
	ReplaceMcr(556, 1, 6, 1);
	ReplaceMcr(324, 1, 340, 1);
	ReplaceMcr(364, 1, 340, 1);
	ReplaceMcr(394, 1, 340, 1); // lost details
	ReplaceMcr(432, 1, 340, 1); // lost details
	ReplaceMcr(551, 5, 265, 5);
	ReplaceMcr(551, 0, 265, 0);
	ReplaceMcr(551, 1, 265, 1);
	ReplaceMcr(261, 0, 14, 0); // lost details
	// ReplaceMcr(545, 0, 14, 0); // lost details
	// ReplaceMcr(18, 9, 6, 9); // lost details
	// ReplaceMcr(34, 9, 6, 9); // lost details
	// ReplaceMcr(37, 9, 6, 9);
	// ReplaceMcr(277, 9, 6, 9); // lost details
	// ReplaceMcr(332, 9, 6, 9); // lost details
	// ReplaceMcr(348, 9, 6, 9); // lost details
	// ReplaceMcr(352, 9, 6, 9); // lost details
	// ReplaceMcr(358, 9, 6, 9); // lost details
	// ReplaceMcr(406, 9, 6, 9); // lost details
	// ReplaceMcr(444, 9, 6, 9); // lost details
	// ReplaceMcr(459, 9, 6, 9); // lost details
	// ReplaceMcr(463, 9, 6, 9); // lost details
	// ReplaceMcr(562, 9, 6, 9); // lost details
	// ReplaceMcr(564, 9, 6, 9); // lost details
	// ReplaceMcr(277, 7, 18, 7); // lost details
	// ReplaceMcr(562, 7, 18, 7); // lost details
	// ReplaceMcr(277, 5, 459, 5); // lost details
	// ReplaceMcr(562, 5, 459, 5); // lost details
	// ReplaceMcr(277, 3, 459, 3); // lost details
	ReplaceMcr(562, 1, 277, 1); // lost details
	// ReplaceMcr(564, 1, 277, 1); // lost details
	ReplaceMcr(585, 1, 284, 1);
	// ReplaceMcr(590, 1, 285, 1); // lost details
	ReplaceMcr(598, 1, 289, 1); // lost details
	// ReplaceMcr(564, 7, 18, 7); // lost details
	// ReplaceMcr(564, 5, 459, 5); // lost details
	// ReplaceMcr(564, 3, 459, 3); // lost details
	// ReplaceMcr(34, 7, 18, 7); // lost details
	// ReplaceMcr(37, 7, 18, 7);
	// ReplaceMcr(84, 7, 18, 7); // lost details
	// ReplaceMcr(406, 7, 18, 7); // lost details
	// ReplaceMcr(444, 7, 18, 7); // lost details
	// ReplaceMcr(463, 7, 18, 7); // lost details
	// ReplaceMcr(332, 7, 18, 7); // lost details
	// ReplaceMcr(348, 7, 18, 7); // lost details
	// ReplaceMcr(352, 7, 18, 7); // lost details
	// ReplaceMcr(358, 7, 18, 7); // lost details
	// ReplaceMcr(459, 7, 18, 7); // lost details
	// ReplaceMcr(34, 5, 18, 5); // lost details
	// ReplaceMcr(348, 5, 332, 5); // lost details
	// ReplaceMcr(352, 5, 332, 5); // lost details
	// ReplaceMcr(358, 5, 332, 5); // lost details
	// ReplaceMcr(34, 3, 18, 3); // lost details
	// ReplaceMcr(358, 3, 18, 3); // lost details
	// ReplaceMcr(348, 3, 332, 3); // lost details
	// ReplaceMcr(352, 3, 332, 3); // lost details
	// ReplaceMcr(34, 0, 18, 0);
	ReplaceMcr(352, 0, 18, 0);
	ReplaceMcr(358, 0, 18, 0);
	ReplaceMcr(406, 0, 18, 0); // lost details
	// ReplaceMcr(34, 1, 18, 1);
	ReplaceMcr(332, 1, 18, 1);
	// ReplaceMcr(348, 1, 352, 1);
	// ReplaceMcr(358, 1, 352, 1);
	// ReplaceMcr(209, 7, 6, 7);
	// ReplaceMcr(80, 9, 6, 9);
	// ReplaceMcr(209, 9, 6, 9);
	ReplaceMcr(616, 9, 6, 9);
	// ReplaceMcr(14, 9, 6, 9);  // lost details
	ReplaceMcr(68, 9, 6, 9);  // lost details
	ReplaceMcr(84, 9, 6, 9);  // lost details
	ReplaceMcr(152, 9, 6, 9); // lost details
	// ReplaceMcr(241, 9, 6, 9); // lost details
	ReplaceMcr(265, 9, 6, 9); // lost details
	ReplaceMcr(269, 9, 6, 9); // lost details
	ReplaceMcr(364, 9, 6, 9); // lost details
	ReplaceMcr(551, 9, 6, 9); // lost details
	// ReplaceMcr(554, 9, 6, 9); // lost details
	ReplaceMcr(556, 9, 6, 9); // lost details
	ReplaceMcr(608, 9, 6, 9); // lost details
	ReplaceMcr(15, 8, 3, 8);
	// ReplaceMcr(23, 8, 3, 8);
	ReplaceMcr(65, 8, 3, 8);
	// ReplaceMcr(77, 8, 3, 8);
	ReplaceMcr(153, 8, 3, 8);
	// ReplaceMcr(206, 8, 3, 8);
	// ReplaceMcr(238, 8, 3, 8);
	ReplaceMcr(250, 8, 3, 8);
	// ReplaceMcr(292, 8, 3, 8);
	// ReplaceMcr(299, 8, 3, 8);
	ReplaceMcr(329, 8, 3, 8);
	ReplaceMcr(337, 8, 3, 8);
	ReplaceMcr(353, 8, 3, 8);
	ReplaceMcr(392, 8, 3, 8);
	ReplaceMcr(401, 8, 3, 8);
	ReplaceMcr(448, 8, 3, 8);
	ReplaceMcr(464, 8, 3, 8);
	ReplaceMcr(530, 8, 3, 8);
	ReplaceMcr(532, 8, 3, 8);
	ReplaceMcr(605, 8, 3, 8);
	ReplaceMcr(613, 8, 3, 8);
	// ReplaceMcr(3, 6, 15, 6);
	// ReplaceMcr(23, 6, 15, 6);
	ReplaceMcr(329, 6, 15, 6);
	ReplaceMcr(377, 6, 15, 6);
	ReplaceMcr(441, 6, 15, 6);
	ReplaceMcr(532, 6, 15, 6);
	ReplaceMcr(605, 6, 15, 6);
	// ReplaceMcr(206, 6, 77, 6);
	ReplaceMcr(534, 6, 254, 6);
	ReplaceMcr(537, 6, 254, 6);
	ReplaceMcr(541, 6, 258, 6);
	ReplaceMcr(250, 6, 353, 6);
	ReplaceMcr(322, 6, 353, 6);
	ReplaceMcr(337, 6, 353, 6);
	ReplaceMcr(392, 6, 353, 6);
	ReplaceMcr(429, 6, 353, 6);
	ReplaceMcr(530, 6, 353, 6);
	ReplaceMcr(613, 6, 353, 6);
	// ReplaceMcr(3, 4, 15, 4);
	// ReplaceMcr(23, 4, 15, 4);
	ReplaceMcr(401, 4, 15, 4);
	ReplaceMcr(605, 4, 15, 4);
	ReplaceMcr(322, 4, 337, 4);
	ReplaceMcr(353, 4, 337, 4);
	ReplaceMcr(377, 4, 337, 4);
	// ReplaceMcr(3, 2, 15, 2);
	// ReplaceMcr(23, 2, 15, 2);
	ReplaceMcr(464, 2, 15, 2);
	ReplaceMcr(541, 2, 258, 2);
	// ReplaceMcr(3, 0, 15, 0);
	// ReplaceMcr(23, 0, 15, 0);
	ReplaceMcr(337, 0, 15, 0);
	ReplaceMcr(322, 0, 392, 0);
	ReplaceMcr(353, 0, 392, 0);
	// ReplaceMcr(3, 1, 15, 1);
	// ReplaceMcr(23, 1, 15, 1);
	ReplaceMcr(250, 1, 15, 1);
	ReplaceMcr(258, 1, 15, 1);
	// ReplaceMcr(543, 1, 15, 1);
	ReplaceMcr(322, 1, 15, 1);
	ReplaceMcr(534, 1, 254, 1);
	ReplaceMcr(541, 1, 530, 1);
	ReplaceMcr(49, 5, 5, 5);
	ReplaceMcr(13, 6, 36, 6);
	ReplaceMcr(13, 4, 36, 4);
	ReplaceMcr(387, 6, 36, 6);
	// ReplaceMcr(390, 2, 19, 2);
	ReplaceMcr(29, 5, 21, 5);
	ReplaceMcr(95, 5, 21, 5);
	// ReplaceMcr(24, 0, 32, 0); // lost details
	// ReplaceMcr(354, 0, 32, 0); // lost details
	// ReplaceMcr(398, 0, 2, 0);
	// ReplaceMcr(398, 1, 2, 1); // lost details
	// ReplaceMcr(540, 0, 257, 0);
	// ReplaceMcr(30, 0, 2, 0);
	// ReplaceMcr(76, 0, 2, 0);
	// ReplaceMcr(205, 0, 2, 0);
	ReplaceMcr(407, 0, 2, 0); // lost details
	ReplaceMcr(379, 0, 5, 0);
	// ReplaceMcr(27, 0, 7, 0);
	// ReplaceMcr(81, 0, 7, 0);
	// ReplaceMcr(210, 0, 7, 0);
	ReplaceMcr(266, 0, 7, 0);
	ReplaceMcr(341, 0, 7, 0);
	ReplaceMcr(349, 0, 7, 0);
	// ReplaceMcr(381, 0, 7, 0);
	ReplaceMcr(460, 0, 7, 0);
	// ReplaceMcr(548, 0, 7, 0); // lost details
	ReplaceMcr(609, 0, 7, 0);
	ReplaceMcr(617, 0, 7, 0); // lost details
	ReplaceMcr(12, 0, 53, 0);
	// ReplaceMcr(54, 0, 53, 0);
	ReplaceMcr(62, 0, 53, 0);
	ReplaceMcr(368, 0, 53, 0); // lost details
	// ReplaceMcr(44, 0, 28, 0);
	// ReplaceMcr(82, 0, 28, 0);
	// ReplaceMcr(106, 0, 28, 0);
	// ReplaceMcr(211, 0, 28, 0);
	// ReplaceMcr(279, 0, 28, 0);
	ReplaceMcr(46, 0, 47, 0);
	// ReplaceMcr(102, 0, 40, 0);
	// ReplaceMcr(273, 0, 40, 0);
	// ReplaceMcr(56, 0, 52, 0);
	// ReplaceMcr(16, 0, 32, 0);
	// ReplaceMcr(38, 0, 32, 0);
	// ReplaceMcr(275, 0, 32, 0);
	// ReplaceMcr(309, 0, 45, 0);
	ReplaceMcr(567, 0, 45, 0); // lost details
	ReplaceMcr(622, 0, 45, 0);
	// ReplaceMcr(625, 0, 45, 0);
	// ReplaceMcr(630, 0, 45, 0);
	// ReplaceMcr(633, 0, 45, 0);
	// ReplaceMcr(90, 0, 32, 0);
	// ReplaceMcr(96, 0, 32, 0);
	// ReplaceMcr(103, 0, 32, 0);
	// ReplaceMcr(108, 0, 32, 0);
	// ReplaceMcr(110, 0, 32, 0);
	// ReplaceMcr(112, 0, 32, 0);
	// ReplaceMcr(223, 0, 32, 0);
	// ReplaceMcr(373, 0, 58, 0);
	ReplaceMcr(548, 0, 166, 0);
	// ReplaceMcr(105, 0, 43, 0);
	// ReplaceMcr(105, 1, 43, 1);
	// ReplaceMcr(278, 0, 43, 0);
	// ReplaceMcr(278, 1, 43, 1);
	// ReplaceMcr(10, 1, 12, 1);
	// ReplaceMcr(11, 1, 12, 1);
	// ReplaceMcr(53, 1, 12, 1);
	ReplaceMcr(577, 1, 12, 1); // lost details
	ReplaceMcr(31, 1, 4, 1);   // lost details
	// ReplaceMcr(279, 1, 28, 1);
	// ReplaceMcr(35, 1, 28, 1);
	// ReplaceMcr(35, 1, 4, 1); // lost details
	// ReplaceMcr(44, 1, 28, 1);
	// ReplaceMcr(110, 1, 28, 1);
	// ReplaceMcr(114, 1, 28, 1);
	// ReplaceMcr(211, 1, 28, 1);
	// ReplaceMcr(225, 1, 28, 1);
	// ReplaceMcr(273, 1, 28, 1); // lost details
	ReplaceMcr(281, 1, 12, 1); // lost details
	ReplaceMcr(356, 1, 7, 1);  // lost details
	ReplaceMcr(574, 1, 4, 1);  // lost details
	ReplaceMcr(612, 1, 4, 1);  // lost details
	// ReplaceMcr(76, 1, 2, 1);
	// ReplaceMcr(205, 1, 2, 1);
	ReplaceMcr(428, 1, 2, 1);
	// ReplaceMcr(41, 1, 4, 1);
	// ReplaceMcr(24, 1, 4, 1); // lost details
	ReplaceMcr(32, 1, 4, 1); // lost details
	// ReplaceMcr(92, 1, 4, 1); // lost details
	// ReplaceMcr(96, 1, 4, 1); // lost details
	// ReplaceMcr(217, 1, 4, 1); // lost details
	// ReplaceMcr(275, 1, 4, 1); // lost details
	// ReplaceMcr(78, 1, 4, 1);
	ReplaceMcr(604, 1, 4, 1); // lost details
	// ReplaceMcr(85, 0, 4, 0);
	// ReplaceMcr(120, 0, 4, 0);
	// ReplaceMcr(231, 0, 4, 0);
	ReplaceMcr(145, 0, 4, 0); // lost details
	ReplaceMcr(145, 1, 4, 1); // lost details
	ReplaceMcr(293, 0, 4, 0); // lost details
	// ReplaceMcr(628, 0, 4, 0); // lost details
	ReplaceMcr(536, 1, 297, 1); // lost details
	// ReplaceMcr(372, 1, 57, 1);
	// ReplaceMcr(8, 1, 85, 1);
	// ReplaceMcr(108, 1, 85, 1);
	// ReplaceMcr(116, 1, 85, 1);
	// ReplaceMcr(227, 1, 85, 1);
	// ReplaceMcr(82, 1, 85, 1);
	// ReplaceMcr(87, 1, 85, 1);
	// ReplaceMcr(94, 1, 85, 1);
	// ReplaceMcr(112, 1, 85, 1);
	// ReplaceMcr(118, 1, 85, 1);
	// ReplaceMcr(120, 1, 85, 1);
	// ReplaceMcr(233, 1, 85, 1);
	// ReplaceMcr(16, 1, 85, 1);
	// ReplaceMcr(50, 1, 85, 1);
	// ReplaceMcr(103, 1, 24, 1);
	// ReplaceMcr(275, 1, 24, 1);
	// ReplaceMcr(304, 1, 48, 1);
	// ReplaceMcr(27, 1, 7, 1);
	// ReplaceMcr(81, 1, 7, 1);
	// ReplaceMcr(210, 1, 7, 1);
	// ReplaceMcr(202, 1, 46, 1);
	ReplaceMcr(47, 1, 46, 1);
	// ReplaceMcr(468, 1, 31, 1);
	// ReplaceMcr(472, 1, 43, 1);
	ReplaceMcr(360, 1, 46, 1);
	// ReplaceMcr(52, 1, 46, 1); // lost details
	// ReplaceMcr(56, 1, 46, 1); // lost details
	// ReplaceMcr(373, 1, 46, 1); // lost details
	ReplaceMcr(592, 1, 46, 1);
	ReplaceMcr(505, 1, 46, 1); // lost details
	// ReplaceMcr(202, 0, 47, 0);
	// ReplaceMcr(195, 0, 48, 0);
	// ReplaceMcr(203, 0, 48, 0);
	// ReplaceMcr(194, 1, 46, 1);
	// ReplaceMcr(198, 1, 46, 1);
	// ReplaceMcr(199, 0, 48, 0);
	ReplaceMcr(572, 0, 48, 0);
	ReplaceMcr(507, 1, 48, 1);
	// ReplaceMcr(471, 7, 265, 7);
	ReplaceMcr(547, 7, 261, 7);
	// ReplaceMcr(471, 9, 6, 9);
	ReplaceMcr(569, 0, 283, 0);
	ReplaceMcr(565, 0, 283, 0);
	// ReplaceMcr(621, 1, 48, 1);
	ReplaceMcr(647, 1, 48, 1);
	// ReplaceMcr(627, 0, 624, 0);
	// ReplaceMcr(632, 0, 627, 0);
	ReplaceMcr(628, 0, 2, 0);
	ReplaceMcr(629, 1, 639, 1);
	// ReplaceMcr(637, 0, 624, 0);
	// ReplaceMcr(643, 0, 631, 0);
	// ReplaceMcr(388, 2, 15, 2);
	// ReplaceMcr(479, 5, 14, 5);
	ReplaceMcr(389, 6, 17, 6); // lost details
	// ReplaceMcr(19, 8, 89, 8);  // lost details
	// ReplaceMcr(390, 8, 89, 8);  // lost details
	// ReplaceMcr(31, 8, 89, 8);
	ReplaceMcr(254, 8, 89, 8); // lost details
	ReplaceMcr(534, 8, 89, 8); // lost details
	ReplaceMcr(537, 8, 89, 8); // lost details
	// ReplaceMcr(333, 8, 89, 8); // lost details
	// ReplaceMcr(345, 8, 89, 8);
	// ReplaceMcr(365, 8, 89, 8); // lost details
	// ReplaceMcr(456, 8, 89, 8);
	// ReplaceMcr(274, 8, 89, 8); // lost details
	// ReplaceMcr(558, 8, 89, 8); // lost details
	// ReplaceMcr(560, 8, 89, 8); // lost details
	ReplaceMcr(258, 8, 3, 8); // lost details
	ReplaceMcr(541, 8, 3, 8); // lost details
	// ReplaceMcr(543, 8, 3, 8); // lost details
	// ReplaceMcr(31, 6, 89, 6);  // lost details
	// ReplaceMcr(274, 6, 89, 6); // lost details
	// ReplaceMcr(558, 6, 89, 6); // lost details
	// ReplaceMcr(560, 6, 89, 6); // lost details
	// ReplaceMcr(356, 6, 89, 6);  // lost details
	// ReplaceMcr(333, 6, 445, 6); // lost details
	// ReplaceMcr(345, 6, 445, 6); // lost details
	// ReplaceMcr(365, 6, 445, 6); // lost details
	// ReplaceMcr(274, 4, 31, 4);  // lost details
	// ReplaceMcr(560, 4, 31, 4); // lost details
	// ReplaceMcr(333, 4, 345, 4); // lost details
	// ReplaceMcr(365, 4, 345, 4); // lost details
	// ReplaceMcr(445, 4, 345, 4); // lost details
	// ReplaceMcr(299, 2, 274, 2); // lost details
	// ReplaceMcr(560, 2, 274, 2); // lost details
	// ReplaceMcr(333, 2, 345, 2); // lost details
	// ReplaceMcr(365, 2, 345, 2); // lost details
	// ReplaceMcr(415, 2, 345, 2); // lost details
	// ReplaceMcr(445, 2, 345, 2); // lost details
	// ReplaceMcr(333, 0, 31, 0);  // lost details
	// ReplaceMcr(345, 0, 31, 0);  // lost details
	// ReplaceMcr(365, 0, 31, 0);  // lost details
	// ReplaceMcr(445, 0, 31, 0);  // lost details
	ReplaceMcr(333, 1, 31, 1);  // lost details
	// ReplaceMcr(365, 1, 31, 1);

	ReplaceMcr(125, 0, 136, 0); // lost details
	ReplaceMcr(125, 1, 136, 1); // lost details
	ReplaceMcr(125, 2, 136, 2); // lost details
	// ReplaceMcr(125, 3, 136, 3); // lost details
	ReplaceMcr(125, 3, 129, 3); // lost details
	ReplaceMcr(508, 2, 136, 2); // lost details
	ReplaceMcr(508, 3, 129, 3); // lost details
	ReplaceMcr(146, 0, 142, 0); // lost details
	ReplaceMcr(146, 1, 15, 1);  // lost details
	ReplaceMcr(136, 3, 129, 3); // lost details TODO: add missing pixels?
	ReplaceMcr(140, 1, 136, 1); // lost details

	ReplaceMcr(63, 8, 95, 8); // lost details
	ReplaceMcr(70, 8, 95, 8); // lost details
	ReplaceMcr(71, 8, 95, 8); // lost details
	ReplaceMcr(73, 8, 95, 8); // lost details
	ReplaceMcr(74, 8, 95, 8); // lost details

	ReplaceMcr(1, 8, 95, 8);  // lost details
	ReplaceMcr(21, 8, 95, 8); // lost details
	ReplaceMcr(36, 8, 95, 8); // lost details
	// ReplaceMcr(75, 8, 95, 8);
	ReplaceMcr(83, 8, 95, 8); // lost details
	// ReplaceMcr(91, 8, 95, 8);
	ReplaceMcr(99, 8, 95, 8);  // lost details
	ReplaceMcr(113, 8, 95, 8); // lost details
	ReplaceMcr(115, 8, 95, 8); // lost details
	ReplaceMcr(119, 8, 95, 8); // lost details
	ReplaceMcr(149, 8, 95, 8); // lost details
	ReplaceMcr(151, 8, 95, 8); // lost details
	// ReplaceMcr(172, 8, 95, 8);
	ReplaceMcr(204, 8, 95, 8);
	// ReplaceMcr(215, 8, 95, 8);
	// ReplaceMcr(220, 8, 95, 8); // lost details
	// ReplaceMcr(224, 8, 95, 8); // lost details
	// ReplaceMcr(226, 8, 95, 8); // lost details
	// ReplaceMcr(230, 8, 95, 8); // lost details
	ReplaceMcr(248, 8, 95, 8); // lost details
	ReplaceMcr(252, 8, 95, 8); // lost details
	ReplaceMcr(256, 8, 95, 8); // lost details
	ReplaceMcr(294, 8, 95, 8); // lost details
	ReplaceMcr(300, 8, 95, 8); // lost details
	ReplaceMcr(321, 8, 95, 8); // lost details
	ReplaceMcr(328, 8, 95, 8); // lost details
	ReplaceMcr(335, 8, 95, 8); // lost details
	ReplaceMcr(351, 8, 95, 8); // lost details
	ReplaceMcr(375, 8, 95, 8); // lost details
	ReplaceMcr(387, 8, 95, 8); // lost details
	ReplaceMcr(391, 8, 95, 8); // lost details
	ReplaceMcr(400, 8, 95, 8); // lost details
	ReplaceMcr(427, 8, 95, 8); // lost details
	ReplaceMcr(439, 8, 95, 8); // lost details
	ReplaceMcr(446, 8, 95, 8); // lost details
	ReplaceMcr(462, 8, 95, 8); // lost details
	ReplaceMcr(529, 8, 95, 8); // lost details
	ReplaceMcr(531, 8, 95, 8); // lost details
	ReplaceMcr(533, 8, 95, 8); // lost details
	ReplaceMcr(535, 8, 95, 8); // lost details
	ReplaceMcr(539, 8, 95, 8); // lost details

	ReplaceMcr(542, 8, 95, 8); // lost details
	ReplaceMcr(603, 8, 95, 8); // lost details
	ReplaceMcr(611, 8, 95, 8); // lost details

	ReplaceMcr(1, 6, 119, 6);   // lost details
	ReplaceMcr(13, 6, 119, 6);  // lost details
	ReplaceMcr(21, 6, 119, 6);  // lost details
	ReplaceMcr(36, 6, 119, 6);  // lost details
	ReplaceMcr(83, 6, 119, 6);  // lost details
	ReplaceMcr(149, 6, 119, 6); // lost details
	ReplaceMcr(387, 6, 119, 6); // lost details
	ReplaceMcr(400, 6, 119, 6); // lost details
	ReplaceMcr(439, 6, 119, 6); // lost details
	ReplaceMcr(462, 6, 119, 6); // lost details
	ReplaceMcr(603, 6, 119, 6); // lost details
	ReplaceMcr(611, 6, 119, 6); // lost details
	// ReplaceMcr(75, 6, 99, 6);   // lost details
	// ReplaceMcr(91, 6, 99, 6);   // lost details
	// ReplaceMcr(115, 6, 99, 6);  // lost details
	// ReplaceMcr(204, 6, 99, 6);  // lost details
	// ReplaceMcr(215, 6, 99, 6);  // lost details

	ReplaceMcr(71, 6, 63, 6);
	ReplaceMcr(71, 7, 67, 7);
	ReplaceMcr(69, 6, 67, 6); // lost details
	ReplaceMcr(69, 4, 63, 2); // lost details
	ReplaceMcr(65, 4, 63, 2); // lost details
	ReplaceMcr(64, 7, 63, 7); // lost details
	ReplaceMcr(64, 5, 63, 3); // lost details
	ReplaceMcr(68, 5, 63, 3); // lost details
	ReplaceMcr(63, 5, 63, 4); // lost details
	ReplaceMcr(67, 5, 67, 4); // lost details
	ReplaceMcr(70, 5, 70, 4); // lost details
	ReplaceMcr(71, 5, 71, 4); // lost details
	ReplaceMcr(72, 5, 72, 4); // lost details
	ReplaceMcr(73, 5, 73, 4); // lost details
	ReplaceMcr(74, 5, 74, 4); // lost details

	ReplaceMcr(529, 6, 248, 6); // lost details
	ReplaceMcr(531, 6, 248, 6); // lost details
	ReplaceMcr(533, 6, 252, 6); // lost details

	ReplaceMcr(542, 6, 256, 6); // lost details

	// ReplaceMcr(300, 6, 294, 6); // lost details
	ReplaceMcr(321, 6, 328, 6); // lost details
	ReplaceMcr(335, 6, 328, 6); // lost details
	ReplaceMcr(351, 6, 328, 6); // lost details
	ReplaceMcr(375, 6, 328, 6); // lost details
	ReplaceMcr(391, 6, 328, 6); // lost details

	ReplaceMcr(13, 4, 1, 4);    // lost details
	ReplaceMcr(21, 4, 1, 4);    // lost details
	ReplaceMcr(36, 4, 1, 4);    // lost details
	ReplaceMcr(328, 4, 1, 4);   // lost details
	ReplaceMcr(375, 4, 1, 4);   // lost details
	ReplaceMcr(531, 4, 248, 4); // lost details
	ReplaceMcr(533, 4, 252, 4); // lost details

	ReplaceMcr(1, 2, 256, 2);   // lost details
	ReplaceMcr(13, 2, 256, 2);  // lost details
	ReplaceMcr(21, 2, 256, 2);  // lost details
	ReplaceMcr(36, 2, 256, 2);  // lost details
	ReplaceMcr(248, 2, 256, 2); // lost details
	ReplaceMcr(83, 2, 256, 2);  // lost details
	ReplaceMcr(119, 2, 256, 2); // lost details
	// ReplaceMcr(230, 2, 256, 2); // lost details

	ReplaceMcr(13, 0, 1, 0);  // lost details
	ReplaceMcr(21, 0, 1, 0);  // lost details
	ReplaceMcr(36, 0, 1, 0);  // lost details
	ReplaceMcr(248, 0, 1, 0); // lost details
	ReplaceMcr(256, 0, 1, 0); // lost details
	ReplaceMcr(328, 0, 1, 0); // lost details

	ReplaceMcr(13, 8, 95, 8);  // lost details
	ReplaceMcr(17, 8, 95, 8);  // lost details
	ReplaceMcr(25, 8, 95, 8);  // lost details
	ReplaceMcr(29, 8, 95, 8);  // lost details
	ReplaceMcr(33, 8, 95, 8);  // lost details
	ReplaceMcr(39, 8, 95, 8);  // lost details
	ReplaceMcr(49, 8, 95, 8);  // lost details
	ReplaceMcr(51, 8, 95, 8);  // lost details
	ReplaceMcr(88, 8, 95, 8);  // lost details
	ReplaceMcr(93, 8, 95, 8);  // lost details
	ReplaceMcr(97, 8, 95, 8);  // lost details
	ReplaceMcr(101, 8, 95, 8); // lost details
	ReplaceMcr(107, 8, 95, 8); // lost details
	ReplaceMcr(109, 8, 95, 8); // lost details
	ReplaceMcr(111, 8, 95, 8); // lost details
	ReplaceMcr(117, 8, 95, 8); // lost details
	ReplaceMcr(121, 8, 95, 8); // lost details
	// ReplaceMcr(218, 8, 95, 8); // lost details
	// ReplaceMcr(222, 8, 95, 8); // lost details
	// ReplaceMcr(228, 8, 95, 8); // lost details
	ReplaceMcr(272, 8, 95, 8);
	ReplaceMcr(331, 8, 95, 8); // lost details
	ReplaceMcr(339, 8, 95, 8); // lost details
	ReplaceMcr(343, 8, 95, 8); // lost details
	ReplaceMcr(347, 8, 95, 8); // lost details
	ReplaceMcr(355, 8, 95, 8); // lost details
	ReplaceMcr(363, 8, 95, 8); // lost details
	ReplaceMcr(366, 8, 95, 8); // lost details
	ReplaceMcr(389, 8, 95, 8); // lost details
	ReplaceMcr(393, 8, 95, 8); // lost details
	ReplaceMcr(397, 8, 95, 8); // lost details
	ReplaceMcr(399, 8, 95, 8); // lost details
	// ReplaceMcr(402, 8, 95, 8); // lost details
	ReplaceMcr(413, 8, 95, 8); // lost details
	ReplaceMcr(417, 8, 95, 8); // lost details
	ReplaceMcr(443, 8, 95, 8); // lost details
	ReplaceMcr(450, 8, 95, 8); // lost details
	ReplaceMcr(454, 8, 95, 8); // lost details
	ReplaceMcr(458, 8, 95, 8); // lost details
	ReplaceMcr(466, 8, 95, 8); // lost details
	ReplaceMcr(478, 8, 95, 8); // lost details
	ReplaceMcr(557, 8, 95, 8);
	ReplaceMcr(559, 8, 95, 8);
	ReplaceMcr(615, 8, 95, 8); // lost details
	ReplaceMcr(421, 8, 55, 8); // lost details
	ReplaceMcr(154, 8, 55, 8); // lost details
	ReplaceMcr(154, 9, 13, 9); // lost details

	ReplaceMcr(9, 6, 25, 6);   // lost details
	ReplaceMcr(33, 6, 25, 6);  // lost details
	ReplaceMcr(51, 6, 25, 6);  // lost details
	ReplaceMcr(93, 6, 25, 6);  // lost details
	ReplaceMcr(97, 6, 25, 6);  // lost details
	ReplaceMcr(327, 6, 25, 6); // lost details
	ReplaceMcr(339, 6, 25, 6); // lost details
	ReplaceMcr(366, 6, 25, 6); // lost details
	ReplaceMcr(383, 6, 25, 6); // lost details
	ReplaceMcr(435, 6, 25, 6); // lost details
	ReplaceMcr(458, 6, 25, 6); // lost details
	ReplaceMcr(615, 6, 25, 6); // lost details

	ReplaceMcr(17, 6, 95, 6);  // lost details
	ReplaceMcr(29, 6, 95, 6);  // lost details
	ReplaceMcr(39, 6, 95, 6);  // lost details
	ReplaceMcr(49, 6, 95, 6);  // lost details
	ReplaceMcr(88, 6, 95, 6);  // lost details
	ReplaceMcr(107, 6, 95, 6); // lost details
	ReplaceMcr(109, 6, 95, 6); // lost details
	ReplaceMcr(111, 6, 95, 6); // lost details
	ReplaceMcr(117, 6, 95, 6); // lost details
	ReplaceMcr(121, 6, 95, 6); // lost details
	// ReplaceMcr(222, 6, 95, 6); // lost details
	// ReplaceMcr(228, 6, 95, 6); // lost details
	ReplaceMcr(272, 6, 95, 6); // lost details
	ReplaceMcr(389, 6, 95, 6); // lost details
	ReplaceMcr(397, 6, 95, 6); // lost details
	// ReplaceMcr(402, 6, 95, 6); // lost details
	ReplaceMcr(443, 6, 95, 6);  // lost details
	ReplaceMcr(466, 6, 95, 6);  // lost details
	ReplaceMcr(478, 6, 95, 6);  // lost details
	// ReplaceMcr(347, 6, 393, 6); // lost details
	ReplaceMcr(399, 6, 393, 6); // lost details
	ReplaceMcr(417, 6, 393, 6); // lost details
	// ReplaceMcr(331, 6, 343, 6); // lost details
	// ReplaceMcr(355, 6, 343, 6); // lost details
	// ReplaceMcr(363, 6, 343, 6); // lost details
	// ReplaceMcr(557, 6, 343, 6); // lost details
	// ReplaceMcr(559, 6, 343, 6); // lost details

	ReplaceMcr(17, 4, 29, 4);   // lost details
	ReplaceMcr(49, 4, 29, 4);   // lost details
	ReplaceMcr(389, 4, 29, 4);  // lost details
	ReplaceMcr(478, 4, 29, 4);  // lost details
	ReplaceMcr(9, 4, 25, 4);    // lost details
	ReplaceMcr(51, 4, 25, 4);   // lost details
	ReplaceMcr(55, 4, 25, 4);   // lost details
	ReplaceMcr(59, 4, 25, 4);   // lost details
	ReplaceMcr(366, 4, 25, 4);  // lost details
	ReplaceMcr(370, 4, 25, 4);  // lost details
	ReplaceMcr(374, 4, 25, 4);  // lost details
	ReplaceMcr(383, 4, 25, 4);  // lost details
	ReplaceMcr(423, 4, 25, 4);  // lost details
	// ReplaceMcr(331, 4, 343, 4); // lost details
	// ReplaceMcr(355, 4, 343, 4); // lost details
	// ReplaceMcr(363, 4, 343, 4); // lost details
	// ReplaceMcr(443, 4, 343, 4); // lost details
	ReplaceMcr(339, 4, 347, 4); // lost details

	ReplaceMcr(393, 4, 347, 4); // lost details
	ReplaceMcr(417, 4, 347, 4); // lost details
	ReplaceMcr(435, 4, 347, 4); // lost details
	ReplaceMcr(450, 4, 347, 4); // lost details
	ReplaceMcr(615, 4, 347, 4); // lost details
	ReplaceMcr(458, 4, 154, 4); // lost details

	ReplaceMcr(9, 2, 29, 2);    // lost details
	ReplaceMcr(17, 2, 29, 2);   // lost details
	ReplaceMcr(25, 2, 29, 2);   // lost details
	ReplaceMcr(33, 2, 29, 2);   // lost details
	ReplaceMcr(49, 2, 29, 2);   // lost details
	ReplaceMcr(51, 2, 29, 2);   // lost details
	ReplaceMcr(55, 2, 29, 2);   // lost details
	ReplaceMcr(59, 2, 29, 2);   // lost details
	ReplaceMcr(93, 2, 29, 2);   // lost details
	ReplaceMcr(97, 2, 29, 2);   // lost details
	// ReplaceMcr(218, 2, 29, 2);  // lost details
	ReplaceMcr(343, 2, 29, 2);  // lost details
	ReplaceMcr(363, 2, 29, 2);  // lost details
	ReplaceMcr(366, 2, 29, 2);  // lost details
	ReplaceMcr(413, 2, 29, 2);  // lost details
	ReplaceMcr(478, 2, 29, 2);  // lost details
	ReplaceMcr(339, 2, 347, 2); // lost details
	ReplaceMcr(355, 2, 347, 2); // lost details
	ReplaceMcr(393, 2, 347, 2); // lost details
	ReplaceMcr(443, 2, 347, 2); // lost details

	ReplaceMcr(9, 0, 33, 0);  // lost details
	ReplaceMcr(17, 0, 33, 0); // lost details
	ReplaceMcr(29, 0, 33, 0); // lost details
	ReplaceMcr(39, 0, 33, 0); // lost details
	ReplaceMcr(49, 0, 33, 0); // lost details
	ReplaceMcr(51, 0, 33, 0); // lost details
	ReplaceMcr(59, 0, 33, 0); // lost details
	ReplaceMcr(93, 0, 33, 0);
	ReplaceMcr(117, 0, 33, 0); // lost details
	ReplaceMcr(121, 0, 33, 0); // lost details
	// ReplaceMcr(218, 0, 33, 0);
	// ReplaceMcr(228, 0, 33, 0);  // lost details
	ReplaceMcr(397, 0, 33, 0);  // lost details
	ReplaceMcr(466, 0, 33, 0);  // lost details
	ReplaceMcr(478, 0, 33, 0);  // lost details
	ReplaceMcr(55, 0, 33, 0);   // lost details
	ReplaceMcr(331, 0, 33, 0);  // lost details
	ReplaceMcr(339, 0, 33, 0);  // lost details
	ReplaceMcr(355, 0, 33, 0);  // lost details
	ReplaceMcr(363, 0, 33, 0);  // lost details
	ReplaceMcr(370, 0, 33, 0);  // lost details
	ReplaceMcr(443, 0, 33, 0);  // lost details
	ReplaceMcr(559, 0, 272, 0); // lost details
	ReplaceMcr(5, 9, 13, 9);    // lost details
	ReplaceMcr(25, 9, 13, 9);   // lost details
	// ReplaceMcr(79, 9, 13, 9);
	ReplaceMcr(93, 9, 13, 9);
	ReplaceMcr(151, 9, 13, 9);
	ReplaceMcr(208, 9, 13, 9);
	// ReplaceMcr(218, 9, 13, 9);
	ReplaceMcr(260, 9, 13, 9); // lost details
	ReplaceMcr(264, 9, 13, 9); // lost details
	ReplaceMcr(268, 9, 13, 9); // lost details
	ReplaceMcr(323, 9, 13, 9); // lost details
	ReplaceMcr(339, 9, 13, 9); // lost details
	ReplaceMcr(379, 9, 13, 9); // lost details
	ReplaceMcr(393, 9, 13, 9); // lost details
	ReplaceMcr(413, 9, 13, 9); // lost details
	ReplaceMcr(431, 9, 13, 9); // lost details
	ReplaceMcr(439, 9, 13, 9); // lost details
	ReplaceMcr(450, 9, 13, 9); // lost details
	ReplaceMcr(544, 9, 13, 9); // lost details
	ReplaceMcr(546, 9, 13, 9); // lost details
	ReplaceMcr(550, 9, 13, 9); // lost details
	ReplaceMcr(552, 9, 13, 9); // lost details
	ReplaceMcr(553, 9, 13, 9); // lost details
	ReplaceMcr(555, 9, 13, 9); // lost details
	ReplaceMcr(607, 9, 13, 9); // lost details
	ReplaceMcr(615, 9, 13, 9);
	ReplaceMcr(162, 9, 158, 9);
	ReplaceMcr(5, 7, 13, 7);
	ReplaceMcr(25, 7, 13, 7); // 25 would be better?
	ReplaceMcr(49, 7, 13, 7);
	// ReplaceMcr(79, 7, 13, 7);
	ReplaceMcr(93, 7, 13, 7);
	ReplaceMcr(107, 7, 13, 7);
	ReplaceMcr(111, 7, 13, 7);
	ReplaceMcr(115, 7, 13, 7);
	ReplaceMcr(117, 7, 13, 7);
	ReplaceMcr(119, 7, 13, 7);
	ReplaceMcr(208, 7, 13, 7);
	// ReplaceMcr(218, 7, 13, 7);
	// ReplaceMcr(222, 7, 13, 7);
	// ReplaceMcr(226, 7, 13, 7);
	// ReplaceMcr(228, 7, 13, 7);
	// ReplaceMcr(230, 7, 13, 7);
	ReplaceMcr(363, 7, 13, 7);
	ReplaceMcr(393, 7, 13, 7);
	ReplaceMcr(413, 7, 13, 7);
	ReplaceMcr(431, 7, 13, 7);
	ReplaceMcr(450, 7, 13, 7);
	ReplaceMcr(478, 7, 13, 7);
	ReplaceMcr(607, 7, 13, 7);
	ReplaceMcr(615, 7, 13, 7);
	ReplaceMcr(546, 7, 260, 7);
	ReplaceMcr(553, 7, 268, 7);
	ReplaceMcr(328, 7, 323, 7);
	ReplaceMcr(339, 7, 323, 7);
	ReplaceMcr(379, 7, 323, 7);
	ReplaceMcr(439, 7, 323, 7);
	ReplaceMcr(5, 5, 25, 5);
	ReplaceMcr(13, 5, 25, 5);
	ReplaceMcr(49, 5, 25, 5);
	ReplaceMcr(107, 5, 25, 5);
	ReplaceMcr(115, 5, 25, 5);
	// ReplaceMcr(226, 5, 25, 5);
	ReplaceMcr(260, 5, 268, 5);
	ReplaceMcr(546, 5, 268, 5);
	ReplaceMcr(323, 5, 328, 5);
	ReplaceMcr(339, 5, 328, 5);
	ReplaceMcr(363, 5, 328, 5);
	ReplaceMcr(379, 5, 328, 5);
	ReplaceMcr(5, 3, 25, 3);
	ReplaceMcr(13, 3, 25, 3);
	ReplaceMcr(49, 3, 25, 3);
	ReplaceMcr(107, 3, 25, 3);
	ReplaceMcr(115, 3, 25, 3);
	// ReplaceMcr(226, 3, 25, 3);
	ReplaceMcr(260, 3, 25, 3);
	ReplaceMcr(268, 3, 25, 3);
	ReplaceMcr(328, 3, 323, 3);
	ReplaceMcr(339, 3, 323, 3);
	ReplaceMcr(546, 3, 323, 3);
	ReplaceMcr(13, 1, 5, 1);
	ReplaceMcr(25, 1, 5, 1);
	ReplaceMcr(49, 1, 5, 1);
	ReplaceMcr(260, 1, 5, 1);
	ReplaceMcr(268, 1, 5, 1);
	ReplaceMcr(544, 1, 5, 1);
	ReplaceMcr(323, 1, 328, 1);
	ReplaceMcr(17, 9, 13, 9); // lost details
	ReplaceMcr(21, 9, 13, 9); // lost details
	ReplaceMcr(29, 9, 13, 9);
	ReplaceMcr(33, 9, 13, 9); // lost details
	ReplaceMcr(36, 9, 13, 9); // lost details
	ReplaceMcr(42, 9, 13, 9); // lost details
	ReplaceMcr(51, 9, 13, 9); // lost details
	ReplaceMcr(55, 9, 13, 9); // lost details
	ReplaceMcr(59, 9, 13, 9); // lost details
	ReplaceMcr(83, 9, 13, 9); // lost details
	ReplaceMcr(88, 9, 13, 9); // lost details
	// ReplaceMcr(91, 9, 13, 9); // lost details
	ReplaceMcr(95, 9, 13, 9);
	ReplaceMcr(97, 9, 13, 9);  // lost details
	ReplaceMcr(99, 9, 13, 9);  // lost details
	ReplaceMcr(104, 9, 13, 9); // lost details
	ReplaceMcr(109, 9, 13, 9); // lost details
	ReplaceMcr(113, 9, 13, 9); // lost details
	ReplaceMcr(121, 9, 13, 9); // lost details
	// ReplaceMcr(215, 9, 13, 9); // lost details
	// ReplaceMcr(220, 9, 13, 9); // lost details
	// ReplaceMcr(224, 9, 13, 9); // lost details
	ReplaceMcr(276, 9, 13, 9); // lost details
	ReplaceMcr(331, 9, 13, 9); // lost details
	ReplaceMcr(335, 9, 13, 9); // lost details
	ReplaceMcr(343, 9, 13, 9);
	ReplaceMcr(347, 9, 13, 9); // lost details
	ReplaceMcr(351, 9, 13, 9); // lost details
	ReplaceMcr(357, 9, 13, 9); // lost details
	ReplaceMcr(366, 9, 13, 9); // lost details
	ReplaceMcr(370, 9, 13, 9); // lost details
	ReplaceMcr(374, 9, 13, 9); // lost details
	ReplaceMcr(389, 9, 13, 9); // lost details
	ReplaceMcr(391, 9, 13, 9); // lost details
	ReplaceMcr(397, 9, 13, 9);
	ReplaceMcr(399, 9, 13, 9); // lost details
	ReplaceMcr(400, 9, 13, 9); // lost details
	ReplaceMcr(417, 9, 13, 9); // lost details
	ReplaceMcr(421, 9, 13, 9); // lost details
	ReplaceMcr(423, 9, 13, 9); // lost details
	ReplaceMcr(443, 9, 13, 9); // lost details
	ReplaceMcr(446, 9, 13, 9); // lost details
	ReplaceMcr(454, 9, 13, 9);
	ReplaceMcr(458, 9, 13, 9);  // lost details
	ReplaceMcr(462, 9, 13, 9);  // lost details
	ReplaceMcr(470, 9, 13, 9);  // lost details
	ReplaceMcr(484, 9, 13, 9);  // lost details
	ReplaceMcr(488, 9, 13, 9);  // lost details
	ReplaceMcr(561, 9, 13, 9);  // lost details
	ReplaceMcr(563, 9, 13, 9);  // lost details
	ReplaceMcr(611, 9, 13, 9);  // lost details
	ReplaceMcr(33, 7, 17, 7);   // lost details
	ReplaceMcr(36, 7, 17, 7);   // lost details
	ReplaceMcr(42, 7, 17, 7);   // lost details
	ReplaceMcr(83, 7, 17, 7);   // lost details
	ReplaceMcr(88, 7, 17, 7);   // lost details
	ReplaceMcr(97, 7, 17, 7);   // lost details
	ReplaceMcr(99, 7, 17, 7);   // lost details
	ReplaceMcr(104, 7, 17, 7);  // lost details
	ReplaceMcr(109, 7, 17, 7);  // lost details
	ReplaceMcr(113, 7, 17, 7);  // lost details
	ReplaceMcr(121, 7, 17, 7);  // lost details
	// ReplaceMcr(220, 7, 17, 7);  // lost details
	// ReplaceMcr(224, 7, 17, 7);  // lost details
	ReplaceMcr(331, 7, 17, 7);  // lost details
	ReplaceMcr(351, 7, 17, 7);  // lost details
	ReplaceMcr(357, 7, 17, 7);  // lost details
	ReplaceMcr(399, 7, 17, 7);  // lost details
	ReplaceMcr(400, 7, 17, 7);  // lost details
	ReplaceMcr(443, 7, 17, 7);  // lost details
	ReplaceMcr(462, 7, 17, 7);  // lost details
	ReplaceMcr(9, 7, 21, 7);    // lost details
	ReplaceMcr(29, 7, 21, 7);   // lost details
	ReplaceMcr(51, 7, 21, 7);   // lost details
	ReplaceMcr(95, 7, 21, 7);   // lost details
	ReplaceMcr(335, 7, 21, 7);  // lost details
	ReplaceMcr(366, 7, 21, 7);  // lost details
	ReplaceMcr(383, 7, 21, 7);  // lost details
	ReplaceMcr(391, 7, 21, 7);  // lost details
	ReplaceMcr(397, 7, 21, 7);  // lost details
	ReplaceMcr(611, 7, 21, 7);  // lost details
	ReplaceMcr(417, 7, 55, 7);  // lost details
	ReplaceMcr(421, 7, 55, 7);  // lost details
	ReplaceMcr(446, 7, 55, 7);  // lost details
	ReplaceMcr(454, 7, 488, 7); // lost details
	ReplaceMcr(484, 7, 488, 7); // lost details
	ReplaceMcr(470, 7, 264, 7); // TODO: 470 would be better?
	ReplaceMcr(458, 7, 276, 7); // lost details
	ReplaceMcr(561, 7, 276, 7); // lost details
	ReplaceMcr(563, 7, 276, 7); // lost details
	ReplaceMcr(17, 5, 33, 5);   // lost details
	ReplaceMcr(36, 5, 33, 5);   // lost details
	ReplaceMcr(331, 5, 33, 5);  // lost details
	ReplaceMcr(351, 5, 33, 5);  // lost details
	ReplaceMcr(389, 5, 33, 5);  // lost details
	ReplaceMcr(462, 5, 33, 5);  // lost details
	ReplaceMcr(9, 5, 21, 5);    // lost details
	ReplaceMcr(29, 5, 21, 5);   // lost details
	ReplaceMcr(51, 5, 21, 5);   // lost details
	ReplaceMcr(55, 5, 21, 5);   // lost details
	ReplaceMcr(59, 5, 21, 5);   // lost details
	ReplaceMcr(95, 5, 21, 5);   // lost details
	ReplaceMcr(335, 5, 21, 5);  // lost details
	ReplaceMcr(366, 5, 21, 5);  // lost details
	ReplaceMcr(383, 5, 21, 5);  // lost details
	ReplaceMcr(391, 5, 21, 5);  // lost details
	ReplaceMcr(421, 5, 21, 5);  // lost details
	ReplaceMcr(423, 5, 21, 5);  // lost details
	ReplaceMcr(611, 5, 21, 5);  // lost details
	ReplaceMcr(561, 5, 276, 5); // lost details
	ReplaceMcr(17, 3, 33, 3);   // lost details
	ReplaceMcr(36, 3, 33, 3);   // lost details
	ReplaceMcr(462, 3, 33, 3);  // lost details
	ReplaceMcr(9, 3, 21, 3);    // lost details
	ReplaceMcr(51, 3, 21, 3);   // lost details
	ReplaceMcr(55, 3, 21, 3);   // lost details
	ReplaceMcr(59, 3, 21, 3);   // lost details
	ReplaceMcr(335, 3, 21, 3);  // lost details
	ReplaceMcr(366, 3, 21, 3);  // lost details
	ReplaceMcr(391, 3, 21, 3);  // lost details
	ReplaceMcr(421, 3, 21, 3);  // lost details
	ReplaceMcr(423, 3, 21, 3);  // lost details
	ReplaceMcr(470, 3, 276, 3); // lost details
	ReplaceMcr(331, 3, 347, 3); // lost details
	ReplaceMcr(351, 3, 347, 3); // lost details
	ReplaceMcr(488, 3, 484, 3); // lost details
	ReplaceMcr(9, 1, 55, 1);    // lost details
	ReplaceMcr(29, 1, 55, 1);   // lost details
	ReplaceMcr(51, 1, 55, 1);   // lost details
	ReplaceMcr(59, 1, 55, 1);   // lost details
	// ReplaceMcr(91, 1, 55, 1);   // lost details
	ReplaceMcr(95, 1, 55, 1);   // lost details
	// ReplaceMcr(215, 1, 55, 1);  // lost details
	ReplaceMcr(335, 1, 55, 1);  // lost details
	ReplaceMcr(391, 1, 55, 1);  // lost details
	ReplaceMcr(331, 1, 357, 1); // lost details
	ReplaceMcr(347, 1, 357, 1); // lost details
	ReplaceMcr(351, 1, 357, 1); // lost details
	ReplaceMcr(17, 1, 33, 1);   // lost details
	ReplaceMcr(36, 1, 33, 1);   // lost details
	ReplaceMcr(470, 1, 276, 1); // lost details
	ReplaceMcr(561, 1, 276, 1); // lost details
	ReplaceMcr(151, 2, 151, 5); // added details
	// eliminate micros of unused subtiles
	// Blk2Mcr(32, 202, 641, ..);
	Blk2Mcr(14, 1);
	Blk2Mcr(14, 5);
	Blk2Mcr(14, 7);
	Blk2Mcr(14, 9);
	HideMcr(37, 1);
	Blk2Mcr(37, 3);
	Blk2Mcr(37, 5);
	Blk2Mcr(37, 7);
	Blk2Mcr(37, 9);
	Blk2Mcr(236, 0);
	Blk2Mcr(236, 1);
	Blk2Mcr(236, 5);
	Blk2Mcr(236, 8);
	Blk2Mcr(237, 0);
	Blk2Mcr(237, 1);
	Blk2Mcr(237, 5);
	Blk2Mcr(237, 7);
	Blk2Mcr(240, 0);
	Blk2Mcr(240, 1);
	Blk2Mcr(240, 5);
	Blk2Mcr(241, 0);
	Blk2Mcr(241, 1);
	Blk2Mcr(241, 5);
	Blk2Mcr(241, 9);
	Blk2Mcr(243, 0);
	Blk2Mcr(243, 1);
	Blk2Mcr(243, 5);
	Blk2Mcr(243, 8);
	Blk2Mcr(244, 0);
	Blk2Mcr(244, 1);
	Blk2Mcr(244, 5);
	Blk2Mcr(244, 6);
	Blk2Mcr(244, 7);
	Blk2Mcr(244, 8);
	Blk2Mcr(245, 0);
	Blk2Mcr(245, 1);
	Blk2Mcr(245, 5);
	Blk2Mcr(246, 0);
	Blk2Mcr(246, 1);
	Blk2Mcr(246, 5);
	Blk2Mcr(246, 8);
	Blk2Mcr(247, 0);
	Blk2Mcr(247, 1);
	Blk2Mcr(247, 5);
	Blk2Mcr(247, 8);
	Blk2Mcr(194, 1);
	Blk2Mcr(195, 0);
	Blk2Mcr(198, 1);
	Blk2Mcr(199, 0);
	Blk2Mcr(203, 0);
	Blk2Mcr(180, 0);
	Blk2Mcr(180, 8);
	Blk2Mcr(180, 9);
	Blk2Mcr(181, 5);
	Blk2Mcr(182, 0);
	Blk2Mcr(183, 0);
	Blk2Mcr(184, 1);
	Blk2Mcr(185, 1);
	Blk2Mcr(186, 0);
	Blk2Mcr(187, 1);
	Blk2Mcr(188, 0);
	Blk2Mcr(188, 3);
	Blk2Mcr(188, 5);
	Blk2Mcr(188, 9);
	Blk2Mcr(189, 0);
	Blk2Mcr(190, 0);
	Blk2Mcr(191, 0);
	Blk2Mcr(192, 0);
	Blk2Mcr(196, 1);
	Blk2Mcr(197, 0);
	Blk2Mcr(200, 1);
	Blk2Mcr(201, 0);
	Blk2Mcr(86, 0);
	Blk2Mcr(86, 1);
	Blk2Mcr(86, 2);
	Blk2Mcr(86, 3);
	Blk2Mcr(86, 4);
	Blk2Mcr(86, 5);
	Blk2Mcr(86, 6);
	Blk2Mcr(86, 7);
	Blk2Mcr(86, 8);
	HideMcr(212, 0);
	HideMcr(212, 1);
	HideMcr(212, 2);
	HideMcr(212, 3);
	HideMcr(212, 4);
	HideMcr(212, 5);
	Blk2Mcr(212, 6);
	Blk2Mcr(212, 7);
	Blk2Mcr(212, 8);
	HideMcr(232, 0);
	HideMcr(232, 2);
	HideMcr(232, 4);
	Blk2Mcr(232, 1);
	Blk2Mcr(232, 3);
	Blk2Mcr(232, 5);
	Blk2Mcr(232, 6);
	Blk2Mcr(232, 7);
	Blk2Mcr(232, 8);
	Blk2Mcr(234, 0);
	Blk2Mcr(234, 2);
	Blk2Mcr(234, 4);
	Blk2Mcr(234, 6);
	Blk2Mcr(234, 7);
	Blk2Mcr(234, 8);
	HideMcr(234, 1);
	HideMcr(234, 3);
	HideMcr(234, 5);
	HideMcr(234, 9);
	Blk2Mcr(213, 0);
	Blk2Mcr(213, 2);
	Blk2Mcr(213, 4);
	Blk2Mcr(213, 6);
	Blk2Mcr(213, 8);
	HideMcr(215, 0);
	HideMcr(215, 2);
	HideMcr(215, 4);
	Blk2Mcr(215, 1);
	Blk2Mcr(215, 6);
	Blk2Mcr(215, 8);
	Blk2Mcr(215, 9);
	Blk2Mcr(216, 0);
	Blk2Mcr(216, 2);
	Blk2Mcr(216, 4);
	Blk2Mcr(216, 6);
	Blk2Mcr(216, 8);
	HideMcr(218, 1);
	HideMcr(218, 3);
	HideMcr(218, 5);
	HideMcr(218, 6);
	Blk2Mcr(218, 0);
	Blk2Mcr(218, 2);
	Blk2Mcr(218, 7);
	Blk2Mcr(218, 8);
	Blk2Mcr(218, 9);
	HideMcr(220, 0);
	HideMcr(220, 2);
	HideMcr(220, 4);
	Blk2Mcr(220, 6);
	Blk2Mcr(220, 7);
	Blk2Mcr(220, 8);
	Blk2Mcr(220, 9);
	HideMcr(222, 1);
	HideMcr(222, 3);
	HideMcr(222, 5);
	Blk2Mcr(222, 6);
	Blk2Mcr(222, 7);
	Blk2Mcr(222, 8);
	HideMcr(224, 0);
	HideMcr(224, 2);
	HideMcr(224, 4);
	Blk2Mcr(224, 6);
	Blk2Mcr(224, 7);
	Blk2Mcr(224, 8);
	Blk2Mcr(224, 9);
	Blk2Mcr(226, 3);
	Blk2Mcr(226, 5);
	Blk2Mcr(226, 6);
	Blk2Mcr(226, 7);
	Blk2Mcr(226, 8);
	HideMcr(226, 0);
	HideMcr(226, 2);
	HideMcr(226, 4);
	Blk2Mcr(228, 0);
	Blk2Mcr(228, 6);
	Blk2Mcr(228, 7);
	Blk2Mcr(228, 8);
	HideMcr(228, 1);
	HideMcr(228, 3);
	HideMcr(228, 5);
	HideMcr(230, 1);
	HideMcr(230, 3);
	HideMcr(230, 5);
	Blk2Mcr(230, 2);
	Blk2Mcr(230, 7);
	Blk2Mcr(230, 8);
	Blk2Mcr(304, 1);
	Blk2Mcr(309, 0);
	Blk2Mcr(340, 0);
	// Blk2Mcr(340, 1); - used in 324...
	Blk2Mcr(340, 3);
	Blk2Mcr(340, 5);
	Blk2Mcr(340, 7);
	Blk2Mcr(340, 9);
	Blk2Mcr(388, 2);
	Blk2Mcr(388, 8);
	Blk2Mcr(390, 2);
	Blk2Mcr(390, 4);
	Blk2Mcr(390, 6);
	Blk2Mcr(402, 0);
	Blk2Mcr(402, 6);
	Blk2Mcr(402, 8);
	Blk2Mcr(479, 1);
	Blk2Mcr(479, 3);
	Blk2Mcr(479, 5);
	Blk2Mcr(479, 7);
	Blk2Mcr(479, 9);
	Blk2Mcr(543, 1);
	Blk2Mcr(543, 2);
	Blk2Mcr(543, 4);
	Blk2Mcr(543, 8);
	Blk2Mcr(545, 0);
	Blk2Mcr(554, 0);
	Blk2Mcr(554, 1);
	Blk2Mcr(554, 3);
	// Blk2Mcr(554, 5); - used in 269
	Blk2Mcr(554, 7);
	Blk2Mcr(554, 9);
	Blk2Mcr(558, 0);
	Blk2Mcr(558, 2);
	Blk2Mcr(558, 4);
	Blk2Mcr(558, 6);
	Blk2Mcr(558, 8);
	Blk2Mcr(590, 1);
	Blk2Mcr(41, 1);
	Blk2Mcr(53, 1);
	Blk2Mcr(54, 0);
	Blk2Mcr(57, 1);
	Blk2Mcr(58, 0);
	Blk2Mcr(61, 1);
	Blk2Mcr(85, 1);
	Blk2Mcr(87, 0);
	Blk2Mcr(118, 1);
	Blk2Mcr(120, 1);
	Blk2Mcr(122, 1);
	Blk2Mcr(229, 1);
	Blk2Mcr(231, 1);
	Blk2Mcr(372, 1);
	// reused micros in fixCryptShadows
	// Blk2Mcr(619, 1);
	// Blk2Mcr(620, 0);
	// Blk2Mcr(621, 1);
	// Blk2Mcr(624, 0);
	// Blk2Mcr(625, 0);
	Blk2Mcr(627, 0);
	Blk2Mcr(630, 0);
	Blk2Mcr(632, 0);
	Blk2Mcr(633, 0);
	Blk2Mcr(637, 0);
	Blk2Mcr(642, 1);
	Blk2Mcr(644, 0);
	Blk2Mcr(645, 1);
	Blk2Mcr(646, 0);
	Blk2Mcr(649, 1);
	Blk2Mcr(650, 0);
	const int unusedSubtiles[] = {
		8, 10, 11, 16, 19, 20, 22, 23, 24, 26, 27, 28, 30, 34, 35, 38, 40, 43, 44, 50, 52, 56, 75, 78, 81, 82, 87, 90, 92, 94, 96, 98, 100, 102, 103, 105, 106, 108, 110, 112, 114, 116, 124, 127, 128, 137, 138, 139, 141, 143, 147, 167, 172, 174, 176, 177, 193, 202, 205, 207, 210, 211, 214, 217, 219, 221, 223, 225, 227, 233, 235, 239, 249, 251, 253, 257, 259, 262, 263, 270, 273, 275, 278, 279, 295, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 354, 365, 373, 381, 390, 398, 468, 472, 489, 490, 540, 560, 564, 640, 643, 648
	};
	for (int n = 0; n < lengthof(unusedSubtiles); n++) {
		for (int i = 0; i < blockSize; i++) {
			Blk2Mcr(unusedSubtiles[n], i);
		}
	}
}

void DRLP_L5_PatchTil(BYTE* buf)
{
	uint16_t* pTiles = (uint16_t*)buf;
	// fix automap of the entrance I.
	pTiles[(53 - 1) * 4 + 1] = SwapLE16(148 - 1);
	pTiles[(53 - 1) * 4 + 3] = SwapLE16(148 - 1);
	pTiles[(54 - 1) * 4 + 3] = SwapLE16(148 - 1);
	// fix 'bad' artifact
	pTiles[(136 - 1) * 4 + 1] = SwapLE16(2 - 1);   // 398
	// fix graphical glitch
	pTiles[(6 - 1) * 4 + 1] = SwapLE16(2 - 1);    // 22
	pTiles[(134 - 1) * 4 + 1] = SwapLE16(2 - 1);
	// use common subtiles
	pTiles[(4 - 1) * 4 + 1] = SwapLE16(6 - 1); // 14
	pTiles[(14 - 1) * 4 + 1] = SwapLE16(6 - 1);
	pTiles[(115 - 1) * 4 + 1] = SwapLE16(6 - 1);
	pTiles[(132 - 1) * 4 + 1] = SwapLE16(6 - 1);
	pTiles[(1 - 1) * 4 + 2] = SwapLE16(15 - 1); // 3
	pTiles[(27 - 1) * 4 + 2] = SwapLE16(15 - 1);
	pTiles[(43 - 1) * 4 + 2] = SwapLE16(15 - 1);
	pTiles[(79 - 1) * 4 + 2] = SwapLE16(15 - 1);
	pTiles[(6 - 1) * 4 + 2] = SwapLE16(15 - 1);   // 23
	pTiles[(7 - 1) * 4 + 2] = SwapLE16(7 - 1);    // 27
	pTiles[(9 - 1) * 4 + 2] = SwapLE16(7 - 1);
	pTiles[(33 - 1) * 4 + 2] = SwapLE16(7 - 1);
	pTiles[(137 - 1) * 4 + 2] = SwapLE16(7 - 1);
	pTiles[(106 - 1) * 4 + 3] = SwapLE16(48 - 1);  // 304
	pTiles[(108 - 1) * 4 + 0] = SwapLE16(45 - 1);  // 309
	pTiles[(127 - 1) * 4 + 2] = SwapLE16(4 - 1);   // 372
	pTiles[(132 - 1) * 4 + 2] = SwapLE16(15 - 1);  // 388
	pTiles[(156 - 1) * 4 + 2] = SwapLE16(31 - 1);  // 468
	pTiles[(179 - 1) * 4 + 1] = SwapLE16(261 - 1); // 545
	pTiles[(183 - 1) * 4 + 1] = SwapLE16(269 - 1); // 554
	pTiles[(205 - 1) * 4 + 3] = SwapLE16(626 - 1); // 627
	// - doors
	pTiles[(25 - 1) * 4 + 0] = SwapLE16(204 - 1); // 75
	pTiles[(26 - 1) * 4 + 0] = SwapLE16(208 - 1); // (79)
	pTiles[(69 - 1) * 4 + 2] = SwapLE16(76 - 1);  // 206 - to make 76 'accessible'
	pTiles[(70 - 1) * 4 + 1] = SwapLE16(79 - 1);  // 209 - to make 79 'accessible'
	// pTiles[(71 - 1) * 4 + 2] = SwapLE16(206 - 1);
	// pTiles[(72 - 1) * 4 + 2] = SwapLE16(206 - 1);
	// use better subtiles
	// - increase glow
	pTiles[(96 - 1) * 4 + 3] = SwapLE16(293 - 1); // 279
	pTiles[(187 - 1) * 4 + 3] = SwapLE16(293 - 1);
	pTiles[(188 - 1) * 4 + 3] = SwapLE16(293 - 1);
	pTiles[(90 - 1) * 4 + 1] = SwapLE16(297 - 1); // 253
	pTiles[(175 - 1) * 4 + 1] = SwapLE16(297 - 1);
	// - reduce glow
	pTiles[(162 - 1) * 4 + 1] = SwapLE16(297 - 1); // 489
	pTiles[(162 - 1) * 4 + 2] = SwapLE16(266 - 1); // 490
	// create separate pillar tile
	pTiles[(28 - 1) * 4 + 0] = SwapLE16(91 - 1);
	pTiles[(28 - 1) * 4 + 1] = SwapLE16(60 - 1);
	pTiles[(28 - 1) * 4 + 2] = SwapLE16(4 - 1);
	pTiles[(28 - 1) * 4 + 3] = SwapLE16(12 - 1);
	// create the new shadows
	// - use the shadows created by fixCryptShadows
	pTiles[(203 - 1) * 4 + 0] = SwapLE16(638 - 1); // 619
	pTiles[(203 - 1) * 4 + 1] = SwapLE16(639 - 1); // 620
	pTiles[(203 - 1) * 4 + 2] = SwapLE16(623 - 1); // 47
	pTiles[(203 - 1) * 4 + 3] = SwapLE16(626 - 1); // 621
	pTiles[(204 - 1) * 4 + 0] = SwapLE16(638 - 1); // 622
	pTiles[(204 - 1) * 4 + 1] = SwapLE16(639 - 1); // 46
	pTiles[(204 - 1) * 4 + 2] = SwapLE16(636 - 1); // 623
	pTiles[(204 - 1) * 4 + 3] = SwapLE16(626 - 1); // 624
	pTiles[(108 - 1) * 4 + 2] = SwapLE16(631 - 1); // 810
	pTiles[(108 - 1) * 4 + 3] = SwapLE16(626 - 1); // 811
	pTiles[(210 - 1) * 4 + 3] = SwapLE16(371 - 1); // 637

	pTiles[(109 - 1) * 4 + 0] = SwapLE16(1 - 1);   // 312
	pTiles[(109 - 1) * 4 + 1] = SwapLE16(2 - 1);   // 313
	pTiles[(109 - 1) * 4 + 2] = SwapLE16(3 - 1);   // 314
	pTiles[(109 - 1) * 4 + 3] = SwapLE16(626 - 1); // 315
	pTiles[(110 - 1) * 4 + 0] = SwapLE16(21 - 1);  // 316
	pTiles[(110 - 1) * 4 + 1] = SwapLE16(2 - 1);   // 313
	pTiles[(110 - 1) * 4 + 2] = SwapLE16(3 - 1);   // 314
	pTiles[(110 - 1) * 4 + 3] = SwapLE16(626 - 1); // 315
	pTiles[(111 - 1) * 4 + 0] = SwapLE16(39 - 1);  // 317
	pTiles[(111 - 1) * 4 + 1] = SwapLE16(4 - 1);   // 318
	pTiles[(111 - 1) * 4 + 2] = SwapLE16(242 - 1); // 319
	pTiles[(111 - 1) * 4 + 3] = SwapLE16(626 - 1); // 320
	pTiles[(215 - 1) * 4 + 0] = SwapLE16(101 - 1); // 645
	pTiles[(215 - 1) * 4 + 1] = SwapLE16(4 - 1);   // 646
	pTiles[(215 - 1) * 4 + 2] = SwapLE16(178 - 1); // 45
	pTiles[(215 - 1) * 4 + 3] = SwapLE16(626 - 1); // 647
	// - 'add' new shadow-types with glow TODO: add wall/grate+glow
	pTiles[(216 - 1) * 4 + 0] = SwapLE16(39 - 1);  // 622
	pTiles[(216 - 1) * 4 + 1] = SwapLE16(4 - 1);   // 46
	pTiles[(216 - 1) * 4 + 2] = SwapLE16(238 - 1); // 648
	pTiles[(216 - 1) * 4 + 3] = SwapLE16(635 - 1); // 624
	pTiles[(217 - 1) * 4 + 0] = SwapLE16(638 - 1); // 625
	pTiles[(217 - 1) * 4 + 1] = SwapLE16(639 - 1); // 46
	pTiles[(217 - 1) * 4 + 2] = SwapLE16(634 - 1); // 649 TODO: could be better
	pTiles[(217 - 1) * 4 + 3] = SwapLE16(635 - 1); // 650
	// - 'add' new shadow-types with horizontal arches
	pTiles[(71 - 1) * 4 + 0] = SwapLE16(5 - 1); // copy from tile 2
	pTiles[(71 - 1) * 4 + 1] = SwapLE16(6 - 1);
	pTiles[(71 - 1) * 4 + 2] = SwapLE16(631 - 1);
	pTiles[(71 - 1) * 4 + 3] = SwapLE16(626 - 1);
	pTiles[(80 - 1) * 4 + 0] = SwapLE16(5 - 1); // copy from tile 2
	pTiles[(80 - 1) * 4 + 1] = SwapLE16(6 - 1);
	pTiles[(80 - 1) * 4 + 2] = SwapLE16(623 - 1);
	pTiles[(80 - 1) * 4 + 3] = SwapLE16(626 - 1);

	pTiles[(81 - 1) * 4 + 0] = SwapLE16(42 - 1); // copy from tile 12
	pTiles[(81 - 1) * 4 + 1] = SwapLE16(18 - 1);
	pTiles[(81 - 1) * 4 + 2] = SwapLE16(631 - 1);
	pTiles[(81 - 1) * 4 + 3] = SwapLE16(626 - 1);
	pTiles[(82 - 1) * 4 + 0] = SwapLE16(42 - 1); // copy from tile 12
	pTiles[(82 - 1) * 4 + 1] = SwapLE16(18 - 1);
	pTiles[(82 - 1) * 4 + 2] = SwapLE16(623 - 1);
	pTiles[(82 - 1) * 4 + 3] = SwapLE16(626 - 1);

	pTiles[(83 - 1) * 4 + 0] = SwapLE16(104 - 1); // copy from tile 36
	pTiles[(83 - 1) * 4 + 1] = SwapLE16(84 - 1);
	pTiles[(83 - 1) * 4 + 2] = SwapLE16(631 - 1);
	pTiles[(83 - 1) * 4 + 3] = SwapLE16(626 - 1);
	pTiles[(84 - 1) * 4 + 0] = SwapLE16(104 - 1); // copy from tile 36
	pTiles[(84 - 1) * 4 + 1] = SwapLE16(84 - 1);
	pTiles[(84 - 1) * 4 + 2] = SwapLE16(623 - 1);
	pTiles[(84 - 1) * 4 + 3] = SwapLE16(626 - 1);

	pTiles[(85 - 1) * 4 + 0] = SwapLE16(25 - 1); // copy from tile 7
	pTiles[(85 - 1) * 4 + 1] = SwapLE16(6 - 1);
	pTiles[(85 - 1) * 4 + 2] = SwapLE16(631 - 1);
	pTiles[(85 - 1) * 4 + 3] = SwapLE16(626 - 1);
	pTiles[(86 - 1) * 4 + 0] = SwapLE16(25 - 1); // copy from tile 7
	pTiles[(86 - 1) * 4 + 1] = SwapLE16(6 - 1);
	pTiles[(86 - 1) * 4 + 2] = SwapLE16(623 - 1);
	pTiles[(86 - 1) * 4 + 3] = SwapLE16(626 - 1);

	pTiles[(87 - 1) * 4 + 0] = SwapLE16(208 - 1); // copy from tile 26
	pTiles[(87 - 1) * 4 + 1] = SwapLE16(80 - 1);
	pTiles[(87 - 1) * 4 + 2] = SwapLE16(623 - 1);
	pTiles[(87 - 1) * 4 + 3] = SwapLE16(626 - 1);
	pTiles[(88 - 1) * 4 + 0] = SwapLE16(208 - 1); // copy from tile 26
	pTiles[(88 - 1) * 4 + 1] = SwapLE16(80 - 1);
	pTiles[(88 - 1) * 4 + 2] = SwapLE16(631 - 1);
	pTiles[(88 - 1) * 4 + 3] = SwapLE16(626 - 1);

	// use common subtiles instead of minor alterations
	pTiles[(7 - 1) * 4 + 1] = SwapLE16(6 - 1); // 26
	pTiles[(159 - 1) * 4 + 1] = SwapLE16(6 - 1); // 479
	pTiles[(133 - 1) * 4 + 2] = SwapLE16(31 - 1); // 390
	pTiles[(10 - 1) * 4 + 1] = SwapLE16(18 - 1); // 37
	pTiles[(138 - 1) * 4 + 1] = SwapLE16(18 - 1);
	pTiles[(188 - 1) * 4 + 1] = SwapLE16(277 - 1); // 564
	pTiles[(178 - 1) * 4 + 2] = SwapLE16(258 - 1); // 543
	pTiles[(5 - 1) * 4 + 2] = SwapLE16(31 - 1); // 19
	pTiles[(14 - 1) * 4 + 2] = SwapLE16(31 - 1);
	pTiles[(159 - 1) * 4 + 2] = SwapLE16(31 - 1);
	pTiles[(185 - 1) * 4 + 2] = SwapLE16(274 - 1); // 558
	pTiles[(186 - 1) * 4 + 2] = SwapLE16(274 - 1); // 560
	pTiles[(139 - 1) * 4 + 0] = SwapLE16(39 - 1); // 402

	pTiles[(2 - 1) * 4 + 3] = SwapLE16(4 - 1); // 8
	pTiles[(3 - 1) * 4 + 1] = SwapLE16(60 - 1); // 10
	pTiles[(114 - 1) * 4 + 1] = SwapLE16(7 - 1);
	pTiles[(3 - 1) * 4 + 2] = SwapLE16(4 - 1); // 11
	pTiles[(114 - 1) * 4 + 2] = SwapLE16(4 - 1);
	pTiles[(5 - 1) * 4 + 3] = SwapLE16(7 - 1); // 20
	pTiles[(14 - 1) * 4 + 3] = SwapLE16(4 - 1);
	pTiles[(133 - 1) * 4 + 3] = SwapLE16(4 - 1);
	pTiles[(125 - 1) * 4 + 3] = SwapLE16(7 - 1); // 50
	pTiles[(159 - 1) * 4 + 3] = SwapLE16(7 - 1);
	pTiles[(4 - 1) * 4 + 3] = SwapLE16(7 - 1); // 16
	pTiles[(132 - 1) * 4 + 3] = SwapLE16(4 - 1);
	pTiles[(9 - 1) * 4 + 1] = SwapLE16(18 - 1);   // 34
	pTiles[(12 - 1) * 4 + 1] = SwapLE16(18 - 1);
	pTiles[(38 - 1) * 4 + 1] = SwapLE16(18 - 1);
	pTiles[(137 - 1) * 4 + 1] = SwapLE16(18 - 1);
	pTiles[(10 - 1) * 4 + 3] = SwapLE16(7 - 1); // 38
	pTiles[(138 - 1) * 4 + 3] = SwapLE16(4 - 1);
	pTiles[(11 - 1) * 4 + 3] = SwapLE16(4 - 1); // 41
	pTiles[(122 - 1) * 4 + 3] = SwapLE16(7 - 1);
	pTiles[(121 - 1) * 4 + 3] = SwapLE16(4 - 1); // 354
	pTiles[(8 - 1) * 4 + 3] = SwapLE16(4 - 1); // 32
	pTiles[(136 - 1) * 4 + 3] = SwapLE16(7 - 1);
	pTiles[(91 - 1) * 4 + 1] = SwapLE16(47 - 1); // 257
	pTiles[(178 - 1) * 4 + 1] = SwapLE16(47 - 1);
	pTiles[(91 - 1) * 4 + 3] = SwapLE16(48 - 1); // 259
	pTiles[(177 - 1) * 4 + 3] = SwapLE16(7 - 1);
	pTiles[(178 - 1) * 4 + 3] = SwapLE16(48 - 1);
	pTiles[(95 - 1) * 4 + 3] = SwapLE16(4 - 1);   // 275
	pTiles[(185 - 1) * 4 + 3] = SwapLE16(4 - 1);
	pTiles[(186 - 1) * 4 + 3] = SwapLE16(4 - 1);
	pTiles[(118 - 1) * 4 + 1] = SwapLE16(324 - 1); // 340
	pTiles[(125 - 1) * 4 + 2] = SwapLE16(333 - 1); // 365
	pTiles[(130 - 1) * 4 + 2] = SwapLE16(395 - 1); // 381
	pTiles[(157 - 1) * 4 + 2] = SwapLE16(4 - 1);   // 472
	pTiles[(177 - 1) * 4 + 1] = SwapLE16(4 - 1);   // 540
	pTiles[(195 - 1) * 4 + 2] = SwapLE16(285 - 1); // 590
	pTiles[(211 - 1) * 4 + 3] = SwapLE16(48 - 1);  // 621
	pTiles[(205 - 1) * 4 + 0] = SwapLE16(45 - 1);  // 625
	pTiles[(207 - 1) * 4 + 0] = SwapLE16(45 - 1);  // 630
	pTiles[(207 - 1) * 4 + 3] = SwapLE16(626 - 1); // 632
	pTiles[(208 - 1) * 4 + 0] = SwapLE16(45 - 1);  // 633

	pTiles[(27 - 1) * 4 + 3] = SwapLE16(4 - 1); // 85
	// pTiles[(28 - 1) * 4 + 3] = SwapLE16(4 - 1); // 87
	pTiles[(29 - 1) * 4 + 3] = SwapLE16(4 - 1); // 90
	pTiles[(30 - 1) * 4 + 3] = SwapLE16(4 - 1); // 92
	pTiles[(31 - 1) * 4 + 3] = SwapLE16(4 - 1); // 94
	pTiles[(32 - 1) * 4 + 3] = SwapLE16(4 - 1); // 96
	pTiles[(33 - 1) * 4 + 3] = SwapLE16(4 - 1); // 98
	pTiles[(34 - 1) * 4 + 3] = SwapLE16(4 - 1); // 100
	pTiles[(37 - 1) * 4 + 3] = SwapLE16(4 - 1); // 108
	pTiles[(38 - 1) * 4 + 3] = SwapLE16(4 - 1); // 110
	pTiles[(39 - 1) * 4 + 3] = SwapLE16(4 - 1); // 112
	pTiles[(40 - 1) * 4 + 3] = SwapLE16(4 - 1); // 114
	pTiles[(41 - 1) * 4 + 3] = SwapLE16(4 - 1); // 116
	pTiles[(42 - 1) * 4 + 3] = SwapLE16(4 - 1); // 118
	pTiles[(43 - 1) * 4 + 3] = SwapLE16(4 - 1); // 120
	pTiles[(44 - 1) * 4 + 3] = SwapLE16(4 - 1); // 122
	pTiles[(45 - 1) * 4 + 3] = SwapLE16(4 - 1); // 124
	// pTiles[(71 - 1) * 4 + 3] = SwapLE16(4 - 1); // 214
	// pTiles[(72 - 1) * 4 + 3] = SwapLE16(4 - 1); // 217
	// pTiles[(73 - 1) * 4 + 3] = SwapLE16(4 - 1); // 219
	// pTiles[(74 - 1) * 4 + 3] = SwapLE16(4 - 1); // 221
	// pTiles[(75 - 1) * 4 + 3] = SwapLE16(4 - 1); // 223
	// pTiles[(76 - 1) * 4 + 3] = SwapLE16(4 - 1); // 225
	// pTiles[(77 - 1) * 4 + 3] = SwapLE16(4 - 1); // 227
	// pTiles[(78 - 1) * 4 + 3] = SwapLE16(4 - 1); // 229
	// pTiles[(79 - 1) * 4 + 3] = SwapLE16(4 - 1); // 231
	// pTiles[(80 - 1) * 4 + 3] = SwapLE16(4 - 1); // 233
	// pTiles[(81 - 1) * 4 + 3] = SwapLE16(4 - 1); // 235
	pTiles[(15 - 1) * 4 + 1] = SwapLE16(4 - 1); // 52
	pTiles[(15 - 1) * 4 + 2] = SwapLE16(4 - 1); // 53
	pTiles[(15 - 1) * 4 + 3] = SwapLE16(4 - 1); // 54
	pTiles[(16 - 1) * 4 + 1] = SwapLE16(4 - 1); // 56
	pTiles[(144 - 1) * 4 + 1] = SwapLE16(4 - 1);
	pTiles[(16 - 1) * 4 + 2] = SwapLE16(4 - 1); // 57
	pTiles[(16 - 1) * 4 + 3] = SwapLE16(4 - 1); // 58
	pTiles[(144 - 1) * 4 + 3] = SwapLE16(7 - 1);
	pTiles[(94 - 1) * 4 + 2] = SwapLE16(60 - 1); // 270
	pTiles[(183 - 1) * 4 + 2] = SwapLE16(60 - 1);
	pTiles[(184 - 1) * 4 + 2] = SwapLE16(60 - 1);
	pTiles[(17 - 1) * 4 + 2] = SwapLE16(4 - 1); // 61
	pTiles[(128 - 1) * 4 + 2] = SwapLE16(4 - 1);
	pTiles[(92 - 1) * 4 + 2] = SwapLE16(62 - 1); // 262
	pTiles[(179 - 1) * 4 + 2] = SwapLE16(62 - 1);
	pTiles[(25 - 1) * 4 + 1] = SwapLE16(4 - 1); // 76
	pTiles[(25 - 1) * 4 + 3] = SwapLE16(4 - 1); // 78
	pTiles[(35 - 1) * 4 + 1] = SwapLE16(4 - 1); // 102
	pTiles[(35 - 1) * 4 + 3] = SwapLE16(4 - 1); // 103
	pTiles[(69 - 1) * 4 + 1] = SwapLE16(4 - 1); // 205
	pTiles[(69 - 1) * 4 + 3] = SwapLE16(4 - 1); // 207
	pTiles[(26 - 1) * 4 + 2] = SwapLE16(4 - 1); // 81
	pTiles[(26 - 1) * 4 + 3] = SwapLE16(4 - 1); // 82
	pTiles[(36 - 1) * 4 + 2] = SwapLE16(4 - 1); // 105
	pTiles[(36 - 1) * 4 + 3] = SwapLE16(4 - 1); // 106
	pTiles[(46 - 1) * 4 + 2] = SwapLE16(4 - 1); // 127
	pTiles[(46 - 1) * 4 + 3] = SwapLE16(4 - 1); // 128
	pTiles[(70 - 1) * 4 + 2] = SwapLE16(4 - 1); // 210
	pTiles[(70 - 1) * 4 + 3] = SwapLE16(4 - 1); // 211
	pTiles[(49 - 1) * 4 + 1] = SwapLE16(4 - 1); // 137
	pTiles[(167 - 1) * 4 + 1] = SwapLE16(4 - 1);
	pTiles[(49 - 1) * 4 + 2] = SwapLE16(4 - 1); // 138
	pTiles[(167 - 1) * 4 + 2] = SwapLE16(4 - 1);
	pTiles[(49 - 1) * 4 + 3] = SwapLE16(4 - 1); // 139
	pTiles[(167 - 1) * 4 + 3] = SwapLE16(4 - 1);
	pTiles[(50 - 1) * 4 + 1] = SwapLE16(4 - 1); // 141
	pTiles[(50 - 1) * 4 + 3] = SwapLE16(4 - 1); // 143
	pTiles[(51 - 1) * 4 + 3] = SwapLE16(4 - 1); // 147
	pTiles[(103 - 1) * 4 + 1] = SwapLE16(4 - 1); // 295
	pTiles[(105 - 1) * 4 + 1] = SwapLE16(4 - 1);
	pTiles[(127 - 1) * 4 + 3] = SwapLE16(4 - 1); // 373
	pTiles[(89 - 1) * 4 + 3] = SwapLE16(4 - 1); // 251
	pTiles[(173 - 1) * 4 + 3] = SwapLE16(7 - 1);
	pTiles[(174 - 1) * 4 + 3] = SwapLE16(7 - 1);
	pTiles[(6 - 1) * 4 + 3] = SwapLE16(4 - 1); // 24
	pTiles[(134 - 1) * 4 + 3] = SwapLE16(7 - 1);
	pTiles[(7 - 1) * 4 + 3] = SwapLE16(7 - 1); // 28
	pTiles[(8 - 1) * 4 + 1] = SwapLE16(2 - 1); // 30
	pTiles[(30 - 1) * 4 + 1] = SwapLE16(2 - 1);
	pTiles[(32 - 1) * 4 + 1] = SwapLE16(2 - 1);
	pTiles[(72 - 1) * 4 + 1] = SwapLE16(2 - 1);
	pTiles[(9 - 1) * 4 + 3] = SwapLE16(4 - 1); // 35
	pTiles[(137 - 1) * 4 + 3] = SwapLE16(4 - 1);
	pTiles[(11 - 1) * 4 + 1] = SwapLE16(4 - 1); // 40
	pTiles[(122 - 1) * 4 + 1] = SwapLE16(4 - 1);
	pTiles[(12 - 1) * 4 + 2] = SwapLE16(4 - 1); // 43
	pTiles[(123 - 1) * 4 + 2] = SwapLE16(4 - 1);
	pTiles[(12 - 1) * 4 + 3] = SwapLE16(4 - 1); // 44
	pTiles[(123 - 1) * 4 + 3] = SwapLE16(7 - 1);
	pTiles[(95 - 1) * 4 + 1] = SwapLE16(4 - 1); // 273
	pTiles[(185 - 1) * 4 + 1] = SwapLE16(7 - 1);
	pTiles[(186 - 1) * 4 + 1] = SwapLE16(4 - 1);
	pTiles[(89 - 1) * 4 + 1] = SwapLE16(293 - 1); // 249
	pTiles[(173 - 1) * 4 + 1] = SwapLE16(293 - 1);
	pTiles[(174 - 1) * 4 + 1] = SwapLE16(293 - 1);
	pTiles[(92 - 1) * 4 + 3] = SwapLE16(271 - 1); // 263
	pTiles[(179 - 1) * 4 + 3] = SwapLE16(271 - 1);
	pTiles[(96 - 1) * 4 + 2] = SwapLE16(12 - 1); // 278
	pTiles[(187 - 1) * 4 + 2] = SwapLE16(12 - 1);
	pTiles[(188 - 1) * 4 + 2] = SwapLE16(12 - 1);
	// eliminate subtiles of unused tiles
	const int unusedTiles[] = {
		/* 29,*/ 30, 31,/* 32,*/ 34,/* 38,*/ 39, 40, 41, 42,/* 43, 44,*/ 52, 58, 61, 62, 63, 64, 65, 66, 67, 68, 72, 73, 74, 75, 76, 77, 78, 79, 212, 213, 214
	};
	constexpr int blankSubtile = 8;
	for (int n = 0; n < lengthof(unusedTiles); n++) {
		int tileId = unusedTiles[n];
		pTiles[(tileId - 1) * 4 + 0] = SwapLE16(blankSubtile - 1);
		pTiles[(tileId - 1) * 4 + 1] = SwapLE16(blankSubtile - 1);
		pTiles[(tileId - 1) * 4 + 2] = SwapLE16(blankSubtile - 1);
		pTiles[(tileId - 1) * 4 + 3] = SwapLE16(blankSubtile - 1);
	}
}
#endif

DEVILUTION_END_NAMESPACE
