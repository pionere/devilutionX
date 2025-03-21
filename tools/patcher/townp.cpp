/**
 * @file townp.cpp
 *
 * Implementation of the town level patching functionality.
 */
#include "all.h"
#include "engine/render/cel_render.h"
#include "engine/render/dun_render.h"

DEVILUTION_BEGIN_NAMESPACE

static void patchTownPotMin(uint16_t* pSubtiles, int potLeftSubtileRef, int potRightSubtileRef)
{
	const unsigned blockSize = BLOCK_SIZE_TOWN;
	unsigned leftIndex0 = MICRO_IDX(potLeftSubtileRef - 1, blockSize, 1);
	unsigned leftFrameRef0 = pSubtiles[leftIndex0] & 0xFFF;
	unsigned leftIndex1 = MICRO_IDX(potLeftSubtileRef - 1, blockSize, 3);
	unsigned leftFrameRef1 = pSubtiles[leftIndex1] & 0xFFF;
	unsigned leftIndex2 = MICRO_IDX(potLeftSubtileRef - 1, blockSize, 5);
	unsigned leftFrameRef2 = pSubtiles[leftIndex2] & 0xFFF;

	if (leftFrameRef1 == 0 || leftFrameRef2 == 0) {
		return; // left frames are empty -> assume it is already done
	}
	if (leftFrameRef0 == 0) {
		return; // something is wrong
	}

	unsigned rightIndex0 = MICRO_IDX(potRightSubtileRef - 1, blockSize, 0);
	unsigned rightFrameRef0 = pSubtiles[rightIndex0] & 0xFFF;
	unsigned rightIndex1 = MICRO_IDX(potRightSubtileRef - 1, blockSize, 2);
	unsigned rightFrameRef1 = pSubtiles[rightIndex1] & 0xFFF;
	unsigned rightIndex2 = MICRO_IDX(potRightSubtileRef - 1, blockSize, 4);
	unsigned rightFrameRef2 = pSubtiles[rightIndex2] & 0xFFF;

	if (rightFrameRef1 != 0 || rightFrameRef2 != 0) {
		return; // right frames are not empty -> assume it is already done
	}
	if (rightFrameRef0 == 0) {
		return; // something is wrong
	}

	// move the frames to the right side
	pSubtiles[rightIndex1] = pSubtiles[leftIndex1];
	pSubtiles[rightIndex2] = pSubtiles[leftIndex2];
	pSubtiles[leftIndex1] = 0;
	pSubtiles[leftIndex2] = 0;
	// convert the left floor to triangle
	pSubtiles[leftIndex0] = (pSubtiles[leftIndex0] & 0xFFF) | (MET_RTRIANGLE << 12);
	// convert the right floor to transparent
	pSubtiles[rightIndex0] = (pSubtiles[rightIndex0] & 0xFFF) | (MET_TRANSPARENT << 12);
}

