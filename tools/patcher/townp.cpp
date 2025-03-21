/**
 * @file townp.cpp
 *
 * Implementation of the town level patching functionality.
 */
#include "all.h"
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
		SetMcr(221, 2, 220, 1);
		SetMcr(220, 1, 17, 1);
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
	// better shadows
	ReplaceMcr(555, 0, 493, 0); // TODO: reduce edges on the right
	ReplaceMcr(728, 0, 872, 0);
	// adjust the shadow of the tree beside the church
	ReplaceMcr(767, 0, 117, 0);
	ReplaceMcr(767, 1, 117, 1);
	ReplaceMcr(768, 0, 158, 0);
	ReplaceMcr(768, 1, 159, 1);
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
	Blk2Mcr(1216, 8);
	Blk2Mcr(1239, 1);
	Blk2Mcr(1254, 0);
	Blk2Mcr(1258, 0);
	Blk2Mcr(1258, 1);
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
		107, 108, 110, 113, 178, 235, 239, 240, 243, 244, 468, 538, 1023, 1132, 1133, 1134, 1139, 1152, 1160, 1162, 1164, 1168, 1196, 1214, 1216, 1239, 1254, 1258
	};
	removeUnusedSubtiles(pSubtiles, dwLen, unusedSubtiles, lengthof(unusedSubtiles), unusedPartialSubtiles, lengthof(unusedPartialSubtiles));

	return buf;
}

DEVILUTION_END_NAMESPACE