static BYTE* patchTownPotCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	typedef struct {
		int subtileIndex;
		unsigned microIndex;
		int res_encoding;
	} CelMicro;
	const CelMicro micros[] = {
		{ 553 - 1, 5, MET_TRANSPARENT },
		{ 553 - 1, 3, MET_TRANSPARENT },
		{ 553 - 1, 1, MET_RTRIANGLE },
		{ 554 - 1, 0, MET_TRANSPARENT },
	};

	const uint16_t* pSubtiles = (const uint16_t*)minBuf;
	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_TOWN;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		// if (micro.subtileIndex < 0) {
		// 	continue;
		// }
		unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
		if ((SwapLE16(pSubtiles[index]) & 0xFFF) == 0) {
			// TODO: report error if not empty both? + additional checks
			return celBuf; // frame is empty -> assume it is already done
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

	// move the image up 553[5] and 553[3]
	for (int x = MICRO_WIDTH / 2; x < MICRO_WIDTH; x++) {
		for (int y = MICRO_HEIGHT / 2; y < 2 * MICRO_HEIGHT; y++) {
			gpBuffer[x + (y - MICRO_HEIGHT / 2) * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
			gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
		}
	}
	// copy image to the other micros 553[1] -> 553[3], 554[0]
	for (int x = MICRO_WIDTH / 2 + 2; x < MICRO_WIDTH - 4; x++) {
		for (int y = 2 * MICRO_HEIGHT; y < 2 * MICRO_HEIGHT + MICRO_HEIGHT / 2 + 8; y++) {
			BYTE color = gpBuffer[x + y * BUFFER_WIDTH];
			if (color == TRANS_COLOR)
				continue;
			if (y < 2 * MICRO_HEIGHT + MICRO_HEIGHT / 2) {
				gpBuffer[x + (y - MICRO_HEIGHT / 2) * BUFFER_WIDTH] = color; // 553[3]
			} else {
				gpBuffer[x + MICRO_WIDTH + (y - 2 * MICRO_HEIGHT - MICRO_HEIGHT / 2) * BUFFER_WIDTH] = color; // 554[0]
			}
			gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
		}
	}

	// create the new CEL file
	BYTE* resCelBuf = DiabloAllocPtr(*celLen + lengthof(micros) * MICRO_WIDTH * MICRO_HEIGHT);

	CelFrameEntry entries[lengthof(micros)];
	xx = 0, yy = MICRO_HEIGHT - 1;
	int idx = 0;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		// if (micro.res_encoding >= 0) {
			entries[idx].encoding = micro.res_encoding;
			unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
			entries[idx].frameRef = SwapLE16(pSubtiles[index]) & 0xFFF;
			entries[idx].frameSrc = &gpBuffer[xx + yy * BUFFER_WIDTH];
			idx++;
		// }
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

static BYTE* patchTownCathedralCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	typedef struct {
		int subtileIndex;
		unsigned microIndex;
		// int res_encoding;
	} CelMicro;
	const CelMicro micros[] = {
		{ 807 - 1, 12 },
		{ 805 - 1, 12 },
		{ 805 - 1, 13 },
		{ 806 - 1, 13 },
	};

	const uint16_t* pSubtiles = (const uint16_t*)minBuf;
	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_TOWN;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		// if (micro.subtileIndex < 0) {
		//	continue;
		// }
		unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
		if ((SwapLE16(pSubtiles[index]) & 0xFFF) == 0) {
			mem_free_dbg(celBuf);
			app_warn("Invalid (empty) cathedral subtile (%d).", micro.subtileIndex + 1);
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

	// draw extra line to each frame
	for (int x = 0; x < MICRO_WIDTH; x++) {
		int y = MICRO_HEIGHT / 2 - x / 2 + 0 * MICRO_HEIGHT;
		gpBuffer[x + y * BUFFER_WIDTH] = gpBuffer[x + (y + 6) * BUFFER_WIDTH]; // 807[12]
	}
	for (int x = 0; x < MICRO_WIDTH - 4; x++) {
		int y = MICRO_HEIGHT / 2 - x / 2 + 1 * MICRO_HEIGHT;
		gpBuffer[x + y * BUFFER_WIDTH] = gpBuffer[x + 4 + (y + 4) * BUFFER_WIDTH]; // 805[12] I.
	}
	for (int x = MICRO_WIDTH - 4; x < MICRO_WIDTH; x++) {
		int y = MICRO_HEIGHT / 2 - x / 2 + 1 * MICRO_HEIGHT;
		gpBuffer[x + y * BUFFER_WIDTH] = gpBuffer[x + (y + 2) * BUFFER_WIDTH]; // 805[12] II.
	}
	for (int x = 0; x < 20; x++) {
		int y = 1 + x / 2 + 2 * MICRO_HEIGHT;
		gpBuffer[x + y * BUFFER_WIDTH] = gpBuffer[x + (y + 1) * BUFFER_WIDTH]; // 805[13] I.
	}
	for (int x = 20; x < MICRO_WIDTH; x++) {
		int y = 1 + x / 2 + 2 * MICRO_HEIGHT;
		gpBuffer[x + y * BUFFER_WIDTH] = gpBuffer[x - 12 + (y - 6) * BUFFER_WIDTH]; // 805[13] II.
	}
	for (int x = 0; x < MICRO_WIDTH; x++) {
		int y = 1 + x / 2 + 3 * MICRO_HEIGHT;
		gpBuffer[x + y * BUFFER_WIDTH] = gpBuffer[x + (y - MICRO_HEIGHT) * BUFFER_WIDTH]; // 806[13]
	}

	// create the new CEL file
	BYTE* resCelBuf = DiabloAllocPtr(*celLen + lengthof(micros) * MICRO_WIDTH * MICRO_HEIGHT);

	CelFrameEntry entries[lengthof(micros)];
	xx = 0, yy = MICRO_HEIGHT - 1;
	int idx = 0;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		// if (micro.res_encoding >= 0) {
			entries[idx].encoding = MET_TRANSPARENT;
			unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
			entries[idx].frameRef = SwapLE16(pSubtiles[index]) & 0xFFF;
			entries[idx].frameSrc = &gpBuffer[xx + yy * BUFFER_WIDTH];
			idx++;
		// }
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

static BYTE* patchTownFloorCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	typedef struct {
		int subtileIndex;
		unsigned microIndex;
		int res_encoding;
	} CelMicro;
	const CelMicro micros[] = {
/*  0 */{ 731 - 1, 9, MET_TRANSPARENT }, // move micro
/*  1 */{ 755 - 1, 0, MET_LTRIANGLE },   // change type
/*  2 */{ 974 - 1, 0, MET_LTRIANGLE },   // change type
/*  3 */{ 1030 - 1, 0, MET_LTRIANGLE },  // change type
/*  4 */{ 220 - 1, 1, MET_TRANSPARENT }, // move micro
/*  5 */{ 221 - 1, 0, MET_TRANSPARENT },
/*  6 */{ 962 - 1, 1, MET_TRANSPARENT },
/*  7 */{ 218 - 1, 0, MET_TRANSPARENT }, // move micro
/*  8 */{ 219 - 1, 1, MET_TRANSPARENT },
/*  9 */{ 1166 - 1, 0, MET_TRANSPARENT }, // move micro
/* 10 */{ 1167 - 1, 1, MET_TRANSPARENT },
/* 11 */{ 1171 - 1, 1, MET_TRANSPARENT },
/* 12 */{ 1172 - 1, 0, MET_TRANSPARENT },
/* 13 */{ 1175 - 1, 1, MET_TRANSPARENT },
/* 14 */{ 1176 - 1, 0, MET_TRANSPARENT },
/* 15 */{  845 - 1, 4, MET_TRANSPARENT },
/* 16 */{  128 - 1, 0, MET_LTRIANGLE },
/* 17 */{  128 - 1, 1, MET_RTRIANGLE },
/* 18 */{  156 - 1, 1, MET_RTRIANGLE },
/* 19 */{  212 - 1, 1, MET_RTRIANGLE },

/* 20 */{  783 - 1, 0, MET_LTRIANGLE }, // fix grass (530)
/* 21 */{  783 - 1, 1, MET_RTRIANGLE },

/* 22 */{  138 - 1, 0, MET_LTRIANGLE }, // merge 94 and 86
/* 23 */{  128 - 1, 0, -1 },

		//{ 493 - 1, 0, MET_LTRIANGLE },   // TODO: fix light?
		//{ 290 - 1, 0, MET_LTRIANGLE },   // TODO: fix grass?
		//{ 290 - 1, 1, MET_RTRIANGLE },
		//{ 334 - 1, 0, MET_LTRIANGLE },   // TODO: fix grass? + (349 & nest)
		//{ 334 - 1, 1, MET_RTRIANGLE },
	};

	const uint16_t* pSubtiles = (const uint16_t*)minBuf;
	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_TOWN;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		// if (micro.subtileIndex < 0) {
		//	continue;
		// }
		unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
		if ((SwapLE16(pSubtiles[index]) & 0xFFF) == 0) {
			// TODO: report error if not empty both? + additional checks
			return celBuf; // frame is empty -> assume it is already done
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

	// move the image up - 731[9]
	for (int i = 0; i < 1; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr + BUFFER_WIDTH * MICRO_HEIGHT / 2];
				if (color != TRANS_COLOR)
					gpBuffer[addr] = color;
			}
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}

	// mask and move down the image - 220[1], 221[0]
	for (int i = 4; i < 5; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			// - mask
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (x < 26 && (x < 23 || color < 110)) { // 110, 112, 113, 119, 121, 126
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
			// - move to 221[0]
			for (int y = MICRO_HEIGHT / 2; y < 21; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR) {
					unsigned addr2 = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr2] = color; // 221[0]
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
			// - move down
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR) {
					gpBuffer[addr + BUFFER_WIDTH * MICRO_HEIGHT / 2] = color;
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask and move down the image - 962[1]
	for (int i = 6; i < 7; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			// - mask
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y > x / 2 && y < MICRO_HEIGHT - x / 2) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
			// - move down
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR) {
					gpBuffer[addr + BUFFER_WIDTH * MICRO_HEIGHT / 2] = color;
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask and move down the image - 218[0], 219[1]
	for (int i = 7; i < 8; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			// - mask
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (x > 10 && (x > 20 || (color < 110 && color != 59 && color != 86 && color != 91 && color != 99 && color != 101))) { // 110, 112, 113, 115, 117, 119, 121, 122, 124, 126
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
			// - move to 219[1]
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR) {
					unsigned addr2 = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr2] = color; // 219[1]
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
			// - move down
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				gpBuffer[addr + BUFFER_WIDTH * MICRO_HEIGHT / 2] = color;
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}
	// mask and move down the image 1166[0], 1167[1]
	for (int i = 9; i < 10; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			// - mask
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (x > 3 && (x > 24 || (color < 112 && color != 0 && color != 59 && color != 86 && color != 91 && color != 99 && color != 101 && color != 110))) { // 110, 112, 113, 115, 117, 119, 121, 122, 124, 126
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
			// - move to 1167[1]
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR) {
					unsigned addr2 = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr2] = color; // 1167[1]
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
			// - move down
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				gpBuffer[addr + BUFFER_WIDTH * MICRO_HEIGHT / 2] = color;
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}
	// mask the image 1171[1], 1172[0], 1175[1] and 1176[0]
	for (int i = 11; i < 15; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color == 107) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask the image 845[4]
	for (int i = 15; i < 16; i++) {
		for (int x = 0; x < 10; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}
	// merge 94 and 86
	for (int i = 22; i < 23; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 14; y < MICRO_HEIGHT; y++) {
				if (x >= 13 && x <= 22) continue;
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
	}
	// fix artifacts of the new micros
	{ //  1166[0]
		int i = 9;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 12 + 28 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 19 + 27 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ //  1167[1]
		int i = 10;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 17 + 5 * BUFFER_WIDTH] = gpBuffer[addr + 16 + 6 * BUFFER_WIDTH];
		gpBuffer[addr + 18 + 5 * BUFFER_WIDTH] = gpBuffer[addr + 16 + 5 * BUFFER_WIDTH];
		gpBuffer[addr + 8 + 4 * BUFFER_WIDTH] = gpBuffer[addr + 5 + 2 * BUFFER_WIDTH];
		gpBuffer[addr + 7 + 4 * BUFFER_WIDTH] = 126;
		gpBuffer[addr + 7 + 5 * BUFFER_WIDTH] = gpBuffer[addr + 8 + 5 * BUFFER_WIDTH];
		gpBuffer[addr + 5 + 1 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	// fix grass 530[0]
	{
		int i = 20;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 21 + 20 * BUFFER_WIDTH] = 85;
		gpBuffer[addr + 30 + 20 * BUFFER_WIDTH] = 102;
		gpBuffer[addr + 13 + 22 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 14 + 22 * BUFFER_WIDTH] = 85;
		gpBuffer[addr + 14 + 23 * BUFFER_WIDTH] = 85;
	}
	// fix grass 530[1]
	{
		int i = 21;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 4 + 6 * BUFFER_WIDTH] = 112;
		gpBuffer[addr + 4 + 7 * BUFFER_WIDTH] = 112;
		gpBuffer[addr + 3 + 8 * BUFFER_WIDTH] = 112;
		gpBuffer[addr + 4 + 8 * BUFFER_WIDTH] = 93;
		gpBuffer[addr + 5 + 8 * BUFFER_WIDTH] = 93;
		gpBuffer[addr + 6 + 8 * BUFFER_WIDTH] = 93;
		gpBuffer[addr + 6 + 9 * BUFFER_WIDTH] = 93;
		gpBuffer[addr + 3 + 10 * BUFFER_WIDTH] = 112;
		gpBuffer[addr + 6 + 10 * BUFFER_WIDTH] = 112;
		gpBuffer[addr + 7 + 10 * BUFFER_WIDTH] = 112;
		gpBuffer[addr + 4 + 11 * BUFFER_WIDTH] = 112;
		gpBuffer[addr + 5 + 11 * BUFFER_WIDTH] = 112;
		gpBuffer[addr + 9 + 11 * BUFFER_WIDTH] = 93;
		gpBuffer[addr + 4 + 12 * BUFFER_WIDTH] = 112;
		gpBuffer[addr + 5 + 16 * BUFFER_WIDTH] = 104;
		gpBuffer[addr + 6 + 16 * BUFFER_WIDTH] = 83;
		gpBuffer[addr + 7 + 16 * BUFFER_WIDTH] = 104;
		gpBuffer[addr + 5 + 17 * BUFFER_WIDTH] = 104;
		gpBuffer[addr + 6 + 17 * BUFFER_WIDTH] = 104;
		gpBuffer[addr + 7 + 17 * BUFFER_WIDTH] = 83;
		gpBuffer[addr + 3 + 18 * BUFFER_WIDTH] = 83;
		gpBuffer[addr + 5 + 18 * BUFFER_WIDTH] = 104;
		gpBuffer[addr + 6 + 18 * BUFFER_WIDTH] = 83;
		gpBuffer[addr + 8 + 18 * BUFFER_WIDTH] = 104;
		gpBuffer[addr + 10 + 18 * BUFFER_WIDTH] = 83;
		gpBuffer[addr + 2 + 19 * BUFFER_WIDTH] = 83;
		gpBuffer[addr + 3 + 19 * BUFFER_WIDTH] = 83;
		gpBuffer[addr + 4 + 19 * BUFFER_WIDTH] = 83;
		gpBuffer[addr + 5 + 19 * BUFFER_WIDTH] = 83;
		gpBuffer[addr + 6 + 19 * BUFFER_WIDTH] = 83;
		gpBuffer[addr + 7 + 19 * BUFFER_WIDTH] = 83;
		gpBuffer[addr + 2 + 20 * BUFFER_WIDTH] = 83;
		gpBuffer[addr + 3 + 20 * BUFFER_WIDTH] = 83;
		gpBuffer[addr + 4 + 20 * BUFFER_WIDTH] = 104;
		gpBuffer[addr + 5 + 20 * BUFFER_WIDTH] = 104;
		gpBuffer[addr + 6 + 20 * BUFFER_WIDTH] = 83;
		gpBuffer[addr + 7 + 20 * BUFFER_WIDTH] = 104;
		gpBuffer[addr + 3 + 21 * BUFFER_WIDTH] = 83;
		gpBuffer[addr + 4 + 21 * BUFFER_WIDTH] = 104;
		gpBuffer[addr + 5 + 21 * BUFFER_WIDTH] = 104;
		gpBuffer[addr + 6 + 21 * BUFFER_WIDTH] = 104;
		gpBuffer[addr + 6 + 23 * BUFFER_WIDTH] = 113;
		gpBuffer[addr + 7 + 23 * BUFFER_WIDTH] = 113;
		gpBuffer[addr + 8 + 23 * BUFFER_WIDTH] = 113;
		gpBuffer[addr + 5 + 24 * BUFFER_WIDTH] = 113;
		gpBuffer[addr + 7 + 24 * BUFFER_WIDTH] = 113;
		gpBuffer[addr + 8 + 24 * BUFFER_WIDTH] = 113;
		gpBuffer[addr + 2 + 25 * BUFFER_WIDTH] = 113;
		gpBuffer[addr + 3 + 25 * BUFFER_WIDTH] = 93;
		gpBuffer[addr + 5 + 25 * BUFFER_WIDTH] = 113;
		gpBuffer[addr + 6 + 25 * BUFFER_WIDTH] = 113;
		gpBuffer[addr + 7 + 25 * BUFFER_WIDTH] = 113;
		gpBuffer[addr + 8 + 25 * BUFFER_WIDTH] = 113;
		gpBuffer[addr + 2 + 26 * BUFFER_WIDTH] = 113;
		gpBuffer[addr + 3 + 26 * BUFFER_WIDTH] = 93;
		gpBuffer[addr + 6 + 26 * BUFFER_WIDTH] = 113;
		gpBuffer[addr + 2 + 27 * BUFFER_WIDTH] = 93;
		gpBuffer[addr + 3 + 27 * BUFFER_WIDTH] = 93;
		gpBuffer[addr + 4 + 27 * BUFFER_WIDTH] = 93;
		gpBuffer[addr + 2 + 28 * BUFFER_WIDTH] = 93;
	}

	// create the new CEL file
	BYTE* resCelBuf = DiabloAllocPtr(*celLen + lengthof(micros) * MICRO_WIDTH * MICRO_HEIGHT);

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

static BYTE* patchTownDoorCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	typedef struct {
		int subtileIndex;
		unsigned microIndex;
		int res_encoding;
	} CelMicro;

	const CelMicro micros[] = {
/*  0 */{ 724 - 1, 0, -1 }, // move micro (used to block subsequent calls)
/*  1 */{ 724 - 1, 1, -1 },
/*  2 */{ 724 - 1, 3, -1 },
/*  3 */{ 723 - 1, 1, -1 },
/*  4 */{ 715 - 1, 11, -1 },
/*  5 */{ 715 - 1, 9, -1 },
/*  6 */{ 715 - 1, 7, -1 }, // unused
/*  7 */{ 715 - 1, 5, -1 }, // unused
/*  8 */{ 715 - 1, 3, -1 },
/*  9 */{ 715 - 1, 1, MET_RTRIANGLE },
/* 10 */{ 721 - 1, 4, MET_SQUARE },
/* 11 */{ 721 - 1, 2, MET_SQUARE },
/* 12 */{ 719 - 1, 4, MET_SQUARE },
/* 13 */{ 719 - 1, 2, MET_SQUARE },
/* 14 */{ 727 - 1, 7, MET_SQUARE },
/* 15 */{ 727 - 1, 5, MET_SQUARE },
/* 16 */{ 725 - 1, 4, MET_TRANSPARENT },
/* 17 */{ 725 - 1, 2, MET_TRANSPARENT },
/* 18 */{ 725 - 1, 0, MET_TRANSPARENT },

/* 19 */{ 428 - 1, 4, -1 },
/* 20 */{ 428 - 1, 2, -1 },
/* 21 */{ 428 - 1, 0, -1 },
/* 22 */{ 418 - 1, 5, MET_SQUARE },
/* 23 */{ 418 - 1, 3, MET_SQUARE },
/* 24 */{ 418 - 1, 1, MET_RTRAPEZOID },
/* 25 */{ 426 - 1, 2, -1 },
/* 26 */{ 426 - 1, 0, -1 },
/* 27 */{ 428 - 1, 1, -1 },
/* 28 */{ 429 - 1, 0, -1 },
/* 29 */{ 419 - 1, 5, MET_SQUARE },
/* 30 */{ 419 - 1, 3, MET_SQUARE },
/* 31 */{ 419 - 1, 1, MET_RTRAPEZOID },

/* 32 */{ 911 - 1, 9, -1 },
/* 33 */{ 911 - 1, 7, -1 },
/* 34 */{ 911 - 1, 5, -1 },
/* 35 */{ 931 - 1, 5, MET_SQUARE },
/* 36 */{ 931 - 1, 3, MET_SQUARE },
/* 37 */{ 931 - 1, 1, MET_RTRAPEZOID },
/* 38 */{ 402 - 1, 0, -1 },
/* 39 */{ 954 - 1, 2, -1 },
/* 40 */{ 919 - 1, 9, -1 },
/* 41 */{ 919 - 1, 5, -1 },
/* 42 */{ 927 - 1, 5, MET_SQUARE },
/* 43 */{ 927 - 1, 1, MET_RTRAPEZOID },
/* 44 */{ 956 - 1, 0, MET_LTRIANGLE }, // unused
		// { 956 - 1, 2, -1 },
/* 45 */{ 954 - 1, 0, MET_LTRIANGLE }, // unused
/* 46 */{ 919 - 1, 7, MET_SQUARE },
/* 47 */{ 918 - 1, 9, -1 },
/* 48 */{ 926 - 1, 5, MET_SQUARE },
/* 49 */{ 927 - 1, 0, -1 },
/* 50 */{ 918 - 1, 3, -1 },
/* 51 */{ 918 - 1, 2, -1 },
/* 52 */{ 918 - 1, 5, MET_SQUARE },
/* 53 */{ 929 - 1, 0, MET_LTRAPEZOID },
/* 54 */{ 929 - 1, 1, MET_RTRAPEZOID },
/* 55 */{ 918 - 1, 8, -1 },
/* 56 */{ 926 - 1, 4, MET_SQUARE },
/* 57 */{ 928 - 1, 4, -1 },
/* 58 */{ 920 - 1, 8, MET_SQUARE },
/* 59 */{ 551 - 1, 0, -1 },
/* 60 */{ 552 - 1, 1, -1 },
/* 61 */{ 519 - 1, 0, -1 },
/* 62 */{ 509 - 1, 5, MET_SQUARE },
/* 63 */{ 509 - 1, 3, MET_SQUARE },
/* 64 */{ 509 - 1, 1, MET_RTRAPEZOID },

/* 65 */{ 510 - 1, 7, -1 },
/* 66 */{ 510 - 1, 5, -1 },
/* 67 */{ 551 - 1, 3, MET_SQUARE },
/* 68 */{ 551 - 1, 1, MET_SQUARE },

/* 69 */{ 728 - 1, 9, -1 },
/* 70 */{ 728 - 1, 7, -1 },
/* 71 */{ 716 - 1, 13, MET_TRANSPARENT },
/* 72 */{ 716 - 1, 11, MET_SQUARE },

/* 73 */{ 910 - 1, 9, -1 },
/* 74 */{ 910 - 1, 7, -1 },
/* 75 */{ 930 - 1, 5, MET_TRANSPARENT },
/* 76 */{ 930 - 1, 3, MET_TRANSPARENT },

/* 77 */{ 537 - 1, 0, -1 },
/* 78 */{ 539 - 1, 0, -1 },
/* 79 */{ 529 - 1, 4, MET_SQUARE },
/* 80 */{ 531 - 1, 4, MET_SQUARE },

/* 81 */{ 478 - 1, 0, -1 },
/* 82 */{ 477 - 1, 1, MET_SQUARE },
/* 83 */{ 480 - 1, 1, MET_RTRAPEZOID },
/* 84 */{ 479 - 1, 1, -1 },
/* 85 */{ 477 - 1, 0, MET_SQUARE },
/* 86 */{ 480 - 1, 0, MET_LTRAPEZOID },

/* 87 */{ 517 - 1, 0, -1 },             // move micros for better light propagation
/* 88 */{ 519 - 1, 1, MET_RTRAPEZOID },
	};

	const uint16_t* pSubtiles = (const uint16_t*)minBuf;
	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_TOWN;
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
	// copy 724[0] to 721[2]
	for (int i = 0; i < 1; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					gpBuffer[x + MICRO_WIDTH * ((i + 11) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 11) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color; // 721[2]
				}
			}
		}
	}
	// copy 724[1] to 719[2]
	for (int i = 1; i < 2; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					gpBuffer[x + MICRO_WIDTH * ((i + 12) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 12) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color; // 719[2]
				}
			}
		}
	}
	// copy 724[3] to 719[4] and 719[2]
	for (int i = 2; i < 3; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					if (y < MICRO_HEIGHT / 2) {
						gpBuffer[x + MICRO_WIDTH * ((i + 10) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 10) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color; // 719[4]
					} else {
						gpBuffer[x + MICRO_WIDTH * ((i + 11) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 11) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color; // 719[2]
					}
				}
			}
		}
	}
	// copy 723[1] to 721[2] and 721[4]
	for (int i = 3; i < 4; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					if (y < MICRO_HEIGHT / 2) {
						gpBuffer[x + MICRO_WIDTH * ((i + 7) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 7) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color; // 721[2]
					} else {
						gpBuffer[x + MICRO_WIDTH * ((i + 8) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 8) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color; // 721[4]
					}
				}
			}
		}
	}
	// copy 715[11] to 727[7]
	for (int i = 4; i < 5; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					gpBuffer[x + MICRO_WIDTH * ((i + 10) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 10) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color; // 727[7]
				}
			}
		}
	}
	// copy 715[9] to 727[5]
	for (int i = 5; i < 6; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					gpBuffer[x + MICRO_WIDTH * ((i + 10) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 10) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color; // 727[5]
				}
			}
		}
	}
	// copy 715[3] to 725[2] and 725[0]
	for (int i = 8; i < 9; i++) {
		for (int x = 9; x < 24; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					if (y < MICRO_HEIGHT / 2) {
						gpBuffer[x + MICRO_WIDTH * ((i + 9) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 9) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color; // 725[2]
					} else {
						gpBuffer[x + MICRO_WIDTH * ((i + 10) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 10) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color; // 725[0]
					}
				}
			}
		}
	}
	// copy 715[1] to 725[0]
	for (int i = 9; i < 10; i++) {
		for (int x = 9; x < 24; x++) {
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR && y <= x / 2) {
					gpBuffer[x + MICRO_WIDTH * ((i + 9) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 9) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color; // 725[0]
				}
			}
		}
	}
	// copy 428[4] to 418[5]
	for (int i = 19; i < 20; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 3) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 3) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					if (gpBuffer[addr] == TRANS_COLOR) {
						gpBuffer[addr] = color; // 418[5]
					}
				}
			}
		}
	}
	// copy 428[2] to 418[5] and 418[3]
	for (int i = 20; i < 21; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr;
					if (y < MICRO_HEIGHT / 2) {
						addr = x + MICRO_WIDTH * ((i + 2) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 2) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 418[5]
					} else {
						addr = x + MICRO_WIDTH * ((i + 3) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 3) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 418[3]
					}
					if (gpBuffer[addr] == TRANS_COLOR) {
						gpBuffer[addr] = color;
					}
				}
			}
		}
	}
	// copy 428[0] to 418[3] and 418[1]
	for (int i = 21; i < 22; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr;
					if (y < MICRO_HEIGHT / 2) {
						addr = x + MICRO_WIDTH * ((i + 2) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 2) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 418[3]
					} else {
						addr = x + MICRO_WIDTH * ((i + 3) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 3) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 418[1]
					}
					if (gpBuffer[addr] == TRANS_COLOR) {
						gpBuffer[addr] = color;
					}
				}
			}
		}
	}
	// copy 426[2] to 419[5]
	for (int i = 25; i < 26; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 4) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 4) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					if (gpBuffer[addr] == TRANS_COLOR) {
						gpBuffer[addr] = color; // 419[5]
					}
				}
			}
		}
	}
	// copy 426[0] to 419[5] and 419[3]
	for (int i = 26; i < 27; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr;
					if (y < MICRO_HEIGHT / 2) {
						addr = x + MICRO_WIDTH * ((i + 3) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 3) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 419[5]
					} else {
						addr = x + MICRO_WIDTH * ((i + 4) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 4) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 419[3]
					}
					if (gpBuffer[addr] == TRANS_COLOR) {
						gpBuffer[addr] = color;
					}
				}
			}
		}
	}
	// copy 428[1] to 419[3]
	for (int i = 27; i < 28; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 3) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 3) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					if (gpBuffer[addr] == TRANS_COLOR) {
						gpBuffer[addr] = color; // 419[3]
					}
				}
			}
		}
	}
	// copy 429[0] to 419[3] and 419[1]
	for (int i = 28; i < 29; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr;
					if (y < MICRO_HEIGHT / 2) {
						addr = x + MICRO_WIDTH * ((i + 2) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 2) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 419[3]
					} else {
						addr = x + MICRO_WIDTH * ((i + 3) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 3) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 419[1]
					}
					if (gpBuffer[addr] == TRANS_COLOR) {
						gpBuffer[addr] = color;
					}
				}
			}
		}
	}
	// copy 911[9] to 931[5]
	// copy 911[7] to 931[3]
	// copy 911[5] to 931[1]
	for (int i = 32; i < 35; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 3) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 3) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 919[9] to 927[5]
	// copy 919[5] to 927[1]
	for (int i = 40; i < 42; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 2) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 2) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 402[0] to 927[1]
	for (int i = 38; i < 39; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 5) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 5) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					if (gpBuffer[addr] == TRANS_COLOR) {
						gpBuffer[addr] = color;
					}
				}
			}
		}
	}
	// copy 954[2] to 919[7] (-> 927[3]) and 927[1]
	for (int i = 39; i < 40; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr;
					if (y < MICRO_HEIGHT / 2) {
						addr = x + MICRO_WIDTH * ((i + 7) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 7) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 919[7]
					} else {
						addr = x + MICRO_WIDTH * ((i + 4) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 4) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 927[1]
					}
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 918[9] to 926[5]
	for (int i = 47; i < 48; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 927[0] to 918[5] and 929[1]
	for (int i = 49; i < 50; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr;
					if (y < MICRO_HEIGHT / 2) {
						addr = x + MICRO_WIDTH * ((i + 3) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 3) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 918[5]
					} else {
						addr = x + MICRO_WIDTH * ((i + 5) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 5) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 929[1]
					}
					if (gpBuffer[addr] == TRANS_COLOR) {
						gpBuffer[addr] = color;
					}
				}
			}
		}
	}
	// copy 918[3] to 929[1]
	for (int i = 50; i < 51; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 4) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 4) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 918[2] to 929[0]
	for (int i = 51; i < 52; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 2) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 2) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 918[8] to 926[4]
	for (int i = 55; i < 56; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 928[4] to 920[8]
	for (int i = 57; i < 58; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 551[0] to 509[5] and 509[3]
	for (int i = 59; i < 60; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr;
					if (y < MICRO_HEIGHT / 2) {
						addr = x + MICRO_WIDTH * ((i + 3) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 3) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 509[5]
					} else {
						addr = x + MICRO_WIDTH * ((i + 4) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 4) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 509[3]
					}
					if (gpBuffer[addr] == TRANS_COLOR) {
						gpBuffer[addr] = color;
					}
				}
			}
		}
	}
	// copy 552[1] to 509[3]
	for (int i = 60; i < 61; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 3) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 3) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 519[0] to 509[3] and 509[1]
	for (int i = 61; i < 62; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr;
					if (y < MICRO_HEIGHT / 2) {
						addr = x + MICRO_WIDTH * ((i + 2) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 2) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 509[3]
					} else {
						addr = x + MICRO_WIDTH * ((i + 3) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 3) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 509[1]
					}
					if (gpBuffer[addr] == TRANS_COLOR) {
						gpBuffer[addr] = color;
					}
				}
			}
		}
	}
	// copy 510[7] to 551[3]
	// copy 510[5] to 551[1]
	for (int i = 65; i < 67; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 2) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 2) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 728[9] to 716[13]
	// copy 728[7] to 716[11]
	for (int i = 69; i < 71; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 2) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 2) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 910[9] to 930[5]
	// copy 910[7] to 930[3]
	for (int i = 73; i < 75; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 2) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 2) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 537[0] to 529[4]
	// copy 539[0] to 531[4]
	for (int i = 77; i < 79; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 2) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 2) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 478[0] to 477[1] and 480[1]
	for (int i = 81; i < 82; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR) {
					if (y < MICRO_HEIGHT / 2) {
						addr = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 477[1]
					} else {
						addr = x + MICRO_WIDTH * ((i + 2) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 2) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 480[1]
					}
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 479[1] to 477[0] and 480[0]
	for (int i = 84; i < 85; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR) {
					if (y < MICRO_HEIGHT / 2) {
						addr = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 477[0]
					} else {
						addr = x + MICRO_WIDTH * ((i + 2) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 2) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 480[0]
					}
					gpBuffer[addr] = color;
				}
			}
		}
	}

	// copy 517[0] to 551[1]
	for (int i = 68; i < 69; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (87 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (87 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 517[0]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 517[0] to 519[1]
	for (int i = 88; i < 89; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (87 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (87 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 517[0]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}

	// create the new CEL file
	constexpr int newEntries = lengthof(micros);
	BYTE* resCelBuf = DiabloAllocPtr(*celLen + newEntries * MICRO_WIDTH * MICRO_HEIGHT);

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

static BYTE* patchTownLightCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	typedef struct {
		int subtileIndex;
		unsigned microIndex;
		int res_encoding;
	} CelMicro;

	const CelMicro micros[] = {
/*  0 */{ 724 - 1, 0, -1 },             // used to block subsequent calls
/*  1 */{ 522 - 1, 1, -1 },             // move micros for better light propagation
/*  2 */{ 510 - 1, 9, MET_SQUARE },

/*  3 */{ 522 - 1, 0, -1 },
/*  4 */{ 509 - 1, 6, -1 },
/*  5 */{ 524 - 1, 1, MET_SQUARE },
/*  6 */{ 521 - 1, 1, MET_SQUARE },

/*  7 */{ 523 - 1, 1, -1 },
/*  8 */{ 509 - 1, 7, -1 },
/*  9 */{ 521 - 1, 0, MET_SQUARE },
/* 10 */{ 524 - 1, 0, MET_SQUARE },

/* 11 */{ 513 - 1, 5, MET_SQUARE },
/* 12 */{ 523 - 1, 0, MET_SQUARE },
/* 13 */{ 523 - 1, 2, MET_SQUARE },
/* 14 */{ 523 - 1, 4, MET_SQUARE },
/* 15 */{ 523 - 1, 6, MET_SQUARE },
/* 16 */{ 523 - 1, 8, MET_TRANSPARENT },
/* 17 */{ 523 - 1, 10, -1 },

/* 18 */{ 386 - 1, 9, MET_SQUARE },
/* 19 */{ 432 - 1, 0, MET_SQUARE },
/* 20 */{ 432 - 1, 2, MET_SQUARE },
/* 21 */{ 432 - 1, 4, MET_TRANSPARENT },
/* 22 */{ 432 - 1, 6, MET_TRANSPARENT },

/* 23 */{ 433 - 1, 0, -1 },
/* 24 */{ 387 - 1, 9, MET_SQUARE },
/* 25 */{ 432 - 1, 1, MET_SQUARE },
/* 26 */{ 430 - 1, 0, MET_SQUARE },
/* 27 */{ 430 - 1, 2, MET_SQUARE },
/* 28 */{ 430 - 1, 4, MET_TRANSPARENT },

/* 29 */{ 433 - 1, 1, -1 },
/* 30 */{ 424 - 1, 8, MET_SQUARE },
/* 31 */{ 431 - 1, 0, MET_SQUARE },
/* 32 */{ 430 - 1, 1, MET_SQUARE },
/* 33 */{ 430 - 1, 3, MET_SQUARE },
/* 34 */{ 430 - 1, 5, MET_TRANSPARENT },

/* 35 */{ 422 - 1, 8, MET_SQUARE },
/* 36 */{ 431 - 1, 1, MET_SQUARE },
/* 37 */{ 431 - 1, 3, MET_SQUARE },
/* 38 */{ 431 - 1, 5, MET_SQUARE },
/* 39 */{ 431 - 1, 7, MET_TRANSPARENT },

/* 40 */{ 422 - 1, 9, MET_SQUARE },
/* 41 */{ 436 - 1, 0, MET_SQUARE },
/* 42 */{ 436 - 1, 2, MET_SQUARE },
/* 43 */{ 436 - 1, 4, MET_SQUARE },
/* 44 */{ 436 - 1, 6, MET_TRANSPARENT },

/* 45 */{ 437 - 1, 0, -1 },
/* 46 */{ 423 - 1, 9, MET_SQUARE },
/* 47 */{ 436 - 1, 1, MET_SQUARE },
/* 48 */{ 434 - 1, 0, MET_SQUARE },
/* 49 */{ 434 - 1, 2, MET_SQUARE },
/* 50 */{ 434 - 1, 4, MET_TRANSPARENT },

/* 51 */{ 437 - 1, 1, -1 },
/* 52 */{ 435 - 1, 0, -1 },
/* 53 */{ 418 - 1, 11, MET_SQUARE },
/* 54 */{ 434 - 1, 1, MET_SQUARE },

/* 55 */{ 435 - 1, 1, -1 },
/* 56 */{ 419 - 1, 13, MET_SQUARE },

/* 57 */{ 440 - 1, 0, -1 },
/* 58 */{ 408 - 1, 12, MET_SQUARE },

/* 59 */{ 440 - 1, 1, -1 },
/* 60 */{ 441 - 1, 0, -1 },
/* 61 */{ 406 - 1, 10, MET_SQUARE },
/* 62 */{ 438 - 1, 0, MET_SQUARE },

/* 63 */{ 441 - 1, 1, -1 },
/* 64 */{ 412 - 1, 8, MET_SQUARE },
/* 65 */{ 439 - 1, 0, MET_SQUARE },
/* 66 */{ 438 - 1, 1, MET_SQUARE },
/* 67 */{ 438 - 1, 3, MET_SQUARE },
/* 68 */{ 438 - 1, 5, MET_TRANSPARENT },

/* 69 */{ 410 - 1, 8, MET_SQUARE },
/* 70 */{ 439 - 1, 1, MET_SQUARE },
/* 71 */{ 439 - 1, 3, MET_TRANSPARENT },
/* 72 */{ 439 - 1, 5, MET_TRANSPARENT },
/* 73 */{ 439 - 1, 7, -1 },

/*
249 == 386 +137
250 == 387 +137

268 == 406 +138
270 == 408 +138
272 == 410 +138
273 == 412 +139
276 == 418 +142
277 == 419 +142
280 == 422
281 == 423
282 == 424

288 == 430
289 == 431 +142
290 == 432
291 == 433
292 == 434
293 == 435
294 == 436
295 == 437
296 == 438
297 == 439
298 == 440
299 == 440 +142
*/
/*
copy 432[0] -> 386[9]
move down 432[0 2 4]
move micro 432[0 2 4] -> 386[11 13 15]

copy 430[0] -> 432[1]
copy 433[0] -> 387[9], 432[1]
move down 430[0 2] mask down 430[4]
move micro 432[1] -> 387[11]
move micro 430[0 2] -> 387[13 15]
blkmcr 433[0]

copy 433[1] -> 424[8], 431[0]
copy 430[1] -> 431[0]
move down 430[1 3] mask down 430[5]
move micro 431[0] -> 424[10]
move micro 430[1 3] -> 424[12 14]
blkmcr 433[1]

copy 431[1] -> 422[8]
move down 431[1 3 5] mask down 431[7]
move micro 431[1 3 5] -> 422[10 12 14]

copy 436[0] -> 422[9]
move down 436[0 2 4]
move micro 436[0 2 4] -> 422[11 13 15]

copy 437[0] -> 436[1], 423[9]
copy 434[0] -> 436[1]
move down 434[0 2]
move micro 436[1] -> 423[11]
move micro 434[0 2] -> 423[13 15]
blkmcr 437[0]

copy 437[1] -> 418[11]
copy 435[0] -> 418[11], 434[1]
move micro 434[1 3] -> 418[13 15]
blkmcr 435[0]
blkmcr 437[1]

copy 435[1] -> 419[13]
move micro 435[3] -> 419[15]
blkmcr 435[1]

copy 440[0] -> 408[12]
move micro 440[2] -> 408[14]
blkmcr 440[0]

copy 441[0] -> 406[10]
copy 440[1] -> 438[0], 406[10]
move micro 438[0 2] -> 406[12 14]
blkmcr 440[1]
blkmcr 441[0]
*/
/*
copy 441[1] -> 412[8], 439[0]
copy 438[1] -> 439[0]
move down 438[1 3]
move micro 439[0] -> 412[10]
move micro 438[1 3] -> 412[12 14]
blkmcr 441[1]

copy 439[1] -> 410[8]
move down 439[1 3 5]
move micro 439[1 3 5] -> 410[10 12 14]
blkmcr 439[7]
*/
	};

	const uint16_t* pSubtiles = (const uint16_t*)minBuf;
	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_TOWN;
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


	// copy 522[1] to 510[9]
	for (int i = 2; i < 3; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color == TRANS_COLOR) {
					unsigned addr2 = x + MICRO_WIDTH * (1 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (1 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 522[1]
					gpBuffer[addr] = gpBuffer[addr2];
				}
			}
		}
	}
	// copy 522[0] to 524[1]
	for (int i = 5; i < 6; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (3 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (3 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 522[0]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 522[0] to 521[1]
	for (int i = 6; i < 7; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (3 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (3 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 522[0]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}

	// copy 517[0] to 551[1] - done in patchTownDoorCel
	// copy 517[0] to 519[1] - done in patchTownDoorCel

	// copy 523[1] to 521[0]
	for (int i = 9; i < 10; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (7 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (7 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 523[1]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 523[1] to 524[0]
	for (int i = 10; i < 11; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (7 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (7 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 523[1]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}

	// copy 509[6] to 524[0]
	for (int i = 10; i < 11; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (4 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (4 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 509[6]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 509[7] to 524[1]
	for (int i = 5; i < 6; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (8 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (8 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 509[7]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}

	// copy 523[0] to 513[5]
	for (int i = 11; i < 12; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (12 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (12 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 523[0]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// move 523[0 2 4 6 8 (10)] down MICRO_HEIGHT / 2
	for (int i = 12; i < 17; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT - 1; y >= 0; y--) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH;
				} else {
					addr2 = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				}
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
	}

	// copy 432[0] to 386[9]
	for (int i = 18; i < 19; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (19 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (19 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 432[0]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}

	// move 432[0 2 4] down MICRO_HEIGHT / 2
	for (int i = 19; i < 22; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT - 1; y >= 0; y--) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH;
				} else {
					addr2 = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				}
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
	}
	// mask 432[6]
	for (int i = 22; i < 23; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}

	// copy 433[0] to 387[9]
	for (int i = 24; i < 25; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (23 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (23 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 433[0]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 433[0] to 432[1]
	for (int i = 25; i < 26; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (23 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (23 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 433[0]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 430[0] to 432[1]
	for (int i = 25; i < 26; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (26 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (26 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 430[0]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// move 430[0 2] down MICRO_HEIGHT / 2
	for (int i = 26; i < 28; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT - 1; y >= 0; y--) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH;
				} else {
					addr2 = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				}
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
	}
	// mask 430[4]
	for (int i = 28; i < 29; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}

	// copy 433[1] to 424[8]
	for (int i = 30; i < 31; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (29 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (29 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 433[1]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 433[1] to 431[0]
	for (int i = 31; i < 32; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (29 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (29 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 433[1]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 430[1] to 431[0]
	for (int i = 31; i < 32; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (32 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (32 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 430[1]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// move 430[1 3] down MICRO_HEIGHT / 2
	for (int i = 32; i < 34; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT - 1; y >= 0; y--) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH;
				} else {
					addr2 = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				}
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
	}
	// mask 430[5]
	for (int i = 34; i < 35; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}

	// copy 431[1] to 422[8]
	for (int i = 35; i < 36; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (36 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (36 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 431[1]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// move 431[1 3 5] down MICRO_HEIGHT / 2
	for (int i = 36; i < 39; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT - 1; y >= 0; y--) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH;
				} else {
					addr2 = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				}
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
	}
	// mask 431[7]
	for (int i = 39; i < 40; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}

	// copy 436[0] to 422[9]
	for (int i = 40; i < 41; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (41 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (41 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 436[0]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// move 436[0 2 4] down MICRO_HEIGHT / 2
	for (int i = 41; i < 44; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT - 1; y >= 0; y--) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH;
				} else {
					addr2 = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				}
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
	}
	// mask 436[6]
	for (int i = 44; i < 45; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}

	// copy 437[0] to 423[9]
	for (int i = 46; i < 47; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (45 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (45 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 437[0]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 437[0] to 436[1]
	for (int i = 47; i < 48; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (45 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (45 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 437[0]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 434[0] to 436[1]
	for (int i = 47; i < 48; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (48 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (48 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 434[0]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// move 434[0 2] down MICRO_HEIGHT / 2
	for (int i = 48; i < 50; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT - 1; y >= 0; y--) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH;
				} else {
					addr2 = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				}
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
	}
	// mask 434[4]
	for (int i = 50; i < 51; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}

	// copy 435[0] to 418[11]
	for (int i = 53; i < 54; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (52 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (52 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 435[0]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 435[0] to 434[1]
	for (int i = 54; i < 55; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (52 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (52 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 435[0]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 437[1] to 418[11]
	for (int i = 53; i < 54; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (51 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (51 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 437[1]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}

	// copy 435[1] to 419[13]
	for (int i = 56; i < 57; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (55 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (55 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 435[1]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}

	// copy 440[0] to 408[12]
	for (int i = 58; i < 59; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (57 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (57 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 440[0]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}

	// copy 440[1] to 406[10]
	for (int i = 61; i < 62; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (59 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (59 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 440[1]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 440[1] to 438[0]
	for (int i = 62; i < 63; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (59 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (59 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 440[1]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 441[0] to 406[10]
	for (int i = 61; i < 62; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (60 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (60 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 441[0]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}

	// copy 441[1] to 412[8]
	for (int i = 64; i < 65; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (63 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (63 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 441[1]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 441[1] to 439[0]
	for (int i = 65; i < 66; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (63 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (63 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 441[1]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// copy 438[1] to 439[0]
	for (int i = 65; i < 66; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (66 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (66 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 438[1]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// move 438[1 3] down MICRO_HEIGHT / 2
	for (int i = 66; i < 68; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT - 1; y >= 0; y--) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH;
				} else {
					addr2 = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				}
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
	}
	// mask 438[5]
	for (int i = 68; i < 69; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}

	// copy 439[1] to 410[8]
	for (int i = 69; i < 70; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (70 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (70 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 439[1]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// move 439[1 3 5 (7)] down MICRO_HEIGHT / 2
	for (int i = 70; i < 73; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT - 1; y >= 0; y--) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH;
				} else {
					addr2 = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				}
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
	}

	// create the new CEL file
	constexpr int newEntries = lengthof(micros);
	BYTE* resCelBuf = DiabloAllocPtr(*celLen + newEntries * MICRO_WIDTH * MICRO_HEIGHT);

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


static BYTE* patchTownChopCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	typedef struct {
		int subtileIndex;
		unsigned microIndex;
		int res_encoding;
	} CelMicro;

	const CelMicro micros[] = {
/*  0 */{ 180 - 1, 1, MET_RTRIANGLE },
/*  1 */{ 180 - 1, 3, MET_TRANSPARENT },

/*  2 */{ 224 - 1, 0, MET_LTRAPEZOID },
/*  3 */{ 224 - 1, 2, MET_TRANSPARENT },
/*  4 */{ 225 - 1, 2, MET_TRANSPARENT },

/*  5 */{ 362 - 1, 9, MET_TRANSPARENT },
/*  6 */{ 383 - 1, 3, MET_SQUARE },

/*  7 */{ 632 - 1, 11, MET_SQUARE },
/*  8 */{ 632 - 1, 13, MET_TRANSPARENT },
/*  9 */{ 631 - 1, 11, MET_TRANSPARENT },

/* 10 */{ 832 - 1, 10, MET_TRANSPARENT },

/* 11 */{ 834 - 1, 10, MET_SQUARE },
/* 12 */{ 834 - 1, 12, MET_TRANSPARENT },
/* 13 */{ 828 - 1, 12, MET_TRANSPARENT },

/* 14 */{ 864 - 1, 12, MET_SQUARE },
/* 15 */{ 864 - 1, 14, MET_TRANSPARENT },

/* 16 */{ 926 - 1, 12, MET_TRANSPARENT },
/* 17 */{ 926 - 1, 13, MET_TRANSPARENT },

/* 18 */{ 944 - 1, 6, MET_SQUARE },
/* 19 */{ 944 - 1, 8, MET_TRANSPARENT },
/* 20 */{ 942 - 1, 6, MET_TRANSPARENT },

/* 21 */{ 955 - 1, 13, MET_TRANSPARENT },
/* 22 */{ 950 - 1, 13, MET_TRANSPARENT },
/* 23 */{ 951 - 1, 13, MET_TRANSPARENT },
/* 24 */{ 946 - 1, 13, MET_TRANSPARENT },
/* 25 */{ 947 - 1, 13, MET_TRANSPARENT },
/* 26 */{ 940 - 1, 12, MET_TRANSPARENT },

/* 27 */{ 383 - 1, 5, MET_SQUARE },
/* 28 */{ 383 - 1, 7, MET_SQUARE },
// mask special cels
/* 29 */{ 213 - 1, 1, MET_RTRIANGLE }, // 138
/* 30 */{ 359 - 1, 1, MET_RTRIANGLE }, // 236
/* 31 */{ 627 - 1, 0, MET_LTRIANGLE }, // 416
/* 32 */{ 414 - 1, 6, MET_TRANSPARENT }, // 274
// TODO: 86?, 88 - mask trunc, 531, 533, 544, 546, 547 mask leaves, 534 mask branches?
	};

	const uint16_t* pSubtiles = (const uint16_t*)minBuf;
	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_TOWN;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		// if (micro.subtileIndex < 0) {
		//	continue;
		// }
		unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
		if ((SwapLE16(pSubtiles[index]) & 0xFFF) == 0) {
			mem_free_dbg(celBuf);
			app_warn("Invalid (empty) town subtile (%d).", micro.subtileIndex + 1);
			return NULL;
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

	// fix bad artifacts
	{ // 828[12]
		int i = 13;
		unsigned addr = 30 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = TRANS_COLOR;
		gpBuffer[addr + 1] = TRANS_COLOR;
	}
	{ // 180[3]
		int i = 1;
		unsigned addr =  1 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (23 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 0 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 1 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 2 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 3 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 4 * BUFFER_WIDTH] = TRANS_COLOR;
		unsigned addr2 =  0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (30 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr2 + 0 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr2 + 1 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // 224[0]
		int i = 2;
		unsigned addr =  0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = gpBuffer[addr + 2];
	}
	{ // + 225[2]
		int i = 4;
		unsigned addr = 31 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (17 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		unsigned addr2 = 29 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (20 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 0 * BUFFER_WIDTH] = gpBuffer[addr2 + 0 * BUFFER_WIDTH];
		gpBuffer[addr + 1 * BUFFER_WIDTH] = gpBuffer[addr2 + 1 * BUFFER_WIDTH];
		gpBuffer[addr - 1 + 1 * BUFFER_WIDTH] = gpBuffer[addr2 - 1 + 1 * BUFFER_WIDTH];
	}
	{ // 362[9]
		int i = 5;
		unsigned addr = 12 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  0 + 0 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  3 + 0 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr -  1 + 1 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  1 + 1 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // 383[3]
		int i = 6;
		unsigned addr  =  0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 3 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		unsigned addr2 =  5 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (18 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = gpBuffer[addr2];
	}
	{ // 632[11]
		int i = 7;
		unsigned addr  =  0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		unsigned addr2 =  7 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 2 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = gpBuffer[addr2];
		gpBuffer[addr + 1] = gpBuffer[addr2 + 1];
	}
	{ // + 632[13]
		int i = 8;
		unsigned addr  =  8 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (30 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		unsigned addr2 =  4 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (31 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = gpBuffer[addr + 2];
		gpBuffer[addr2] = gpBuffer[addr2 + 2];
	}
	{ // 832[10]
		int i = 10;
		unsigned addr  = 22 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = TRANS_COLOR;
		gpBuffer[addr - 1 + 0 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 0 + 1 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 1 + 2 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 2 + 3 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 1 + 3 * BUFFER_WIDTH] = gpBuffer[addr - 2 + 1 * BUFFER_WIDTH];
		gpBuffer[addr + 0 + 2 * BUFFER_WIDTH] = gpBuffer[addr - 2 + 0 * BUFFER_WIDTH];
		gpBuffer[addr - 2] = gpBuffer[addr - 2 + 1 * BUFFER_WIDTH];
		gpBuffer[addr - 3] = gpBuffer[addr - 5];
	}
	{ // 834[10]
		int i = 11;
		unsigned addr  =  0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		unsigned addr2 =  1 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 1 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = gpBuffer[addr2];
		gpBuffer[addr + 1] = gpBuffer[addr2 + 1];
	}
	{ // + 834[12]
		int i = 12;
		unsigned addr  =  3 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (31 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		unsigned addr2 =  6 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (30 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = gpBuffer[addr + 2];
		gpBuffer[addr + 1] = gpBuffer[addr + 3];
		gpBuffer[addr2] = gpBuffer[addr2 + 2];
	}
	{ // + 828[12]
		int i = 13;
		unsigned addr  = 29 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (17 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		unsigned addr2 = 27 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (18 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = gpBuffer[addr2 + 1];
		gpBuffer[addr + 1] = gpBuffer[addr2 + 2];
		gpBuffer[addr2] = gpBuffer[addr2 + 3];
		gpBuffer[addr2 - 3 + 1 * BUFFER_WIDTH] = gpBuffer[addr2 + 3];
	}
	{ // 864[12]
		int i = 14;
		unsigned addr  =  0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = gpBuffer[addr + 2];
	}
	{ // + 864[14]
		int i = 15;
		unsigned addr  =  4 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 30 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = gpBuffer[addr + 1];
		gpBuffer[addr - 2 + 1 * BUFFER_WIDTH] = gpBuffer[addr + 1];
	}
	{ // . 926[12]
		int i = 16;
		unsigned addr  =  26 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = TRANS_COLOR;
		gpBuffer[addr + 1] = TRANS_COLOR;
		gpBuffer[addr + 2] = TRANS_COLOR;
		gpBuffer[addr + 0 + 1 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr - 1 + 2 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // . 926[13]
		int i = 17;
		unsigned addr  =  7 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = TRANS_COLOR;
		gpBuffer[addr - 1] = TRANS_COLOR;
		gpBuffer[addr + 7 + 3 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // 944[6]
		int i = 18;
		unsigned addr  = 31 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = gpBuffer[addr - 2];
	}
	{ // + 944[8]
		int i = 19;
		unsigned addr  = 30 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 31 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = gpBuffer[addr - 2];
		gpBuffer[addr - 1 - 1 * BUFFER_WIDTH] = gpBuffer[addr - 2];
	}
	{ // + 942[6]
		int i = 20;
		unsigned addr  = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 17 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = gpBuffer[addr + BUFFER_WIDTH];
	}
	{ // . 955[13]
		int i = 21;
		unsigned addr  = 31 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 14 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = TRANS_COLOR;
		gpBuffer[addr - 1] = TRANS_COLOR;
		gpBuffer[addr - 1 - 1 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr - 2 - 1 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // . 950[13]
		int i = 22;
		unsigned addr  = 3 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = TRANS_COLOR;
		unsigned addr2  = 31 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 14 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr2] = TRANS_COLOR;
		gpBuffer[addr2 - 2 - 1 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // . 951[13]
		int i = 23;
		unsigned addr  = 3 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = TRANS_COLOR;
	}
	{ // . 946[13]
		int i = 24;
		unsigned addr  = 2 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = TRANS_COLOR;
	}
	{ // . 947[13]
		int i = 25;
		unsigned addr  = 3 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = TRANS_COLOR;
	}
	{ // . 940[12]
		int i = 26;
		unsigned addr  = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 14 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = TRANS_COLOR;
	}
	{ // 383[5] <- 180[1]
		int i = 27;
		int j = 0;
		unsigned addr  = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		BYTE color = gpBuffer[4 + MICRO_WIDTH * (j / DRAW_HEIGHT) + ( 8 + MICRO_HEIGHT * (j % DRAW_HEIGHT)) * BUFFER_WIDTH];
		gpBuffer[addr +  0 * BUFFER_WIDTH] = color;
		gpBuffer[addr +  9 * BUFFER_WIDTH] = color;
		gpBuffer[addr + 10 * BUFFER_WIDTH] = color;
		gpBuffer[addr + 20 * BUFFER_WIDTH] = color;
		gpBuffer[addr + 30 * BUFFER_WIDTH] = color;
	}
	{ // 383[7] <- 180[1]
		int i = 28;
		int j = 0;
		unsigned addr  = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		BYTE color = gpBuffer[4 + MICRO_WIDTH * (j / DRAW_HEIGHT) + ( 7 + MICRO_HEIGHT * (j % DRAW_HEIGHT)) * BUFFER_WIDTH];
		gpBuffer[addr +  6 * BUFFER_WIDTH] = color;
		gpBuffer[addr + 12 * BUFFER_WIDTH] = color;
		gpBuffer[addr + 20 * BUFFER_WIDTH] = color;
		gpBuffer[addr + 26 * BUFFER_WIDTH] = color;
	}
	{ // 414[6] - mask special cel
		int i = 32;
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 0; x < MICRO_WIDTH; x++) {
				if (x > 2 && y >= 28 - x) {
					unsigned addr  = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr + x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
		}
	}

	// create the new CEL file
	constexpr int newEntries = lengthof(micros);
	BYTE* resCelBuf = DiabloAllocPtr(*celLen + newEntries * MICRO_WIDTH * MICRO_HEIGHT);

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

BYTE* Town_PatchCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	// patch subtiles around the pot of Adria to prevent graphical glitch when a player passes it II.
	celBuf = patchTownPotCel(minBuf, minLen, celBuf, celLen);
	if (celBuf == NULL) {
		return NULL;
	}
	// patch subtiles of the cathedral to fix graphical glitch
	celBuf = patchTownCathedralCel(minBuf, minLen, celBuf, celLen);
	if (celBuf == NULL) {
		return NULL;
	}
	celBuf = patchTownFloorCel(minBuf, minLen, celBuf, celLen);
	if (celBuf == NULL) {
		return NULL;
	}
	celBuf = patchTownDoorCel(minBuf, minLen, celBuf, celLen);
	if (celBuf == NULL) {
		return NULL;
	}
	celBuf = patchTownLightCel(minBuf, minLen, celBuf, celLen);
	if (celBuf == NULL) {
		return NULL;
	}
	celBuf = patchTownChopCel(minBuf, minLen, celBuf, celLen);
	return celBuf;
}

typedef struct POS {
	BYTE x;
	BYTE y;
} POS;

typedef struct POScol {
	BYTE x;
	BYTE y;
	BYTE col;
	BYTE align;
} POScol;

static const POS trans18[] = {
#if 0
{ 61, 134 - MICRO_HEIGHT },
{ 59, 135 - MICRO_HEIGHT },
{ 60, 135 - MICRO_HEIGHT },
{ 61, 135 - MICRO_HEIGHT },
{ 62, 135 - MICRO_HEIGHT },
{ 63, 135 - MICRO_HEIGHT },
{ 59, 136 - MICRO_HEIGHT },
{ 60, 136 - MICRO_HEIGHT },
{ 61, 136 - MICRO_HEIGHT },
{ 62, 136 - MICRO_HEIGHT },
{ 63, 136 - MICRO_HEIGHT },
{ 59, 137 - MICRO_HEIGHT },
{ 60, 137 - MICRO_HEIGHT },
{ 61, 137 - MICRO_HEIGHT },
{ 62, 137 - MICRO_HEIGHT },
{ 63, 137 - MICRO_HEIGHT },
{ 59, 138 - MICRO_HEIGHT },
{ 60, 138 - MICRO_HEIGHT },
{ 61, 138 - MICRO_HEIGHT },
{ 62, 138 - MICRO_HEIGHT },
{ 59, 139 - MICRO_HEIGHT },
{ 60, 139 - MICRO_HEIGHT },
{ 59, 143 - MICRO_HEIGHT },
{ 58, 168 - MICRO_HEIGHT },
{ 59, 168 - MICRO_HEIGHT },
{ 60, 168 - MICRO_HEIGHT },
{ 60, 169 - MICRO_HEIGHT },
{ 62, 169 - MICRO_HEIGHT },
{ 63, 170 - MICRO_HEIGHT },
{ 55, 173 - MICRO_HEIGHT },
{ 56, 173 - MICRO_HEIGHT },
{ 57, 173 - MICRO_HEIGHT },
{ 56, 174 - MICRO_HEIGHT },
#endif
{ 44, 186 - MICRO_HEIGHT },
{ 42, 187 - MICRO_HEIGHT },
{ 43, 187 - MICRO_HEIGHT },
{ 43, 188 - MICRO_HEIGHT },
{ 44, 188 - MICRO_HEIGHT },
{ 45, 188 - MICRO_HEIGHT },
{ 45, 189 - MICRO_HEIGHT },
{ 46, 189 - MICRO_HEIGHT },
{ 47, 189 - MICRO_HEIGHT },
{ 47, 190 - MICRO_HEIGHT },
{ 48, 190 - MICRO_HEIGHT },
{ 61, 190 - MICRO_HEIGHT },
{ 49, 191 - MICRO_HEIGHT },
{ 50, 191 - MICRO_HEIGHT },
{ 52, 191 - MICRO_HEIGHT },
{ 55, 191 - MICRO_HEIGHT },
{ 59, 191 - MICRO_HEIGHT },
{ 60, 191 - MICRO_HEIGHT },
{ 51, 192 - MICRO_HEIGHT },
{ 52, 192 - MICRO_HEIGHT },
{ 53, 192 - MICRO_HEIGHT },
{ 54, 192 - MICRO_HEIGHT },
{ 55, 192 - MICRO_HEIGHT },
{ 56, 192 - MICRO_HEIGHT },
{ 57, 192 - MICRO_HEIGHT },
{ 58, 192 - MICRO_HEIGHT },
{ 59, 192 - MICRO_HEIGHT },
{ 60, 192 - MICRO_HEIGHT },
{ 53, 193 - MICRO_HEIGHT },
{ 54, 193 - MICRO_HEIGHT },
{ 55, 193 - MICRO_HEIGHT },
{ 56, 193 - MICRO_HEIGHT },
{ 57, 193 - MICRO_HEIGHT },
{ 58, 193 - MICRO_HEIGHT },
{ 59, 193 - MICRO_HEIGHT },
{ 56, 194 - MICRO_HEIGHT },
{ 57, 194 - MICRO_HEIGHT },
{ 58, 194 - MICRO_HEIGHT },
{ 59, 194 - MICRO_HEIGHT },
{ 58, 195 - MICRO_HEIGHT },
{ 59, 195 - MICRO_HEIGHT },
{ 60, 195 - MICRO_HEIGHT },
{ 60, 196 - MICRO_HEIGHT },
{ 61, 196 - MICRO_HEIGHT },
{ 62, 196 - MICRO_HEIGHT },
{ 63, 196 - MICRO_HEIGHT },
{ 62, 197 - MICRO_HEIGHT },
};

static const POS trans19[] = {
{ 24, 68 },
{ 22, 69 },
{ 23, 69 },
{ 24, 69 },
{ 25, 69 },
{ 20, 70 },
{ 21, 70 },
{ 22, 70 },
{ 23, 70 },
{ 24, 70 },
{ 25, 70 },
{ 26, 70 },
{ 18, 71 },
{ 19, 71 },
{ 20, 71 },
{ 21, 71 },
{ 22, 71 },
{ 23, 71 },
{ 24, 71 },
{ 25, 71 },
{ 26, 71 },
{ 27, 71 },
{ 16, 72 },
{ 17, 72 },
{ 19, 72 },
{ 20, 72 },
{ 21, 72 },
{ 22, 72 },
{ 23, 72 },
{ 24, 72 },
{ 25, 72 },
{ 26, 72 },
{ 27, 72 },
{ 28, 72 },
{ 20, 73 },
{ 21, 73 },
{ 22, 73 },
{ 26, 73 },
{ 27, 73 },
{ 28, 73 },
{ 29, 73 },
{ 12, 74 },
{ 13, 74 },
{ 20, 74 },
{ 21, 74 },
{ 26, 74 },
{ 27, 74 },
{ 28, 74 },
{ 29, 74 },
{ 30, 74 },
{ 10, 75 },
{ 11, 75 },
{ 12, 75 },
{ 13, 75 },
{ 14, 75 },
{ 27, 75 },
{ 28, 75 },
{ 29, 75 },
{ 30, 75 },
{ 31, 75 },
{ 11, 76 },
{ 12, 76 },
{ 13, 76 },
{ 14, 76 },
{ 28, 76 },
{ 29, 76 },
{ 30, 76 },
{ 31, 76 },
{ 11, 77 },
{ 12, 77 },
{ 13, 77 },
{ 14, 77 },
{ 28, 77 },
{ 29, 77 },
{ 30, 77 },
{ 31, 77 },
{ 5, 78 },
{ 12, 78 },
{ 13, 78 },
{ 28, 78 },
{ 29, 78 },
{ 30, 78 },
{ 31, 78 },
{ 5, 79 },
{ 6, 79 },
{ 7, 79 },
{ 12, 79 },
{ 13, 79 },
{ 20, 79 },
{ 21, 79 },
{ 27, 79 },
{ 28, 79 },
{ 29, 79 },
{ 30, 79 },
{ 31, 79 },
{ 0, 80 },
{ 5, 80 },
{ 6, 80 },
{ 7, 80 },
{ 13, 80 },
{ 14, 80 },
{ 15, 80 },
{ 20, 80 },
{ 21, 80 },
{ 27, 80 },
{ 28, 80 },
{ 29, 80 },
{ 30, 80 },
{ 31, 80 },
{ 0, 81 },
{ 6, 81 },
{ 7, 81 },
{ 8, 81 },
{ 12, 81 },
{ 13, 81 },
{ 14, 81 },
{ 15, 81 },
{ 16, 81 },
{ 19, 81 },
{ 20, 81 },
{ 21, 81 },
{ 22, 81 },
{ 27, 81 },
{ 28, 81 },
{ 31, 81 },
{ 0, 82 },
{ 1, 82 },
{ 7, 82 },
{ 8, 82 },
{ 9, 82 },
{ 12, 82 },
{ 13, 82 },
{ 14, 82 },
{ 15, 82 },
{ 16, 82 },
{ 17, 82 },
{ 18, 82 },
{ 19, 82 },
{ 20, 82 },
{ 21, 82 },
{ 32, 82 },
{ 33, 82 },
{ 34, 82 },
{ 35, 82 },
{ 36, 82 },
{ 37, 82 },
{ 38, 82 },
{ 39, 82 },
{ 0, 83 },
{ 1, 83 },
{ 8, 83 },
{ 14, 83 },
{ 17, 83 },
{ 18, 83 },
{ 19, 83 },
{ 32, 83 },
{ 33, 83 },
{ 34, 83 },
{ 35, 83 },
{ 36, 83 },
{ 37, 83 },
{ 38, 83 },
{ 39, 83 },
{ 0, 84 },
{ 18, 84 },
{ 19, 84 },
{ 33, 84 },
{ 34, 84 },
{ 35, 84 },
{ 36, 84 },
{ 37, 84 },
{ 38, 84 },
{ 39, 84 },
{ 19, 85 },
{ 33, 85 },
{ 34, 85 },
{ 35, 85 },
{ 36, 85 },
{ 37, 85 },
{ 38, 85 },
{ 39, 85 },
{ 19, 86 },
{ 26, 86 },
{ 34, 86 },
{ 35, 86 },
{ 36, 86 },
{ 37, 86 },
{ 38, 86 },
{ 39, 86 },
{ 19, 87 },
{ 25, 87 },
{ 26, 87 },
{ 27, 87 },
{ 34, 87 },
{ 35, 87 },
{ 36, 87 },
{ 37, 87 },
{ 38, 87 },
{ 39, 87 },
{ 19, 88 },
{ 26, 88 },
{ 27, 88 },
{ 28, 88 },
{ 33, 88 },
{ 34, 88 },
{ 35, 88 },
{ 36, 88 },
{ 37, 88 },
{ 38, 88 },
{ 39, 88 },
{ 6, 89 },
{ 7, 89 },
{ 8, 89 },
// { 21, 89 },
{ 27, 89 },
{ 33, 89 },
{ 34, 89 },
{ 35, 89 },
{ 36, 89 },
{ 37, 89 },
{ 38, 89 },
{ 39, 89 },
{ 5, 90 },
{ 6, 90 },
{ 7, 90 },
{ 8, 90 },
{ 17, 90 },
{ 18, 90 },
{ 34, 90 },
{ 35, 90 },
{ 37, 90 },
{ 38, 90 },
{ 39, 90 },
{ 6, 91 },
{ 7, 91 },
{ 14, 91 },
{ 18, 91 },
{ 19, 91 },
{ 38, 91 },
{ 39, 91 },
{ 13, 92 },
{ 37, 92 },
{ 38, 92 },
{ 39, 92 },
// { 14, 93 },
// { 24, 93 },
// { 25, 93 },
{ 30, 93 },
{ 31, 93 },
{ 37, 93 },
{ 39, 93 },
{ 13, 94 },
{ 14, 94 },
//{ 26, 94 },
//{ 27, 94 },
{ 31, 94 },
{ 39, 94 },
{ 11, 95 },
{ 24, 95 },
//{ 25, 95 },
{ 12, 96 },
{ 25, 96 },
{ 32, 96 },
{ 33, 96 },
{ 38, 96 },
{ 32, 97 },
{ 37, 97 },
{ 38, 97 },
{ 37, 98 },
{ 38, 98 },
{ 39, 98 },
{ 38, 99 },
{ 39, 99 },
{ 24, 100 },
{ 25, 100 },
{ 37, 100 },
{ 38, 100 },
{ 39, 100 },
{ 2, 101 },
{ 3, 101 },
{ 4, 101 },
{ 5, 101 },
{ 6, 101 },
{ 14, 101 },
{ 26, 101 },
{ 27, 101 },
{ 38, 101 },
{ 39, 101 },
{ 1, 102 },
{ 2, 102 },
{ 3, 102 },
{ 4, 102 },
{ 7, 102 },
{ 13, 102 },
{ 14, 102 },
{ 15, 102 },
{ 29, 102 },
{ 30, 102 },
{ 38, 102 },
{ 39, 102 },
{ 3, 103 },
{ 8, 103 },
{ 12, 103 },
{ 13, 103 },
{ 14, 103 },
{ 15, 103 },
{ 16, 103 },
{ 29, 103 },
{ 30, 103 },
{ 31, 103 },
{ 38, 103 },
{ 39, 103 },
{ 12, 104 },
{ 13, 104 },
{ 14, 104 },
{ 15, 104 },
{ 16, 104 },
{ 29, 104 },
{ 30, 104 },
{ 31, 104 },
{ 32, 104 },
{ 37, 104 },
{ 38, 104 },
{ 39, 104 },
{ 15, 105 },
{ 16, 105 },
{ 28, 105 },
{ 29, 105 },
{ 30, 105 },
{ 31, 105 },
{ 32, 105 },
{ 37, 105 },
{ 38, 105 },
{ 39, 105 },
{ 16, 106 },
{ 30, 106 },
{ 31, 106 },
{ 32, 106 },
{ 34, 106 },
{ 35, 106 },
{ 36, 106 },
{ 37, 106 },
{ 38, 106 },
{ 39, 106 },
{ 31, 107 },
{ 32, 107 },
{ 33, 107 },
{ 34, 107 },
{ 35, 107 },
{ 36, 107 },
{ 37, 107 },
{ 38, 107 },
{ 39, 107 },
{ 32, 108 },
{ 33, 108 },
{ 34, 108 },
{ 35, 108 },
{ 36, 108 },
{ 37, 108 },
{ 38, 108 },
{ 39, 108 },
{ 32, 109 },
{ 33, 109 },
{ 34, 109 },
{ 35, 109 },
{ 36, 109 },
{ 37, 109 },
{ 38, 109 },
{ 39, 109 },
{ 31, 110 },
{ 32, 110 },
{ 33, 110 },
{ 34, 110 },
{ 35, 110 },
{ 36, 110 },
{ 37, 110 },
{ 38, 110 },
{ 39, 110 },
{ 35, 111 },
{ 36, 111 },
{ 37, 111 },
{ 38, 111 },
{ 39, 111 },
//{ 15, 112 },
{ 17, 112 },
{ 18, 112 },
{ 36, 112 },
{ 37, 112 },
{ 38, 112 },
{ 39, 112 },
{ 18, 113 },
{ 19, 113 },
{ 36, 113 },
{ 37, 113 },
{ 38, 113 },
{ 39, 113 },
{ 2, 114 },
{ 3, 114 },
//{ 15, 114 },
{ 25, 114 },
{ 26, 114 },
{ 38, 114 },
{ 39, 114 },
{ 0, 115 },
{ 1, 115 },
{ 2, 115 },
{ 3, 115 },
{ 26, 115 },
{ 27, 115 },
{ 39, 115 },
{ 0, 116 },
{ 1, 116 },
{ 2, 116 },
{ 26, 116 },
{ 27, 116 },
{ 38, 116 },
{ 39, 116 },
{ 0, 117 },
{ 22, 117 },
{ 27, 117 },
{ 38, 117 },
{ 39, 117 },
{ 22, 118 },
{ 34, 118 },
{ 38, 118 },
{ 39, 118 },
{ 11, 119 },
{ 23, 119 },
{ 24, 119 },
{ 34, 119 },
{ 35, 119 },
{ 37, 119 },
{ 38, 119 },
{ 39, 119 },
{ 12, 120 },
{ 24, 120 },
{ 35, 120 },
{ 36, 120 },
{ 37, 120 },
{ 38, 120 },
{ 39, 120 },
{ 13, 121 },
{ 14, 121 },
{ 15, 121 },
{ 24, 121 },
{ 25, 121 },
{ 26, 121 },
{ 34, 121 },
{ 35, 121 },
{ 36, 121 },
{ 37, 121 },
{ 38, 121 },
{ 39, 121 },
{ 0, 122 },
{ 13, 122 },
{ 14, 122 },
{ 23, 122 },
{ 24, 122 },
{ 25, 122 },
{ 26, 122 },
{ 27, 122 },
{ 32, 122 },
{ 33, 122 },
{ 34, 122 },
{ 35, 122 },
{ 36, 122 },
{ 37, 122 },
{ 38, 122 },
{ 39, 122 },
{ 14, 123 },
{ 23, 123 },
{ 24, 123 },
{ 25, 123 },
{ 26, 123 },
{ 27, 123 },
{ 28, 123 },
{ 29, 123 },
{ 30, 123 },
{ 31, 123 },
{ 32, 123 },
{ 33, 123 },
{ 34, 123 },
{ 35, 123 },
{ 36, 123 },
{ 37, 123 },
{ 38, 123 },
{ 39, 123 },
{ 13, 124 },
{ 14, 124 },
{ 15, 124 },
{ 24, 124 },
{ 26, 124 },
{ 27, 124 },
{ 28, 124 },
{ 29, 124 },
{ 30, 124 },
{ 31, 124 },
{ 32, 124 },
{ 33, 124 },
{ 34, 124 },
{ 35, 124 },
{ 36, 124 },
{ 37, 124 },
{ 38, 124 },
{ 39, 124 },
{ 12, 125 },
{ 13, 125 },
{ 14, 125 },
{ 15, 125 },
{ 16, 125 },
{ 27, 125 },
{ 28, 125 },
{ 29, 125 },
{ 30, 125 },
{ 31, 125 },
{ 32, 125 },
{ 33, 125 },
{ 34, 125 },
{ 35, 125 },
{ 36, 125 },
{ 37, 125 },
{ 12, 126 },
{ 13, 126 },
{ 14, 126 },
{ 15, 126 },
{ 22, 126 },
{ 26, 126 },
{ 27, 126 },
{ 28, 126 },
{ 29, 126 },
{ 30, 126 },
{ 31, 126 },
{ 32, 126 },
{ 33, 126 },
{ 34, 126 },
{ 35, 126 },
{ 11, 127 },
{ 12, 127 },
{ 13, 127 },
{ 14, 127 },
{ 15, 127 },
{ 23, 127 },
{ 24, 127 },
{ 27, 127 },
{ 28, 127 },
{ 29, 127 },
{ 30, 127 },
{ 31, 127 },
{ 32, 127 },
{ 33, 127 },
{ 11, 128 },
{ 12, 128 },
{ 13, 128 },
{ 14, 128 },
{ 15, 128 },
{ 22, 128 },
{ 23, 128 },
{ 24, 128 },
{ 25, 128 },
{ 26, 128 },
{ 27, 128 },
{ 28, 128 },
{ 29, 128 },
{ 30, 128 },
{ 31, 128 },
{ 12, 129 },
{ 13, 129 },
{ 14, 129 },
{ 15, 129 },
{ 22, 129 },
{ 23, 129 },
{ 24, 129 },
{ 25, 129 },
{ 26, 129 },
{ 27, 129 },
{ 28, 129 },
{ 29, 129 },
{ 13, 130 },
{ 14, 130 },
{ 15, 130 },
{ 16, 130 },
{ 18, 130 },
{ 20, 130 },
{ 21, 130 },
{ 22, 130 },
{ 23, 130 },
{ 24, 130 },
{ 25, 130 },
{ 26, 130 },
{ 27, 130 },
{ 13, 131 },
{ 14, 131 },
{ 15, 131 },
{ 16, 131 },
{ 17, 131 },
{ 18, 131 },
{ 19, 131 },
{ 20, 131 },
{ 21, 131 },
{ 22, 131 },
{ 23, 131 },
{ 24, 131 },
{ 25, 131 },
{ 13, 132 },
{ 14, 132 },
{ 15, 132 },
{ 16, 132 },
{ 17, 132 },
{ 18, 132 },
{ 19, 132 },
{ 20, 132 },
{ 21, 132 },
{ 22, 132 },
{ 23, 132 },
{ 13, 133 },
{ 14, 133 },
{ 15, 133 },
{ 16, 133 },
{ 17, 133 },
{ 18, 133 },
{ 19, 133 },
{ 20, 133 },
{ 21, 133 },
{ 13, 134 },
{ 14, 134 },
{ 15, 134 },
{ 16, 134 },
{ 17, 134 },
{ 18, 134 },
{ 19, 134 },
{ 4, 135 },
{ 5, 135 },
{ 6, 135 },
{ 13, 135 },
{ 14, 135 },
{ 15, 135 },
{ 16, 135 },
{ 17, 135 },
{ 5, 136 },
{ 6, 136 },
{ 12, 136 },
{ 13, 136 },
{ 14, 136 },
{ 15, 136 },
{ 6, 137 },
{ 11, 137 },
{ 12, 137 },
{ 13, 137 },
{ 0, 138 },
{ 11, 138 },
{ 12, 138 },
{ 13, 138 },
{ 0, 139 },
{ 1, 139 },
{ 2, 139 },
{ 5, 139 },
{ 6, 139 },
{ 12, 139 },
{ 13, 139 },
{ 0, 140 },
{ 1, 140 },
{ 2, 140 },
{ 3, 140 },
{ 4, 140 },
{ 5, 140 },
{ 13, 140 },
{ 0, 141 },
{ 1, 141 },
{ 2, 141 },
{ 3, 141 },
{ 4, 141 },
{ 5, 141 },
{ 13, 141 },
{ 0, 142 },
{ 1, 142 },
{ 2, 142 },
{ 3, 142 },
{ 4, 142 },
{ 5, 142 },
{ 0, 143 },
{ 1, 143 },
{ 2, 143 },
{ 3, 143 },
{ 4, 143 },
{ 5, 143 },
{ 13, 143 },
{ 0, 144 },
{ 1, 144 },
{ 2, 144 },
{ 3, 144 },
{ 4, 144 },
{ 5, 144 },
{ 6, 144 },
{ 7, 144 },
{ 2, 145 },
{ 3, 145 },
{ 4, 145 },
{ 5, 145 },
{ 6, 145 },
{ 7, 145 },
{ 8, 145 },
{ 12, 145 },
{ 13, 145 },
{ 4, 146 },
{ 5, 146 },
{ 6, 146 },
{ 7, 146 },
{ 8, 146 },
{ 9, 146 },
{ 10, 146 },
{ 11, 146 },
{ 12, 146 },
{ 13, 146 },
{ 6, 147 },
{ 7, 147 },
{ 8, 147 },
{ 9, 147 },
{ 10, 147 },
{ 11, 147 },
{ 12, 147 },
{ 13, 147 },
{ 8, 148 },
{ 9, 148 },
{ 10, 148 },
{ 11, 148 },
{ 12, 148 },
{ 13, 148 },
{ 10, 149 },
{ 11, 149 },
{ 12, 149 },
{ 13, 149 },
{ 12, 150 },
{ 13, 150 },
{ 0, 63 },
{ 0, 64 },
{ 1, 64 },
{ 2, 64 },
{ 1, 65 },
{ 2, 65 },
{ 3, 65 },
{ 4, 65 },
{ 1, 66 },
{ 2, 66 },
{ 3, 66 },
{ 4, 66 },
{ 5, 66 },
{ 6, 66 },
{ 2, 67 },
{ 3, 67 },
{ 4, 67 },
{ 5, 67 },
{ 6, 67 },
{ 7, 67 },
{ 8, 67 },
{ 4, 68 },
{ 5, 68 },
{ 6, 68 },
{ 7, 68 },
{ 8, 68 },
{ 9, 68 },
{ 10, 68 },
{ 0, 69 },
{ 1, 69 },
{ 5, 69 },
{ 6, 69 },
{ 7, 69 },
{ 8, 69 },
{ 9, 69 },
{ 10, 69 },
{ 0, 70 },
{ 1, 70 },
{ 2, 70 },
{ 3, 70 },
{ 6, 70 },
{ 7, 70 },
{ 8, 70 },
{ 9, 70 },
{ 10, 70 },
{ 0, 71 },
{ 1, 71 },
{ 2, 71 },
{ 3, 71 },
{ 4, 71 },
{ 7, 71 },
{ 0, 72 },
{ 1, 72 },
{ 2, 72 },
{ 3, 72 },
{ 4, 72 },
{ 5, 72 },
{ 18, 72 },
{ 0, 73 },
{ 1, 73 },
{ 2, 73 },
{ 3, 73 },
{ 4, 73 },
{ 5, 73 },
{ 6, 73 },
{ 9, 73 },
{ 10, 73 },
{ 11, 73 },
{ 17, 73 },
{ 18, 73 },
{ 19, 73 },
{ 1, 74 },
{ 2, 74 },
{ 3, 74 },
{ 4, 74 },
{ 5, 74 },
{ 10, 74 },
{ 11, 74 },
{ 18, 74 },
{ 19, 74 },
{ 1, 75 },
{ 2, 75 },
{ 3, 75 },
{ 4, 75 },
{ 5, 75 },
{ 19, 75 },
{ 3, 76 },
{ 4, 76 },
{ 5, 76 },
{ 5, 77 },
};

static const POScol cols19[] =
{
{ 23, 93, 30, 0 },
{ 22, 94, 57, 0 },
{ 23, 94, 57, 0 },
{ 24, 94, 30, 0 },
{ 25, 94, 30, 0 },
};

static const POS trans20[] =
{
{ 32, 48 },
{ 33, 48 },
{ 34, 48 },
{ 35, 48 },
{ 36, 48 },
{ 37, 48 },
{ 32, 49 },
{ 33, 49 },
{ 34, 49 },
{ 35, 49 },
{ 36, 49 },
{ 32, 50 },
{ 33, 50 },
{ 34, 50 },
{ 35, 50 },
{ 42, 50 },
{ 32, 51 },
{ 33, 51 },
{ 34, 51 },
{ 43, 51 },
{ 44, 51 },
{ 32, 52 },
{ 33, 52 },
{ 34, 52 },
{ 42, 52 },
{ 43, 52 },
{ 44, 52 },
{ 45, 52 },
{ 46, 52 },
{ 42, 53 },
{ 43, 53 },
{ 44, 53 },
{ 45, 53 },
{ 46, 53 },
{ 47, 53 },
{ 48, 53 },
{ 40, 54 },
{ 41, 54 },
{ 42, 54 },
{ 43, 54 },
{ 44, 54 },
{ 45, 54 },
{ 46, 54 },
{ 47, 54 },
{ 48, 54 },
{ 49, 54 },
{ 50, 54 },
{ 34, 55 },
{ 42, 55 },
{ 43, 55 },
{ 44, 55 },
{ 45, 55 },
{ 46, 55 },
{ 47, 55 },
{ 48, 55 },
{ 49, 55 },
{ 50, 55 },
{ 51, 55 },
{ 52, 55 },
{ 47, 56 },
{ 48, 56 },
{ 49, 56 },
{ 50, 56 },
{ 51, 56 },
{ 52, 56 },
{ 53, 56 },
{ 54, 56 },
{ 39, 57 },
{ 43, 57 },
{ 44, 57 },
{ 45, 57 },
{ 49, 57 },
{ 50, 57 },
{ 51, 57 },
{ 52, 57 },
{ 53, 57 },
{ 54, 57 },
{ 55, 57 },
{ 56, 57 },
{ 37, 58 },
{ 38, 58 },
{ 39, 58 },
{ 40, 58 },
{ 44, 58 },
{ 45, 58 },
{ 46, 58 },
{ 47, 58 },
{ 51, 58 },
{ 52, 58 },
{ 53, 58 },
{ 54, 58 },
{ 55, 58 },
{ 56, 58 },
{ 57, 58 },
{ 58, 58 },

{ 37, 59 },
{ 38, 59 },
{ 39, 59 },
{ 40, 59 },
{ 41, 59 },
{ 44, 59 },
{ 45, 59 },
{ 46, 59 },
{ 47, 59 },
{ 48, 59 },
{ 49, 59 },
{ 53, 59 },
{ 54, 59 },
{ 55, 59 },
{ 56, 59 },
{ 57, 59 },
{ 58, 59 },
{ 59, 59 },
{ 60, 59 },
{ 36, 60 },
{ 37, 60 },
{ 38, 60 },
{ 39, 60 },
{ 40, 60 },
{ 41, 60 },
{ 42, 60 },
{ 43, 60 },
{ 47, 60 },
{ 48, 60 },
{ 49, 60 },
{ 56, 60 },
{ 57, 60 },
{ 58, 60 },
{ 59, 60 },
{ 60, 60 },
{ 61, 60 },
{ 36, 61 },
{ 37, 61 },
{ 38, 61 },
{ 39, 61 },
{ 40, 61 },
{ 41, 61 },
{ 42, 61 },
{ 43, 61 },
{ 44, 61 },
{ 47, 61 },
{ 48, 61 },
{ 49, 61 },
{ 57, 61 },
{ 58, 61 },
{ 59, 61 },
{ 60, 61 },
{ 38, 62 },
{ 39, 62 },
{ 40, 62 },
{ 41, 62 },
{ 42, 62 },
{ 48, 62 },
{ 58, 62 },
{ 59, 62 },
{ 33, 63 },
{ 39, 63 },
{ 40, 63 },
{ 48, 63 },
{ 5, 64 },
{ 19, 64 },
{ 20, 64 },
{ 22, 64 },
{ 23, 64 },
{ 39, 64 },
{ 18, 65 },
{ 19, 65 },
{ 20, 65 },
{ 21, 65 },
{ 22, 65 },
{ 24, 65 },
{ 30, 65 },
{ 31, 65 },
{ 39, 65 },
{ 58, 65 },
{ 59, 65 },
{ 63, 65 },
{ 3, 66 },
{ 19, 66 },
{ 20, 66 },
{ 21, 66 },
{ 24, 66 },
{ 29, 66 },
{ 39, 66 },
{ 40, 66 },
{ 59, 66 },
{ 60, 66 },
{ 61, 66 },
{ 0, 67 },
{ 3, 67 },
{ 19, 67 },
{ 21, 67 },
{ 23, 67 },
{ 24, 67 },
{ 29, 67 },
{ 30, 67 },
{ 31, 67 },
{ 32, 67 },
{ 47, 67 },
{ 48, 67 },
{ 60, 67 },
{ 61, 67 },
{ 62, 67 },
{ 20, 68 },
{ 21, 68 },
{ 22, 68 },
{ 23, 68 },
{ 24, 68 },
{ 29, 68 },
{ 30, 68 },
{ 31, 68 },
{ 32, 68 },
{ 33, 68 },
{ 45, 68 },
{ 46, 68 },
{ 47, 68 },
{ 48, 68 },
{ 49, 68 },
{ 60, 68 },
{ 61, 68 },
{ 62, 68 },
{ 63, 68 },
{ 21, 69 },
{ 22, 69 },
{ 32, 69 },
{ 33, 69 },
{ 34, 69 },
{ 44, 69 },
{ 45, 69 },
{ 46, 69 },
{ 47, 69 },
{ 48, 69 },
{ 49, 69 },
{ 50, 69 },
{ 54, 69 },
{ 29, 69 },
{ 30, 69 },
{ 31, 69 },
{ 61, 69 },
{ 62, 69 },
{ 63, 69 },
{ 28, 70 },
{ 29, 70 },
{ 30, 70 },
{ 31, 70 },
{ 32, 70 },
{ 40, 70 },
{ 43, 70 },
{ 44, 70 },
{ 45, 70 },
{ 46, 70 },
{ 47, 70 },
{ 48, 70 },
{ 49, 70 },
{ 50, 70 },
{ 54, 70 },
{ 55, 70 },
{ 61, 70 },
{ 62, 70 },
{ 63, 70 },
{ 5, 71 },
{ 27, 71 },
{ 28, 71 },
{ 31, 71 },
{ 32, 71 },
{ 41, 71 },
{ 42, 71 },
{ 43, 71 },
{ 44, 71 },
{ 45, 71 },
{ 51, 71 },
{ 52, 71 },
{ 55, 71 },
{ 56, 71 },
{ 57, 71 },
{ 60, 71 },
{ 61, 71 },
{ 62, 71 },
{ 63, 71 },
{ 5, 72 },
{ 12, 72 },
{ 24, 72 },
{ 27, 72 },
{ 28, 72 },
{ 29, 72 },
{ 30, 72 },
{ 32, 72 },
{ 42, 72 },
{ 43, 72 },
{ 44, 72 },
{ 45, 72 },
{ 46, 72 },
{ 52, 72 },
{ 59, 72 },
{ 60, 72 },
{ 61, 72 },
{ 62, 72 },
{ 63, 72 },
{ 9, 73 },
{ 10, 73 },
{ 11, 73 },
{ 14, 73 },
{ 15, 73 },
{ 24, 73 },
{ 27, 73 },
{ 30, 73 },
{ 31, 73 },
{ 32, 73 },
{ 38, 73 },
{ 39, 73 },
{ 43, 73 },
{ 44, 73 },
{ 52, 73 },
{ 58, 73 },
{ 59, 73 },
{ 60, 73 },
{ 61, 73 },
{ 62, 73 },
{ 63, 73 },
{ 8, 74 },
{ 9, 74 },
{ 10, 74 },
{ 14, 74 },
{ 15, 74 },
{ 16, 74 },
{ 30, 74 },
{ 31, 74 },
{ 32, 74 },
{ 37, 74 },
{ 38, 74 },
{ 39, 74 },
{ 40, 74 },
{ 52, 74 },
{ 53, 74 },
{ 54, 74 },
{ 58, 74 },
{ 59, 74 },
{ 60, 74 },
{ 61, 74 },
{ 62, 74 },
{ 63, 74 },
{ 15, 75 },
{ 16, 75 },
{ 17, 75 },
{ 32, 75 },
{ 33, 75 },
{ 35, 75 },
{ 36, 75 },
{ 37, 75 },
{ 38, 75 },
{ 39, 75 },
{ 40, 75 },
{ 41, 75 },
{ 52, 75 },
{ 53, 75 },
{ 54, 75 },
{ 53, 73 },
{ 55, 75 },
{ 58, 75 },
{ 59, 75 },
{ 60, 75 },
{ 61, 75 },
{ 62, 75 },
{ 63, 75 },
{ 6, 76 },
{ 16, 76 },
{ 17, 76 },
{ 21, 76 },
{ 32, 76 },
{ 33, 76 },
{ 34, 76 },
{ 35, 76 },
{ 36, 76 },
{ 37, 76 },
{ 38, 76 },
{ 39, 76 },
{ 40, 76 },
{ 41, 76 },
{ 42, 76 },
{ 53, 76 },
{ 54, 76 },
{ 55, 76 },
{ 56, 76 },
{ 59, 76 },
{ 60, 76 },
{ 61, 76 },
{ 62, 76 },
{ 63, 76 },
{ 1, 77 },
{ 17, 77 },
{ 33, 77 },
{ 34, 77 },
{ 35, 77 },
{ 36, 77 },
{ 37, 77 },
{ 38, 77 },
{ 39, 77 },
{ 40, 77 },
{ 41, 77 },
{ 42, 77 },
{ 43, 77 },
{ 51, 77 },
{ 52, 77 },
{ 53, 77 },
{ 54, 77 },
{ 55, 77 },
{ 56, 77 },
{ 59, 77 },
{ 60, 77 },
{ 61, 77 },
{ 62, 77 },
{ 63, 77 },
{ 0, 78 },
{ 5, 78 },
{ 6, 78 },
{ 33, 78 },
{ 34, 78 },
{ 35, 78 },
{ 36, 78 },
{ 37, 78 },
{ 38, 78 },
{ 39, 78 },
{ 40, 78 },
{ 41, 78 },
{ 42, 78 },
{ 43, 78 },
{ 44, 78 },
{ 52, 78 },
{ 53, 78 },
{ 54, 78 },
{ 55, 78 },
{ 56, 78 },
{ 57, 78 },
{ 60, 78 },
{ 61, 78 },
{ 62, 78 },
{ 63, 78 },
{ 0, 79 },
{ 5, 79 },
{ 6, 79 },
{ 7, 79 },
{ 8, 79 },
{ 20, 79 },
{ 23, 79 },
{ 33, 79 },
{ 34, 79 },
{ 35, 79 },
{ 36, 79 },
{ 37, 79 },
{ 38, 79 },
{ 39, 79 },
{ 40, 79 },
{ 41, 79 },
{ 42, 79 },
{ 43, 79 },
{ 44, 79 },
{ 47, 79 },
{ 48, 79 },
{ 53, 79 },
{ 54, 79 },
{ 55, 79 },
{ 56, 79 },
{ 57, 79 },
{ 58, 79 },
{ 61, 79 },
{ 62, 79 },
{ 63, 79 },
{ 0, 80 },
{ 2, 80 },
{ 3, 80 },
{ 5, 80 },
{ 6, 80 },
{ 7, 80 },
{ 8, 80 },
{ 9, 80 },
{ 15, 80 },
{ 20, 80 },
{ 24, 80 },
{ 25, 80 },
{ 26, 80 },
{ 33, 80 },
{ 34, 80 },
{ 35, 80 },
{ 36, 80 },
{ 37, 80 },
{ 38, 80 },
{ 39, 80 },
{ 40, 80 },
{ 41, 80 },
{ 42, 80 },
{ 43, 80 },
{ 44, 80 },
{ 45, 80 },
{ 47, 80 },
{ 48, 80 },
{ 49, 80 },
{ 50, 80 },
{ 54, 80 },
{ 55, 80 },
{ 56, 80 },
{ 57, 80 },
{ 58, 80 },
{ 62, 80 },
{ 63, 80 },
{ 0, 81 },
{ 1, 81 },
{ 5, 81 },
{ 6, 81 },
{ 7, 81 },
{ 9, 81 },
{ 11, 81 },
{ 25, 81 },
{ 26, 81 },
{ 28, 81 },
{ 33, 81 },
{ 34, 81 },
{ 35, 81 },
{ 36, 81 },
{ 37, 81 },
{ 38, 81 },
{ 39, 81 },
{ 40, 81 },
{ 41, 81 },
{ 42, 81 },
{ 43, 81 },
{ 44, 81 },
{ 45, 81 },
{ 48, 81 },
{ 49, 81 },
{ 50, 81 },
{ 51, 81 },
{ 52, 81 },
{ 56, 81 },
{ 61, 81 },
{ 62, 81 },
{ 63, 81 },
{ 0, 82 },
{ 1, 82 },
{ 4, 82 },
{ 5, 82 },
{ 6, 82 },
{ 8, 82 },
{ 21, 82 },
{ 25, 82 },
{ 26, 82 },
{ 27, 82 },
{ 28, 82 },
{ 32, 82 },
{ 33, 82 },
{ 34, 82 },
{ 35, 82 },
{ 36, 82 },
{ 37, 82 },
{ 38, 82 },
{ 39, 82 },
{ 40, 82 },
{ 41, 82 },
{ 42, 82 },
{ 43, 82 },
{ 44, 82 },
{ 45, 82 },
{ 46, 82 },
{ 49, 82 },
{ 50, 82 },
{ 51, 82 },
{ 52, 82 },
{ 53, 82 },
{ 54, 82 },
{ 0, 83 },
{ 1, 83 },
{ 2, 83 },
{ 6, 83 },
{ 7, 83 },
{ 8, 83 },
{ 9, 83 },
{ 20, 83 },
{ 21, 83 },
{ 24, 83 },
{ 25, 83 },
{ 26, 83 },
{ 27, 83 },
{ 28, 83 },
{ 29, 83 },
{ 32, 83 },
{ 33, 83 },
{ 34, 83 },
{ 35, 83 },
{ 36, 83 },
{ 37, 83 },
{ 38, 83 },
{ 39, 83 },
{ 40, 83 },
{ 41, 83 },
{ 42, 83 },
{ 43, 83 },
{ 44, 83 },
{ 45, 83 },
{ 46, 83 },
{ 47, 83 },
{ 50, 83 },
{ 51, 83 },
{ 52, 83 },
{ 53, 83 },
{ 54, 83 },
{ 63, 83 },
{ 0, 84 },
{ 2, 84 },
{ 3, 84 },
{ 6, 84 },
{ 7, 84 },
{ 9, 84 },
{ 10, 84 },
{ 11, 84 },
{ 18, 84 },
{ 19, 84 },
{ 20, 84 },
{ 22, 84 },
{ 24, 84 },
{ 25, 84 },
{ 26, 84 },
{ 27, 84 },
{ 28, 84 },
{ 32, 84 },
{ 33, 84 },
{ 34, 84 },
{ 35, 84 },
{ 36, 84 },
{ 37, 84 },
{ 38, 84 },
{ 39, 84 },
{ 40, 84 },
{ 41, 84 },
{ 42, 84 },
{ 43, 84 },
{ 44, 84 },
{ 45, 84 },
{ 46, 84 },
{ 47, 84 },
{ 51, 84 },
{ 52, 84 },
{ 53, 84 },
{ 54, 84 },
{ 1, 85 },
{ 2, 85 },
{ 3, 85 },
{ 6, 85 },
{ 7, 85 },
{ 8, 85 },
{ 9, 85 },
{ 11, 85 },
{ 12, 85 },
{ 13, 85 },
{ 14, 85 },
{ 17, 85 },
{ 18, 85 },
{ 19, 85 },
{ 20, 85 },
{ 24, 85 },
{ 25, 85 },
{ 27, 85 },
{ 28, 85 },
{ 29, 85 },
{ 30, 85 },
{ 33, 85 },
{ 34, 85 },
{ 35, 85 },
{ 36, 85 },
{ 37, 85 },
{ 38, 85 },
{ 39, 85 },
{ 40, 85 },
{ 41, 85 },
{ 42, 85 },
{ 43, 85 },
{ 44, 85 },
{ 45, 85 },
{ 46, 85 },
{ 47, 85 },
{ 48, 85 },
{ 53, 85 },
{ 54, 85 },
{ 2, 86 },
{ 3, 86 },
{ 7, 86 },
{ 8, 86 },
{ 9, 86 },
{ 10, 86 },
{ 13, 86 },
{ 14, 86 },
{ 15, 86 },
{ 16, 86 },
{ 17, 86 },
{ 19, 86 },
{ 20, 86 },
{ 24, 86 },
{ 25, 86 },
{ 26, 86 },
{ 28, 86 },
{ 29, 86 },
{ 30, 86 },
{ 33, 86 },
{ 34, 86 },
{ 35, 86 },
{ 36, 86 },
{ 37, 86 },
{ 38, 86 },
{ 39, 86 },
{ 40, 86 },
{ 41, 86 },
{ 42, 86 },
{ 43, 86 },
{ 44, 86 },
{ 45, 86 },
{ 46, 86 },
{ 47, 86 },
{ 48, 86 },
{ 49, 86 },
{ 53, 86 },
{ 54, 86 },
{ 55, 86 },
{ 1, 87 },
{ 3, 87 },
{ 4, 87 },
{ 5, 87 },
{ 6, 87 },
{ 7, 87 },
{ 8, 87 },
{ 9, 87 },
{ 10, 87 },
{ 11, 87 },
{ 12, 87 },
{ 13, 87 },
{ 14, 87 },
{ 16, 87 },
{ 17, 87 },
{ 18, 87 },
{ 20, 87 },
{ 24, 87 },
{ 25, 87 },
{ 27, 87 },
{ 34, 87 },
{ 35, 87 },
{ 36, 87 },
{ 37, 87 },
{ 38, 87 },
{ 39, 87 },
{ 40, 87 },
{ 41, 87 },
{ 42, 87 },
{ 43, 87 },
{ 44, 87 },
{ 45, 87 },
{ 46, 87 },
{ 47, 87 },
{ 48, 87 },
{ 49, 87 },
{ 50, 87 },
{ 54, 87 },
{ 2, 88 },
{ 3, 88 },
{ 4, 88 },
{ 6, 88 },
{ 7, 88 },
{ 8, 88 },
{ 9, 88 },
{ 12, 88 },
{ 14, 88 },
{ 16, 88 },
{ 17, 88 },
{ 20, 88 },
{ 21, 88 },
{ 25, 88 },
{ 26, 88 },
{ 35, 88 },
{ 36, 88 },
{ 37, 88 },
{ 38, 88 },
{ 39, 88 },
{ 40, 88 },
{ 41, 88 },
{ 42, 88 },
{ 43, 88 },
{ 44, 88 },
{ 45, 88 },
{ 46, 88 },
{ 47, 88 },
{ 48, 88 },
{ 49, 88 },
{ 50, 88 },
{ 51, 88 },
{ 54, 88 },
{ 1, 89 },
{ 4, 89 },
{ 5, 89 },
{ 6, 89 },
{ 10, 89 },
{ 11, 89 },
{ 12, 89 },
{ 13, 89 },
{ 14, 89 },
{ 15, 89 },
{ 33, 89 },
{ 36, 89 },
{ 37, 89 },
{ 38, 89 },
{ 39, 89 },
{ 40, 89 },
{ 41, 89 },
{ 42, 89 },
{ 43, 89 },
{ 46, 89 },
{ 47, 89 },
{ 48, 89 },
{ 49, 89 },
{ 50, 89 },
{ 51, 89 },
{ 2, 90 },
{ 3, 90 },
{ 4, 90 },
{ 7, 90 },
{ 9, 90 },
{ 10, 90 },
{ 11, 90 },
{ 12, 90 },
{ 13, 90 },
{ 14, 90 },
{ 34, 90 },
{ 37, 90 },
{ 38, 90 },
{ 39, 90 },
{ 40, 90 },
{ 41, 90 },
{ 42, 90 },
{ 47, 90 },
{ 48, 90 },
{ 49, 90 },
{ 51, 90 },
{ 1, 91 },
{ 2, 91 },
{ 7, 91 },
{ 9, 91 },
{ 10, 91 },
{ 11, 91 },
{ 12, 91 },
{ 13, 91 },
{ 14, 91 },
{ 34, 91 },
{ 37, 91 },
{ 38, 91 },
{ 53, 91 },
{ 2, 92 },
{ 5, 92 },
{ 8, 92 },
{ 9, 92 },
{ 11, 92 },
{ 12, 92 },
{ 13, 92 },
{ 32, 92 },
{ 33, 92 },
{ 34, 92 },
{ 1, 93 },
{ 3, 93 },
{ 4, 93 },
{ 5, 93 },
{ 7, 93 },
{ 8, 93 },
{ 9, 93 },
{ 10, 93 },
{ 11, 93 },
{ 12, 93 },
{ 26, 93 },
{ 27, 93 },
{ 32, 93 },
{ 33, 93 },
{ 34, 93 },
{ 35, 93 },
{ 36, 93 },
{ 59, 93 },
{ 60, 93 },
{ 3, 94 },
{ 4, 94 },
{ 6, 94 },
{ 7, 94 },
{ 9, 94 },
{ 11, 94 },
{ 12, 94 },
{ 20, 94 },
{ 25, 94 },
{ 26, 94 },
{ 28, 94 },
{ 29, 94 },
{ 30, 94 },
{ 31, 94 },
{ 32, 94 },
{ 33, 94 },
{ 34, 94 },
{ 35, 94 },
{ 59, 94 },
{ 60, 94 },
{ 1, 95 },
{ 2, 95 },
{ 3, 95 },
{ 4, 95 },
{ 5, 95 },
{ 6, 95 },
{ 7, 95 },
{ 8, 95 },
{ 9, 95 },
{ 11, 95 },
{ 12, 95 },
{ 20, 95 },
{ 21, 95 },
{ 27, 95 },
{ 28, 95 },
{ 29, 95 },
{ 30, 95 },
{ 35, 95 },
{ 36, 95 },
{ 37, 95 },
{ 1, 96 },
{ 3, 96 },
{ 4, 96 },
{ 5, 96 },
{ 6, 96 },
{ 7, 96 },
{ 8, 96 },
{ 12, 96 },
{ 18, 96 },
{ 19, 96 },
{ 23, 96 },
{ 24, 96 },
{ 25, 96 },
{ 26, 96 },
{ 27, 96 },
{ 28, 96 },
{ 29, 96 },
{ 39, 96 },
{ 54, 96 },
{ 55, 96 },
{ 3, 97 },
{ 5, 97 },
{ 6, 97 },
{ 7, 97 },
{ 12, 97 },
{ 13, 97 },
{ 14, 97 },
{ 15, 97 },
{ 16, 97 },
{ 17, 97 },
{ 18, 97 },
{ 19, 97 },
{ 25, 97 },
{ 26, 97 },
{ 27, 97 },
{ 28, 97 },
{ 29, 97 },
{ 39, 97 },
{ 40, 97 },
{ 52, 97 },
{ 53, 97 },
{ 54, 97 },
{ 55, 97 },
{ 56, 97 },
{ 4, 98 },
{ 5, 98 },
{ 6, 98 },
{ 7, 98 },
{ 10, 98 },
{ 12, 98 },
{ 13, 98 },
{ 14, 98 },
{ 15, 98 },
{ 16, 98 },
{ 26, 98 },
{ 27, 98 },
{ 28, 98 },
{ 29, 98 },
{ 40, 98 },
{ 41, 98 },
{ 46, 98 },
{ 51, 98 },
{ 52, 98 },
{ 53, 98 },
{ 54, 98 },
{ 55, 98 },
{ 56, 98 },
{ 57, 98 },
{ 58, 98 },
{ 3, 99 },
{ 4, 99 },
{ 5, 99 },
{ 6, 99 },
{ 7, 99 },
{ 11, 99 },
{ 12, 99 },
{ 13, 99 },
{ 26, 99 },
{ 27, 99 },
{ 28, 99 },
{ 29, 99 },
{ 30, 99 },
{ 40, 99 },
{ 41, 99 },
{ 42, 99 },
{ 43, 99 },
{ 44, 99 },
{ 45, 99 },
{ 49, 99 },
{ 50, 99 },
{ 51, 99 },
{ 52, 99 },
{ 53, 99 },
{ 54, 99 },
{ 55, 99 },
{ 56, 99 },
{ 57, 99 },
{ 58, 99 },
{ 63, 99 },
{ 4, 100 },
{ 5, 100 },
{ 6, 100 },
{ 27, 100 },
{ 29, 100 },
{ 30, 100 },
{ 31, 100 },
{ 32, 100 },
{ 40, 100 },
{ 41, 100 },
{ 42, 100 },
{ 43, 100 },
{ 44, 100 },
{ 55, 100 },
{ 56, 100 },
{ 2, 101 },
{ 3, 101 },
{ 4, 101 },
{ 5, 101 },
{ 6, 101 },
{ 31, 101 },
{ 32, 101 },
{ 33, 101 },
{ 43, 101 },
{ 44, 101 },
{ 5, 102 },
{ 6, 102 },
{ 23, 102 },
{ 62, 102 },
{ 63, 102 },
{ 3, 103 },
{ 4, 103 },
{ 23, 103 },
{ 24, 103 },
{ 54, 103 },
{ 61, 103 },
{ 62, 103 },
{ 63, 103 },
{ 3, 104 },
{ 4, 104 },
{ 6, 104 },
{ 7, 104 },
{ 23, 104 },
{ 24, 104 },
{ 25, 104 },
{ 26, 104 },
{ 59, 104 },
{ 61, 104 },
{ 62, 104 },
{ 63, 104 },
{ 2, 105 },
{ 3, 105 },
{ 14, 105 },
{ 23, 105 },
{ 24, 105 },
{ 25, 105 },
{ 26, 105 },
{ 27, 105 },
{ 37, 105 },
{ 49, 105 },
{ 62, 105 },
{ 12, 106 },
{ 13, 106 },
{ 22, 106 },
{ 23, 106 },
{ 24, 106 },
{ 25, 106 },
{ 26, 106 },
{ 27, 106 },
{ 36, 106 },
{ 37, 106 },
{ 48, 106 },
{ 49, 106 },
{ 54, 106 },
{ 10, 107 },
{ 12, 107 },
{ 23, 107 },
{ 24, 107 },
{ 25, 107 },
{ 26, 107 },
{ 27, 107 },
{ 36, 107 },
{ 37, 107 },
{ 38, 107 },
{ 47, 107 },
{ 48, 107 },
{ 49, 107 },
{ 50, 107 },
{ 54, 107 },
{ 8, 108 },
{ 26, 108 },
{ 27, 108 },
{ 35, 108 },
{ 36, 108 },
{ 37, 108 },
{ 38, 108 },
{ 39, 108 },
{ 40, 108 },
{ 47, 108 },
{ 48, 108 },
{ 49, 108 },
{ 50, 108 },
{ 53, 108 },
{ 54, 108 },
{ 61, 108 },
{ 6, 109 },
{ 7, 109 },
{ 8, 109 },
{ 18, 109 },
{ 19, 109 },
{ 20, 109 },
{ 27, 109 },
{ 34, 109 },
{ 35, 109 },
{ 36, 109 },
{ 46, 109 },
{ 47, 109 },
{ 48, 109 },
{ 49, 109 },
{ 50, 109 },
{ 52, 109 },
{ 53, 109 },
{ 54, 109 },
{ 61, 109 },
{ 62, 109 },
{ 4, 110 },
{ 5, 110 },
{ 6, 110 },
{ 7, 110 },
{ 18, 110 },
{ 19, 110 },
{ 20, 110 },
{ 32, 110 },
{ 33, 110 },
{ 34, 110 },
{ 35, 110 },
{ 45, 110 },
{ 46, 110 },
{ 47, 110 },
{ 48, 110 },
{ 51, 110 },
{ 52, 110 },
{ 53, 110 },
{ 54, 110 },
{ 55, 110 },
{ 61, 110 },
{ 62, 110 },
{ 63, 110 },
{ 2, 111 },
{ 3, 111 },
{ 4, 111 },
{ 5, 111 },
{ 6, 111 },
{ 7, 111 },
{ 18, 111 },
{ 19, 111 },
{ 20, 111 },
{ 30, 111 },
{ 31, 111 },
{ 32, 111 },
{ 33, 111 },
{ 34, 111 },
{ 44, 111 },
{ 53, 111 },
{ 54, 111 },
{ 55, 111 },
{ 0, 112 },
{ 1, 112 },
{ 2, 112 },
{ 3, 112 },
{ 4, 112 },
{ 5, 112 },
{ 6, 112 },
{ 13, 112 },
{ 19, 112 },
{ 30, 112 },
{ 31, 112 },
{ 32, 112 },
{ 33, 112 },
{ 34, 112 },
{ 45, 112 },
{ 53, 112 },
{ 54, 112 },
{ 55, 112 },
{ 0, 113 },
{ 1, 113 },
{ 2, 113 },
{ 3, 113 },
{ 4, 113 },
{ 5, 113 },
{ 6, 113 },
{ 7, 113 },
{ 8, 113 },
{ 9, 113 },
{ 11, 113 },
{ 12, 113 },
{ 22, 113 },
{ 31, 113 },
{ 32, 113 },
{ 33, 113 },
{ 43, 113 },
{ 44, 113 },
{ 53, 113 },
{ 54, 113 },
{ 55, 113 },
{ 56, 113 },
{ 0, 114 },
{ 1, 114 },
{ 2, 114 },
{ 3, 114 },
{ 4, 114 },
{ 5, 114 },
{ 6, 114 },
{ 7, 114 },
{ 8, 114 },
{ 10, 114 },
{ 11, 114 },
{ 12, 114 },
{ 13, 114 },
{ 21, 114 },
{ 22, 114 },
{ 23, 114 },
{ 33, 114 },
{ 52, 114 },
{ 53, 114 },
{ 54, 114 },
{ 0, 115 },
{ 1, 115 },
{ 2, 115 },
{ 3, 115 },
{ 4, 115 },
{ 5, 115 },
{ 6, 115 },
{ 7, 115 },
{ 8, 115 },
{ 9, 115 },
{ 10, 115 },
{ 21, 115 },
{ 22, 115 },
{ 23, 115 },
{ 24, 115 },
{ 25, 115 },
{ 26, 115 },
{ 53, 115 },
{ 54, 115 },
{ 0, 116 },
{ 1, 116 },
{ 2, 116 },
{ 3, 116 },
{ 4, 116 },
{ 5, 116 },
{ 6, 116 },
{ 7, 116 },
{ 8, 116 },
{ 22, 116 },
{ 23, 116 },
{ 24, 116 },
{ 25, 116 },
{ 26, 116 },
{ 53, 116 },
{ 0, 117 },
{ 1, 117 },
{ 2, 117 },
{ 3, 117 },
{ 4, 117 },
{ 19, 117 },
{ 22, 117 },
{ 23, 117 },
{ 24, 117 },
{ 25, 117 },
{ 53, 117 },
{ 0, 118 },
{ 1, 118 },
{ 2, 118 },
{ 3, 118 },
{ 8, 118 },
{ 18, 118 },
{ 19, 118 },
{ 22, 118 },
{ 23, 118 },
{ 24, 118 },
{ 25, 118 },
{ 35, 118 },
{ 0, 119 },
{ 1, 119 },
{ 2, 119 },
{ 16, 119 },
{ 17, 119 },
{ 18, 119 },
{ 19, 119 },
{ 21, 119 },
{ 22, 119 },
{ 23, 119 },
{ 24, 119 },
{ 25, 119 },
{ 0, 120 },
{ 1, 120 },
{ 2, 120 },
{ 16, 120 },
{ 17, 120 },
{ 18, 120 },
{ 19, 120 },
{ 22, 120 },
{ 23, 120 },
{ 24, 120 },
{ 25, 120 },
{ 26, 120 },
{ 27, 120 },
{ 28, 120 },
{ 32, 120 },
{ 0, 121 },
{ 1, 121 },
{ 14, 121 },
{ 15, 121 },
{ 16, 121 },
{ 17, 121 },
{ 18, 121 },
{ 19, 121 },
{ 22, 121 },
{ 23, 121 },
{ 24, 121 },
{ 25, 121 },
{ 26, 121 },
{ 27, 121 },
{ 28, 121 },
{ 29, 121 },
{ 33, 121 },
{ 0, 122 },
{ 1, 122 },
{ 15, 122 },
{ 16, 122 },
{ 17, 122 },
{ 18, 122 },
{ 19, 122 },
{ 20, 122 },
{ 22, 122 },
{ 23, 122 },
{ 24, 122 },
{ 25, 122 },
{ 26, 122 },
{ 27, 122 },
{ 28, 122 },
{ 29, 122 },
{ 32, 122 },
{ 33, 122 },
{ 0, 123 },
{ 1, 123 },
{ 8, 123 },
{ 9, 123 },
{ 16, 123 },
{ 17, 123 },
{ 18, 123 },
{ 19, 123 },
{ 21, 123 },
{ 23, 123 },
{ 24, 123 },
{ 25, 123 },
{ 26, 123 },
{ 27, 123 },
{ 28, 123 },
{ 29, 123 },
{ 30, 123 },
{ 33, 123 },
{ 0, 124 },
{ 6, 124 },
{ 7, 124 },
{ 8, 124 },
{ 9, 124 },
{ 18, 124 },
{ 19, 124 },
{ 21, 124 },
{ 24, 124 },
{ 25, 124 },
{ 26, 124 },
{ 27, 124 },
{ 28, 124 },
{ 33, 124 },
{ 43, 124 },
{ 44, 124 },
{ 45, 124 },
{ 0, 125 },
{ 1, 125 },
{ 5, 125 },
{ 6, 125 },
{ 7, 125 },
{ 8, 125 },
{ 24, 125 },
{ 25, 125 },
{ 26, 125 },
{ 27, 125 },
{ 28, 125 },
{ 43, 125 },
{ 44, 125 },
{ 0, 126 },
{ 1, 126 },
{ 2, 126 },
{ 4, 126 },
{ 5, 126 },
{ 24, 126 },
{ 25, 126 },
{ 26, 126 },
{ 27, 126 },
{ 0, 127 },
{ 1, 127 },
{ 2, 127 },
{ 3, 127 },
{ 24, 127 },
{ 25, 127 },
{ 26, 127 },
{ 0, 128 },
{ 1, 128 },
{ 2, 128 },
{ 25, 128 },
{ 26, 128 },
{ 38, 128 },
{ 0, 129 },
{ 1, 129 },
{ 25, 129 },
{ 26, 129 },
{ 37, 129 },
{ 38, 129 },
{ 41, 129 },
{ 42, 129 },
{ 53, 129 },
{ 0, 130 },
{ 1, 130 },
{ 24, 130 },
{ 25, 130 },
{ 39, 130 },
{ 40, 130 },
{ 41, 130 },
{ 42, 130 },
{ 50, 130 },
{ 52, 130 },
{ 53, 130 },
{ 0, 131 },
{ 16, 131 },
{ 24, 131 },
{ 25, 131 },
{ 27, 131 },
{ 42, 131 },
{ 53, 131 },
{ 0, 132 },
{ 9, 132 },
{ 10, 132 },
{ 15, 132 },
{ 17, 132 },
{ 27, 132 },
{ 60, 132 },
{ 61, 132 },
{ 62, 132 },
{ 0, 133 },
{ 8, 133 },
{ 9, 133 },
{ 10, 133 },
{ 11, 133 },
{ 15, 133 },
{ 17, 133 },
{ 18, 133 },
{ 28, 133 },
{ 29, 133 },
{ 33, 133 },
{ 34, 133 },
{ 44, 133 },
{ 52, 133 },
{ 0, 134 },
{ 8, 134 },
{ 10, 134 },
{ 32, 134 },
{ 33, 134 },
{ 34, 134 },
{ 44, 134 },
{ 52, 134 },
{ 0, 135 },
{ 7, 135 },
{ 8, 135 },
{ 9, 135 },
{ 20, 135 },
{ 29, 135 },
{ 30, 135 },
{ 31, 135 },
{ 32, 135 },
{ 33, 135 },
{ 34, 135 },
{ 52, 135 },
{ 59, 135 },
{ 0, 136 },
{ 1, 136 },
{ 6, 136 },
{ 7, 136 },
{ 8, 136 },
{ 9, 136 },
{ 10, 136 },
{ 30, 136 },
{ 31, 136 },
{ 34, 136 },
{ 35, 136 },
{ 36, 136 },
{ 51, 136 },
{ 52, 136 },
{ 61, 136 },
{ 62, 136 },
{ 0, 137 },
{ 1, 137 },
{ 2, 137 },
{ 3, 137 },
{ 7, 137 },
{ 8, 137 },
{ 9, 137 },
{ 10, 137 },
{ 35, 137 },
{ 36, 137 },
{ 37, 137 },
{ 38, 137 },
{ 41, 137 },
{ 50, 137 },
{ 51, 137 },
{ 61, 137 },
{ 62, 137 },
{ 63, 137 },
{ 0, 138 },
{ 1, 138 },
{ 2, 138 },
{ 8, 138 },
{ 9, 138 },
{ 10, 138 },
{ 20, 138 },
{ 29, 138 },
{ 30, 138 },
{ 36, 138 },
{ 37, 138 },
{ 38, 138 },
{ 39, 138 },
{ 40, 138 },
{ 41, 138 },
{ 42, 138 },
{ 43, 138 },
{ 48, 138 },
{ 49, 138 },
{ 50, 138 },
{ 51, 138 },
{ 60, 138 },
{ 61, 138 },
{ 62, 138 },
{ 63, 138 },
{ 0, 139 },
{ 1, 139 },
{ 2, 139 },
{ 3, 139 },
{ 9, 139 },
{ 10, 139 },
{ 27, 139 },
{ 28, 139 },
{ 29, 139 },
{ 30, 139 },
{ 31, 139 },
{ 36, 139 },
{ 37, 139 },
{ 38, 139 },
{ 39, 139 },
{ 40, 139 },
{ 41, 139 },
{ 42, 139 },
{ 43, 139 },
{ 44, 139 },
{ 45, 139 },
{ 46, 139 },
{ 47, 139 },
{ 48, 139 },
{ 49, 139 },
{ 50, 139 },
{ 51, 139 },
{ 60, 139 },
{ 61, 139 },
{ 62, 139 },
{ 63, 139 },
{ 0, 140 },
{ 1, 140 },
{ 2, 140 },
{ 3, 140 },
{ 9, 140 },
{ 10, 140 },
{ 11, 140 },
{ 27, 140 },
{ 28, 140 },
{ 29, 140 },
{ 30, 140 },
{ 31, 140 },
{ 32, 140 },
{ 36, 140 },
{ 37, 140 },
{ 38, 140 },
{ 39, 140 },
{ 40, 140 },
{ 41, 140 },
{ 42, 140 },
{ 43, 140 },
{ 44, 140 },
{ 45, 140 },
{ 46, 140 },
{ 47, 140 },
{ 48, 140 },
{ 49, 140 },
{ 50, 140 },
{ 51, 140 },
{ 52, 140 },
{ 60, 140 },
{ 61, 140 },
{ 62, 140 },
{ 63, 140 },
{ 0, 141 },
{ 1, 141 },
{ 2, 141 },
{ 3, 141 },
{ 9, 141 },
{ 10, 141 },
{ 11, 141 },
{ 12, 141 },
{ 16, 141 },
{ 17, 141 },
{ 26, 141 },
{ 27, 141 },
{ 28, 141 },
{ 29, 141 },
{ 30, 141 },
{ 31, 141 },
{ 32, 141 },
{ 33, 141 },
{ 35, 141 },
{ 36, 141 },
{ 37, 141 },
{ 38, 141 },
{ 39, 141 },
{ 40, 141 },
{ 41, 141 },
{ 42, 141 },
{ 43, 141 },
{ 44, 141 },
{ 45, 141 },
{ 46, 141 },
{ 47, 141 },
{ 48, 141 },
{ 49, 141 },
{ 50, 141 },
{ 51, 141 },
{ 60, 141 },
{ 61, 141 },
{ 62, 141 },
{ 63, 141 },
{ 0, 142 },
{ 1, 142 },
{ 2, 142 },
{ 3, 142 },
{ 4, 142 },
{ 9, 142 },
{ 10, 142 },
{ 11, 142 },
{ 12, 142 },
{ 13, 142 },
{ 14, 142 },
{ 15, 142 },
{ 16, 142 },
{ 17, 142 },
{ 18, 142 },
{ 19, 142 },
{ 25, 142 },
{ 26, 142 },
{ 27, 142 },
{ 28, 142 },
{ 29, 142 },
{ 30, 142 },
{ 31, 142 },
{ 32, 142 },
{ 33, 142 },
{ 34, 142 },
{ 35, 142 },
{ 36, 142 },
{ 37, 142 },
{ 38, 142 },
{ 39, 142 },
{ 40, 142 },
{ 41, 142 },
{ 42, 142 },
{ 43, 142 },
{ 44, 142 },
{ 45, 142 },
{ 46, 142 },
{ 47, 142 },
{ 48, 142 },
{ 49, 142 },
{ 50, 142 },
{ 51, 142 },
{ 52, 142 },
{ 53, 142 },
{ 61, 142 },
{ 62, 142 },
{ 63, 142 },
{ 0, 143 },
{ 1, 143 },
{ 2, 143 },
{ 3, 143 },
{ 4, 143 },
{ 9, 143 },
{ 10, 143 },
{ 11, 143 },
{ 12, 143 },
{ 13, 143 },
{ 14, 143 },
{ 15, 143 },
{ 16, 143 },
{ 17, 143 },
{ 18, 143 },
{ 25, 143 },
{ 26, 143 },
{ 27, 143 },
{ 28, 143 },
{ 29, 143 },
{ 30, 143 },
{ 31, 143 },
{ 32, 143 },
{ 33, 143 },
{ 34, 143 },
{ 35, 143 },
{ 36, 143 },
{ 37, 143 },
{ 38, 143 },
{ 39, 143 },
{ 40, 143 },
{ 41, 143 },
{ 42, 143 },
{ 43, 143 },
{ 44, 143 },
{ 45, 143 },
{ 46, 143 },
{ 47, 143 },
{ 48, 143 },
{ 49, 143 },
{ 50, 143 },
{ 51, 143 },
{ 52, 143 },
{ 53, 143 },
{ 61, 143 },
{ 62, 143 },
{ 63, 143 },
{ 0, 144 },
{ 1, 144 },
{ 2, 144 },
{ 3, 144 },
{ 4, 144 },
{ 5, 144 },
{ 8, 144 },
{ 9, 144 },
{ 10, 144 },
{ 11, 144 },
{ 12, 144 },
{ 13, 144 },
{ 14, 144 },
{ 15, 144 },
{ 16, 144 },
{ 17, 144 },
{ 26, 144 },
{ 27, 144 },
{ 28, 144 },
{ 29, 144 },
{ 30, 144 },
{ 31, 144 },
{ 32, 144 },
{ 33, 144 },
{ 34, 144 },
{ 35, 144 },
{ 36, 144 },
{ 37, 144 },
{ 38, 144 },
{ 39, 144 },
{ 40, 144 },
{ 41, 144 },
{ 42, 144 },
{ 43, 144 },
{ 44, 144 },
{ 45, 144 },
{ 46, 144 },
{ 47, 144 },
{ 48, 144 },
{ 49, 144 },
{ 50, 144 },
{ 51, 144 },
{ 52, 144 },
{ 53, 144 },
{ 54, 144 },
{ 60, 144 },
{ 61, 144 },
{ 62, 144 },
{ 63, 144 },
{ 2, 145 },
{ 3, 145 },
{ 4, 145 },
{ 5, 145 },
{ 6, 145 },
{ 7, 145 },
{ 8, 145 },
{ 9, 145 },
{ 10, 145 },
{ 11, 145 },
{ 12, 145 },
{ 13, 145 },
{ 14, 145 },
{ 15, 145 },
{ 16, 145 },
{ 17, 145 },
{ 18, 145 },
{ 28, 145 },
{ 29, 145 },
{ 30, 145 },
{ 31, 145 },
{ 32, 145 },
{ 33, 145 },
{ 34, 145 },
{ 35, 145 },
{ 36, 145 },
{ 37, 145 },
{ 38, 145 },
{ 39, 145 },
{ 40, 145 },
{ 41, 145 },
{ 42, 145 },
{ 43, 145 },
{ 44, 145 },
{ 45, 145 },
{ 46, 145 },
{ 47, 145 },
{ 48, 145 },
{ 49, 145 },
{ 50, 145 },
{ 51, 145 },
{ 52, 145 },
{ 53, 145 },
{ 54, 145 },
{ 55, 145 },
{ 56, 145 },
{ 57, 145 },
{ 58, 145 },
{ 59, 145 },
{ 60, 145 },
{ 61, 145 },
{ 4, 146 },
{ 5, 146 },
{ 6, 146 },
{ 7, 146 },
{ 8, 146 },
{ 9, 146 },
{ 10, 146 },
{ 11, 146 },
{ 12, 146 },
{ 13, 146 },
{ 14, 146 },
{ 15, 146 },
{ 16, 146 },
{ 17, 146 },
{ 18, 146 },
{ 28, 146 },
{ 29, 146 },
{ 30, 146 },
{ 31, 146 },
{ 32, 146 },
{ 33, 146 },
{ 34, 146 },
{ 35, 146 },
{ 36, 146 },
{ 37, 146 },
{ 38, 146 },
{ 39, 146 },
{ 40, 146 },
{ 41, 146 },
{ 42, 146 },
{ 43, 146 },
{ 44, 146 },
{ 45, 146 },
{ 46, 146 },
{ 47, 146 },
{ 48, 146 },
{ 49, 146 },
{ 50, 146 },
{ 51, 146 },
{ 52, 146 },
{ 53, 146 },
{ 54, 146 },
{ 55, 146 },
{ 56, 146 },
{ 57, 146 },
{ 58, 146 },
{ 59, 146 },
{ 6, 147 },
{ 7, 147 },
{ 8, 147 },
{ 9, 147 },
{ 10, 147 },
{ 11, 147 },
{ 12, 147 },
{ 13, 147 },
{ 14, 147 },
{ 15, 147 },
{ 16, 147 },
{ 17, 147 },
{ 18, 147 },
{ 19, 147 },
{ 29, 147 },
{ 30, 147 },
{ 31, 147 },
{ 32, 147 },
{ 33, 147 },
{ 34, 147 },
{ 35, 147 },
{ 36, 147 },
{ 37, 147 },
{ 38, 147 },
{ 39, 147 },
{ 40, 147 },
{ 41, 147 },
{ 42, 147 },
{ 43, 147 },
{ 44, 147 },
{ 45, 147 },
{ 46, 147 },
{ 47, 147 },
{ 48, 147 },
{ 49, 147 },
{ 50, 147 },
{ 51, 147 },
{ 52, 147 },
{ 53, 147 },
{ 54, 147 },
{ 55, 147 },
{ 56, 147 },
{ 57, 147 },
{ 8, 148 },
{ 9, 148 },
{ 10, 148 },
{ 11, 148 },
{ 12, 148 },
{ 13, 148 },
{ 14, 148 },
{ 15, 148 },
{ 16, 148 },
{ 17, 148 },
{ 18, 148 },
{ 19, 148 },
{ 29, 148 },
{ 30, 148 },
{ 31, 148 },
{ 32, 148 },
{ 33, 148 },
{ 34, 148 },
{ 35, 148 },
{ 36, 148 },
{ 37, 148 },
{ 38, 148 },
{ 39, 148 },
{ 40, 148 },
{ 41, 148 },
{ 42, 148 },
{ 43, 148 },
{ 44, 148 },
{ 45, 148 },
{ 46, 148 },
{ 47, 148 },
{ 48, 148 },
{ 49, 148 },
{ 50, 148 },
{ 51, 148 },
{ 52, 148 },
{ 53, 148 },
{ 54, 148 },
{ 55, 148 },
{ 10, 149 },
{ 11, 149 },
{ 12, 149 },
{ 13, 149 },
{ 14, 149 },
{ 15, 149 },
{ 16, 149 },
{ 17, 149 },
{ 18, 149 },
{ 19, 149 },
{ 20, 149 },
{ 21, 149 },
{ 29, 149 },
{ 30, 149 },
{ 31, 149 },
{ 32, 149 },
{ 33, 149 },
{ 34, 149 },
{ 35, 149 },
{ 36, 149 },
{ 37, 149 },
{ 38, 149 },
{ 39, 149 },
{ 40, 149 },
{ 41, 149 },
{ 42, 149 },
{ 43, 149 },
{ 44, 149 },
{ 45, 149 },
{ 46, 149 },
{ 47, 149 },
{ 48, 149 },
{ 49, 149 },
{ 50, 149 },
{ 51, 149 },
{ 52, 149 },
{ 53, 149 },
{ 12, 150 },
{ 13, 150 },
{ 14, 150 },
{ 15, 150 },
{ 16, 150 },
{ 17, 150 },
{ 18, 150 },
{ 19, 150 },
{ 20, 150 },
{ 21, 150 },
{ 22, 150 },
{ 23, 150 },
{ 28, 150 },
{ 29, 150 },
{ 30, 150 },
{ 31, 150 },
{ 32, 150 },
{ 33, 150 },
{ 34, 150 },
{ 35, 150 },
{ 36, 150 },
{ 37, 150 },
{ 38, 150 },
{ 39, 150 },
{ 40, 150 },
{ 41, 150 },
{ 42, 150 },
{ 43, 150 },
{ 44, 150 },
{ 45, 150 },
{ 46, 150 },
{ 47, 150 },
{ 48, 150 },
{ 49, 150 },
{ 50, 150 },
{ 51, 150 },
{ 14, 151 },
{ 15, 151 },
{ 16, 151 },
{ 17, 151 },
{ 18, 151 },
{ 19, 151 },
{ 20, 151 },
{ 21, 151 },
{ 22, 151 },
{ 23, 151 },
{ 24, 151 },
{ 25, 151 },
{ 26, 151 },
{ 27, 151 },
{ 28, 151 },
{ 29, 151 },
{ 30, 151 },
{ 31, 151 },
{ 32, 151 },
{ 33, 151 },
{ 34, 151 },
{ 35, 151 },
{ 36, 151 },
{ 37, 151 },
{ 38, 151 },
{ 39, 151 },
{ 40, 151 },
{ 41, 151 },
{ 42, 151 },
{ 43, 151 },
{ 44, 151 },
{ 45, 151 },
{ 46, 151 },
{ 47, 151 },
{ 48, 151 },
{ 49, 151 },
{ 16, 152 },
{ 17, 152 },
{ 18, 152 },
{ 19, 152 },
{ 20, 152 },
{ 21, 152 },
{ 22, 152 },
{ 23, 152 },
{ 24, 152 },
{ 25, 152 },
{ 26, 152 },
{ 27, 152 },
{ 28, 152 },
{ 29, 152 },
{ 30, 152 },
{ 31, 152 },
{ 32, 152 },
{ 33, 152 },
{ 34, 152 },
{ 35, 152 },
{ 36, 152 },
{ 37, 152 },
{ 38, 152 },
{ 39, 152 },
{ 40, 152 },
{ 41, 152 },
{ 42, 152 },
{ 43, 152 },
{ 44, 152 },
{ 45, 152 },
{ 46, 152 },
{ 47, 152 },
{ 18, 153 },
{ 19, 153 },
{ 20, 153 },
{ 21, 153 },
{ 22, 153 },
{ 23, 153 },
{ 24, 153 },
{ 25, 153 },
{ 26, 153 },
{ 27, 153 },
{ 28, 153 },
{ 29, 153 },
{ 30, 153 },
{ 31, 153 },
{ 32, 153 },
{ 33, 153 },
{ 34, 153 },
{ 35, 153 },
{ 36, 153 },
{ 37, 153 },
{ 38, 153 },
{ 39, 153 },
{ 40, 153 },
{ 41, 153 },
{ 42, 153 },
{ 43, 153 },
{ 44, 153 },
{ 45, 153 },
{ 20, 154 },
{ 21, 154 },
{ 22, 154 },
{ 23, 154 },
{ 24, 154 },
{ 25, 154 },
{ 26, 154 },
{ 27, 154 },
{ 28, 154 },
{ 29, 154 },
{ 30, 154 },
{ 31, 154 },
{ 32, 154 },
{ 33, 154 },
{ 34, 154 },
{ 35, 154 },
{ 36, 154 },
{ 37, 154 },
{ 38, 154 },
{ 39, 154 },
{ 40, 154 },
{ 41, 154 },
{ 42, 154 },
{ 43, 154 },
{ 22, 155 },
{ 23, 155 },
{ 24, 155 },
{ 25, 155 },
{ 26, 155 },
{ 27, 155 },
{ 28, 155 },
{ 29, 155 },
{ 30, 155 },
{ 31, 155 },
{ 32, 155 },
{ 33, 155 },
{ 34, 155 },
{ 35, 155 },
{ 36, 155 },
{ 37, 155 },
{ 38, 155 },
{ 39, 155 },
{ 40, 155 },
{ 41, 155 },
{ 24, 156 },
{ 25, 156 },
{ 26, 156 },
{ 27, 156 },
{ 28, 156 },
{ 29, 156 },
{ 30, 156 },
{ 31, 156 },
{ 32, 156 },
{ 33, 156 },
{ 34, 156 },
{ 35, 156 },
{ 36, 156 },
{ 37, 156 },
{ 38, 156 },
{ 39, 156 },
{ 26, 157 },
{ 27, 157 },
{ 28, 157 },
{ 29, 157 },
{ 30, 157 },
{ 31, 157 },
{ 32, 157 },
{ 33, 157 },
{ 34, 157 },
{ 35, 157 },
{ 36, 157 },
{ 37, 157 },
{ 28, 158 },
{ 29, 158 },
{ 30, 158 },
{ 31, 158 },
{ 32, 158 },
{ 33, 158 },
{ 34, 158 },
{ 35, 158 },
{ 30, 159 },
{ 31, 159 },
{ 32, 159 },
{ 33, 159 },
};

static const POScol cols20[] =
{
{ 39, 72, 121, 0 },
{ 40, 73, 121, 0 },
{ 41, 73, 121, 0 },
{ 41, 74, 121, 0 },
{ 42, 74, 121, 0 },
{ 42, 75, 121, 0 },
{ 43, 76, 121, 0 },
{ 46, 78, 121, 0 },
{ 47, 78, 97, 0 },
{ 45, 79, 121, 0 },
{ 48, 79, 97, 0 },
{ 52, 80, 121, 0 },
{ 46, 81, 121, 0 },
{ 53, 81, 121, 0 },
{ 47, 82, 121, 0 },
{ 29, 118, 47, 0 },
{ 29, 119, 47, 0 },
{ 30, 121, 30, 0 },
{ 35, 138, 60, 0 },
{ 35, 139, 89, 0 },
{ 35, 140, 89, 0 },
{ 31, 85, 112, 0 },
{ 32, 86, 117, 0 },
};

const POS trans21[] = {
{ 30, 96 },
{ 31, 96 },
{ 29, 97 },
{ 30, 97 },
{ 31, 97 },
{ 28, 98 },
{ 29, 98 },
{ 27, 99 },
{ 28, 99 },
{ 29, 99 },
{ 26, 100 },
{ 27, 100 },
{ 28, 100 },
{ 25, 101 },
{ 26, 101 },
{ 27, 101 },
{ 24, 102 },
{ 23, 103 },
{ 22, 104 },
{ 29, 104 },
{ 30, 104 },
{ 21, 105 },
{ 20, 106 },
{ 27, 106 },
{ 26, 107 },
{ 22, 108 },
{ 23, 108 },
{ 25, 108 },
{ 26, 108 },
{ 22, 109 },
{ 23, 109 },
{ 24, 109 },
{ 21, 110 },
{ 22, 110 },
{ 23, 110 },
{ 22, 111 },
{ 21, 112 },
{ 21, 113 },
{ 20, 114 },
{ 18, 115 },
{ 19, 115 },
{ 20, 115 },
{ 27, 115 },
{ 17, 116 },
{ 18, 116 },
{ 19, 116 },
{ 26, 116 },
{ 17, 117 },
{ 18, 117 },
{ 18, 118 },
{ 25, 118 },
{ 17, 119 },
{ 24, 119 },
{ 17, 120 },
{ 16, 121 },
{ 16, 122 },
{ 62, 83 },
{ 63, 83 },
{ 60, 84 },
{ 61, 84 },
{ 62, 84 },
{ 63, 84 },
{ 58, 85 },
{ 59, 85 },
{ 60, 85 },
{ 61, 85 },
{ 62, 85 },
{ 63, 85 },
{ 56, 86 },
{ 57, 86 },
{ 58, 86 },
{ 59, 86 },
{ 60, 86 },
{ 61, 86 },
{ 62, 86 },
{ 54, 87 },
{ 55, 87 },
{ 56, 87 },
{ 57, 87 },
{ 58, 87 },
{ 59, 87 },
{ 55, 88 },
{ 56, 88 },
{ 57, 88 },
{ 58, 88 },
{ 58, 89 },
{ 59, 89 },
{ 59, 90 },
{ 59, 91 },
{ 44, 92 },
{ 40, 94 },
{ 41, 94 },
{ 49, 94 },
{ 47, 96 },
{ 53, 96 },
{ 54, 96 },
{ 55, 96 },
{ 57, 96 },
{ 59, 96 },
{ 34, 97 },
{ 35, 97 },
{ 46, 97 },
{ 54, 97 },
{ 56, 97 },
{ 57, 97 },
{ 58, 97 },
{ 30, 98 },
{ 32, 98 },
{ 33, 98 },
{ 38, 98 },
{ 45, 98 },
{ 30, 99 },
{ 41, 99 },
{ 42, 99 },
{ 45, 99 },
{ 55, 99 },
{ 56, 99 },
{ 59, 99 },
{ 63, 99 },
{ 35, 100 },
{ 41, 100 },
{ 42, 100 },
{ 44, 100 },
{ 45, 100 },
{ 59, 100 },
{ 60, 100 },
{ 61, 100 },
{ 62, 100 },
{ 32, 101 },
{ 33, 101 },
{ 34, 101 },
{ 35, 101 },
{ 36, 101 },
{ 37, 101 },
{ 41, 101 },
{ 44, 101 },
{ 45, 101 },
{ 59, 101 },
{ 60, 101 },
{ 61, 101 },
{ 31, 102 },
{ 32, 102 },
{ 33, 102 },
{ 34, 102 },
{ 35, 102 },
{ 61, 102 },
{ 62, 102 },
{ 30, 103 },
{ 32, 103 },
{ 33, 103 },
{ 34, 103 },
{ 35, 103 },
{ 61, 103 },
{ 34, 104 },
{ 29, 105 },
{ 63, 105 },
{ 63, 106 },
{ 35, 107 },
{ 36, 107 },
{ 63, 107 },
{ 31, 108 },
{ 32, 108 },
{ 34, 108 },
{ 35, 108 },
{ 44, 108 },
{ 45, 108 },
{ 62, 108 },
{ 17, 109 },
{ 18, 109 },
{ 30, 109 },
{ 31, 109 },
{ 32, 109 },
{ 33, 109 },
{ 34, 109 },
{ 45, 109 },
{ 18, 110 },
{ 30, 110 },
{ 31, 110 },
{ 32, 110 },
{ 39, 110 },
{ 44, 110 },
{ 29, 111 },
{ 30, 111 },
{ 31, 111 },
{ 42, 111 },
{ 44, 111 },
{ 61, 111 },
{ 29, 112 },
{ 36, 112 },
{ 61, 112 },
{ 28, 113 },
{ 29, 113 },
{ 36, 113 },
{ 38, 113 },
{ 59, 113 },
{ 60, 113 },
{ 12, 114 },
{ 27, 114 },
{ 28, 114 },
{ 36, 114 },
{ 37, 114 },
{ 38, 114 },
{ 40, 114 },
{ 45, 114 },
{ 58, 114 },
{ 59, 114 },
{ 60, 114 },
{ 11, 115 },
{ 26, 115 },
{ 34, 115 },
{ 36, 115 },
{ 45, 115 },
{ 56, 115 },
{ 57, 115 },
{ 58, 115 },
{ 60, 115 },
{ 10, 116 },
{ 11, 116 },
{ 16, 116 },
{ 25, 116 },
{ 34, 116 },
{ 38, 116 },
{ 45, 116 },
{ 46, 116 },
{ 47, 116 },
{ 9, 117 },
{ 10, 117 },
{ 11, 117 },
{ 25, 117 },
{ 30, 117 },
{ 31, 117 },
{ 35, 117 },
{ 36, 117 },
{ 37, 117 },
{ 45, 117 },
{ 46, 117 },
{ 47, 117 },
{ 59, 117 },
{ 9, 118 },
{ 10, 118 },
{ 29, 118 },
{ 30, 118 },
{ 44, 118 },
{ 45, 118 },
{ 59, 118 },
{ 9, 119 },
{ 44, 119 },
{ 59, 119 },
{ 23, 120 },
{ 51, 120 },
{ 52, 120 },
{ 58, 120 },
{ 37, 121 },
{ 41, 121 },
{ 51, 121 },
{ 52, 121 },
{ 58, 121 },
{ 33, 122 },
{ 36, 122 },
{ 37, 122 },
{ 40, 122 },
{ 41, 122 },
{ 42, 122 },
{ 50, 122 },
{ 15, 123 },
{ 17, 123 },
{ 33, 123 },
{ 36, 123 },
{ 40, 123 },
{ 41, 123 },
{ 42, 123 },
{ 44, 123 },
{ 49, 123 },
{ 9, 124 },
{ 20, 124 },
{ 31, 124 },
{ 37, 124 },
{ 40, 124 },
{ 41, 124 },
{ 42, 124 },
{ 43, 124 },
{ 44, 124 },
{ 46, 124 },
{ 48, 124 },
{ 9, 125 },
{ 10, 125 },
{ 30, 125 },
{ 36, 125 },
{ 37, 125 },
{ 42, 125 },
{ 43, 125 },
{ 44, 125 },
{ 45, 125 },
{ 46, 125 },
{ 47, 125 },
{ 9, 126 },
{ 10, 126 },
{ 18, 126 },
{ 33, 126 },
{ 36, 126 },
{ 43, 126 },
{ 44, 126 },
{ 45, 126 },
{ 46, 126 },
{ 47, 126 },
{ 57, 126 },
{ 9, 127 },
{ 17, 127 },
{ 29, 127 },
{ 45, 127 },
{ 46, 127 },
{ 47, 127 },
{ 56, 127 },
{ 58, 127 },
{ 9, 128 },
{ 16, 128 },
{ 17, 128 },
{ 45, 128 },
{ 46, 128 },
{ 47, 128 },
{ 56, 128 },
{ 9, 129 },
{ 11, 129 },
{ 14, 129 },
{ 16, 129 },
{ 54, 129 },
{ 11, 130 },
{ 14, 130 },
{ 51, 130 },
{ 52, 130 },
{ 53, 130 },
{ 56, 130 },
{ 11, 131 },
{ 13, 131 },
{ 14, 131 },
{ 46, 131 },
{ 51, 131 },
{ 52, 131 },
{ 56, 131 },
{ 9, 132 },
{ 13, 132 },
{ 14, 132 },
{ 29, 132 },
{ 45, 132 },
{ 46, 132 },
{ 47, 132 },
{ 51, 132 },
{ 52, 132 },
{ 54, 132 },
{ 55, 132 },
{ 56, 132 },
{ 9, 133 },
{ 10, 133 },
{ 12, 133 },
{ 13, 133 },
{ 29, 133 },
{ 44, 133 },
{ 45, 133 },
{ 46, 133 },
{ 47, 133 },
{ 50, 133 },
{ 55, 133 },
{ 56, 133 },
{ 9, 134 },
{ 12, 134 },
{ 28, 134 },
{ 34, 134 },
{ 43, 134 },
{ 44, 134 },
{ 45, 134 },
{ 55, 134 },
{ 56, 134 },
{ 9, 135 },
{ 12, 135 },
{ 13, 135 },
{ 22, 135 },
{ 28, 135 },
{ 38, 135 },
{ 40, 135 },
{ 44, 135 },
{ 45, 135 },
{ 9, 136 },
{ 11, 136 },
{ 12, 136 },
{ 21, 136 },
{ 22, 136 },
{ 28, 136 },
{ 40, 136 },
{ 9, 137 },
{ 11, 137 },
{ 12, 137 },
{ 21, 137 },
{ 22, 137 },
{ 46, 137 },
{ 9, 138 },
{ 10, 138 },
{ 11, 138 },
{ 12, 138 },
{ 13, 138 },
{ 14, 138 },
{ 15, 138 },
{ 16, 138 },
{ 28, 138 },
{ 29, 138 },
{ 46, 138 },
{ 58, 138 },
{ 59, 138 },
{ 9, 139 },
{ 10, 139 },
{ 11, 139 },
{ 28, 139 },
{ 29, 139 },
{ 30, 139 },
{ 38, 139 },
{ 46, 139 },
{ 60, 139 },
{ 9, 140 },
{ 10, 140 },
{ 11, 140 },
{ 12, 140 },
{ 19, 140 },
{ 20, 140 },
{ 28, 140 },
{ 29, 140 },
{ 30, 140 },
{ 37, 140 },
{ 45, 140 },
{ 46, 140 },
{ 10, 141 },
{ 11, 141 },
{ 18, 141 },
{ 19, 141 },
{ 27, 141 },
{ 28, 141 },
{ 29, 141 },
{ 30, 141 },
{ 31, 141 },
{ 46, 141 },
{ 14, 142 },
{ 15, 142 },
{ 17, 142 },
{ 18, 142 },
{ 28, 142 },
{ 29, 142 },
{ 30, 142 },
{ 31, 142 },
{ 32, 142 },
{ 53, 142 },
{ 14, 143 },
{ 15, 143 },
{ 16, 143 },
{ 17, 143 },
{ 28, 143 },
{ 29, 143 },
{ 30, 143 },
{ 53, 143 },
{ 16, 144 },
{ 17, 144 },
{ 27, 144 },
{ 28, 144 },
{ 46, 144 },
{ 24, 145 },
{ 25, 145 },
{ 26, 145 },
{ 44, 145 },
{ 45, 145 },
{ 23, 146 },
{ 24, 146 },
{ 25, 146 },
{ 26, 146 },
{ 27, 146 },
{ 44, 146 },
{ 45, 146 },
{ 22, 147 },
{ 23, 147 },
{ 24, 147 },
{ 25, 147 },
{ 26, 147 },
{ 27, 147 },
{ 44, 147 },
{ 45, 147 },
{ 24, 148 },
{ 25, 148 },
{ 26, 148 },
{ 27, 148 },
{ 44, 148 },
{ 26, 149 },
{ 27, 149 },
{ 28, 149 },
{ 36, 149 },
{ 44, 149 },
{ 28, 150 },
{ 29, 150 },
{ 35, 150 },
{ 36, 150 },
{ 37, 150 },
{ 38, 150 },
{ 39, 150 },
{ 41, 150 },
{ 42, 150 },
{ 43, 150 },
{ 44, 150 },
{ 45, 150 },
{ 46, 150 },
{ 52, 150 },
{ 30, 151 },
{ 31, 151 },
{ 32, 151 },
{ 33, 151 },
{ 34, 151 },
{ 35, 151 },
{ 36, 151 },
{ 37, 151 },
{ 38, 151 },
{ 39, 151 },
{ 40, 151 },
{ 41, 151 },
{ 42, 151 },
{ 43, 151 },
{ 44, 151 },
{ 45, 151 },
{ 46, 151 },
{ 47, 151 },
{ 48, 151 },
{ 49, 151 },
{ 50, 151 },
{ 51, 151 },
{ 52, 151 },
{ 32, 152 },
{ 33, 152 },
{ 34, 152 },
{ 35, 152 },
{ 36, 152 },
{ 37, 152 },
{ 38, 152 },
{ 39, 152 },
{ 40, 152 },
{ 41, 152 },
{ 42, 152 },
{ 43, 152 },
{ 44, 152 },
{ 45, 152 },
{ 46, 152 },
{ 47, 152 },
{ 48, 152 },
{ 49, 152 },
{ 50, 152 },
{ 51, 152 },
{ 52, 152 },
{ 53, 152 },
{ 34, 153 },
{ 35, 153 },
{ 36, 153 },
{ 37, 153 },
{ 38, 153 },
{ 39, 153 },
{ 40, 153 },
{ 41, 153 },
{ 42, 153 },
{ 43, 153 },
{ 44, 153 },
{ 45, 153 },
{ 46, 153 },
{ 47, 153 },
{ 48, 153 },
{ 49, 153 },
{ 50, 153 },
{ 51, 153 },
{ 52, 153 },
{ 36, 154 },
{ 37, 154 },
{ 38, 154 },
{ 39, 154 },
{ 40, 154 },
{ 41, 154 },
{ 42, 154 },
{ 43, 154 },
{ 44, 154 },
{ 45, 154 },
{ 46, 154 },
{ 47, 154 },
{ 48, 154 },
{ 49, 154 },
{ 50, 154 },
{ 51, 154 },
{ 52, 154 },
{ 38, 155 },
{ 39, 155 },
{ 40, 155 },
{ 41, 155 },
{ 44, 155 },
{ 45, 155 },
{ 46, 155 },
{ 47, 155 },
{ 48, 155 },
{ 49, 155 },
{ 50, 155 },
{ 51, 155 },
{ 52, 155 },
{ 46, 156 },
{ 47, 156 },
{ 48, 156 },
{ 49, 156 },
{ 50, 156 },
{ 51, 156 },
{ 52, 156 },
{ 53, 156 },
{ 48, 157 },
{ 49, 157 },
{ 50, 157 },
{ 51, 157 },
{ 52, 157 },
{ 53, 157 },
{ 50, 158 },
{ 51, 158 },
{ 52, 158 },
{ 53, 158 },
{ 54, 158 },
{ 52, 159 },
{ 53, 159 },
{ 54, 159 },
{ 55, 159 },
{ 56, 159 },
{ 57, 159 },
{ 54, 160 },
{ 55, 160 },
{ 56, 160 },
{ 57, 160 },
{ 58, 160 },
{ 63, 160 },
{ 56, 161 },
{ 57, 161 },
{ 58, 161 },
{ 59, 161 },
{ 60, 161 },
{ 61, 161 },
{ 62, 161 },
{ 63, 161 },
{ 58, 162 },
{ 59, 162 },
{ 60, 162 },
{ 61, 162 },
{ 62, 162 },
{ 63, 162 },
{ 60, 163 },
{ 61, 163 },
{ 62, 163 },
{ 63, 163 },
{ 62, 164 },
{ 63, 164 },
};

const POScol cols21[] =
{
{ 51, 102, 78, 0 },
{ 44, 103, 111, 0 },
{ 32, 104, 113, 0 },
{ 33, 104, 113, 0 },
{ 45, 104, 95, 0 },
{ 54, 104, 89, 0 },
{ 33, 105, 113, 0 },
{ 31, 107, 111, 0 },
{ 30, 108, 111, 0 },
{ 17, 110, 97, 0 },
{ 37, 110, 111, 0 },
{ 34, 112, 111, 0 },
{ 31, 113, 113, 0 },
{ 33, 118, 113, 0 },
{ 35, 119, 113, 0 },
{ 34, 120, 113, 0 },
{ 30, 122, 113, 0 },
{ 58, 122, 89, 0 },
{ 57, 123, 109, 0 },
{ 14, 124, 97, 0 },
{ 15, 124, 109, 0 },
{ 16, 124, 109, 0 },
{ 57, 124, 89, 0 },
{ 57, 125, 109, 0 },
{ 27, 127, 66, 0 },
{ 12, 129, 109, 0 },
{ 28, 148, 78, 0 },
{ 47, 150, 85, 0 },
{ 48, 150, 111, 0 },
{ 49, 150, 85, 0 },
{ 50, 150, 85, 0 },
};

const POS trans22[] =
{
{ 38, 96 },
{ 39, 96 },
{ 38, 97 },
{ 39, 97 },
{ 39, 98 },
{ 39, 99 },
{ 39, 100 },
{ 39, 101 },
{ 39, 102 },
{ 32, 103 },
{ 33, 103 },
{ 34, 103 },
{ 39, 103 },
{ 32, 104 },
{ 33, 104 },
{ 34, 104 },
{ 35, 104 },
{ 36, 104 },
{ 37, 104 },
{ 38, 104 },
{ 39, 104 },
{ 34, 105 },
{ 35, 105 },
{ 35, 106 },
{ 35, 107 },
{ 35, 108 },
{ 35, 109 },
{ 35, 110 },
{ 35, 111 },
{ 35, 112 },
{ 34, 113 },
{ 35, 113 },
{ 32, 114 },
{ 33, 114 },
{ 34, 114 },
{ 35, 114 },
{ 32, 115 },
{ 33, 115 },
{ 34, 115 },
{ 35, 115 },
{ 32, 116 },
{ 33, 116 },
{ 34, 116 },
{ 35, 116 },
{ 32, 117 },
{ 33, 117 },
{ 34, 117 },
{ 35, 117 },
{ 33, 118 },
{ 34, 118 },
{ 35, 118 },
{ 34, 119 },
{ 35, 119 },
{ 34, 120 },
{ 35, 120 },
{ 34, 121 },
{ 35, 121 },
{ 35, 122 },
{ 36, 123 },
{ 37, 123 },
{ 38, 123 },
{ 39, 123 },
{ 37, 124 },
{ 38, 124 },
{ 39, 124 },
{ 38, 125 },
{ 39, 125 },
{ 38, 126 },
{ 39, 126 },
{ 39, 127 },
{ 39, 128 },
{ 39, 129 },
{ 32, 132 },
{ 32, 136 },
{ 38, 136 },
{ 39, 136 },
{ 32, 137 },
{ 33, 137 },
{ 34, 137 },
{ 35, 137 },
{ 36, 137 },
{ 37, 137 },
{ 38, 137 },
{ 39, 137 },
{ 32, 138 },
{ 33, 138 },
{ 34, 138 },
{ 35, 138 },
{ 36, 138 },
{ 37, 138 },
{ 38, 138 },
{ 39, 138 },
};

BYTE* Town_PatchSpec(const BYTE* minBuf, size_t minLen, const BYTE* celBuf, size_t celLen, BYTE* sCelBuf, size_t* sCelLen)
{
	typedef struct {
		int subtileIndex;
		unsigned microIndex;
		// int res_encoding;
	} CelMicro;

	const CelMicro micros[] = {
/*  0 */{ 168  - 1, 0 }, // 111
/*  1 */{ 168  - 1, 2 }, // 111
/*  2 */{ 168  - 1, 4 }, // 111
/*  3 */{ 168  - 1, 6 }, // 111
/*  4 */{ 168  - 1, 1 }, // 111
/*  5 */{ 168  - 1, 3 }, // 111
/*  6 */{ 168  - 1, 5 }, // 111
/*  7 */{ 168  - 1, 7 }, // 111
/*  8 */{ 169  - 1, 5 }, // 112
/*  9 */{ 169  - 1, 7 }, // 112
/* 10 */{ 170  - 1, 4 }, // 113
/* 11 */{ 170  - 1, 6 }, // 113
/* 12 */{ 180  - 1, 0 }, // 119
/* 13 */{ 180  - 1, 2 }, // 119
/* 14 */{ 180  - 1, 4 }, // 119
/* 15 */{ 180  - 1, 6 }, // 119
/* 16 */{ 180  - 1, 3 }, // 119
/* 17 */{ 180  - 1, 5 }, // 119
/* 18 */{ 180  - 1, 7 }, // 119
/* 19 */{ 181  - 1, 5 }, // 120
/* 20 */{ 182  - 1, 4 }, // 121
/* 21 */{ 182  - 1, 6 }, // 121
/* 22 */{ 359  - 1, 1 }, // 236
/* 23 */{ 359  - 1, 3 }, // 236
/* 24 */{ 359  - 1, 5 }, // 236
/* 25 */{ 359  - 1, 7 }, // 236
/* 26 */{ 359  - 1, 9 }, // 236
/* 27 */{ 359  - 1, 11 }, // 236
/* 28 */{ 359  - 1, 13 }, // 236
/* 29 */{ 362  - 1, 2 }, // 239
/* 30 */{ 362  - 1, 4 }, // 239
/* 31 */{ 362  - 1, 6 }, // 239
/* 32 */{ 362  - 1, 8 }, // 239
/* 33 */{ 362  - 1, 10 }, // 239
/* 34 */{ 362  - 1, 3 }, // 239
/* 35 */{ 362  - 1, 5 }, // 239
/* 36 */{ 362  - 1, 7 }, // 239
/* 37 */{ 362  - 1, 9 }, // 239
/* 38 */{ 363  - 1, 7 }, // 240
/* 39 */{ 363  - 1, 9 }, // 240
/* 40 */{ 364  - 1, 4 }, // 241
/* 41 */{ 364  - 1, 6 }, // 241
/* 42 */{ 364  - 1, 8 }, // 241
/* 43 */{ 364  - 1, 10 }, // 241
/* 44 */{ 414  - 1, 2 }, // 274
/* 45 */{ 414  - 1, 4 }, // 274
/* 46 */{ 414  - 1, 6 }, // 274
/* 47 */{ 414  - 1, 3 }, // 274
/* 48 */{ 414  - 1, 5 }, // 274
/* 49 */{ 414  - 1, 7 }, // 274
/* 50 */{ 627  - 1, 1 }, // 416
/* 51 */{ 627  - 1, 0 }, // 416
/* 52 */{ 627  - 1, 2 }, // 416
/* 53 */{ 627  - 1, 4 }, // 416
/* 54 */{ 627  - 1, 6 }, // 416
/* 55 */{ 627  - 1, 8 }, // 416
/* 56 */{ 629  - 1, 2 }, // 418

/* 57 */{ 128  - 1, 0 }, // 86
/* 58 */{ 130  - 1, 1 }, // 88

/* 59 */{ 213  - 1, 1 }, // 138
/* 60 */{ 213  - 1, 3 }, // 138
/* 61 */{ 213  - 1, 5 }, // 138
/* 62 */{ 213  - 1, 7 }, // 138
/* 63 */{ 213  - 1, 9 }, // 138
/* 64 */{ 213  - 1, 11 }, // 138

/* 65 */{ 774  - 1, 0 }, // 521
/* 66 */{ 773  - 1, 1 }, // 520
/* 67 */{ 779  - 1, 0 }, // 526
/* 68 */{ 788  - 1, 1 }, // 535
/* 69 */{ 786  - 1, 0 }, // 533
/* 70 */{ 785  - 1, 1 }, // 532
/* 71 */{ 779  - 1, 1 }, // 526
/* 72 */{ 777  - 1, 0 }, // 524
/* 73 */{ 786  - 1, 1 }, // 533

/* 74 */{ 775  - 1, 0 }, // 522
/* 75 */{ 784  - 1, 1 }, // 531
/* 76 */{ 782  - 1, 0 }, // 529
/* 77 */{ 781  - 1, 1 }, // 528
/* 78 */{ 787  - 1, 0 }, // 534
/* 79 */{ 775  - 1, 1 }, // 522
/* 80 */{ 773  - 1, 0 }, // 520
/* 81 */{ 782  - 1, 1 }, // 529
/* 82 */{ 788  - 1, 0 }, // 535
/* 83 */{ 787  - 1, 1 }, // 534
/* 84 */{ 785  - 1, 0 }, // 532

/* 85 */{ 807  - 1, 0 }, // 546
/* 86 */{ 807  - 1, 2 }, // 546

/* 87 */{ 808  - 1, 0 }, // 547
/* 88 */{ 807  - 1, 1 }, // 546
/* 89 */{ 805  - 1, 0 }, // 544
/* 90 */{ 805  - 1, 2 }, // 544

/* 91 */{ 808  - 1, 1 }, // 547
/* 92 */{ 806  - 1, 0 }, // 545
/* 93 */{ 805  - 1, 1 }, // 544
	};

	constexpr BYTE TRANS_COLOR = 128;
	constexpr BYTE SUB_HEADER_SIZE = 10;
	constexpr int FRAME_WIDTH = TILE_WIDTH;
	constexpr int FRAME_HEIGHT = 7 * TILE_HEIGHT;

	DWORD* srcHeaderCursor = (DWORD*)sCelBuf;
	int srcCelEntries = SwapLE32(srcHeaderCursor[0]);
	srcHeaderCursor++;

	if (srcCelEntries != 18) {
		return sCelBuf; // assume it is already done
	}

	// calculate the number of frames in the result
	int resCelEntries = 18 + 3;// 10;
	// render template micros
	const uint16_t* pSubtiles = (const uint16_t*)minBuf;
	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_TOWN;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		// if (micro.subtileIndex < 0) {
		//	continue;
		// }
		unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
		if ((SwapLE16(pSubtiles[index]) & 0xFFF) == 0) {
			app_warn("Invalid (empty) town subtile (%d).", micro.subtileIndex + 1);
			return NULL;
		}
	}

	// TODO: check celLen
	// draw the micros to the back-buffer
	pMicrosCel = (BYTE*)celBuf;
	// constexpr BYTE TRANS_COLOR = 128;
	constexpr int DRAW_HEIGHT = 8;
	memset(&gpBuffer[0 + FRAME_HEIGHT * BUFFER_WIDTH], TRANS_COLOR, DRAW_HEIGHT * BUFFER_WIDTH * MICRO_HEIGHT);

	unsigned xx = 0, yy = MICRO_HEIGHT - 1;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		// if (micro.subtileIndex >= 0) {
			unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
			RenderMicro(&gpBuffer[xx + (yy + FRAME_HEIGHT) * BUFFER_WIDTH], SwapLE16(pSubtiles[index]), DMT_NONE);
		// }
		yy += MICRO_HEIGHT;
		if (yy == (DRAW_HEIGHT + 1) * MICRO_HEIGHT - 1) {
			yy = MICRO_HEIGHT - 1;
			xx += MICRO_WIDTH;
		}
	}

	// create the new CEL file
	size_t maxCelSize = 2 * *sCelLen;
	BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
	memset(resCelBuf, 0, maxCelSize);

	DWORD* dstHeaderCursor = (DWORD*)resCelBuf;
	*dstHeaderCursor = SwapLE32(resCelEntries);
	dstHeaderCursor++;

	BYTE* dstDataCursor = resCelBuf + 4 * (resCelEntries + 2);
	BYTE* resCelBufEnd = resCelBuf + maxCelSize;
	for (int i = 0; i < resCelEntries; i++) {
		// draw the frame to the back-buffer
		memset(&gpBuffer[0], TRANS_COLOR, FRAME_HEIGHT * BUFFER_WIDTH);
		if (i != 3 - 1 && i != 4 - 1 && i != 5 - 1 && i != 9 - 1 && i != 18 - 1 && i < srcCelEntries)
		CelClippedDraw(0, FRAME_HEIGHT - 1, sCelBuf, i + 1, FRAME_WIDTH);

		// shift the frame by (0; -HEIGHT) to add to a new subtile based on 10?
		if (i == 6 - 1) {
			for (int y = TILE_HEIGHT; y < FRAME_HEIGHT; y++) {
				for (int x = 0; x < FRAME_WIDTH; x++) {
					gpBuffer[(x + 0) + (y - TILE_HEIGHT) * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
		}
		// shift the frame by (-HEIGHT/2, WIDTH/2) add frame 9 shifted by (-HEIGHT/2, -WIDTH/2) to add to 81
		if (i == 7 - 1) {
			for (int y = TILE_HEIGHT; y < FRAME_HEIGHT; y++) {
				for (int x = 0; x < FRAME_WIDTH / 2; x++) {
					gpBuffer[(x + FRAME_WIDTH / 2) + (y - TILE_HEIGHT / 2) * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
			// add frame 9
			CelClippedDraw(FRAME_WIDTH, FRAME_HEIGHT - 1, sCelBuf, 9, FRAME_WIDTH);
			for (int y = TILE_HEIGHT; y < FRAME_HEIGHT; y++) {
				for (int x = FRAME_WIDTH / 2; x < FRAME_WIDTH; x++) {
					gpBuffer[(x - FRAME_WIDTH / 2) + (y - TILE_HEIGHT / 2) * BUFFER_WIDTH] = gpBuffer[(x + FRAME_WIDTH) + y * BUFFER_WIDTH];
				}
			}
		}
		// shift the frame by (0; -HEIGHT) + add trunc from 88r to add to 89
		if (i == 8 - 1) {
			for (int y = TILE_HEIGHT; y < FRAME_HEIGHT; y++) {
				for (int x = 0; x < FRAME_WIDTH; x++) {
					gpBuffer[(x + 0) + (y - TILE_HEIGHT) * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
			// add trunc from 88
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				for (int x = 16; x < 23; x++) {
					BYTE color = gpBuffer[(x + (58 / DRAW_HEIGHT) * MICRO_WIDTH) + (y + (58 % DRAW_HEIGHT) * MICRO_HEIGHT + FRAME_HEIGHT) * BUFFER_WIDTH];
					if (color != TRANS_COLOR)
						gpBuffer[(x + 0) + (y + FRAME_HEIGHT - TILE_HEIGHT - MICRO_HEIGHT / 2) * BUFFER_WIDTH] = color;
				}
			}
		}
		// add part of frame 12 shifted by (-HEIGHT/2, -WIDTH/2)
		if (i == 10 - 1) {
			CelClippedDraw(FRAME_WIDTH, FRAME_HEIGHT - 1, sCelBuf, 12, FRAME_WIDTH);
			for (int y = TILE_HEIGHT; y < FRAME_HEIGHT; y++) {
				for (int x = FRAME_WIDTH / 2; x < FRAME_WIDTH; x++) {
					if ((y - TILE_HEIGHT / 2) < 174) // y < 110 || x < 36)
						continue;
					BYTE color = gpBuffer[(x + FRAME_WIDTH) + y * BUFFER_WIDTH];
					if (color != TRANS_COLOR && (color == 0 || color == 110 || color == 117 || color == 119 || color == 121 || color == 122 || color == 124 || color == 126))
						gpBuffer[(x - FRAME_WIDTH / 2) + (y - TILE_HEIGHT / 2) * BUFFER_WIDTH] = color;
				}
			}
		}
		// shift the frame by (0; -HEIGHT) to add to 102
		if (i == 11 - 1) {
			for (int y = TILE_HEIGHT; y < FRAME_HEIGHT; y++) {
				for (int x = 0; x < FRAME_WIDTH; x++) {
					gpBuffer[(x + 0) + (y - TILE_HEIGHT) * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
		}
		// shift the frame by (0; -HEIGHT) and remove trunc to add to 107
		if (i == 12 - 1) {
			for (int y = TILE_HEIGHT; y < FRAME_HEIGHT; y++) {
				for (int x = 0; x < FRAME_WIDTH; x++) {
					BYTE color = gpBuffer[x + y * BUFFER_WIDTH];
					// if (color == TRANS_COLOR) continue;
					if ((y - TILE_HEIGHT + MICRO_HEIGHT / 2) >= 174 //y >= 110 && x >= 36
					 && color != TRANS_COLOR && (color == 0 || color == 110 || color == 117 || color == 119 || color == 121 || color == 122 || color == 124 || color == 126))
						color = TRANS_COLOR;
					gpBuffer[(x + 0) + (y - TILE_HEIGHT) * BUFFER_WIDTH] = color;
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
		}
		// shift the frame by (WIDTH/2; -HEIGHT/2) to add to 97?
		if (i == 13 - 1) {
			for (int y = TILE_HEIGHT; y < FRAME_HEIGHT; y++) {
				for (int x = 0; x < FRAME_WIDTH / 2; x++) {
					gpBuffer[(x + TILE_WIDTH / 2) + (y - TILE_HEIGHT / 2) * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
		}
		// shift the frame by (0; -HEIGHT) to add to a new subtile
		if (i == 14 - 1) {
			for (int y = TILE_HEIGHT; y < FRAME_HEIGHT; y++) {
				for (int x = 0; x < FRAME_WIDTH; x++) {
					gpBuffer[(x + 0) + (y - TILE_HEIGHT) * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
		}
		// add left side of frame 16 shifted by (-HEIGHT/2, WIDTH/2)
		if (i == 15 - 1) {
			CelClippedDraw(FRAME_WIDTH, FRAME_HEIGHT - 1, sCelBuf, 16, FRAME_WIDTH);
			for (int y = TILE_HEIGHT; y < FRAME_HEIGHT; y++) {
				for (int x = 0; x < FRAME_WIDTH / 2; x++) {
					gpBuffer[(x + FRAME_WIDTH / 2) + (y - TILE_HEIGHT / 2) * BUFFER_WIDTH] = gpBuffer[(x + FRAME_WIDTH) + y * BUFFER_WIDTH];
				}
			}
		}
		// shift the frame by (-HEIGHT/2, -WIDTH/2) and add 138r to use in 138
		if (i == 16 - 1) {
			for (int y = TILE_HEIGHT; y < FRAME_HEIGHT; y++) {
				for (int x = FRAME_WIDTH / 2; x < FRAME_WIDTH; x++) {
					gpBuffer[(x - FRAME_WIDTH / 2) + (y - TILE_HEIGHT / 2) * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
			// add subtile 138
			for (int j = 0; j < 6; j++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 0; x < MICRO_WIDTH; x++) {
						if (j == 0 && y > x / 2)
							continue;
						BYTE color = gpBuffer[(x + ((59 + j) / DRAW_HEIGHT) * MICRO_WIDTH) + (y + ((59 + j) % DRAW_HEIGHT) * MICRO_HEIGHT + FRAME_HEIGHT) * BUFFER_WIDTH];
						if (color != TRANS_COLOR)
							gpBuffer[(x + MICRO_WIDTH) + (y + FRAME_HEIGHT - TILE_HEIGHT - MICRO_HEIGHT * (j + 0)) * BUFFER_WIDTH] = color;
					}
				}
			}
		}
		// add frame 18 shifted by (-HEIGHT/2, WIDTH/2)
		if (i == 17 - 1) {
			// add frame 18
			CelClippedDraw(FRAME_WIDTH, FRAME_HEIGHT - 1, sCelBuf, 18, FRAME_WIDTH);
			for (int y = TILE_HEIGHT; y < FRAME_HEIGHT; y++) {
				for (int x = 0; x < FRAME_WIDTH / 2; x++) {
					gpBuffer[(x + FRAME_WIDTH / 2) + (y - TILE_HEIGHT / 2) * BUFFER_WIDTH] = gpBuffer[(x + FRAME_WIDTH) + y * BUFFER_WIDTH];
				}
			}
		}
		// create new frame from 239 to add to 239
		if (i == 3 - 1) {
			for (int j = 0; j < 5; j++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 0; x < MICRO_WIDTH; x++) {
						BYTE color = gpBuffer[(x + ((29 + j) / DRAW_HEIGHT) * MICRO_WIDTH) + (y + ((29 + j) % DRAW_HEIGHT) * MICRO_HEIGHT + FRAME_HEIGHT) * BUFFER_WIDTH];
						if (color != TRANS_COLOR)
							gpBuffer[(x + 0) + (y + FRAME_HEIGHT - TILE_HEIGHT - MICRO_HEIGHT * (j + 1)) * BUFFER_WIDTH] = color;
					}
				}
			}
			for (int j = 0; j < 4; j++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 0; x < MICRO_WIDTH; x++) {
						BYTE color = gpBuffer[(x + ((34 + j) / DRAW_HEIGHT) * MICRO_WIDTH) + (y + ((34 + j) % DRAW_HEIGHT) * MICRO_HEIGHT + FRAME_HEIGHT) * BUFFER_WIDTH];
						if (color != TRANS_COLOR)
							gpBuffer[(x + MICRO_WIDTH) + (y + FRAME_HEIGHT - TILE_HEIGHT - MICRO_HEIGHT * (j + 1)) * BUFFER_WIDTH] = color;
					}
				}
			}
		}
		// create new frame from 236 and 241 to add to 234
		if (i == 4 - 1) {
			for (int j = 0; j < 7; j++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 0; x < MICRO_WIDTH; x++) {
						if (j == 0 && y >= x / 2)
							continue;
						BYTE color = gpBuffer[(x + ((22 + j) / DRAW_HEIGHT) * MICRO_WIDTH) + (y + ((22 + j) % DRAW_HEIGHT) * MICRO_HEIGHT + FRAME_HEIGHT) * BUFFER_WIDTH];
						if (color != TRANS_COLOR)
							gpBuffer[(x + 0) + (y + FRAME_HEIGHT + TILE_HEIGHT - MICRO_HEIGHT / 2 - MICRO_HEIGHT * (j + 1)) * BUFFER_WIDTH] = color;
					}
				}
			}
			for (int j = 0; j < 4; j++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 0; x < MICRO_WIDTH; x++) {
						BYTE color = gpBuffer[(x + ((40 + j) / DRAW_HEIGHT) * MICRO_WIDTH) + (y + ((40 + j) % DRAW_HEIGHT) * MICRO_HEIGHT + FRAME_HEIGHT) * BUFFER_WIDTH];
						if (color != TRANS_COLOR)
							gpBuffer[(x + MICRO_WIDTH) + (y + FRAME_HEIGHT + TILE_HEIGHT - MICRO_HEIGHT / 2 - MICRO_HEIGHT * (j + 3)) * BUFFER_WIDTH] = color;
					}
				}
			}
		}
#if 0
		// create new frame from 240 to add to 240
		if (i == 5 - 1) {
			for (int j = 0; j < 2; j++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 0; x < MICRO_WIDTH; x++) {
						BYTE color = gpBuffer[(x + ((38 + j) / DRAW_HEIGHT) * MICRO_WIDTH) + (y + ((38 + j) % DRAW_HEIGHT) * MICRO_HEIGHT + FRAME_HEIGHT) * BUFFER_WIDTH];
						if (color != TRANS_COLOR)
							gpBuffer[(x + MICRO_WIDTH) + (y + FRAME_HEIGHT - TILE_HEIGHT - MICRO_HEIGHT * (j + 3)) * BUFFER_WIDTH] = color;
					}
				}
			}
		}
#endif
		// create new frame from 274 shifted by (0; -2*HEIGHT) to add to a new subtile(777) based on 1
		if (i == 9 - 1) {
			for (int j = 0; j < 3; j++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 0; x < MICRO_WIDTH; x++) {
						if (j == 0 && x <= 2 && y >= 24 - 2 * x)
							continue;
						if (j == 2 && y < 25 - x)
							continue;
						BYTE color = gpBuffer[(x + ((44 + j) / DRAW_HEIGHT) * MICRO_WIDTH) + (y + ((44 + j) % DRAW_HEIGHT) * MICRO_HEIGHT + FRAME_HEIGHT) * BUFFER_WIDTH];
						if (color != TRANS_COLOR)
							gpBuffer[(x + 0) + (y + FRAME_HEIGHT - 3 * TILE_HEIGHT - MICRO_HEIGHT * (j + 1)) * BUFFER_WIDTH] = color;
					}
				}
			}
			for (int j = 0; j < 3; j++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 0; x < MICRO_WIDTH; x++) {
						BYTE color = gpBuffer[(x + ((47 + j) / DRAW_HEIGHT) * MICRO_WIDTH) + (y + ((47 + j) % DRAW_HEIGHT) * MICRO_HEIGHT + FRAME_HEIGHT) * BUFFER_WIDTH];
						if (color != TRANS_COLOR)
							gpBuffer[(x + MICRO_WIDTH) + (y + FRAME_HEIGHT - 3 * TILE_HEIGHT - MICRO_HEIGHT * (j + 1)) * BUFFER_WIDTH] = color;
					}
				}
			}
		}
		// create new frame from 416 shifted by (WIDTH/2; -HEIGHT - HEIGHT/2) and 418 shifted by (0; -TILE_HEIGHT) to add to 398
		if (i == 18 - 1) {
			for (int j = 0; j < 5; j++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 0; x < MICRO_WIDTH; x++) {
						if (j == 0 && ((x > 8 && y >= 18 + (x - 22) / 2) || (x <= 8 && y >= 16 - x / 2)))
							continue;
#if 0
						if (j == 1 && ((x >= 27 && y <= 23 + (x - 27) / 2) || (x == 26 && y < 20)))
							continue;
#endif
						BYTE color = gpBuffer[(x + ((51 + j) / DRAW_HEIGHT) * MICRO_WIDTH) + (y + ((51 + j) % DRAW_HEIGHT) * MICRO_HEIGHT + FRAME_HEIGHT) * BUFFER_WIDTH];
						if (color != TRANS_COLOR)
							gpBuffer[(x + MICRO_WIDTH) + (y + FRAME_HEIGHT - TILE_HEIGHT - MICRO_HEIGHT - MICRO_HEIGHT / 2 - MICRO_HEIGHT * (j + 0)) * BUFFER_WIDTH] = color;
					}
				}
			}
			for (int j = 0; j < 1; j++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 0; x < MICRO_WIDTH; x++) {
						BYTE color = gpBuffer[(x + ((56 + j) / DRAW_HEIGHT) * MICRO_WIDTH) + (y + ((56 + j) % DRAW_HEIGHT) * MICRO_HEIGHT + FRAME_HEIGHT) * BUFFER_WIDTH];
						if (color != TRANS_COLOR)
							gpBuffer[(x + 0) + (y + FRAME_HEIGHT - TILE_HEIGHT - MICRO_HEIGHT - MICRO_HEIGHT * (j + 1)) * BUFFER_WIDTH] = color;
					}
				}
			}
			// fix remaining artifacts
			for (int j = 0; j < lengthof(trans18); j++)
				gpBuffer[trans18[j].x + trans18[j].y * BUFFER_WIDTH] = TRANS_COLOR;
		}
		// create new frame from 521 526 533 520r 535r to add to 668 and eliminate the other 668
		if (i == 19 - 1) {
			for (int j = 0; j < 6; j++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 0; x < MICRO_WIDTH; x++) {
						if (j == 0 && x > 13)
							continue;
						if (j == 4 && y < x - 20)
							continue;
						if (j == 5 && y <= (29 + x) / 2)
							continue;
						BYTE color = gpBuffer[(x + ((65 + j) / DRAW_HEIGHT) * MICRO_WIDTH) + (y + ((65 + j) % DRAW_HEIGHT) * MICRO_HEIGHT + FRAME_HEIGHT) * BUFFER_WIDTH];
						if (color != TRANS_COLOR)
							gpBuffer[(x + 0) + (y + FRAME_HEIGHT - 2 * TILE_HEIGHT - MICRO_HEIGHT - (MICRO_HEIGHT / 2) * (j + 0)) * BUFFER_WIDTH] = color;
					}
				}
			}
			for (int j = 0; j < 3; j++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 0; x < 8; x++) {
						if (j == 2 && y < 18)
							continue;
						BYTE color = gpBuffer[(x + ((71 + j) / DRAW_HEIGHT) * MICRO_WIDTH) + (y + ((71 + j) % DRAW_HEIGHT) * MICRO_HEIGHT + FRAME_HEIGHT) * BUFFER_WIDTH];
						if (color != TRANS_COLOR)
							gpBuffer[(x + MICRO_WIDTH) + (y + FRAME_HEIGHT - 2 * TILE_HEIGHT - MICRO_HEIGHT - (MICRO_HEIGHT / 2) * (j + 2)) * BUFFER_WIDTH] = color;
					}
				}
			}
			// fix remaining artifacts
			for (int j = 0; j < lengthof(trans19); j++)
				gpBuffer[trans19[j].x + trans19[j].y * BUFFER_WIDTH] = TRANS_COLOR;
			for (int j = 0; j < lengthof(cols19); j++)
				gpBuffer[cols19[j].x + cols19[j].y * BUFFER_WIDTH] = cols19[j].col;
		}
		// create new frame from 522 529 534 531r 520l 535l to add to a new subtile
		if (i == 20 - 1) {
			for (int j = 0; j < 5; j++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 0; x < MICRO_WIDTH; x++) {
						BYTE color = gpBuffer[(x + ((74 + j) / DRAW_HEIGHT) * MICRO_WIDTH) + (y + ((74 + j) % DRAW_HEIGHT) * MICRO_HEIGHT + FRAME_HEIGHT) * BUFFER_WIDTH];
						if (j >= 3 && (color == 62 || color == 52 || color == 59 || ((color == 113 || color == 118 || color == 121 || color == 122 || color == 126) && x <= 16)))
							continue;
						if (color != TRANS_COLOR)
							gpBuffer[(x + 0) + (y + FRAME_HEIGHT - 2 * TILE_HEIGHT - MICRO_HEIGHT - (MICRO_HEIGHT / 2) * (j + 0)) * BUFFER_WIDTH] = color;
					}
				}
			}
			for (int j = 0; j < 6; j++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 0; x < MICRO_WIDTH; x++) {
						if (j == 5 && y <= (x - 7) / 2)
							continue;
						BYTE color = gpBuffer[(x + ((79 + j) / DRAW_HEIGHT) * MICRO_WIDTH) + (y + ((79 + j) % DRAW_HEIGHT) * MICRO_HEIGHT + FRAME_HEIGHT) * BUFFER_WIDTH];
						if (color != TRANS_COLOR)
							gpBuffer[(x + MICRO_WIDTH) + (y + FRAME_HEIGHT - 2 * TILE_HEIGHT - MICRO_HEIGHT - (MICRO_HEIGHT / 2) * (j + 0)) * BUFFER_WIDTH] = color;
					}
				}
			}
			// fix remaining artifacts
			for (int j = 0; j < lengthof(trans20); j++)
				gpBuffer[trans20[j].x + trans20[j].y * BUFFER_WIDTH] = TRANS_COLOR;
			for (int j = 0; j < lengthof(cols20); j++)
				gpBuffer[cols20[j].x + cols20[j].y * BUFFER_WIDTH] = cols20[j].col;
		}
		// create new frame from 546 544l 547l to add to 515
		if (i == 21 - 1) {
			for (int j = 0; j < 2; j++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 9; x < MICRO_WIDTH; x++) {
						if (j == 1 && y < 30 - x)
							continue;
						if (j == 0 && y > 8 + x / 2)
							continue;
						BYTE color = gpBuffer[(x + ((85 + j) / DRAW_HEIGHT) * MICRO_WIDTH) + (y + ((85 + j) % DRAW_HEIGHT) * MICRO_HEIGHT + FRAME_HEIGHT) * BUFFER_WIDTH];
						if (color != TRANS_COLOR)
							gpBuffer[(x + 0) + (y + FRAME_HEIGHT - 2 * TILE_HEIGHT - MICRO_HEIGHT - MICRO_HEIGHT * (j + 0)) * BUFFER_WIDTH] = color;
					}
				}
			}
			for (int j = 0; j < 4; j++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 0; x < MICRO_WIDTH; x++) {
						if (j == 0 && y > 5 + x / 2)
							continue;
						if (j == 1 && y > 24 + x / 2)
							continue;
						if (j == 3 && y < 18 - x / 2)
							continue;
						BYTE color = gpBuffer[(x + ((87 + j) / DRAW_HEIGHT) * MICRO_WIDTH) + (y + ((87 + j) % DRAW_HEIGHT) * MICRO_HEIGHT + FRAME_HEIGHT) * BUFFER_WIDTH];
						if (color != TRANS_COLOR)
							gpBuffer[(x + MICRO_WIDTH) + (y + FRAME_HEIGHT - 2 * TILE_HEIGHT - MICRO_HEIGHT - (MICRO_HEIGHT / 2) * (j + (j < 3 ? -1 : 0))) * BUFFER_WIDTH] = color;
					}
				}
			}
			// fix remaining artifacts
			for (int j = 0; j < lengthof(trans21); j++)
				gpBuffer[trans21[j].x + trans21[j].y * BUFFER_WIDTH] = TRANS_COLOR;
			for (int j = 0; j < lengthof(cols21); j++)
				gpBuffer[cols21[j].x + cols21[j].y * BUFFER_WIDTH] = cols21[j].col;
		}
		// create new frame from 544r 547r to add to 516
		if (i == 5 - 1) {
			for (int j = 0; j < 3; j++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 0; x < 8; x++) {
						if (j == 0 && y > 10)
							continue;
						if (j == 2 && y > 8 && y <= 26 && x > 3)
							continue;
						BYTE color = gpBuffer[(x + ((91 + j) / DRAW_HEIGHT) * MICRO_WIDTH) + (y + ((91 + j) % DRAW_HEIGHT) * MICRO_HEIGHT + FRAME_HEIGHT) * BUFFER_WIDTH];
						if (color != TRANS_COLOR)
							gpBuffer[(x + MICRO_WIDTH) + (y + FRAME_HEIGHT - 2 * TILE_HEIGHT - MICRO_HEIGHT - (MICRO_HEIGHT / 2) * (j + 0)) * BUFFER_WIDTH] = color;
					}
				}
			}
			// fix remaining artifacts
			for (int j = 0; j < lengthof(trans22); j++)
				gpBuffer[trans22[j].x + trans22[j].y * BUFFER_WIDTH] = TRANS_COLOR;
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

static void removeSubtile(uint16_t* pSubtiles, size_t* dwLen, int subtileIdx)
{
	constexpr int blockSize = BLOCK_SIZE_TOWN;
	*dwLen -= blockSize * 2;
	memmove(&pSubtiles[blockSize * subtileIdx], &pSubtiles[blockSize * subtileIdx + blockSize], *dwLen - 2 * blockSize * subtileIdx);
}

static void removeUnusedSubtiles(uint16_t* pSubtiles, size_t* dwLen, const int* unusedSubtiles, int numUnusedSubtiles, const int* unusedPartialSubtiles, int numUnusedPartialSubtiles)
{
	int n1 = numUnusedSubtiles - 1;
	int n2 = numUnusedPartialSubtiles - 1;
	while (n1 >= 0 || n2 >= 0) {
		int sn1 = n1 >= 0 ? unusedSubtiles[n1] : -1;
		int sn2 = n2 >= 0 ? unusedPartialSubtiles[n2] : -1;
		int subtileRef;
		if (sn1 > sn2) {
			subtileRef = sn1;
			n1--;
		} else {
			subtileRef = sn2;
			n2--;
		}
		removeSubtile(pSubtiles, dwLen, subtileRef - 1);
	}
}

BYTE* Town_PatchMin(BYTE* buf, size_t* dwLen, bool isHellfireTown)
{
	uint16_t* pSubtiles = (uint16_t*)buf;
	constexpr int blockSize = BLOCK_SIZE_TOWN;
	// pointless tree micros (re-drawn by dSpecial)
	Blk2Mcr(117, 3);
	Blk2Mcr(117, 5);
	Blk2Mcr(128, 2);
	Blk2Mcr(128, 3);
	Blk2Mcr(128, 4);
	Blk2Mcr(128, 5);
	Blk2Mcr(128, 6);
	Blk2Mcr(128, 7);
	Blk2Mcr(129, 3);
	Blk2Mcr(129, 5);
	Blk2Mcr(129, 7);
	Blk2Mcr(130, 2);
	Blk2Mcr(130, 4);
	Blk2Mcr(130, 6);
	Blk2Mcr(156, 2);
	Blk2Mcr(156, 3);
	Blk2Mcr(156, 4);
	Blk2Mcr(156, 5);
	Blk2Mcr(156, 6);
	Blk2Mcr(156, 7);
	Blk2Mcr(156, 8);
	Blk2Mcr(156, 9);
	Blk2Mcr(156, 10);
	Blk2Mcr(156, 11);
	Blk2Mcr(157, 3);
	Blk2Mcr(157, 5);
	Blk2Mcr(157, 7);
	Blk2Mcr(157, 9);
	Blk2Mcr(157, 11);
	Blk2Mcr(158, 2);
	Blk2Mcr(158, 4);
	Blk2Mcr(160, 2);
	Blk2Mcr(160, 3);
	Blk2Mcr(160, 4);
	Blk2Mcr(160, 5);
	Blk2Mcr(160, 6);
	Blk2Mcr(160, 7);
	Blk2Mcr(160, 8);
	Blk2Mcr(160, 9);
	Blk2Mcr(162, 2);
	Blk2Mcr(162, 4);
	Blk2Mcr(162, 6);
	Blk2Mcr(162, 8);
	Blk2Mcr(162, 10);
	Blk2Mcr(212, 3);
	Blk2Mcr(212, 4);
	Blk2Mcr(212, 5);
	Blk2Mcr(212, 6);
	Blk2Mcr(212, 7);
	Blk2Mcr(212, 8);
	Blk2Mcr(212, 9);
	Blk2Mcr(212, 10);
	Blk2Mcr(212, 11);
	Blk2Mcr(214, 4); // optional
	Blk2Mcr(214, 6); // optional
	Blk2Mcr(216, 2);
	Blk2Mcr(216, 4);
	Blk2Mcr(216, 6);
	Blk2Mcr(217, 4);  // optional
	Blk2Mcr(217, 6);  // optional
	Blk2Mcr(217, 8);  // optional
	Blk2Mcr(358, 4);  // optional
	Blk2Mcr(358, 5);  // optional
	Blk2Mcr(358, 6);  // optional
	Blk2Mcr(358, 7);  // optional
	Blk2Mcr(358, 8);  // optional
	Blk2Mcr(358, 9);  // optional
	Blk2Mcr(358, 10); // optional
	Blk2Mcr(358, 11); // optional
	Blk2Mcr(358, 12); // optional
	Blk2Mcr(358, 13); // optional
	Blk2Mcr(360, 4);  // optional
	Blk2Mcr(360, 6);  // optional
	Blk2Mcr(360, 8);  // optional
	Blk2Mcr(360, 10); // optional
	// fix bad artifacts
	Blk2Mcr(233, 6);
	Blk2Mcr(828, 13);
	Blk2Mcr(1018, 2);
	// useless black (hidden) micros
	Blk2Mcr(426, 1);
	Blk2Mcr(427, 0);
	Blk2Mcr(427, 1);
	Blk2Mcr(429, 1);
	Blk2Mcr(494, 0);
	Blk2Mcr(494, 1);
	Blk2Mcr(550, 1);
	HideMcr(587, 0);
	Blk2Mcr(624, 1);
	Blk2Mcr(626, 1);
	HideMcr(926, 0);
	HideMcr(926, 1);
	HideMcr(928, 0);
	HideMcr(928, 1);
	// Blk2Mcr(1143, 0);
	// Blk2Mcr(1145, 0);
	// Blk2Mcr(1145, 1);
	// Blk2Mcr(1146, 0);
	// Blk2Mcr(1153, 0);
	// Blk2Mcr(1155, 1);
	// Blk2Mcr(1156, 0);
	// Blk2Mcr(1169, 1);
	Blk2Mcr(1172, 1);
	Blk2Mcr(1176, 1);
	Blk2Mcr(1199, 1);
	Blk2Mcr(1203, 1);
	Blk2Mcr(1205, 1);
	Blk2Mcr(1212, 0);
	Blk2Mcr(1219, 0);
	if (isHellfireTown) {
		// fix bad artifacts
		Blk2Mcr(1273, 7);
		Blk2Mcr(1303, 7);
	}
	// - overwrite subtile 237 with subtile 402 to make the inner tile of Griswold's house non-walkable
	// ReplaceMcr(237, 0, 402, 0);
	// ReplaceMcr(237, 1, 402, 1);
	// patch subtiles around the pot of Adria to prevent graphical glitch when a player passes it I.
	patchTownPotMin(pSubtiles, 553, 554);
	// eliminate micros after patchTownChopCel
	{
		Blk2Mcr(362, 11);
		Blk2Mcr(832, 12);
		Blk2Mcr(926, 14);
		Blk2Mcr(926, 15);
		Blk2Mcr(946, 15);
		Blk2Mcr(947, 15);
		Blk2Mcr(950, 15);
		Blk2Mcr(951, 15);

		SetFrameType(180, 1, MET_RTRIANGLE);
		SetFrameType(224, 0, MET_LTRAPEZOID);
		SetFrameType(383, 3, MET_SQUARE);
		SetFrameType(632, 11, MET_SQUARE);
		SetFrameType(834, 10, MET_SQUARE);
		SetFrameType(864, 12, MET_SQUARE);
		SetFrameType(944, 6, MET_SQUARE);

		SetFrameType(383, 5, MET_SQUARE);
		SetFrameType(383, 7, MET_SQUARE);
	}
	// use the micros created by patchTownFloorCel
	if (pSubtiles[MICRO_IDX(731 - 1, blockSize, 9)] != 0) {
		MoveMcr(732, 8, 731, 9);
		SetFrameType(755, 0, MET_LTRIANGLE);
		SetFrameType(974, 0, MET_LTRIANGLE);
		SetFrameType(1030, 0, MET_LTRIANGLE);
		Blk2Mcr(974, 2);
		Blk2Mcr(1030, 2);
		SetFrameType(221, 0, MET_TRANSPARENT);
		ReplaceMcr(220, 0, 17, 0);
		SetMcr(221, 2, 223, 2);
		ReplaceMcr(220, 1, 17, 1);
		SetFrameType(219, 1, MET_TRANSPARENT);
		ReplaceMcr(218, 1, 25, 1);
		SetMcr(219, 3, 218, 0);
		SetMcr(218, 0, 25, 0);
		SetFrameType(1167, 1, MET_TRANSPARENT);
		ReplaceMcr(1166, 1, 281, 1);
		SetMcr(1167, 3, 1166, 0);
		SetMcr(1166, 0, 19, 0);
		ReplaceMcr(962, 0, 14, 0);
		SetMcr(963, 2, 962, 1);
		SetMcr(962, 1, 14, 1);
		SetFrameType(1171, 1, MET_TRANSPARENT);
		SetFrameType(1172, 0, MET_TRANSPARENT);
		SetFrameType(1175, 1, MET_TRANSPARENT);
		SetFrameType(1176, 0, MET_TRANSPARENT);
		SetFrameType(128, 0, MET_LTRIANGLE);
		SetFrameType(128, 1, MET_RTRIANGLE);
		SetFrameType(156, 1, MET_RTRIANGLE);
		SetFrameType(212, 1, MET_RTRIANGLE);
	}
	// use micros created by patchTownDoorCel and patchTownLightCel
	if (pSubtiles[MICRO_IDX(724 - 1, blockSize, 0)] != 0) {
		Blk2Mcr(724, 0);
		Blk2Mcr(724, 1);
		Blk2Mcr(724, 3);
		Blk2Mcr(723, 1);
		Blk2Mcr(715, 11);
		Blk2Mcr(715, 9);
		Blk2Mcr(715, 3);

		Blk2Mcr(428, 4);
		Blk2Mcr(428, 2);
		Blk2Mcr(428, 0);
		Blk2Mcr(428, 1);
		Blk2Mcr(426, 2);
		Blk2Mcr(426, 0);
		Blk2Mcr(429, 0);

		Blk2Mcr(911, 9);
		Blk2Mcr(911, 7);
		Blk2Mcr(911, 5);
		Blk2Mcr(919, 9);
		Blk2Mcr(919, 5);

		// Blk2Mcr(402, 0);
		Blk2Mcr(954, 2);
		Blk2Mcr(956, 2);
		Blk2Mcr(918, 9);
		Blk2Mcr(927, 0);
		Blk2Mcr(918, 3);
		Blk2Mcr(918, 2);
		Blk2Mcr(918, 8);
		Blk2Mcr(928, 4);
		Blk2Mcr(237, 0);
		Blk2Mcr(237, 1);

		Blk2Mcr(551, 0);
		Blk2Mcr(552, 1);
		Blk2Mcr(519, 0);
		Blk2Mcr(510, 7);
		Blk2Mcr(510, 5);

		Blk2Mcr(728, 9);
		Blk2Mcr(728, 7);

		Blk2Mcr(910, 9);
		Blk2Mcr(910, 7);

		Blk2Mcr(537, 0);
		Blk2Mcr(539, 0);
		Blk2Mcr(478, 0);
		Blk2Mcr(479, 1);

		SetFrameType(715, 1, MET_RTRIANGLE);
		SetFrameType(956, 0, MET_LTRIANGLE);
		SetFrameType(954, 0, MET_LTRIANGLE);

		SetFrameType(721, 4, MET_SQUARE);
		SetFrameType(721, 2, MET_SQUARE);
		SetFrameType(719, 4, MET_SQUARE);
		SetFrameType(719, 2, MET_SQUARE);
		SetFrameType(727, 7, MET_SQUARE);
		SetFrameType(727, 5, MET_SQUARE);
		//SetFrameType(725, 4, MET_TRANSPARENT);
		//SetFrameType(725, 2, MET_TRANSPARENT);
		//SetFrameType(725, 0, MET_TRANSPARENT);

		SetFrameType(418, 5, MET_SQUARE);
		SetFrameType(418, 3, MET_SQUARE);
		SetFrameType(418, 1, MET_RTRAPEZOID);
		SetFrameType(419, 5, MET_SQUARE);
		SetFrameType(419, 3, MET_SQUARE);
		SetFrameType(419, 1, MET_RTRAPEZOID);

		SetFrameType(931, 5, MET_SQUARE);
		SetFrameType(931, 3, MET_SQUARE);
		SetFrameType(931, 1, MET_RTRAPEZOID);
		SetFrameType(927, 5, MET_SQUARE);
		SetFrameType(927, 1, MET_RTRAPEZOID);
		SetFrameType(919, 7, MET_SQUARE);
		MoveMcr(927, 3, 919, 7);
		SetFrameType(926, 5, MET_SQUARE);
		SetFrameType(918, 5, MET_SQUARE);
		MoveMcr(929, 2, 918, 4);
		MoveMcr(929, 3, 918, 5);
		MoveMcr(929, 4, 918, 6);
		MoveMcr(929, 5, 918, 7);
		SetFrameType(929, 0, MET_LTRAPEZOID);
		SetFrameType(929, 1, MET_RTRAPEZOID);
		SetFrameType(926, 4, MET_SQUARE);
		SetFrameType(920, 8, MET_SQUARE);

		SetFrameType(509, 5, MET_SQUARE);
		SetFrameType(509, 3, MET_SQUARE);
		SetFrameType(509, 1, MET_RTRAPEZOID);
		SetFrameType(551, 3, MET_SQUARE);
		SetFrameType(551, 1, MET_SQUARE); // MET_RTRAPEZOID);
		SetFrameType(510, 9, MET_SQUARE);
		MoveMcr(551, 5, 510, 9);
		Blk2Mcr(522, 1);

		//SetFrameType(716, 13, MET_TRANSPARENT);
		SetFrameType(716, 11, MET_SQUARE);

		//SetFrameType(930, 5, MET_TRANSPARENT);
		//SetFrameType(930, 3, MET_TRANSPARENT);

		SetFrameType(529, 4, MET_SQUARE);
		MoveMcr(529, 6, 537, 2);
		MoveMcr(529, 8, 537, 4);
		MoveMcr(529, 10, 537, 6);
		MoveMcr(529, 12, 537, 8);

		SetFrameType(529, 4, MET_SQUARE);
		SetFrameType(531, 4, MET_SQUARE);

		SetFrameType(477, 1, MET_SQUARE);
		SetFrameType(480, 1, MET_RTRAPEZOID);
		SetFrameType(477, 0, MET_SQUARE);
		SetFrameType(480, 0, MET_LTRAPEZOID);
		MoveMcr(480, 2, 477, 0);
		MoveMcr(480, 3, 477, 1);
		MoveMcr(480, 4, 477, 2);
		MoveMcr(480, 5, 477, 3);
		MoveMcr(480, 6, 477, 4);
		MoveMcr(480, 7, 477, 5);
		MoveMcr(480, 8, 477, 6);
		MoveMcr(480, 9, 477, 7);
		MoveMcr(480, 10, 477, 8);

		// patchTownLightCel
		SetFrameType(524, 0, MET_SQUARE);
		ReplaceMcr(509, 6, 524, 0);
		SetFrameType(524, 1, MET_SQUARE);
		ReplaceMcr(509, 7, 524, 1);
		SetFrameType(521, 0, MET_SQUARE);
		MoveMcr(509,  8, 521, 0);
		SetFrameType(521, 1, MET_SQUARE);
		MoveMcr(509,  9, 521, 1);
		MoveMcr(509, 10, 521, 2);
		MoveMcr(509, 11, 521, 3);
		MoveMcr(509, 12, 521, 4);
		MoveMcr(509, 13, 521, 5);
		MoveMcr(509, 14, 521, 6);
		MoveMcr(509, 15, 521, 7);
		MoveMcr(552, 14, 521, 8);
		MoveMcr(552, 15, 521, 9);
		Blk2Mcr(522, 0);

		MoveMcr(531, 6, 539, 2);
		MoveMcr(531, 8, 539, 4);
		MoveMcr(531, 10, 539, 6);
		MoveMcr(531, 12, 539, 8);
		MoveMcr(531, 14, 539, 10);

		MoveMcr(551, 7, 522, 3);
		MoveMcr(551, 9, 522, 5);
		MoveMcr(551, 11, 522, 7);
		MoveMcr(551, 13, 522, 9);
		MoveMcr(551, 15, 522, 11);

		// SetFrameType(551, 1, MET_SQUARE);
		SetFrameType(519, 1, MET_RTRAPEZOID);
		Blk2Mcr(517, 0);
		MoveMcr(519, 3, 551, 1);
		// SetFrameType(524, 0, MET_LTRAPEZOID);
		// SetFrameType(521, 0, MET_SQUARE);
		Blk2Mcr(523, 1);
		SetFrameType(513, 5, MET_SQUARE);
		SetFrameType(523, 0, MET_SQUARE);
		/*SetFrameType(523, 2, MET_SQUARE);
		SetFrameType(523, 4, MET_SQUARE);
		SetFrameType(523, 6, MET_SQUARE);
		SetFrameType(523, 8, MET_TRANSPARENT);*/

		MoveMcr(513, 7, 523, 0);
		MoveMcr(513, 9, 523, 2);
		MoveMcr(513, 11, 523, 4);
		MoveMcr(513, 13, 523, 6);
		MoveMcr(513, 15, 523, 8);
		Blk2Mcr(523, 10);

		SetFrameType(386, 9, MET_SQUARE);
		SetFrameType(432, 0, MET_SQUARE);
		// SetFrameType(432, 2, MET_SQUARE);
		// SetFrameType(432, 4, MET_TRANSPARENT);
		// SetFrameType(432, 6, MET_TRANSPARENT);
		MoveMcr(386, 11, 432, 0);
		MoveMcr(386, 13, 432, 2);
		MoveMcr(386, 15, 432, 4);

		SetFrameType(387, 9, MET_SQUARE);
		SetFrameType(432, 1, MET_SQUARE);
		SetFrameType(430, 0, MET_SQUARE);
		// SetFrameType(430, 2, MET_SQUARE);
		SetFrameType(430, 4, MET_TRANSPARENT);
		MoveMcr(387, 11, 432, 1);
		MoveMcr(387, 13, 430, 0);
		MoveMcr(387, 15, 430, 2);
		Blk2Mcr(433, 0);

		SetFrameType(424, 8, MET_SQUARE);
		SetFrameType(431, 0, MET_SQUARE);
		SetFrameType(430, 1, MET_SQUARE);
		// SetFrameType(430, 3, MET_SQUARE);
		SetFrameType(430, 5, MET_TRANSPARENT);
		MoveMcr(424, 10, 431, 0);
		MoveMcr(424, 12, 430, 1);
		MoveMcr(424, 14, 430, 3);
		Blk2Mcr(433, 1);

		SetFrameType(422, 8, MET_SQUARE);
		SetFrameType(431, 1, MET_SQUARE);
		// SetFrameType(431, 3, MET_SQUARE);
		// SetFrameType(431, 5, MET_SQUARE);
		SetFrameType(431, 7, MET_TRANSPARENT);
		MoveMcr(422, 10, 431, 1);
		MoveMcr(422, 12, 431, 3);
		MoveMcr(422, 14, 431, 5);

		SetFrameType(422, 9, MET_SQUARE);
		SetFrameType(436, 0, MET_SQUARE);
		// SetFrameType(436, 2, MET_SQUARE);
		// SetFrameType(436, 4, MET_SQUARE);
		SetFrameType(436, 6, MET_TRANSPARENT);
		MoveMcr(422, 11, 436, 0);
		MoveMcr(422, 13, 436, 2);
		MoveMcr(422, 15, 436, 4);

		SetFrameType(423, 9, MET_SQUARE);
		SetFrameType(436, 1, MET_SQUARE);
		SetFrameType(434, 0, MET_SQUARE);
		// SetFrameType(434, 2, MET_SQUARE);
		SetFrameType(434, 4, MET_TRANSPARENT);
		MoveMcr(423, 11, 436, 1);
		MoveMcr(423, 13, 434, 0);
		MoveMcr(423, 15, 434, 2);
		Blk2Mcr(437, 0);

		SetFrameType(418, 11, MET_SQUARE);
		SetFrameType(434, 1, MET_SQUARE);
		MoveMcr(418, 13, 434, 1);
		MoveMcr(418, 15, 434, 3);
		Blk2Mcr(435, 0);
		Blk2Mcr(437, 1);

		SetFrameType(419, 13, MET_SQUARE);
		MoveMcr(419, 15, 435, 3);
		Blk2Mcr(435, 1);

		SetFrameType(408, 12, MET_SQUARE);
		MoveMcr(408, 14, 440, 2);
		Blk2Mcr(440, 0);

		SetFrameType(406, 10, MET_SQUARE);
		SetFrameType(438, 0, MET_SQUARE);
		MoveMcr(406, 12, 438, 0);
		MoveMcr(406, 14, 438, 2);
		Blk2Mcr(440, 1);
		Blk2Mcr(441, 0);

		SetFrameType(412, 8, MET_SQUARE);
		SetFrameType(439, 0, MET_SQUARE);
		SetFrameType(438, 1, MET_SQUARE);
		// SetFrameType(438, 3, MET_SQUARE);
		SetFrameType(438, 5, MET_TRANSPARENT);
		MoveMcr(412, 10, 439, 0);
		MoveMcr(412, 12, 438, 1);
		MoveMcr(412, 14, 438, 3);
		Blk2Mcr(441, 1);

		SetFrameType(410, 8, MET_SQUARE);
		SetFrameType(439, 1, MET_SQUARE);
		SetFrameType(439, 3, MET_TRANSPARENT);
		// SetFrameType(439, 5, MET_TRANSPARENT);
		MoveMcr(410, 10, 439, 1);
		MoveMcr(410, 12, 439, 3);
		MoveMcr(410, 14, 439, 5);
		Blk2Mcr(439, 7);

		// move micros for better light propagation
		MoveMcr(433, 6, 430, 4);
		MoveMcr(433, 7, 430, 5);
		MoveMcr(428, 15, 435, 5);
		MoveMcr(441, 6, 438, 4);
		// MoveMcr(?, 14, 440, 4);
		MoveMcr(923, 4, 920, 6);
		MoveMcr(923, 6, 920, 8);
	}
	// prepare new subtiles for Town_PatchSpec
	{
		Blk2Mcr(1216, 8);
		HideMcr(1216, 10);
		HideMcr(1216, 2);
		HideMcr(1216, 4);
		HideMcr(1216, 6);
		SetMcr(1216, 0, 1, 0);
		SetMcr(1216, 1, 1, 1); // 777

		SetMcr(1239, 0, 20, 0);
		ReplaceMcr(1239, 1, 20, 1); // 782

		ReplaceMcr(1254, 0, 10, 0);
		SetMcr(1254, 1, 10, 1); // 797

		HideMcr(1258, 2);
		ReplaceMcr(1258, 0, 1, 0);
		ReplaceMcr(1258, 1, 1, 1); // 800

		// mask the micros
		Blk2Mcr(213, 3); // 138
		Blk2Mcr(213, 5);
		Blk2Mcr(213, 7);
		Blk2Mcr(213, 9);
		Blk2Mcr(213, 11);
		SetFrameType(213, 1, MET_RTRIANGLE); // 138

		Blk2Mcr(362, 2); // 239
		Blk2Mcr(362, 4);
		Blk2Mcr(362, 6);
		Blk2Mcr(362, 8);
		Blk2Mcr(362, 10);
		Blk2Mcr(362, 3);
		Blk2Mcr(362, 5);
		Blk2Mcr(362, 7);
		Blk2Mcr(362, 9);

		Blk2Mcr(359, 3); // 236
		Blk2Mcr(359, 5);
		Blk2Mcr(359, 7);
		Blk2Mcr(359, 9);
		Blk2Mcr(359, 11);
		Blk2Mcr(359, 13);
		SetFrameType(359, 1, MET_RTRIANGLE); // 236

		Blk2Mcr(364, 4); // 241
		Blk2Mcr(364, 6); // 241
		Blk2Mcr(364, 8); // 241
		Blk2Mcr(364, 10); // 241

		ReplaceMcr(414, 2, 533, 3); // 363[3] -> 274[2, 4]
		ReplaceMcr(414, 4, 533, 3);
		Blk2Mcr(414, 3); // 274
		Blk2Mcr(414, 5); // 274
		Blk2Mcr(414, 7); // 274

		Blk2Mcr(627, 2); // 416
		Blk2Mcr(627, 4);
		Blk2Mcr(627, 6);
		Blk2Mcr(627, 8);
		SetFrameType(627, 0, MET_LTRIANGLE);
		Blk2Mcr(629, 2); // 418

		ReplaceMcr(782, 1, 138, 1); // 94[1] -> 529[1]

		ReplaceMcr(788, 0, 7, 0); // 7[0] -> 535[0]
		ReplaceMcr(788, 1, 7, 1); // 7[1] -> 535[1]

		// merge 86 and 94
		ReplaceMcr(128, 0, 138, 0); // 94[0] -> 86[0]
		ReplaceMcr(128, 1, 138, 1); // 94[1] -> 86[1]

		// TODO: eliminate unused subtiles 773 (520), 774 (521), 775 (522), 779 (526)
	}
	// better shadows
	ReplaceMcr(555, 0, 493, 0); // TODO: reduce edges on the right
	ReplaceMcr(728, 0, 872, 0);
	// adjust the shadow of the tree beside the church
	ReplaceMcr(767, 0, 117, 0);
	ReplaceMcr(767, 1, 117, 1);
	ReplaceMcr(768, 0, 158, 0); // 516
	ReplaceMcr(768, 1, 159, 1);
	ReplaceMcr(60, 0, 158, 0); // 54
	ReplaceMcr(60, 1, 159, 1);
	// reuse subtiles
	ReplaceMcr(129, 1, 2, 1);  // lost details
	ReplaceMcr(160, 0, 11, 0); // lost details
	ReplaceMcr(160, 1, 12, 1); // lost details
	ReplaceMcr(165, 0, 2, 0);
	ReplaceMcr(169, 0, 129, 0);
	ReplaceMcr(169, 1, 2, 1);
	ReplaceMcr(177, 0, 1, 0);
	ReplaceMcr(178, 1, 118, 1);
	ReplaceMcr(181, 0, 129, 0);
	ReplaceMcr(181, 1, 2, 1);
	ReplaceMcr(188, 0, 3, 0);  // lost details
	ReplaceMcr(198, 1, 1, 1);  // lost details
	ReplaceMcr(281, 0, 19, 0); // lost details
	ReplaceMcr(319, 0, 7, 0);  // lost details
	ReplaceMcr(414, 1, 9, 1);  // lost details
	ReplaceMcr(443, 1, 379, 1);
	ReplaceMcr(471, 0, 3, 0);  // lost details
	ReplaceMcr(472, 0, 5, 0);  // lost details
	ReplaceMcr(475, 0, 7, 0);  // lost details
	ReplaceMcr(476, 0, 4, 0);  // lost details
	ReplaceMcr(484, 1, 4, 1);  // lost details
	ReplaceMcr(486, 1, 20, 1); // lost details
	ReplaceMcr(488, 1, 14, 1); // lost details
	ReplaceMcr(493, 1, 3, 1);  // lost details
	ReplaceMcr(496, 1, 4, 1);  // lost details
	ReplaceMcr(507, 1, 61, 1); // lost details
	ReplaceMcr(512, 0, 3, 0);  // lost details
	ReplaceMcr(532, 1, 14, 1); // lost details
	ReplaceMcr(556, 0, 3, 0);  // lost details
	ReplaceMcr(559, 0, 59, 0); // lost details
	ReplaceMcr(559, 1, 59, 1); // lost details
	ReplaceMcr(563, 0, 2, 0);  // lost details
	ReplaceMcr(569, 1, 3, 1);  // lost details
	ReplaceMcr(592, 0, 11, 0); // lost details
	ReplaceMcr(611, 1, 9, 1);  // lost details
	ReplaceMcr(612, 0, 3, 0);  // lost details
	ReplaceMcr(614, 1, 14, 1); // lost details
	ReplaceMcr(619, 1, 13, 1); // lost details
	ReplaceMcr(624, 0, 1, 0);  // lost details
	ReplaceMcr(640, 1, 9, 1);  // lost details
	ReplaceMcr(653, 0, 1, 0);  // lost details
	ReplaceMcr(660, 0, 10, 0); // lost details
	ReplaceMcr(663, 1, 7, 1);  // lost details
	ReplaceMcr(683, 1, 731, 1);
	ReplaceMcr(685, 0, 15, 0); // lost details
	// ReplaceMcr(690, 1, 2, 1);  // lost details
	ReplaceMcr(694, 0, 17, 0);
	ReplaceMcr(774, 1, 16, 1); // lost details
	ReplaceMcr(789, 1, 10, 1); // lost details
	ReplaceMcr(795, 1, 13, 1); // lost details
	ReplaceMcr(850, 1, 9, 1);  // lost details
	ReplaceMcr(826, 12, 824, 12);
	ReplaceMcr(892, 0, 92, 0);    // lost details
	ReplaceMcr(871, 11, 824, 12); // lost details
	ReplaceMcr(908, 0, 3, 0);     // lost details
	ReplaceMcr(905, 1, 8, 1);     // lost details
	ReplaceMcr(943, 1, 7, 1);     // lost details
	// ReplaceMcr(955, 15, 950, 15); // lost details
	ReplaceMcr(902, 1, 5, 1); // lost details
	// ReplaceMcr(962, 0, 10, 0);    // lost details
	ReplaceMcr(986, 0, 3, 0);  // lost details
	ReplaceMcr(1011, 0, 7, 0); // lost details
	ReplaceMcr(1028, 1, 3, 1); // lost details
	// ReplaceMcr(1030, 2, 974, 2);
	ReplaceMcr(1034, 1, 4, 1); // lost details
	ReplaceMcr(1042, 0, 8, 0); // lost details
	ReplaceMcr(1043, 1, 5, 1); // lost details
	ReplaceMcr(1119, 0, 9, 0); // lost details
	ReplaceMcr(1159, 1, 291, 1);
	// ReplaceMcr(1166, 1, 281, 1);
	ReplaceMcr(1180, 1, 2, 1);  // lost details
	ReplaceMcr(1187, 0, 29, 0); // lost details
	ReplaceMcr(1215, 0, 1207, 0);
	ReplaceMcr(1215, 9, 1207, 9);
	// ReplaceMcr(871, 11, 358, 12);
	// ReplaceMcr(947, 15, 946, 15);
	ReplaceMcr(1175, 4, 1171, 4);
	// ReplaceMcr(1218, 3, 1211, 3);
	// ReplaceMcr(1218, 5, 1211, 5);
	// eliminate micros of unused subtiles
	// Blk2Mcr(178, 538, 1133, 1134 ..);
	Blk2Mcr(107, 1);
	Blk2Mcr(108, 1);
	Blk2Mcr(110, 0);
	Blk2Mcr(113, 0);
	Blk2Mcr(235, 0);
	Blk2Mcr(239, 0);
	Blk2Mcr(240, 0);
	Blk2Mcr(243, 0);
	Blk2Mcr(244, 0);
	Blk2Mcr(468, 0);
	Blk2Mcr(1023, 0);
	Blk2Mcr(1132, 2);
	Blk2Mcr(1132, 3);
	Blk2Mcr(1132, 4);
	Blk2Mcr(1132, 5);
	Blk2Mcr(1139, 0);
	Blk2Mcr(1139, 1);
	Blk2Mcr(1139, 2);
	Blk2Mcr(1139, 3);
	Blk2Mcr(1139, 4);
	Blk2Mcr(1139, 5);
	Blk2Mcr(1139, 6);
	Blk2Mcr(1152, 0);
	Blk2Mcr(1160, 1);
	Blk2Mcr(1162, 1);
	Blk2Mcr(1164, 1);
	Blk2Mcr(1168, 0);
	Blk2Mcr(1196, 0);
	Blk2Mcr(1214, 1);
	Blk2Mcr(1214, 2);
	Blk2Mcr(1214, 3);
	Blk2Mcr(1214, 4);
	Blk2Mcr(1214, 5);
	Blk2Mcr(1214, 6);
	Blk2Mcr(1214, 7);
	Blk2Mcr(1214, 8);
	Blk2Mcr(1214, 9);
	const int unusedSubtiles[] = {
		40, 43, 49, 50, 51, 52, 66, 67, 69, 70, 71, 72, 73, 74, 75, 76, 77, 79, 80, 81, 83, 85, 86, 89, 90, 91, 93, 94, 95, 97, 99, 100, 101, 102, 103, 122, 123, 124, 136, 137, 140, 141, 142, 145, 147, 150, 151, 155, 161, 163, 164, 166, 167, 171, 176, 179, 183, 190, 191, 193, 194, 195, 196, 197, 199, 204, 205, 206, 208, 209, 228, 230, 236, 238, 241, 242, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 256, 278, 280, 291, 298, 299, 304, 305, 314, 316, 318, 320, 321, 328, 329, 335, 336, 337, 342, 350, 351, 352, 353, 354, 355, 356, 357, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 380, 392, 411, 413, 415, 417, 442, 444, 446, 447, 448, 449, 450, 451, 452, 453, 455, 456, 457, 460, 461, 462, 464, 467, 490, 491, 492, 497, 499, 500, 505, 506, 508, 534, 536, 544, 546, 548, 549, 558, 560, 565, 566, 567, 568, 570, 572, 573, 574, 575, 576, 577, 578, 579, 580, 581, 582, 583, 584, 585, 589, 591, 594, 595, 597, 598, 599, 600, 602, 609, 615, 622, 625, 648, 650, 654, 662, 664, 666, 667, 679, 680, 681, 682, 688, 690, 691, 693, 695, 696, 698, 699, 700, 701, 702, 703, 705, 730, 735, 737, 741, 742, 747, 748, 749, 750, 751, 752, 753, 756, 758, 760, 765, 766, 769, 790, 792, 796, 798, 800, 801, 802, 804, 851, 857, 859, 860, 861, 863, 865, 876, 877, 878, 879, 880, 881, 882, 883, 884, 885, 887, 888, 889, 890, 891, 893, 894, 895, 896, 897, 901, 903, 937, 960, 961, 964, 965, 967, 968, 969, 972, 973, 976, 977, 979, 980, 981, 984, 985, 988, 989, 991, 992, 993, 996, 997, 1000, 1001, 1003, 1004, 1005, 1008, 1009, 1012, 1013, 1016, 1017, 1019, 1020, 1021, 1022, 1024, 1029, 1032, 1033, 1035, 1036, 1037, 1039, 1040, 1041, 1044, 1045, 1047, 1048, 1049, 1050, 1051, 1064, 1066, 1067, 1068, 1069, 1070, 1071, 1072, 1073, 1075, 1076, 1077, 1078, 1079, 1080, 1081, 1084, 1085, 1088, 1092, 1093, 1100, 1101, 1102, 1103, 1104, 1105, 1106, 1107, 1108, 1109, 1110, 1111, 1112, 1113, 1114, 1115, 1116, 1117, 1118, 1121, 1123, 1135, 1136, 1137, 1138, 1140, 1141, 1142, 1143, 1144, 1145, 1146, 1147, 1148, 1149, 1150, 1151, 1153, 1154, 1155, 1156, 1157, 1158, 1159, 1161, 1163, 1165, 1169, 1170, 1184, 1186, 1189, 1190, 1193, 1194, 1198, 1199, 1200, 1201, 1202, 1218, 1221, 1222, 1223, 1224, 1225, 1226, 1227, 1228, 1229, 1230, 1231, 1232, 1233, 1234, 1235, 1236, 1237, 1256
	};
	for (int n = 0; n < lengthof(unusedSubtiles); n++) {
		for (int i = 0; i < blockSize; i++) {
			Blk2Mcr(unusedSubtiles[n], i);
		}
	}
	if (isHellfireTown) {
		// reuse subtiles
		ReplaceMcr(1269, 0, 302, 0);
		ReplaceMcr(1281, 0, 290, 0);
		ReplaceMcr(1273, 1, 2, 1);
		ReplaceMcr(1276, 1, 11, 1);
		ReplaceMcr(1265, 0, 1297, 0);
		ReplaceMcr(1314, 0, 293, 0);
		ReplaceMcr(1321, 0, 6, 0);
		ReplaceMcr(1304, 1, 4, 1);
		// eliminate micros of unused subtiles
		Blk2Mcr(1266, 0);
		Blk2Mcr(1267, 1);
		Blk2Mcr(1295, 1);
		Blk2Mcr(1298, 1);
		Blk2Mcr(1360, 0);
		Blk2Mcr(1370, 0);
		Blk2Mcr(1376, 0);
		const int unusedSubtilesHellfire[] = {
			1260, 1268, 1274, 1283, 1284, 1291, 1292, 1293, 1322, 1340, 1341, 1342, 1343, 1344, 1345, 1346, 1347, 1348, 1349, 1350, 1351, 1352, 1353, 1354, 1355, 1356, 1357, 1358, 1359, 1361, 1362, 1363, 1364, 1365, 1366, 1367, 1368, 1369, 1371, 1372, 1373, 1374, 1375, 1377, 1378, 1379
		};
		for (int n = 0; n < lengthof(unusedSubtilesHellfire); n++) {
			for (int i = 0; i < blockSize; i++) {
				Blk2Mcr(unusedSubtilesHellfire[n], i);
			}
		}

		const int unusedPartialSubtilesHellfire[] = {
			1266, 1267, 1295, 1298, 1360, 1370, 1376
		};

		removeUnusedSubtiles(pSubtiles, dwLen, unusedSubtilesHellfire, lengthof(unusedSubtilesHellfire), unusedPartialSubtilesHellfire, lengthof(unusedPartialSubtilesHellfire));
	}

	const int unusedPartialSubtiles[] = {
		107, 108, 110, 113, 178, 235, 239, 240, 243, 244, 468, 538, 1023, 1132, 1133, 1134, 1139, 1152, 1160, 1162, 1164, 1168, 1196, 1214
	};
	removeUnusedSubtiles(pSubtiles, dwLen, unusedSubtiles, lengthof(unusedSubtiles), unusedPartialSubtiles, lengthof(unusedPartialSubtiles));

	return buf;
}

DEVILUTION_END_NAMESPACE
