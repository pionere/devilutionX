/**
 * @file drlp_l2.cpp
 *
 * Implementation of the catacombs level patching functionality.
 */
#include "all.h"
#include "engine/render/cel_render.h"
#include "engine/render/dun_render.h"

DEVILUTION_BEGIN_NAMESPACE

BYTE* DRLP_L2_PatchDoors(BYTE* celBuf, size_t* celLen)
{
	const int frames[] = { 0, 1 };
	constexpr int FRAME_WIDTH = 64;
	constexpr int FRAME_HEIGHT = 128;

	constexpr BYTE TRANS_COLOR = 128;
	constexpr BYTE SUB_HEADER_SIZE = 10;
	int idx = 0;

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
		const int frameIndex = frames[idx];
		if (i == frameIndex) {
			// draw the frame to the back-buffer
			memset(&gpBuffer[0], TRANS_COLOR, (size_t)FRAME_HEIGHT * BUFFER_WIDTH);
			CelClippedDraw(0, FRAME_HEIGHT - 1, celBuf, frameIndex + 1, FRAME_WIDTH);

			if (idx == 0) {
				for (int y = 44; y < 55; y++) {
					gpBuffer[41 - (y - 44) * 2 + y * BUFFER_WIDTH] = 62;
				}
			}
			if (idx == 1) {
				for (int x = 19; x < 40; x++) {
					gpBuffer[x + (44 + (x / 2 - 10)) * BUFFER_WIDTH] = 62;
				}
				for (int y = 55; y < 112; y++) {
					gpBuffer[40 + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}

			dstHeaderCursor[0] = SwapLE32((DWORD)((size_t)dstDataCursor - (size_t)resCelBuf));
			dstHeaderCursor++;

			dstDataCursor = EncodeFrame(dstDataCursor, FRAME_WIDTH, FRAME_HEIGHT, SUB_HEADER_SIZE, TRANS_COLOR);

			// skip the original frame
			srcHeaderCursor++;

			idx++;
		} else {
			dstHeaderCursor[0] = SwapLE32((DWORD)((size_t)dstDataCursor - (size_t)resCelBuf));
			dstHeaderCursor++;
			DWORD len = srcHeaderCursor[1] - srcHeaderCursor[0];
			memcpy(dstDataCursor, celBuf + srcHeaderCursor[0], len);
			dstDataCursor += len;
			srcHeaderCursor++;
		}
	}
	// add file-size
	*celLen = (size_t)dstDataCursor - (size_t)resCelBuf;
	dstHeaderCursor[0] = SwapLE32((DWORD)(*celLen));

	return resCelBuf;
}

BYTE* DRLP_L2_PatchSpec(BYTE* celBuf, size_t* celLen)
{
	const int frames[] = { 0, 1, 4 };
	constexpr int FRAME_WIDTH = 64;
	constexpr int FRAME_HEIGHT = 160;

	constexpr BYTE TRANS_COLOR = 128;
	constexpr BYTE SUB_HEADER_SIZE = 10;
	int idx = 0;

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
		const int frameIndex = frames[idx];
		if (i == frameIndex) {
			// draw the frame to the back-buffer
			memset(&gpBuffer[0], TRANS_COLOR, (size_t)FRAME_HEIGHT * BUFFER_WIDTH);
			CelClippedDraw(0, FRAME_HEIGHT - 1, celBuf, frameIndex + 1, FRAME_WIDTH);

			if (idx == 0) {
				gpBuffer[10 + 52 * BUFFER_WIDTH] = 55;
				gpBuffer[11 + 52 * BUFFER_WIDTH] = 53;
				gpBuffer[13 + 53 * BUFFER_WIDTH] = 53;
				gpBuffer[19 + 55 * BUFFER_WIDTH] = 55;
				gpBuffer[23 + 57 * BUFFER_WIDTH] = 53;
				gpBuffer[25 + 58 * BUFFER_WIDTH] = 53;
				gpBuffer[26 + 59 * BUFFER_WIDTH] = 55;
				gpBuffer[27 + 60 * BUFFER_WIDTH] = 53;
				gpBuffer[28 + 61 * BUFFER_WIDTH] = 54;

				gpBuffer[29 + 97 * BUFFER_WIDTH] = 76;
				gpBuffer[30 + 95 * BUFFER_WIDTH] = 60;
				gpBuffer[30 + 96 * BUFFER_WIDTH] = 61;
				gpBuffer[31 + 93 * BUFFER_WIDTH] = 57;
			}
			if (idx == 1) {
				gpBuffer[ 2 + 104 * BUFFER_WIDTH] = 76;
			}
			if (idx == 2) {
				gpBuffer[ 9 + 148 * BUFFER_WIDTH] = 39;
				gpBuffer[10 + 148 * BUFFER_WIDTH] = 66;
				gpBuffer[10 + 149 * BUFFER_WIDTH] = 50;
				gpBuffer[11 + 149 * BUFFER_WIDTH] = 36;
			}

			dstHeaderCursor[0] = SwapLE32((DWORD)((size_t)dstDataCursor - (size_t)resCelBuf));
			dstHeaderCursor++;

			dstDataCursor = EncodeFrame(dstDataCursor, FRAME_WIDTH, FRAME_HEIGHT, SUB_HEADER_SIZE, TRANS_COLOR);

			// skip the original frame
			srcHeaderCursor++;

			idx++;
		} else {
			dstHeaderCursor[0] = SwapLE32((DWORD)((size_t)dstDataCursor - (size_t)resCelBuf));
			dstHeaderCursor++;
			DWORD len = srcHeaderCursor[1] - srcHeaderCursor[0];
			memcpy(dstDataCursor, celBuf + srcHeaderCursor[0], len);
			dstDataCursor += len;
			srcHeaderCursor++;
		}
	}
	// add file-size
	*celLen = (size_t)dstDataCursor - (size_t)resCelBuf;
	dstHeaderCursor[0] = SwapLE32((DWORD)(*celLen));

	return resCelBuf;
}

static BYTE shadowColorCatacombs(BYTE color)
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
static BYTE* patchCatacombsStairs(/*const BYTE* tilBuf, size_t tilLen,*/ const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t *celLen, int backTileIndex1, int backTileIndex2, int stairsSubtileRef1, int stairsSubtileRef2, int stairsExtSubtileRef1, int stairsExtSubtileRef2)
{
	/*const uint16_t* pTiles = (const uint16_t*)tilBuf;
	// TODO: check tilLen
	int topLeftSubtileIndex = SwapLE16(pTiles[backTileIndex1 * 4 + 0]);
	int bottomLeftSubtileIndex = SwapLE16(pTiles[backTileIndex1 * 4 + 2]);
	int bottomRightSubtileIndex = SwapLE16(pTiles[backTileIndex1 * 4 + 3]);*/

	constexpr int backSubtileRef0 = 250;
	constexpr int backSubtileRef2 = 251;
	constexpr int backSubtileRef3 = 252;
	constexpr int backSubtileRef0Replacement = 9;
	constexpr int backSubtileRef2Replacement = 11;
	/*if (topLeftSubtileIndex != (backSubtileRef0 - 1) || bottomLeftSubtileIndex != (backSubtileRef2 - 1) || bottomRightSubtileIndex != (backSubtileRef3 - 1)) {
		return celBuf; // non-original subtiles -> assume it is already done
	}*/

	const uint16_t* pSubtiles = (const uint16_t*)minBuf;

	// TODO: check minLen
	constexpr unsigned blockSize = BLOCK_SIZE_L2;
	unsigned back3_FrameIndex0 = MICRO_IDX((backSubtileRef3 - 1), blockSize, 0);
	unsigned back3_FrameRef0 = pSubtiles[back3_FrameIndex0] & 0xFFF; // 252[0]
	unsigned back2_FrameIndex1 = MICRO_IDX((backSubtileRef2 - 1), blockSize, 1);
	unsigned back2_FrameRef1 = pSubtiles[back2_FrameIndex1] & 0xFFF; // 251[1]
	unsigned back0_FrameIndex0 = MICRO_IDX((backSubtileRef0 - 1), blockSize, 0);
	unsigned back0_FrameRef0 = pSubtiles[back0_FrameIndex0] & 0xFFF; // 250[0]

	unsigned stairs_FrameIndex0 = MICRO_IDX((stairsSubtileRef1 - 1), blockSize, 0);
	unsigned stairs_FrameRef0 = pSubtiles[stairs_FrameIndex0] & 0xFFF; // 267[0]
	unsigned stairs_FrameIndex2 = MICRO_IDX((stairsSubtileRef1 - 1), blockSize, 2);
	unsigned stairs_FrameRef2 = pSubtiles[stairs_FrameIndex2] & 0xFFF; // 267[2]
	unsigned stairs_FrameIndex4 = MICRO_IDX((stairsSubtileRef1 - 1), blockSize, 4);
	unsigned stairs_FrameRef4 = pSubtiles[stairs_FrameIndex4] & 0xFFF; // 267[4]
	unsigned stairs_FrameIndex6 = MICRO_IDX((stairsSubtileRef1 - 1), blockSize, 6);
	unsigned stairs_FrameRef6 = pSubtiles[stairs_FrameIndex6] & 0xFFF; // 267[6]

	unsigned stairsExt_FrameIndex1 = MICRO_IDX((stairsExtSubtileRef1 - 1), blockSize, 1);
	unsigned stairsExt_FrameRef1 = pSubtiles[stairsExt_FrameIndex1] & 0xFFF; // 265[1]
	unsigned stairsExt_FrameIndex3 = MICRO_IDX((stairsExtSubtileRef1 - 1), blockSize, 3);
	unsigned stairsExt_FrameRef3 = pSubtiles[stairsExt_FrameIndex3] & 0xFFF; // 265[3]
	unsigned stairsExt_FrameIndex5 = MICRO_IDX((stairsExtSubtileRef1 - 1), blockSize, 5);
	unsigned stairsExt_FrameRef5 = pSubtiles[stairsExt_FrameIndex5] & 0xFFF; // 265[5]

	if (stairs_FrameRef0 == 0 || stairs_FrameRef2 == 0 || stairs_FrameRef4 == 0 || stairs_FrameRef6 == 0
		|| stairsExt_FrameRef1 == 0 || stairsExt_FrameRef3 == 0 || stairsExt_FrameRef5 == 0) {
		return celBuf;
	}

	if (back3_FrameRef0 == 0 || back2_FrameRef1 == 0 || back0_FrameRef0 == 0) {
		mem_free_dbg(celBuf);
		app_warn("The back-stairs tile (%d) has invalid (missing) frames.", backTileIndex1 + 1);
		return NULL;
	}

	// draw the micros to the back-buffer
	pMicrosCel = celBuf;
	constexpr BYTE TRANS_COLOR = 128;
	memset(&gpBuffer[0], TRANS_COLOR, 5 * BUFFER_WIDTH * MICRO_HEIGHT);

	// RenderMicro(&gpBuffer[0 + (MICRO_HEIGHT * 3 - 1) * BUFFER_WIDTH], pSubtiles[back0_FrameIndex0], DMT_NONE); // 716
	// RenderMicro(&gpBuffer[0 + (MICRO_HEIGHT * 4 - MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[back2_FrameIndex1], DMT_NONE); // 251[1]
	RenderMicro(&gpBuffer[0 + (MICRO_HEIGHT * 4 - 1) * BUFFER_WIDTH], pSubtiles[back3_FrameIndex0], DMT_NONE); // 252[0]

	RenderMicro(&gpBuffer[0 + (MICRO_HEIGHT * 2 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[stairsExt_FrameIndex5], DMT_NONE); // 265[5]
	RenderMicro(&gpBuffer[0 + (MICRO_HEIGHT * 3 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[stairsExt_FrameIndex3], DMT_NONE); // 265[3]
	// RenderMicro(&gpBuffer[0 + (MICRO_HEIGHT * 4 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[stairsExt_FrameIndex1], DMT_NONE); // 265[1]

	RenderMicro(&gpBuffer[MICRO_WIDTH + (MICRO_HEIGHT * 1 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[stairs_FrameIndex6], DMT_NONE); // 267[6]
	RenderMicro(&gpBuffer[MICRO_WIDTH + (MICRO_HEIGHT * 2 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[stairs_FrameIndex4], DMT_NONE); // 267[4]
	RenderMicro(&gpBuffer[MICRO_WIDTH + (MICRO_HEIGHT * 3 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[stairs_FrameIndex2], DMT_NONE); // 267[2]
	RenderMicro(&gpBuffer[MICRO_WIDTH + (MICRO_HEIGHT * 4 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[stairs_FrameIndex0], DMT_NONE); // 267[0]

	RenderMicro(&gpBuffer[2 * MICRO_WIDTH + (MICRO_HEIGHT * 1 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[back0_FrameIndex0], DMT_NONE);     // 250[0]
	RenderMicro(&gpBuffer[2 * MICRO_WIDTH + (MICRO_HEIGHT * 2 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[back2_FrameIndex1], DMT_NONE);     // 251[1]
	RenderMicro(&gpBuffer[2 * MICRO_WIDTH + (MICRO_HEIGHT * 3 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[stairsExt_FrameIndex1], DMT_NONE); // 265[1]
	RenderMicro(&gpBuffer[2 * MICRO_WIDTH + (MICRO_HEIGHT * 4 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[stairs_FrameIndex0], DMT_NONE);    // 267[0]

	RenderMicro(&gpBuffer[3 * MICRO_WIDTH + (MICRO_HEIGHT * 3 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[stairsExt_FrameIndex1], DMT_NONE); // 265[1]

	// add micros to be used as masks
	RenderMicro(&gpBuffer[3 * MICRO_WIDTH + (MICRO_HEIGHT * 1 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[back3_FrameIndex0], DMT_NONE); // 252[0]
	RenderMicro(&gpBuffer[3 * MICRO_WIDTH + (MICRO_HEIGHT * 2 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[back2_FrameIndex1], DMT_NONE); // 251[1]

	// mask the drawing
	for (int x = 0; x < MICRO_WIDTH; x++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			BYTE pixel = gpBuffer[3 * MICRO_WIDTH + x + (MICRO_HEIGHT * 1 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH];
			if (pixel == TRANS_COLOR) {
				gpBuffer[2 * MICRO_WIDTH + x + (MICRO_HEIGHT * 4 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH] = TRANS_COLOR; // mask the stair-floor with 252[0]
			} else {
				gpBuffer[MICRO_WIDTH + x + (MICRO_HEIGHT * 4 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH] = TRANS_COLOR;// mask the stairs with ~252[0]
			}
		}
	}
	for (int x = 0; x < MICRO_WIDTH; x++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			BYTE pixel = gpBuffer[3 * MICRO_WIDTH + x + (MICRO_HEIGHT * 2 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH];
			if (pixel == TRANS_COLOR) {
				gpBuffer[2 * MICRO_WIDTH + x + (MICRO_HEIGHT * 3 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH] = TRANS_COLOR; // mask the external back-subtile floor with 251[1]
			} else {
				gpBuffer[3 * MICRO_WIDTH + x + (MICRO_HEIGHT * 3 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH] = TRANS_COLOR; // mask the external back-subtile with ~251[1]
			}
		}
	}
	// mask the shadows
	for (int x = 0; x < MICRO_WIDTH; x++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			BYTE pixel = gpBuffer[2 * MICRO_WIDTH + x + (MICRO_HEIGHT * 1 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH];
			if (pixel == TRANS_COLOR)
				continue;
			if (x < 22 || y >= MICRO_HEIGHT - 20 || (pixel % 16) < 11) {
				gpBuffer[2 * MICRO_WIDTH + x + (MICRO_HEIGHT * 1 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH] = TRANS_COLOR; // 250[0]
			}
		}
	}
	for (int x = 0; x < MICRO_WIDTH; x++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			BYTE pixel = gpBuffer[2 * MICRO_WIDTH + x + (MICRO_HEIGHT * 2 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH];
			if (pixel == TRANS_COLOR)
				continue;
			if (x < 22 || y >= MICRO_HEIGHT - 12 || y < MICRO_HEIGHT - 20 || (pixel % 16) < 11) {
				gpBuffer[2 * MICRO_WIDTH + x + (MICRO_HEIGHT * 2 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH] = TRANS_COLOR; // 251[1]
			}
		}
	}
	// copy the stair of back-subtile to its position
	for (int x = 0; x < MICRO_WIDTH; x++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			BYTE pixel = gpBuffer[3 * MICRO_WIDTH + x + (MICRO_HEIGHT * 3 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH]; // 265[1]
			if (pixel == TRANS_COLOR)
				continue;
			// if (gpBuffer[0 + x + (MICRO_HEIGHT * 3 - y - 1) * BUFFER_WIDTH] == TRANS_COLOR)
				gpBuffer[0 + x + (MICRO_HEIGHT * 4 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH] = pixel;
		}
	}
	// copy the shadows to their position
	for (int x = 0; x < MICRO_WIDTH; x++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			BYTE pixel = gpBuffer[2 * MICRO_WIDTH + x + (MICRO_HEIGHT * 1 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH]; // 250[0]
			if (pixel == TRANS_COLOR)
				continue;
			if (gpBuffer[0 + x + (MICRO_HEIGHT * 3 - y - 1) * BUFFER_WIDTH] == TRANS_COLOR)
				gpBuffer[0 + x + (MICRO_HEIGHT * 3 - y - 1) * BUFFER_WIDTH] = pixel;
		}
	}
	for (int x = 0; x < MICRO_WIDTH; x++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			BYTE pixel = gpBuffer[2 * MICRO_WIDTH + x + (MICRO_HEIGHT * 2 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH]; // 251[1]
			if (pixel == TRANS_COLOR)
				continue;
			if (gpBuffer[0 + x + (MICRO_HEIGHT * 4 - MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH] == TRANS_COLOR)
				gpBuffer[0 + x + (MICRO_HEIGHT * 4 - MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH] = pixel; // 265[3], 252[0]
		}
	}

	// complete stairs-micro to square
	gpBuffer[MICRO_WIDTH + 0 + (MICRO_HEIGHT * 3 - (MICRO_HEIGHT - 0 - 1) - 1) * BUFFER_WIDTH] = 40; // 267[2]

	// fix bad artifacts
	gpBuffer[          0 + 23 + (MICRO_HEIGHT * 3 - (MICRO_HEIGHT - 20 - 1) - 1) * BUFFER_WIDTH] = TRANS_COLOR; // 265[3]
	gpBuffer[          0 + 24 + (MICRO_HEIGHT * 3 - (MICRO_HEIGHT - 20 - 1) - 1) * BUFFER_WIDTH] = TRANS_COLOR; // 265[3]
	gpBuffer[          0 + 22 + (MICRO_HEIGHT * 3 - (MICRO_HEIGHT - 21 - 1) - 1) * BUFFER_WIDTH] = TRANS_COLOR; // 265[3]
	gpBuffer[          0 + 23 + (MICRO_HEIGHT * 3 - (MICRO_HEIGHT - 21 - 1) - 1) * BUFFER_WIDTH] = TRANS_COLOR; // 265[3]
	gpBuffer[          0 + 22 + (MICRO_HEIGHT * 3 - (MICRO_HEIGHT - 30 - 1) - 1) * BUFFER_WIDTH] = 78;          // 265[3]

	gpBuffer[MICRO_WIDTH +  5 + (MICRO_HEIGHT * 2 - (MICRO_HEIGHT - 22 - 1) - 1) * BUFFER_WIDTH] = 55; // 267[4]
	gpBuffer[MICRO_WIDTH + 19 + (MICRO_HEIGHT * 2 - (MICRO_HEIGHT -  7 - 1) - 1) * BUFFER_WIDTH] = 71; // 267[4]
	gpBuffer[MICRO_WIDTH + 19 + (MICRO_HEIGHT * 2 - (MICRO_HEIGHT -  9 - 1) - 1) * BUFFER_WIDTH] = 36; // 267[4]

	gpBuffer[          0 + 22 + (MICRO_HEIGHT * 4 - (MICRO_HEIGHT -  4 - 1) - 1) * BUFFER_WIDTH] = 78; // 252[0]
	gpBuffer[          0 + 23 + (MICRO_HEIGHT * 4 - (MICRO_HEIGHT -  4 - 1) - 1) * BUFFER_WIDTH] = 31; // 252[0]

	// create the new CEL file
	size_t maxCelSize = *celLen + 8 * MICRO_WIDTH * MICRO_HEIGHT;
	BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
	memset(resCelBuf, 0, maxCelSize);

	typedef struct {
		int type;
		unsigned frameRef;
	} CelFrameEntry;
	CelFrameEntry entries[8];
	entries[0].type = 0; // 265[1]
	// assert(stairsExt_FrameRef1 == 762);
	entries[0].frameRef = stairsExt_FrameRef1;
	entries[1].type = 1; // 267[0]
	// assert(stairs_FrameRef0 == 770);
	entries[1].frameRef = stairs_FrameRef0;
	entries[2].type = 2; // 267[6]
	// assert(stairs_FrameRef6 == 767);
	entries[2].frameRef = stairs_FrameRef6;
	entries[3].type = 3; // 267[4]
	// assert(stairs_FrameRef4 == 768);
	entries[3].frameRef = stairs_FrameRef4;
	entries[4].type = 4; // 267[2]
	// assert(stairs_FrameRef2 == 769);
	entries[4].frameRef = stairs_FrameRef2;
	entries[5].type = 5; // 265[5]
	// assert(stairsExt_FrameRef5 == 760);
	entries[5].frameRef = stairsExt_FrameRef5;
	entries[6].type = 6; // 252[0]
	// assert(back3_FrameRef0 == 719);
	entries[6].frameRef = back3_FrameRef0;
	entries[7].type = 7; // 265[3]
	// assert(stairsExt_FrameRef3 == 761);
	entries[7].frameRef = stairsExt_FrameRef3;
	DWORD* srcHeaderCursor = (DWORD*)celBuf;
	DWORD celEntries = SwapLE32(srcHeaderCursor[0]);
	srcHeaderCursor++;
	DWORD* dstHeaderCursor = (DWORD*)resCelBuf;
	dstHeaderCursor[0] = SwapLE32(celEntries);
	dstHeaderCursor++;
	BYTE* dstDataCursor = resCelBuf + 4 * (celEntries + 2);
	while (true) {
		// select the next frame
		int next = -1;
		for (int i = 0; i < lengthof(entries); i++) {
			if (entries[i].frameRef != 0 && (next == -1 || entries[i].frameRef < entries[next].frameRef)) {
				next = i;
			}
		}
		if (next == -1)
			break;

		// copy entries till the next frame
		int numEntries = entries[next].frameRef - (unsigned)((size_t)srcHeaderCursor - (size_t)celBuf) / 4;
		for (int i = 0; i < numEntries; i++) {
			dstHeaderCursor[0] = SwapLE32((DWORD)((size_t)dstDataCursor - (size_t)resCelBuf));
			dstHeaderCursor++;
			DWORD len = srcHeaderCursor[1] - srcHeaderCursor[0];
			memcpy(dstDataCursor, celBuf + srcHeaderCursor[0], len);
			dstDataCursor += len;
			srcHeaderCursor++;
		}
		// add the next frame
		dstHeaderCursor[0] = SwapLE32((DWORD)((size_t)dstDataCursor - (size_t)resCelBuf));
		dstHeaderCursor++;
		
		BYTE* frameSrc;
		int encoding = MET_TRANSPARENT;
		switch (entries[next].type) {
		case 0: // 265[1]
			frameSrc = &gpBuffer[2 * MICRO_WIDTH + (MICRO_HEIGHT * 3 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH];
			encoding = MET_RTRIANGLE;
			break;
		case 1: // 267[0]
			frameSrc = &gpBuffer[2 * MICRO_WIDTH + (MICRO_HEIGHT * 4 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH];
			encoding = MET_LTRIANGLE;
			break;
		case 2: // 267[6]
			frameSrc = &gpBuffer[MICRO_WIDTH + (MICRO_HEIGHT * 1 - 1) * BUFFER_WIDTH];
			break;
		case 3: // 267[4]
			frameSrc = &gpBuffer[MICRO_WIDTH + (MICRO_HEIGHT * 2 - 1) * BUFFER_WIDTH];
			break;
		case 4: // 267[2]
			frameSrc = &gpBuffer[MICRO_WIDTH + (MICRO_HEIGHT * 3 - 1) * BUFFER_WIDTH];
			encoding = MET_SQUARE;
			break;
		case 5: // 265[5]
			frameSrc = &gpBuffer[MICRO_WIDTH + (MICRO_HEIGHT * 4 - 1) * BUFFER_WIDTH];
			encoding = MET_RTRAPEZOID;
			break;
		case 6: // 252[0]
			frameSrc = &gpBuffer[0 + (MICRO_HEIGHT * 4 - 1) * BUFFER_WIDTH];
			break;
		case 7: // 265[3]
			frameSrc = &gpBuffer[0 + (MICRO_HEIGHT * 3 - 1) * BUFFER_WIDTH];
			break;
		}
		dstDataCursor = EncodeMicro(encoding, dstDataCursor, frameSrc, TRANS_COLOR);

		// skip the original frame
		srcHeaderCursor++;

		// remove entry
		entries[next].frameRef = 0;
	}
	// add remaining entries
	int numEntries = celEntries + 1 - (unsigned)((size_t)srcHeaderCursor - (size_t)celBuf) / 4;
	for (int i = 0; i < numEntries; i++) {
		dstHeaderCursor[0] = SwapLE32((DWORD)((size_t)dstDataCursor - (size_t)resCelBuf));
		dstHeaderCursor++;
		DWORD len = srcHeaderCursor[1] - srcHeaderCursor[0];
		memcpy(dstDataCursor, celBuf + srcHeaderCursor[0], len);
		dstDataCursor += len;
		srcHeaderCursor++;
	}
	// add file-size
	dstHeaderCursor[0] = SwapLE32((DWORD)((size_t)dstDataCursor - (size_t)resCelBuf));

	*celLen = SwapLE32(dstHeaderCursor[0]);

	mem_free_dbg(celBuf);

	return resCelBuf;
}

static BYTE* fixCatacombsShadows(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	const CelMicro micros[] = {
/*  0 */{ 151 - 1, 0, -1 }, // used to block subsequent calls
/*  1 */{  33 - 1, 0, -1 },
/*  2 */{ 268 - 1, 0, MET_LTRAPEZOID },
/*  3 */{  33 - 1, 1, -1 },
/*  4 */{ 268 - 1, 1, MET_RTRAPEZOID },
/*  5 */{  23 - 1, 1, -1 },
/*  6 */{ 148 - 1, 1, MET_RTRIANGLE },
/*  7 */{   6 - 1, 3, -1 },
/*  8 */{ 152 - 1, 0, MET_SQUARE },
/*  9 */{   6 - 1, 1, -1 },
/* 10 */{ 250 - 1, 0, MET_RTRAPEZOID },
/* 11 */{   5 - 1, 1, -1 },
/* 12 */{ 514 - 1, 1, MET_RTRAPEZOID },
/* 13 */{ 515 - 1, 0, MET_LTRIANGLE },
/* 14 */{ 155 - 1, 1, MET_RTRIANGLE },
	};

	const uint16_t* pSubtiles = (const uint16_t*)minBuf;
	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_L2;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		// if (micro.subtileIndex < 0) {
		//	continue;
		// }
		unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
		if ((SwapLE16(pSubtiles[index]) & 0xFFF) == 0) {
			return celBuf;
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
	// draw new shadow micros 268[0], 268[1], 148[1], 152[0], 250[0] using base micros 33[0], 33[1], 23[1], 6[3], 6[1]
	for (int i = 2; i < 11; i++) {
		if (i & 1) {
			continue;
		}
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * ((i - 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 1) % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
		// draw shadow 268[0]
		if (i == 2) {
			for (int x = 0; x < MICRO_WIDTH; x++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					if (y > 49 - x) {
						unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
						BYTE color = gpBuffer[addr];
						if (color != TRANS_COLOR) {
							color = shadowColorCatacombs(color);
							gpBuffer[addr] = color;
						}
					}
				}
			}
		}
		// draw shadows 268[1], 152[0]
		if (i == 4 || i == 8) {
			for (int x = 0; x < MICRO_WIDTH; x++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
					BYTE color = gpBuffer[addr];
					if (color != TRANS_COLOR) {
						color = shadowColorCatacombs(color);
						gpBuffer[addr] = color;
					}
				}
			}
		}
		// draw shadow 148[1]
		if (i == 6) {
			for (int x = 0; x < MICRO_WIDTH; x++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					if (y > 22 - x / 2
					 || (x < 6 && y > 14)) { // extend the shadow to make the micro more usable
						unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
						BYTE color = gpBuffer[addr];
						if (color != TRANS_COLOR) {
							color = shadowColorCatacombs(color);
							gpBuffer[addr] = color;
						}
					}
				}
			}
		}
		// draw shadow 250[0]
		if (i == 10) {
			for (int x = 0; x < 5; x++) {
				for (int y = 0; y < 12; y++) {
					unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
					if (y < (4 - x) * 3) {
						BYTE color = gpBuffer[addr];
						color = shadowColorCatacombs(color);
						gpBuffer[addr] = color;
					}
				}
			}
		}
	}
	// fix shadow on 514[1] using 5[1]
	for (int i = 12; i < 13; i++) {
		for (int x = 26; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;              // 514[1]
				unsigned addr2 = x + MICRO_WIDTH * ((i - 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 1) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 5[1]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					color = shadowColorCatacombs(color);
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// fix shadow on 515[0]
	for (int i = 13; i < 14; i++) {
		for (int x = 20; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < 18; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR) {
					if (x < 26) {
						if (y > 37 - x) {
							continue;
						}
					} else if (x < 29) {
						if (y > 15 - x / 8) {
							continue;
						}
					} else {
						if (y > 40 - x) {
							continue;
						}
					}
					color = shadowColorCatacombs(color);
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// draw shadow 155[1]
	for (int i = 14; i < 15; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				if (y > 22 - x / 2) { // extend the shadow to make the micro more usable
					unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
					BYTE color = gpBuffer[addr];
					if (color != TRANS_COLOR) {
						color = shadowColorCatacombs(color);
						gpBuffer[addr] = color;
					}
				}
			}
		}
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

static BYTE* patchCatacombsFloorCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	const CelMicro micros[] = {
/*  0 */{ 323 - 1, 2, MET_TRANSPARENT }, // used to block subsequent calls
/*  1 */{ 134 - 1, 5, MET_SQUARE },     // change type
/*  2 */{ 283 - 1, 1, MET_RTRIANGLE },  // change type
/*  3 */{ 482 - 1, 1, MET_RTRIANGLE },  // change type

/*  4 */{ 17 - 1, 1, MET_TRANSPARENT }, // mask door
/*  5 */{ 17 - 1, 0, MET_TRANSPARENT }, // unused
/*  6 */{ 17 - 1, 2, MET_TRANSPARENT }, // unused
/*  7 */{ 17 - 1, 4, MET_TRANSPARENT }, // unused
/*  8 */{ 551 - 1, 0, MET_TRANSPARENT },
/*  9 */{ 551 - 1, 2, MET_TRANSPARENT },
/* 10 */{ 551 - 1, 4, MET_TRANSPARENT },
/* 11 */{ 551 - 1, 5, MET_TRANSPARENT },
/* 12 */{ 13 - 1, 0, MET_TRANSPARENT },
/* 13 */{ 13 - 1, 1, MET_TRANSPARENT }, // unused
/* 14 */{ 13 - 1, 3, MET_TRANSPARENT }, // unused
/* 15 */{ 13 - 1, 5, MET_TRANSPARENT }, // unused
/* 16 */{ 553 - 1, 1, MET_TRANSPARENT },
/* 17 */{ 553 - 1, 3, MET_TRANSPARENT },
/* 18 */{ 553 - 1, 4, MET_TRANSPARENT },
/* 19 */{ 553 - 1, 5, MET_TRANSPARENT },

/* 20 */{ 289 - 1, 0, MET_TRANSPARENT }, // mask column
/* 21 */{ 288 - 1, 1, MET_TRANSPARENT },
/* 22 */{ 287 - 1, 0, MET_LTRAPEZOID },
/* 23 */{ 21 - 1, 2, -1 },
/* 24 */{ 21 - 1, 3, -1 },
/* 25 */{ 21 - 1, 4, -1 },
/* 26 */{ 21 - 1, 5, -1 },
/* 27 */{ 287 - 1, 2, MET_TRANSPARENT },
/* 28 */{ 287 - 1, 3, MET_TRANSPARENT },
/* 29 */{ 287 - 1, 4, MET_TRANSPARENT },
/* 30 */{ 287 - 1, 5, MET_TRANSPARENT },

/* 31 */{ 323 - 1, 0, MET_LTRIANGLE }, // redraw floor
/* 32 */{ 323 - 1, 1, MET_RTRIANGLE },
/* 33 */{ 324 - 1, 0, MET_LTRIANGLE }, // unused
/* 34 */{ 324 - 1, 1, MET_RTRIANGLE }, // unused
/* 35 */{ 332 - 1, 0, MET_LTRIANGLE },
/* 36 */{ 332 - 1, 1, MET_RTRIANGLE },
/* 37 */{ 331 - 1, 0, MET_LTRIANGLE },
/* 38 */{ 331 - 1, 1, MET_RTRIANGLE },
/* 39 */{ 325 - 1, 0, MET_LTRIANGLE },
/* 40 */{ 325 - 1, 1, MET_RTRIANGLE },
/* 41 */{ 342 - 1, 0, MET_LTRIANGLE },
/* 42 */{ 342 - 1, 1, MET_RTRIANGLE },
/* 43 */{ 348 - 1, 0, MET_LTRIANGLE },
/* 44 */{ 348 - 1, 1, MET_RTRIANGLE },

// unify the columns
/* 45 */{ 267 - 1, 1, -1 },
/* 46 */{ 23 - 1, 1, MET_RTRIANGLE },
/* 47 */{ 135 - 1, 0, -1 },
/* 48 */{ 26 - 1, 0, MET_LTRIANGLE },
/* 49 */{ 21 - 1, 1, -1 },
/* 50 */{ 134 - 1, 1, MET_RTRAPEZOID },
/* 51 */{ 10 - 1, 1, -1 },
/* 52 */{ 135 - 1, 1, MET_RTRIANGLE },
/* 53 */{ 9 - 1, 0, -1 },
/* 54 */{ 146 - 1, 0, MET_LTRIANGLE },
/* 55 */{ 147 - 1, 0, -1 },
/* 56 */{ 167 - 1, 0, MET_LTRIANGLE },
/* 57 */{ 270 - 1, 1, MET_RTRIANGLE }, // change type
/* 58 */{ 271 - 1, 0, MET_LTRIANGLE }, // reduce shadow
	};

	const uint16_t* pSubtiles = (const uint16_t*)minBuf;
	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_L2;
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

	// mask 17[1]
	for (int i = 4; i < 5; i++) {
		for (int x = 8; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y < (x - 8) / 2 + 22) {
					gpBuffer[addr] = TRANS_COLOR; // 17[1]
				}
			}
		}
	}
	/*// mask 17[0]
	for (int i = 5; i < 6; i++) {
		for (int x = 19; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y < 15 - (x + 1) / 2) {
					gpBuffer[addr] = TRANS_COLOR; // 17[0]
				}
			}
		}
	}
	// mask 17[2]
	for (int i = 6; i < 7; i++) {
		for (int x = 19; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y != 31 || (x != 30 && x != 31)) {
					gpBuffer[addr] = TRANS_COLOR; // 17[2]
				}
			}
		}
	}
	// mask 17[4]
	for (int i = 7; i < 8; i++) {
		for (int x = 19; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr] = TRANS_COLOR; // 17[4]
			}
		}
	}*/
	// mask 551[0]
	for (int i = 8; i < 9; i++) {
		for (int x = 0; x < 21; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y < 20 - (x - 20) / 2) {
					gpBuffer[addr] = TRANS_COLOR; // 551[0]
				}
			}
		}
	}
	// mask 551[2]
	for (int i = 9; i < 10; i++) {
		for (int x = 0; x < 21; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr] = TRANS_COLOR; // 551[2]
			}
		}
	}
	// mask 551[4]
	for (int i = 10; i < 11; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (x < 21 || y < 22 - (x - 21) / 2) {
					gpBuffer[addr] = TRANS_COLOR; // 551[4]
				}
			}
		}
	}
	// mask 551[5]
	for (int i = 11; i < 12; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y < 17 - (x + 1) / 2) {
					gpBuffer[addr] = TRANS_COLOR; // 551[5]
				}
			}
		}
	}
	// mask 13[0]
	for (int i = 12; i < 13; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < 23; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				//if ((x < 21 && y < 30 - (x + 1) / 2) || (x > 24 && (y < (x + 1) / 2 - 12))) {
				if (x < 21 && y < 30 - (x + 1) / 2) {
					gpBuffer[addr] = TRANS_COLOR; // 13[0]
				}
			}
		}
	}
	/*// mask 13[1]
	for (int i = 13; i < 14; i++) {
		for (int x = 0; x < 10; x++) {
			for (int y = 0; y < 7; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y < 4 + x / 2 && (y != 6 || (x != 8 && x != 9))) {
					gpBuffer[addr] = TRANS_COLOR; // 13[1]
				}
			}
		}
	}
	// mask 13[3], 13[5]
	for (int i = 14; i < 16; i++) {
		for (int x = 0; x < 10; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}*/
	// mask 553[1]
	for (int i = 16; i < 17; i++) {
		for (int x = 8; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y < (x - 8) / 2 + 22) {
					gpBuffer[addr] = TRANS_COLOR; // 553[1]
				}
			}
		}
	}
	// mask 553[3]
	for (int i = 17; i < 18; i++) {
		for (int x = 8; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr] = TRANS_COLOR; // 553[3]
			}
		}
	}
	// mask 553[4]
	for (int i = 18; i < 19; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y < 2 + x / 2) {
					gpBuffer[addr] = TRANS_COLOR; // 553[4]
				}
			}
		}
	}
	// mask 553[5]
	for (int i = 19; i < 20; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (x > 7 || y < 18 + x / 2) {
					gpBuffer[addr] = TRANS_COLOR; // 553[5]
				}
			}
		}
	}

	// mask 289[0]
	for (int i = 20; i < 21; i++) {
		for (int x = 15; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < 9; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if ((y < 8 - (x - 14) / 2 && (x != 19 || y != 5) && (x != 20 || y != 4) && (x != 21 || y != 4) && (x != 22 || y != 3) && (x != 23 || y != 3) && (x != 27 || y != 1))
				 || (x == 16 && y == 7) || (x == 18 && y == 6)) {
					gpBuffer[addr] = TRANS_COLOR; // 289[0]
				}
			}
		}
	}
	// mask 288[1]
	for (int i = 21; i < 22; i++) {
		for (int x = 0; x < 17; x++) {
			for (int y = 0; y < 9; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if ((x < 9 && (y < x / 2 - 2 || (x == 3 && y == 0))) || (x >= 9 && y < x / 2)) {
					gpBuffer[addr] = TRANS_COLOR; // 288[1]
				}
			}
		}
	}
	// mask 287[2, 3, 4, 5] using 21[2, 3, 4, 5]
	for (int i = 27; i < 31; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * ((i - 4) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 4) % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (gpBuffer[addr2] == TRANS_COLOR) {
					if (i == 27 && ((x == 4 && ((y > 3 && y < 7) || (y > 10 && y < 16)))) || (x == 5 && y != 17)) {
						continue; // 287[2]
					}
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}

	// redraw 323[0]
	for (int i = 31; i < 32; i++) {
		for (int x = 4; x < MICRO_WIDTH; x++) {
			for (int y = 18 - x / 2; y > 12 - x / 2 && y >= 0; y--) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + 6 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr2] = gpBuffer[addr]; // 323[0]
				}
			}
		}
	}
	// redraw 323[1]
	for (int i = 32; i < 33; i++) {
		for (int x = 24; x < MICRO_WIDTH; x++) {
			for (int y = 13; y < 20; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x - 24 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y - 12 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (gpBuffer[addr] != TRANS_COLOR) {
					gpBuffer[addr2] = gpBuffer[addr]; // 323[1]
				}
			}
		}
	}
	// redraw 332[0]
	for (int i = 35; i < 36; i++) {
		// move border down
		for (int x = 6; x < MICRO_WIDTH - 3; x++) {
			for (int y = 10 + (x + 1) / 2; y < 11 + (x + 1) / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + 4 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr3 = x + 2 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y - 1 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr2] = gpBuffer[addr];
				gpBuffer[addr] = gpBuffer[addr3];
				if ((x & 1) == 0)
					gpBuffer[addr + 1] = gpBuffer[addr3 + 1];
			}
		}
		// fix artifacts
		{
			unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
			gpBuffer[addr +  3 + 16 * BUFFER_WIDTH] = 73;
			gpBuffer[addr +  4 + 16 * BUFFER_WIDTH] = 40;
			gpBuffer[addr +  5 + 17 * BUFFER_WIDTH] = 55;

			gpBuffer[addr + 30 + 29 * BUFFER_WIDTH] = 71;
			gpBuffer[addr + 31 + 30 * BUFFER_WIDTH] = 73;
            gpBuffer[addr + 29 + 25 * BUFFER_WIDTH] = 26;
            gpBuffer[addr + 31 + 25 * BUFFER_WIDTH] = 27;
            gpBuffer[addr + 31 + 26 * BUFFER_WIDTH] = 26;
            gpBuffer[addr + 28 + 29 * BUFFER_WIDTH] = 41;
            gpBuffer[addr + 26 + 25 * BUFFER_WIDTH] = 68;
		}
		// extend border
		for (int x = 0; x < 8; x++) {
			for (int y = 13; y < 20; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + 24 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y - 12 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (gpBuffer[addr] != TRANS_COLOR) {
					gpBuffer[addr2] = gpBuffer[addr];
				}
			}
		}
	}
	// redraw 332[1]
	for (int i = 36; i < 37; i++) {
		for (int x = 0; x < 30; x++) {
			for (int y = x / 2 + 2; y > x / 2 - 4 && y >= 0; y--) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + 6 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr2] = gpBuffer[addr]; // 332[1]
				}
			}
		}
		{
			unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
			gpBuffer[addr + 2 + 27 * BUFFER_WIDTH] = 25;
			gpBuffer[addr + 3 + 27 * BUFFER_WIDTH] = 26;
			gpBuffer[addr + 4 + 28 * BUFFER_WIDTH] = 25;
			gpBuffer[addr + 5 + 29 * BUFFER_WIDTH] = 26;
			gpBuffer[addr + 3 + 28 * BUFFER_WIDTH] = 26;
			gpBuffer[addr + 0 + 31 * BUFFER_WIDTH] = 41;
			gpBuffer[addr + 1 + 30 * BUFFER_WIDTH] = 68;
			gpBuffer[addr + 3 + 30 * BUFFER_WIDTH] = 68;
		}
	}
	// redraw 331[0]
	for (int i = 37; i < 38; i++) {
		for (int x = 4; x < MICRO_WIDTH; x++) {
			for (int y = 18 - x / 2; y > 12 - x / 2 && y >= 0; y--) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + 6 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr2] = gpBuffer[addr]; // 331[0]
				}
			}
		}
	}
	// redraw 331[1]
	for (int i = 38; i < 39; i++) {
		for (int x = 0; x < 30; x++) {
			for (int y = x / 2 + 2; y > x / 2 - 4 && y >= 0; y--) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + 6 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr2] = gpBuffer[addr]; // 331[1]
				}
			}
		}
	}
	// redraw 325[0]
	for (int i = 39; i < 40; i++) {
		for (int x = 4; x < MICRO_WIDTH; x++) {
			for (int y = 18 - x / 2; y > 12 - x / 2 && y >= 0; y--) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + 6 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr2] = gpBuffer[addr]; // 325[0]
				}
			}
		}
	}
	// redraw 325[1]
	for (int i = 40; i < 41; i++) {
		// reduce border on the right
		for (int x = 14; x < 25; x++) {
			for (int y = 16; y < 23; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y - 6 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (gpBuffer[addr2] != TRANS_COLOR && color != TRANS_COLOR && (color == 34 || color == 37 || (color > 51 && color < 60) || (color > 77 && color < 75))) {
					gpBuffer[addr] = gpBuffer[addr2]; // 325[1]
				}
			}
		}
		// extend border on top
		for (int x = 24; x < MICRO_WIDTH; x++) {
			for (int y = 13; y < 20; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x - 24 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y - 12 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (gpBuffer[addr] != TRANS_COLOR) {
					gpBuffer[addr2] = gpBuffer[addr]; // 325[1]
				}
			}
		}
	}
	// redraw 342[0] - move border on top
	for (int i = 41; i < 42; i++) {
		// remove border on the right
		for (int x = 26; x < MICRO_WIDTH; x++) {
			for (int y = 1; y < 7; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + 6 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr3 = (57 - x) + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + 3 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR && color != 34 && color != 37 && color != 68 && color != 70) {
					gpBuffer[addr2] = color; // 342[0]
					gpBuffer[addr3] = color; // 342[0]
				}
			}
		}
		// add border on top
		for (int x = 20; x < 26; x++) {
			for (int y = 4; y < 10; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + 6 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y - 3 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR) {
					gpBuffer[addr2] = color; // 342[0]
				}
			}
		}
		// reduce border on top
		for (int x = 20; x < 26; x++) {
			for (int y = 4; y < 11; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x - 8 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + 4 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr] = gpBuffer[addr2]; // 342[0]
			}
		}
	}
	// redraw 342[1] - move border on top
	for (int i = 42; i < 43; i++) {
		// remove border on the left
		for (int x = 0; x < 6; x++) {
			for (int y = 1; y < 7; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + 6 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr3 = (5 - x) + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + 3 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR && color != 34 && color != 39 && color != 54 && (color < 70 || color > 72)) {
					gpBuffer[addr2] = color; // 342[1]
					gpBuffer[addr3] = color; // 342[1]
				}
			}
		}
		// add border on top
		for (int x = 6; x < 12; x++) {
			for (int y = 4; y < 10; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x - 6 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y - 3 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR) {
					gpBuffer[addr2] = color; // 342[1]
				}
			}
		}
		// reduce border on top
		for (int x = 6; x < 12; x++) {
			for (int y = 4; y < 11; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + 8 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + 4 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr] = gpBuffer[addr2]; // 342[1]
			}
		}
	}
	// redraw 348[0]
	for (int i = 43; i < 44; i++) {
		// reduce border on the left
		for (int x = 8; x < 20; x++) {
			for (int y = 16; y < 23; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y - 7 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (gpBuffer[addr2] != TRANS_COLOR && color != TRANS_COLOR && (color == 35 || color == 37 || color == 39 || (color > 49 && color < 57) || (color > 66 && color < 72))) {
					gpBuffer[addr] = gpBuffer[addr2]; // 348[0]
				}
			}
		}
		// fix artifacts
		{
			unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
			gpBuffer[addr +  2 + 15 * BUFFER_WIDTH] = 35;
			gpBuffer[addr +  2 + 16 * BUFFER_WIDTH] = 37;
			gpBuffer[addr +  3 + 16 * BUFFER_WIDTH] = 39;
			gpBuffer[addr +  4 + 16 * BUFFER_WIDTH] = 50;
			gpBuffer[addr +  5 + 17 * BUFFER_WIDTH] = 54;
			gpBuffer[addr +  6 + 17 * BUFFER_WIDTH] = 56;
			gpBuffer[addr +  7 + 18 * BUFFER_WIDTH] = 55;
			gpBuffer[addr +  8 + 18 * BUFFER_WIDTH] = 54;
		}
		// extend border on top
		for (int x = 0; x < 6; x++) {
			for (int y = 13; y < 20; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + 24 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y - 12 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (gpBuffer[addr] != TRANS_COLOR) {
					gpBuffer[addr2] = gpBuffer[addr]; // 348[0]
				}
			}
		}
	}
	// redraw 348[1]
	for (int i = 44; i < 45; i++) {
		for (int x = 0; x < 30; x++) {
			for (int y = x / 2 + 2; y > x / 2 - 4 && y >= 0; y--) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + 6 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr2] = gpBuffer[addr]; // 348[1]
				}
			}
		}
	}
	// erase stone in 23[1] using 267[1]
	for (int i = 46; i < 47; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;              // 23[1]
				unsigned addr2 = x + MICRO_WIDTH * ((i - 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 1) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 267[1]
				if (x > 11 || y > 18) {
					gpBuffer[addr] = gpBuffer[addr2];
				}
			}
		}
	}
	// erase stone in 26[0] using 135[0]
	for (int i = 48; i < 49; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;              // 26[0]
				unsigned addr2 = x + MICRO_WIDTH * ((i - 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 1) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 135[0]
				if (x > 24 && y < 11) {
					gpBuffer[addr] = gpBuffer[addr2];
				}
			}
		}
	}
	// erase stone in 134[1] using 21[1]
	for (int i = 50; i < 51; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;              // 134[1]
				unsigned addr2 = x + MICRO_WIDTH * ((i - 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 1) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 21[1]
				if (x < 15 && y > 19) {
					gpBuffer[addr] = gpBuffer[addr2];
				}
			}
		}
	}
	// erase stone in 135[1] using 10[1]
	for (int i = 52; i < 53; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;              // 135[1]
				unsigned addr2 = x + MICRO_WIDTH * ((i - 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 1) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 10[1]
				if (x < 17 && y > 18) {
					gpBuffer[addr] = gpBuffer[addr2];
				}
			}
		}
	}
	// erase stone in 146[0] using 9[0]
	for (int i = 54; i < 55; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;              // 146[0]
				unsigned addr2 = x + MICRO_WIDTH * ((i - 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 1) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 9[0]
				if (x < 20) {
					gpBuffer[addr] = gpBuffer[addr2];
				}
			}
		}
	}
	// fix shadow in 167[0] using 147[0]
	for (int i = 56; i < 57; i++) {
		for (int x = 28; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;              // 167[0]
				unsigned addr2 = x + MICRO_WIDTH * ((i - 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 1) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 147[0]
				if (x == 28 && y < 20) {
					continue;
				}
				if (x == 29 && y < 21) {
					continue;
				}
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
	}
	// fix artifacts
	{ // 287[0]
		int i = 22;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  0 + 13 * BUFFER_WIDTH] = 62;
		gpBuffer[addr +  0 + 14 * BUFFER_WIDTH] = 76;
		gpBuffer[addr +  0 + 15 * BUFFER_WIDTH] = 77;
		gpBuffer[addr +  0 + 16 * BUFFER_WIDTH] = 77;
		gpBuffer[addr +  1 + 13 * BUFFER_WIDTH] = 62;
		gpBuffer[addr +  1 + 14 * BUFFER_WIDTH] = 77;
		gpBuffer[addr +  1 + 15 * BUFFER_WIDTH] = 59;
		gpBuffer[addr +  1 + 16 * BUFFER_WIDTH] = 78;
	}
	{ // 287[5]
		int i = 30;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 27 + 15 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 27 + 14 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 27 + 13 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 26 + 14 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 26 + 13 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 26 + 12 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 26 + 11 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 25 + 12 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 25 + 11 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 27 +  8 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 27 +  7 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 27 +  6 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 27 +  5 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 27 +  4 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 26 +  7 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 26 +  6 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 26 +  5 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // 271[0]
		int i = 58;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 30 +  1 * BUFFER_WIDTH] = 41;
		gpBuffer[addr + 31 +  1 * BUFFER_WIDTH] = 27;
		gpBuffer[addr + 31 +  2 * BUFFER_WIDTH] = 26;
		gpBuffer[addr + 31 +  3 * BUFFER_WIDTH] = 43;
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

BYTE* DRLP_L2_PatchCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	celBuf = patchCatacombsStairs(/*tilBuf, tilLen, */minBuf, minLen, celBuf, celLen, 72 - 1, 158 - 1, 267, 559, 265, 556);
	if (celBuf == NULL) {
		return NULL;
	}
	celBuf = patchCatacombsFloorCel(minBuf, minLen, celBuf, celLen);
	if (celBuf == NULL) {
		return NULL;
	}
	celBuf = fixCatacombsShadows(minBuf, minLen, celBuf, celLen);
	return celBuf;
}

void DRLP_L2_PatchMin(BYTE* buf)
{
	uint16_t* pSubtiles = (uint16_t*)buf;
	constexpr unsigned blockSize = BLOCK_SIZE_L2;

	// adjust the frame types
	// - after patchCatacombsFloor
	SetFrameType(134, 5, MET_SQUARE);
	SetFrameType(138, 5, MET_SQUARE);
	SetFrameType(143, 5, MET_SQUARE);
	
	SetFrameType(283, 1, MET_RTRIANGLE);
	SetFrameType(482, 1, MET_RTRIANGLE);

	SetFrameType(287, 2, MET_TRANSPARENT);
	SetFrameType(287, 4, MET_TRANSPARENT);
	SetFrameType(287, 5, MET_TRANSPARENT);

	SetFrameType(323, 0, MET_LTRIANGLE);
	SetFrameType(332, 1, MET_RTRIANGLE);
	SetFrameType(331, 0, MET_LTRIANGLE);
	SetFrameType(331, 1, MET_RTRIANGLE);
	SetFrameType(325, 0, MET_LTRIANGLE);
	SetFrameType(348, 1, MET_RTRIANGLE);

	// unify the columns
	SetFrameType(22, 1, MET_RTRIANGLE);
	// SetFrameType(26, 1, MET_RTRIANGLE);
	SetFrameType(135, 1, MET_RTRIANGLE);
	SetFrameType(270, 1, MET_RTRIANGLE);
	Blk2Mcr(22, 3);
	Blk2Mcr(26, 3);
	Blk2Mcr(132, 2);
	Blk2Mcr(270, 3);
	ReplaceMcr(25, 1, 21, 1);
	ReplaceMcr(26, 1, 10, 1);
	ReplaceMcr(132, 0, 23, 0);
	ReplaceMcr(132, 1, 23, 1);
	ReplaceMcr(135, 0, 26, 0);
	Blk2Mcr(22, 0);
	Blk2Mcr(22, 1);
	Blk2Mcr(35, 0);
	Blk2Mcr(35, 1);
	// TODO: add decorations 26[1], 22, 35
	// cleaned broken column
	ReplaceMcr(287, 6, 33, 6);
	ReplaceMcr(287, 7, 25, 7);
	Blk2Mcr(288, 3);
	Blk2Mcr(289, 8); // pointless from the beginning
	Blk2Mcr(289, 6);
	Blk2Mcr(289, 4);
	Blk2Mcr(289, 2);
	// separate subtiles for automap
	// - 55 := 21
	ReplaceMcr(55, 0, 33, 0);
	SetMcr(55, 1, 21, 1);
	ReplaceMcr(55, 2, 33, 2);
	SetMcr(55, 3, 25, 3);
	ReplaceMcr(55, 4, 33, 4);
	ReplaceMcr(55, 5, 25, 5);
	ReplaceMcr(55, 6, 33, 6);
	ReplaceMcr(55, 7, 25, 7);
	// - 269 := 21
	ReplaceMcr(269, 0, 33, 0);
	ReplaceMcr(269, 1, 21, 1);
	ReplaceMcr(269, 2, 33, 2);
	ReplaceMcr(269, 3, 25, 3);
	ReplaceMcr(269, 4, 33, 4);
	ReplaceMcr(269, 5, 25, 5);
	ReplaceMcr(269, 6, 33, 6);
	ReplaceMcr(269, 7, 25, 7);
	// - 48 := 45
	ReplaceMcr(48, 4, 45, 4);
	ReplaceMcr(48, 5, 45, 5);
	ReplaceMcr(48, 6, 45, 6);
	ReplaceMcr(48, 7, 45, 7);
	// - 50 := 45
	ReplaceMcr(50, 4, 45, 4);
	ReplaceMcr(50, 5, 45, 5);
	ReplaceMcr(50, 6, 45, 6);
	ReplaceMcr(50, 7, 45, 7);
	// - 53 := 45
	ReplaceMcr(53, 4, 45, 4);
	ReplaceMcr(53, 5, 45, 5);
	ReplaceMcr(53, 6, 45, 6);
	ReplaceMcr(53, 7, 45, 7);
	// - after fixCatacombsShadows
	if (pSubtiles[MICRO_IDX(151 - 1, blockSize, 0)] != 0) {
		Blk2Mcr(161, 0);
		MoveMcr(161, 0, 151, 0);
		Blk2Mcr(151, 1);

		SetFrameType(268, 0, MET_LTRAPEZOID);
		SetFrameType(268, 1, MET_RTRAPEZOID);
		HideMcr(24, 1);  // optional
		HideMcr(133, 1); // optional
		// SetFrameType(148, 1, MET_RTRIANGLE);
		SetFrameType(152, 0, MET_SQUARE);
		SetFrameType(250, 0, MET_RTRAPEZOID);
		// SetFrameType(514, 1, MET_RTRAPEZOID);
		// SetFrameType(515, 0, MET_LTRIANGLE);
		// SetFrameType(155, 1, MET_RTRIANGLE);

		ReplaceMcr(147, 1, 154, 1);
		ReplaceMcr(167, 1, 154, 1);

		ReplaceMcr(150, 0, 9, 0);
		SetMcr(150, 1, 9, 1);
		SetMcr(153, 0, 11, 0);
		ReplaceMcr(153, 1, 11, 1);
		ReplaceMcr(156, 0, 161, 0);
		SetMcr(156, 1, 161, 1);
		ReplaceMcr(158, 0, 166, 0);
		SetMcr(158, 1, 166, 1);
		SetMcr(165, 0, 167, 0);
		ReplaceMcr(165, 1, 167, 1);
		ReplaceMcr(164, 0, 147, 0);
		ReplaceMcr(164, 1, 147, 1);

		SetMcr(268, 2, 33, 2);
		SetMcr(268, 3, 29, 3);
		SetMcr(268, 4, 33, 4);
		SetMcr(268, 5, 29, 5);
		SetMcr(268, 6, 33, 6);
		SetMcr(268, 7, 29, 7);

		MoveMcr(515, 3, 152, 0);
		MoveMcr(515, 1, 250, 0);

		// extend the shadow to make the micro more usable
		SetMcr(148, 0, 155, 0);
	}
	// pointless door micros (re-drawn by dSpecial or the object)
	// - vertical doors	
	// Blk2Mcr(13, 2);
	ReplaceMcr(538, 0, 13, 0);
	ReplaceMcr(538, 1, 13, 1);
	ReplaceMcr(538, 2, 13, 2);
	ReplaceMcr(538, 3, 13, 3);
	// Blk2Mcr(538, 2);
	Blk2Mcr(538, 4);
	ReplaceMcr(538, 5, 13, 5);
	Blk2Mcr(538, 6);
	Blk2Mcr(538, 7);
	ReplaceMcr(537, 0, 13, 0);
	ReplaceMcr(537, 1, 13, 1);
	SetMcr(537, 2, 13, 2);
	SetMcr(537, 3, 13, 3);
	HideMcr(537, 4);
	SetMcr(537, 5, 13, 5);
	Blk2Mcr(537, 6);
	HideMcr(537, 7);
	// -- new vertical doors
	ReplaceMcr(172, 0, 178, 0);
	ReplaceMcr(172, 1, 178, 1);
	ReplaceMcr(172, 2, 178, 2);
	ReplaceMcr(172, 3, 178, 3);
	Blk2Mcr(172, 4);
	ReplaceMcr(172, 5, 178, 5);
	Blk2Mcr(172, 6);
	ReplaceMcr(173, 0, 178, 0);
	ReplaceMcr(173, 1, 178, 1);
	SetMcr(173, 2, 178, 2);
	SetMcr(173, 3, 178, 3);
	SetMcr(173, 5, 178, 5);
	// - horizontal doors
	// Blk2Mcr(17, 3);
	ReplaceMcr(540, 0, 17, 0);
	ReplaceMcr(540, 1, 17, 1);
	ReplaceMcr(540, 2, 17, 2);
	ReplaceMcr(540, 3, 17, 3);
	ReplaceMcr(540, 4, 17, 4);
	Blk2Mcr(540, 5);
	Blk2Mcr(540, 6);
	Blk2Mcr(540, 7);
	SetMcr(539, 0, 17, 0);
	ReplaceMcr(539, 1, 17, 1);
	SetMcr(539, 2, 17, 2);
	SetMcr(539, 3, 17, 3);
	SetMcr(539, 4, 17, 4);
	HideMcr(539, 6);
	// - reduce pointless bone-chamber complexity I.
	Blk2Mcr(323, 2);
	Blk2Mcr(325, 2);
	Blk2Mcr(331, 2);
	Blk2Mcr(331, 3);
	Blk2Mcr(332, 3);
	Blk2Mcr(348, 3);
	Blk2Mcr(326, 0);
	Blk2Mcr(326, 1);
	Blk2Mcr(333, 0);
	Blk2Mcr(333, 1);
	Blk2Mcr(333, 2);
	Blk2Mcr(340, 0);
	Blk2Mcr(340, 1);
	Blk2Mcr(341, 0);
	Blk2Mcr(341, 1);
	Blk2Mcr(347, 0);
	Blk2Mcr(347, 1);
	Blk2Mcr(347, 3);
	Blk2Mcr(350, 0);
	Blk2Mcr(350, 1);

	ReplaceMcr(324, 0, 339, 0);
	Blk2Mcr(334, 0);
	Blk2Mcr(334, 1);
	Blk2Mcr(339, 1);
	Blk2Mcr(349, 0);
	Blk2Mcr(349, 1);
	// pointless pixels
	Blk2Mcr(103, 6);
	Blk2Mcr(107, 6);
	Blk2Mcr(111, 2);
	Blk2Mcr(283, 3);
	Blk2Mcr(283, 7);
	Blk2Mcr(295, 6);
	Blk2Mcr(299, 4);
	Blk2Mcr(494, 6);
	Blk2Mcr(551, 7);
	Blk2Mcr(482, 3);
	Blk2Mcr(482, 7);
	Blk2Mcr(553, 6);
	// fix the upstairs III.
	if (pSubtiles[MICRO_IDX(265 - 1, blockSize, 3)] != 0) {
		// move the frames to the back subtile
		// - left side
		MoveMcr(252, 2, 265, 3);
		HideMcr(556, 3); // optional

		// - right side
		MoveMcr(252, 1, 265, 5);
		HideMcr(556, 5); // optional

		MoveMcr(252, 3, 267, 2);
		// Blk2Mcr(559, 2);

		MoveMcr(252, 5, 267, 4);
		// Blk2Mcr(559, 4);

		MoveMcr(252, 7, 267, 6);
		HideMcr(559, 6); // optional

		// - adjust the frame types
		SetFrameType(267, 0, MET_LTRIANGLE);
		SetFrameType(559, 0, MET_LTRIANGLE);
		SetFrameType(252, 3, MET_SQUARE);
		SetFrameType(252, 1, MET_RTRAPEZOID);
		SetFrameType(265, 1, MET_RTRIANGLE);
		SetFrameType(556, 1, MET_RTRIANGLE);
		SetFrameType(252, 0, MET_TRANSPARENT);
	}
	// fix bad artifact
	Blk2Mcr(288, 7);
	// fix graphical glitch
	Blk2Mcr(279, 7);
	// ReplaceMcr(548, 0, 99, 0);
	ReplaceMcr(552, 1, 244, 1);

	// reuse subtiles
	ReplaceMcr(27, 6, 3, 6);
	ReplaceMcr(62, 6, 3, 6);
	ReplaceMcr(66, 6, 3, 6);
	ReplaceMcr(78, 6, 3, 6);
	ReplaceMcr(82, 6, 3, 6);
	// ReplaceMcr(85, 6, 3, 6);
	ReplaceMcr(88, 6, 3, 6);
	// ReplaceMcr(92, 6, 3, 6);
	ReplaceMcr(96, 6, 3, 6);
	// ReplaceMcr(117, 6, 3, 6);
	// ReplaceMcr(120, 6, 3, 6);
	ReplaceMcr(129, 6, 3, 6);
	ReplaceMcr(132, 6, 3, 6);
	// ReplaceMcr(172, 6, 3, 6);
	ReplaceMcr(176, 6, 3, 6);
	ReplaceMcr(184, 6, 3, 6);
	// ReplaceMcr(236, 6, 3, 6);
	ReplaceMcr(240, 6, 3, 6);
	ReplaceMcr(244, 6, 3, 6);
	ReplaceMcr(277, 6, 3, 6);
	ReplaceMcr(285, 6, 3, 6);
	ReplaceMcr(305, 6, 3, 6);
	ReplaceMcr(416, 6, 3, 6);
	ReplaceMcr(420, 6, 3, 6);
	ReplaceMcr(480, 6, 3, 6);
	ReplaceMcr(484, 6, 3, 6);

	ReplaceMcr(27, 4, 3, 4);
	ReplaceMcr(62, 4, 3, 4);
	ReplaceMcr(78, 4, 3, 4);
	ReplaceMcr(82, 4, 3, 4);
	// ReplaceMcr(85, 4, 3, 4);
	ReplaceMcr(88, 4, 3, 4);
	// ReplaceMcr(92, 4, 3, 4);
	ReplaceMcr(96, 4, 3, 4);
	// ReplaceMcr(117, 4, 3, 4);
	// ReplaceMcr(120, 4, 3, 4);
	ReplaceMcr(129, 4, 3, 4);
	// ReplaceMcr(172, 4, 3, 4);
	ReplaceMcr(176, 4, 3, 4);
	// ReplaceMcr(236, 4, 66, 4);
	ReplaceMcr(240, 4, 3, 4);
	ReplaceMcr(244, 4, 66, 4);
	ReplaceMcr(277, 4, 66, 4);
	ReplaceMcr(281, 4, 3, 4);
	ReplaceMcr(285, 4, 3, 4);
	ReplaceMcr(305, 4, 3, 4);
	ReplaceMcr(480, 4, 3, 4);
	ReplaceMcr(552, 4, 3, 4);

	ReplaceMcr(27, 2, 3, 2);
	ReplaceMcr(62, 2, 3, 2);
	ReplaceMcr(78, 2, 3, 2);
	ReplaceMcr(82, 2, 3, 2);
	// ReplaceMcr(85, 2, 3, 2);
	ReplaceMcr(88, 2, 3, 2);
	// ReplaceMcr(92, 2, 3, 2);
	ReplaceMcr(96, 2, 3, 2);
	// ReplaceMcr(117, 2, 3, 2);
	ReplaceMcr(129, 2, 3, 2);
	// ReplaceMcr(172, 2, 3, 2);
	ReplaceMcr(176, 2, 3, 2);
	ReplaceMcr(180, 2, 3, 2);
	// ReplaceMcr(236, 2, 66, 2);
	ReplaceMcr(244, 2, 66, 2);
	ReplaceMcr(277, 2, 66, 2);
	ReplaceMcr(281, 2, 3, 2);
	ReplaceMcr(285, 2, 3, 2);
	ReplaceMcr(305, 2, 3, 2);
	ReplaceMcr(448, 2, 3, 2);
	ReplaceMcr(480, 2, 3, 2);
	ReplaceMcr(552, 2, 3, 2);

	ReplaceMcr(78, 0, 3, 0);
	ReplaceMcr(88, 0, 3, 0);
	// ReplaceMcr(92, 0, 3, 0);
	ReplaceMcr(96, 0, 62, 0);
	// ReplaceMcr(117, 0, 62, 0);
	// ReplaceMcr(120, 0, 62, 0);
	// ReplaceMcr(236, 0, 62, 0);
	ReplaceMcr(240, 0, 62, 0);
	ReplaceMcr(244, 0, 62, 0);
	ReplaceMcr(277, 0, 66, 0);
	ReplaceMcr(281, 0, 62, 0);
	ReplaceMcr(285, 0, 62, 0);
	ReplaceMcr(305, 0, 62, 0);
	ReplaceMcr(448, 0, 3, 0);
	ReplaceMcr(480, 0, 62, 0);
	ReplaceMcr(552, 0, 62, 0);

	// ReplaceMcr(85, 1, 82, 1);
	// ReplaceMcr(117, 1, 244, 1);
	// ReplaceMcr(120, 1, 244, 1);
	// ReplaceMcr(236, 1, 244, 1);
	ReplaceMcr(240, 1, 62, 1);
	ReplaceMcr(452, 1, 244, 1);
	// ReplaceMcr(539, 1, 15, 1);

	// TODO: ReplaceMcr(30, 7, 6, 7); ?
	ReplaceMcr(34, 7, 30, 7);
	ReplaceMcr(69, 7, 6, 7);
	ReplaceMcr(73, 7, 30, 7);
	ReplaceMcr(99, 7, 6, 7);
	ReplaceMcr(104, 7, 6, 7);
	ReplaceMcr(108, 7, 6, 7);
	ReplaceMcr(112, 7, 6, 7);
	ReplaceMcr(128, 7, 30, 7);
	ReplaceMcr(135, 7, 6, 7);
	// ReplaceMcr(139, 7, 6, 7);
	ReplaceMcr(187, 7, 6, 7);
	ReplaceMcr(191, 7, 6, 7);
	// ReplaceMcr(195, 7, 6, 7);
	ReplaceMcr(254, 7, 6, 7);
	ReplaceMcr(258, 7, 30, 7);
	ReplaceMcr(262, 7, 6, 7);
	ReplaceMcr(292, 7, 30, 7);
	ReplaceMcr(296, 7, 6, 7);
	ReplaceMcr(300, 7, 6, 7);
	ReplaceMcr(304, 7, 30, 7);
	ReplaceMcr(423, 7, 6, 7);
	ReplaceMcr(427, 7, 6, 7);
	ReplaceMcr(455, 7, 6, 7);
	ReplaceMcr(459, 7, 6, 7);
	ReplaceMcr(495, 7, 6, 7);
	ReplaceMcr(499, 7, 6, 7);

	ReplaceMcr(30, 5, 6, 5);
	ReplaceMcr(34, 5, 6, 5);
	ReplaceMcr(69, 5, 6, 5);
	ReplaceMcr(99, 5, 6, 5);
	ReplaceMcr(108, 5, 104, 5);
	ReplaceMcr(112, 5, 6, 5);
	ReplaceMcr(128, 5, 6, 5);
	ReplaceMcr(183, 5, 6, 5);
	ReplaceMcr(187, 5, 6, 5);
	ReplaceMcr(191, 5, 6, 5);
	// ReplaceMcr(195, 5, 6, 5);
	ReplaceMcr(254, 5, 6, 5);
	ReplaceMcr(258, 5, 73, 5);
	ReplaceMcr(262, 5, 6, 5);
	ReplaceMcr(292, 5, 73, 5);
	ReplaceMcr(296, 5, 6, 5);
	ReplaceMcr(300, 5, 6, 5);
	ReplaceMcr(304, 5, 6, 5);
	ReplaceMcr(455, 5, 6, 5);
	ReplaceMcr(459, 5, 6, 5);
	ReplaceMcr(499, 5, 6, 5);
	// ReplaceMcr(548, 5, 6, 5);

	ReplaceMcr(30, 3, 6, 3);
	ReplaceMcr(34, 3, 6, 3);
	ReplaceMcr(69, 3, 6, 3);
	ReplaceMcr(99, 3, 6, 3);
	ReplaceMcr(108, 3, 104, 3);
	ReplaceMcr(112, 3, 6, 3);
	ReplaceMcr(128, 3, 6, 3);
	ReplaceMcr(183, 3, 6, 3);
	ReplaceMcr(187, 3, 6, 3);
	ReplaceMcr(191, 3, 6, 3);
	// ReplaceMcr(195, 3, 6, 3);
	ReplaceMcr(254, 3, 6, 3);
	ReplaceMcr(258, 3, 73, 3);
	ReplaceMcr(262, 3, 6, 3);
	ReplaceMcr(292, 3, 73, 3);
	ReplaceMcr(296, 3, 6, 3);
	ReplaceMcr(300, 3, 6, 3);
	ReplaceMcr(304, 3, 6, 3);
	ReplaceMcr(455, 3, 6, 3);
	ReplaceMcr(459, 3, 6, 3);
	ReplaceMcr(499, 3, 6, 3);
	// ReplaceMcr(548, 3, 6, 3);

	ReplaceMcr(30, 1, 34, 1);
	ReplaceMcr(69, 1, 6, 1);
	ReplaceMcr(104, 1, 99, 1);
	ReplaceMcr(112, 1, 99, 1);
	ReplaceMcr(128, 1, 34, 1);
	ReplaceMcr(254, 1, 6, 1);
	ReplaceMcr(258, 1, 73, 1);
	ReplaceMcr(262, 1, 99, 1);
	ReplaceMcr(292, 1, 73, 1);
	ReplaceMcr(296, 1, 99, 1);
	ReplaceMcr(300, 1, 99, 1);
	ReplaceMcr(304, 1, 34, 1);
	ReplaceMcr(427, 1, 6, 1);
	ReplaceMcr(459, 1, 6, 1);
	ReplaceMcr(499, 1, 6, 1);
	// ReplaceMcr(548, 1, 6, 1);

	ReplaceMcr(1, 6, 60, 6);
	ReplaceMcr(21, 6, 33, 6);
	ReplaceMcr(29, 6, 25, 6);
	// ReplaceMcr(48, 6, 45, 6);
	// ReplaceMcr(50, 6, 45, 6);
	// ReplaceMcr(53, 6, 45, 6);
	ReplaceMcr(80, 6, 60, 6);
	ReplaceMcr(84, 6, 60, 6);
	ReplaceMcr(94, 6, 60, 6);
	ReplaceMcr(127, 6, 25, 6);
	ReplaceMcr(131, 6, 25, 6);
	ReplaceMcr(134, 6, 33, 6);
	ReplaceMcr(138, 6, 25, 6);
	ReplaceMcr(141, 6, 25, 6);
	ReplaceMcr(143, 6, 25, 6);
	ReplaceMcr(174, 6, 60, 6);
	ReplaceMcr(182, 6, 25, 6);
	ReplaceMcr(234, 6, 60, 6);
	ReplaceMcr(238, 6, 60, 6);
	ReplaceMcr(242, 6, 60, 6);
	ReplaceMcr(275, 6, 60, 6);
	ReplaceMcr(279, 6, 60, 6);
	ReplaceMcr(283, 6, 60, 6);
	ReplaceMcr(303, 6, 25, 6);
	ReplaceMcr(414, 6, 60, 6);
	ReplaceMcr(418, 6, 60, 6);
	ReplaceMcr(446, 6, 60, 6);
	ReplaceMcr(450, 6, 60, 6);
	ReplaceMcr(478, 6, 60, 6);
	ReplaceMcr(482, 6, 60, 6);
	ReplaceMcr(510, 6, 60, 6);

	ReplaceMcr(21, 4, 33, 4);
	ReplaceMcr(29, 4, 25, 4);
	ReplaceMcr(60, 4, 1, 4);
	ReplaceMcr(94, 4, 1, 4);
	ReplaceMcr(102, 4, 98, 4);
	ReplaceMcr(127, 4, 25, 4);
	ReplaceMcr(134, 4, 33, 4);
	ReplaceMcr(143, 4, 25, 4);
	ReplaceMcr(174, 4, 1, 4);
	ReplaceMcr(182, 4, 25, 4);
	ReplaceMcr(238, 4, 1, 4);
	ReplaceMcr(242, 4, 64, 4);
	ReplaceMcr(275, 4, 64, 4);
	ReplaceMcr(418, 4, 1, 4);

	ReplaceMcr(21, 2, 33, 2);
	ReplaceMcr(29, 2, 25, 2);
	ReplaceMcr(60, 2, 1, 2);
	ReplaceMcr(94, 2, 1, 2);
	ReplaceMcr(102, 2, 98, 2);
	ReplaceMcr(107, 2, 103, 2);
	ReplaceMcr(127, 2, 25, 2);
	ReplaceMcr(134, 2, 33, 2);
	ReplaceMcr(141, 2, 131, 2);
	ReplaceMcr(143, 2, 25, 2);
	ReplaceMcr(174, 2, 1, 2);
	ReplaceMcr(182, 2, 25, 2);

	ReplaceMcr(21, 0, 33, 0);
	ReplaceMcr(29, 0, 25, 0);
	ReplaceMcr(84, 0, 80, 0);
	ReplaceMcr(127, 0, 25, 0);
	ReplaceMcr(131, 0, 33, 0);
	ReplaceMcr(134, 0, 33, 0);
	ReplaceMcr(138, 0, 33, 0);
	ReplaceMcr(141, 0, 33, 0);
	ReplaceMcr(143, 0, 25, 0);
	ReplaceMcr(182, 0, 25, 0);
	ReplaceMcr(234, 0, 64, 0);
	ReplaceMcr(446, 0, 1, 0);
	ReplaceMcr(450, 0, 1, 0);
	ReplaceMcr(478, 0, 283, 0);

	ReplaceMcr(84, 1, 80, 1);
	ReplaceMcr(127, 1, 33, 1);
	ReplaceMcr(234, 1, 64, 1);
	ReplaceMcr(253, 1, 111, 1);
	ReplaceMcr(454, 1, 68, 1);
	ReplaceMcr(458, 1, 111, 1);

	ReplaceMcr(21, 7, 25, 7);
	ReplaceMcr(131, 7, 25, 7);
	ReplaceMcr(266, 7, 25, 7);
	ReplaceMcr(33, 7, 29, 7);
	ReplaceMcr(98, 7, 5, 7);
	ReplaceMcr(102, 7, 5, 7);
	ReplaceMcr(103, 7, 5, 7);
	ReplaceMcr(107, 7, 5, 7);
	ReplaceMcr(111, 7, 5, 7);
	ReplaceMcr(127, 7, 29, 7);
	ReplaceMcr(134, 7, 29, 7);
	ReplaceMcr(138, 7, 29, 7);
	ReplaceMcr(141, 7, 29, 7);
	ReplaceMcr(143, 7, 29, 7);
	ReplaceMcr(295, 7, 5, 7);
	ReplaceMcr(299, 7, 5, 7);
	ReplaceMcr(494, 7, 5, 7);
	ReplaceMcr(68, 7, 5, 7);
	ReplaceMcr(72, 7, 5, 7);
	ReplaceMcr(186, 7, 5, 7);
	ReplaceMcr(190, 7, 5, 7);
	ReplaceMcr(253, 7, 5, 7);
	ReplaceMcr(257, 7, 5, 7);
	ReplaceMcr(261, 7, 5, 7);
	ReplaceMcr(291, 7, 5, 7);
	ReplaceMcr(422, 7, 5, 7);
	ReplaceMcr(426, 7, 5, 7);
	ReplaceMcr(454, 7, 5, 7);
	ReplaceMcr(458, 7, 5, 7);
	ReplaceMcr(498, 7, 5, 7);

	ReplaceMcr(21, 5, 25, 5);
	ReplaceMcr(33, 5, 29, 5);
	ReplaceMcr(111, 5, 5, 5);
	ReplaceMcr(127, 5, 29, 5);
	ReplaceMcr(131, 5, 25, 5);
	ReplaceMcr(141, 5, 29, 5);
	ReplaceMcr(299, 5, 5, 5);
	ReplaceMcr(68, 5, 5, 5);
	ReplaceMcr(186, 5, 5, 5);
	ReplaceMcr(190, 5, 5, 5);
	ReplaceMcr(253, 5, 5, 5);
	ReplaceMcr(257, 5, 72, 5);
	ReplaceMcr(266, 5, 25, 5);
	ReplaceMcr(422, 5, 5, 5);
	ReplaceMcr(454, 5, 5, 5);
	ReplaceMcr(458, 5, 5, 5);

	ReplaceMcr(21, 3, 25, 3);
	ReplaceMcr(33, 3, 29, 3);
	ReplaceMcr(111, 3, 5, 3);
	ReplaceMcr(127, 3, 29, 3);
	ReplaceMcr(131, 3, 25, 3);
	ReplaceMcr(141, 3, 29, 3);
	ReplaceMcr(68, 3, 5, 3);
	ReplaceMcr(186, 3, 5, 3);
	ReplaceMcr(190, 3, 5, 3);
	ReplaceMcr(266, 3, 25, 3);
	ReplaceMcr(454, 3, 5, 3);
	ReplaceMcr(458, 3, 5, 3);
	ReplaceMcr(514, 3, 5, 3);

	ReplaceMcr(28, 1, 12, 1); // lost details
	ReplaceMcr(36, 0, 12, 0); // lost details
	ReplaceMcr(61, 1, 10, 1); // lost details
	ReplaceMcr(63, 1, 12, 1); // lost details
	ReplaceMcr(65, 1, 10, 1); // lost details
	ReplaceMcr(67, 1, 12, 1); // lost details
	ReplaceMcr(74, 0, 11, 0); // lost details
	ReplaceMcr(75, 0, 12, 0); // lost details
	ReplaceMcr(77, 1, 10, 1); // lost details
	ReplaceMcr(79, 1, 12, 1); // lost details
	ReplaceMcr(87, 1, 10, 1); // lost details
	ReplaceMcr(83, 1, 12, 1); // lost details
	ReplaceMcr(89, 1, 12, 1); // lost details
	ReplaceMcr(91, 1, 10, 1); // lost details
	ReplaceMcr(93, 1, 12, 1); // lost details
	ReplaceMcr(105, 0, 11, 0); // lost details
	ReplaceMcr(113, 0, 11, 0); // lost details
	// ReplaceMcr(136, 1, 23, 1); // lost details
	ReplaceMcr(239, 1, 10, 1); // lost details
	ReplaceMcr(241, 1, 12, 1); // lost details
	ReplaceMcr(245, 1, 4, 1); // lost details
	ReplaceMcr(248, 0, 11, 0); // lost details
	ReplaceMcr(260, 0, 12, 0); // lost details
	ReplaceMcr(263, 0, 11, 0); // lost details
	ReplaceMcr(293, 0, 11, 0); // lost details
	ReplaceMcr(273, 1, 10, 1); // lost details
	ReplaceMcr(301, 0, 11, 0); // lost details
	ReplaceMcr(371, 1, 9, 1); // lost details
	ReplaceMcr(373, 1, 11, 1); // lost details
	ReplaceMcr(377, 0, 11, 0); // lost details
	ReplaceMcr(380, 1, 10, 1); // lost details
	ReplaceMcr(383, 1, 9, 1); // lost details
	ReplaceMcr(408, 0, 11, 0); // lost details
	ReplaceMcr(411, 1, 10, 1); // lost details
	ReplaceMcr(419, 1, 10, 1); // lost details
	ReplaceMcr(431, 1, 10, 1); // lost details
	ReplaceMcr(436, 0, 11, 0); // lost details
	ReplaceMcr(443, 1, 10, 1); // lost details
	ReplaceMcr(451, 1, 10, 1); // lost details
	ReplaceMcr(456, 0, 11, 0); // lost details
	ReplaceMcr(468, 0, 11, 0); // lost details
	ReplaceMcr(471, 1, 10, 1); // lost details
	ReplaceMcr(490, 1, 9, 1); // lost details
	ReplaceMcr(508, 0, 11, 0); // lost details
	ReplaceMcr(510, 1, 1, 1); // lost details
	ReplaceMcr(544, 1, 10, 1); // lost details
	ReplaceMcr(546, 1, 16, 1); // lost details
	ReplaceMcr(549, 0, 11, 0); // lost details
	ReplaceMcr(550, 0, 12, 0); // lost details

	// eliminate micros of unused subtiles
	// Blk2Mcr(554,  ...),
	Blk2Mcr(24, 0);
	Blk2Mcr(31, 0);
	Blk2Mcr(31, 1);
	Blk2Mcr(31, 2);
	Blk2Mcr(31, 4);
	Blk2Mcr(31, 6);
	Blk2Mcr(46, 5);
	Blk2Mcr(46, 6);
	Blk2Mcr(46, 7);
	Blk2Mcr(47, 4);
	Blk2Mcr(47, 6);
	Blk2Mcr(47, 7);
	Blk2Mcr(49, 4);
	Blk2Mcr(49, 6);
	Blk2Mcr(49, 7);
	Blk2Mcr(51, 5);
	Blk2Mcr(51, 6);
	Blk2Mcr(51, 7);
	Blk2Mcr(52, 7);
	Blk2Mcr(54, 6);
	Blk2Mcr(81, 0);
	Blk2Mcr(81, 1);
	Blk2Mcr(85, 1);
	Blk2Mcr(85, 2);
	Blk2Mcr(85, 4);
	Blk2Mcr(85, 6);
	Blk2Mcr(92, 0);
	Blk2Mcr(92, 1);
	Blk2Mcr(92, 2);
	Blk2Mcr(92, 4);
	Blk2Mcr(92, 6);
	Blk2Mcr(115, 0);
	Blk2Mcr(115, 1);
	Blk2Mcr(115, 2);
	Blk2Mcr(115, 3);
	Blk2Mcr(115, 4);
	Blk2Mcr(115, 5);
	Blk2Mcr(119, 0);
	Blk2Mcr(119, 1);
	Blk2Mcr(119, 2);
	Blk2Mcr(119, 4);
	Blk2Mcr(121, 0);
	Blk2Mcr(121, 1);
	Blk2Mcr(121, 2);
	Blk2Mcr(121, 3);
	Blk2Mcr(121, 4);
	Blk2Mcr(121, 5);
	Blk2Mcr(121, 6);
	Blk2Mcr(121, 7);
	Blk2Mcr(125, 0);
	Blk2Mcr(125, 1);
	Blk2Mcr(125, 3);
	Blk2Mcr(125, 5);
	Blk2Mcr(125, 7);
	Blk2Mcr(133, 0);
	Blk2Mcr(136, 1);
	Blk2Mcr(139, 0);
	Blk2Mcr(139, 1);
	Blk2Mcr(139, 7);
	Blk2Mcr(142, 0);
	Blk2Mcr(144, 1);
	Blk2Mcr(144, 2);
	Blk2Mcr(144, 4);
	Blk2Mcr(144, 6);
	// reused for the new shadows
	// Blk2Mcr(148, 1);
	// Blk2Mcr(150, 0);
	// Blk2Mcr(151, 0);
	// Blk2Mcr(151, 1);
	// Blk2Mcr(153, 1);
	// Blk2Mcr(156, 0);
	// Blk2Mcr(158, 0);
	// Blk2Mcr(164, 0);
	// Blk2Mcr(164, 1);
	// Blk2Mcr(165, 1);
	// Blk2Mcr(268, 0);
	// Blk2Mcr(268, 1);
	Blk2Mcr(152, 0);
	Blk2Mcr(250, 0);
	Blk2Mcr(251, 0);
	Blk2Mcr(251, 1);
	Blk2Mcr(265, 1);
	// Blk2Mcr(269, 0);
	// Blk2Mcr(269, 1);
	// Blk2Mcr(269, 2);
	// Blk2Mcr(269, 3);
	// Blk2Mcr(269, 4);
	// Blk2Mcr(269, 5);
	// Blk2Mcr(269, 6);
	// Blk2Mcr(269, 7);
	Blk2Mcr(365, 1);
	Blk2Mcr(395, 1);
	Blk2Mcr(513, 0);
	Blk2Mcr(517, 1);
	Blk2Mcr(519, 0);
	Blk2Mcr(520, 0);
	Blk2Mcr(520, 1);
	Blk2Mcr(521, 0);
	Blk2Mcr(521, 1);
	Blk2Mcr(522, 0);
	Blk2Mcr(522, 1);
	Blk2Mcr(523, 0);
	Blk2Mcr(523, 1);
	Blk2Mcr(524, 0);
	Blk2Mcr(524, 1);
	Blk2Mcr(525, 0);
	Blk2Mcr(525, 1);
	Blk2Mcr(526, 0);
	Blk2Mcr(526, 1);
	Blk2Mcr(527, 0);
	Blk2Mcr(527, 1);
	Blk2Mcr(528, 0);
	Blk2Mcr(528, 1);
	Blk2Mcr(529, 0);
	Blk2Mcr(529, 1);
	Blk2Mcr(529, 5);
	Blk2Mcr(529, 6);
	Blk2Mcr(529, 7);
	Blk2Mcr(530, 0);
	Blk2Mcr(530, 1);
	Blk2Mcr(530, 4);
	Blk2Mcr(530, 6);
	Blk2Mcr(530, 7);
	Blk2Mcr(532, 0);
	Blk2Mcr(532, 1);
	Blk2Mcr(532, 4);
	Blk2Mcr(532, 6);
	Blk2Mcr(532, 7);
	Blk2Mcr(534, 0);
	Blk2Mcr(534, 1);
	Blk2Mcr(534, 5);
	Blk2Mcr(534, 6);
	Blk2Mcr(534, 7);
	Blk2Mcr(535, 0);
	Blk2Mcr(535, 1);
	Blk2Mcr(535, 7);
	Blk2Mcr(542, 0);
	Blk2Mcr(542, 2);
	Blk2Mcr(542, 3);
	Blk2Mcr(542, 4);
	Blk2Mcr(542, 5);
	Blk2Mcr(542, 6);
	Blk2Mcr(542, 7);
	Blk2Mcr(543, 0);
	Blk2Mcr(543, 1);
	Blk2Mcr(543, 2);
	Blk2Mcr(543, 3);
	Blk2Mcr(543, 4);
	Blk2Mcr(543, 5);
	Blk2Mcr(543, 6);
	Blk2Mcr(543, 7);
	Blk2Mcr(545, 0);
	Blk2Mcr(545, 1);
	Blk2Mcr(545, 2);
	Blk2Mcr(545, 4);
	Blk2Mcr(547, 0);
	Blk2Mcr(547, 1);
	Blk2Mcr(547, 2);
	Blk2Mcr(547, 3);
	Blk2Mcr(547, 4);
	Blk2Mcr(547, 5);
	Blk2Mcr(547, 6);
	Blk2Mcr(547, 7);
	Blk2Mcr(548, 0);
	Blk2Mcr(548, 1);
	Blk2Mcr(548, 3);
	Blk2Mcr(548, 5);
	Blk2Mcr(554, 0);
	Blk2Mcr(554, 1);
	Blk2Mcr(555, 1);
	Blk2Mcr(556, 0);
	Blk2Mcr(556, 1);
	Blk2Mcr(557, 1);
	Blk2Mcr(558, 0);
	Blk2Mcr(558, 1);
	Blk2Mcr(558, 2);
	Blk2Mcr(558, 3);
	Blk2Mcr(558, 4);
	Blk2Mcr(558, 5);
	Blk2Mcr(558, 7);
	Blk2Mcr(559, 2);
	Blk2Mcr(559, 4);
	const int unusedSubtiles[] = {
		2, 7, 14, 19, 20, 56, 57, 58, 59, 70, 71, 106, 109, 110, 116, 117, 118, 120, 122, 123, 124, 126, 137, 140, 145, 149, 157, 159, 160, 168, 170, 171, 192, 193, 194, 195, 196, 197, 198, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 235, 236, 243, 246, 247, 255, 256, 264, 327, 328, 329, 330, 335, 336, 337, 338, 343, 344, 345, 346, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 366, 367, 368, 369, 370, 376, 391, 397, 400, 434, 487, 489, 491, 493, 504, 505, 507, 509, 511, 516, 518, 531, 533, 536, 541,
	};

	for (int n = 0; n < lengthof(unusedSubtiles); n++) {
		for (int i = 0; i < blockSize; i++) {
			Blk2Mcr(unusedSubtiles[n], i);
		}
	}
}

void DRLP_L2_PatchTil(BYTE* buf)
{
	uint16_t* pTiles = (uint16_t*)buf;
	// unified columns
	pTiles[(6 - 1) * 4 + 1] = SwapLE16(26 - 1);  // 22
	pTiles[(6 - 1) * 4 + 3] = SwapLE16(12 - 1);  // 24
	pTiles[(39 - 1) * 4 + 1] = SwapLE16(26 - 1); // 22
	pTiles[(39 - 1) * 4 + 3] = SwapLE16(12 - 1); // 133
	pTiles[(40 - 1) * 4 + 3] = SwapLE16(12 - 1); // 137
	pTiles[(41 - 1) * 4 + 3] = SwapLE16(12 - 1); // 140
	pTiles[(77 - 1) * 4 + 1] = SwapLE16(26 - 1); // 22
	pTiles[(9 - 1) * 4 + 2] = SwapLE16(23 - 1);  // 35
	pTiles[(50 - 1) * 4 + 3] = SwapLE16(12 - 1); // 165
	// use common subtile
	pTiles[(13 - 1) * 4 + 1] = SwapLE16(42 - 1); // 46
	pTiles[(13 - 1) * 4 + 2] = SwapLE16(39 - 1); // 47
	// pTiles[(14 - 1) * 4 + 0] = SwapLE16(45 - 1); // 48
	pTiles[(14 - 1) * 4 + 2] = SwapLE16(39 - 1); // 49
	// pTiles[(15 - 1) * 4 + 0] = SwapLE16(45 - 1); // 50
	pTiles[(15 - 1) * 4 + 1] = SwapLE16(42 - 1); // 51
	pTiles[(15 - 1) * 4 + 2] = SwapLE16(43 - 1); // 52
	// pTiles[(16 - 1) * 4 + 0] = SwapLE16(45 - 1); // 53
	pTiles[(16 - 1) * 4 + 1] = SwapLE16(38 - 1); // 54
	pTiles[(16 - 1) * 4 + 2] = SwapLE16(43 - 1); // 52
	pTiles[(24 - 1) * 4 + 1] = SwapLE16(77 - 1); // 81
	pTiles[(25 - 1) * 4 + 1] = SwapLE16(77 - 1);
	pTiles[(25 - 1) * 4 + 2] = SwapLE16(82 - 1); // 85
	pTiles[(27 - 1) * 4 + 2] = SwapLE16(78 - 1); // 92
	pTiles[(40 - 1) * 4 + 2] = SwapLE16(23 - 1); // 136
	pTiles[(41 - 1) * 4 + 1] = SwapLE16(135 - 1); // 139
	// pTiles[(41 - 1) * 4 + 3] = SwapLE16(137 - 1); // 140
	pTiles[(43 - 1) * 4 + 2] = SwapLE16(27 - 1); // 144
	pTiles[(43 - 1) * 4 + 3] = SwapLE16(28 - 1); // 145
	pTiles[(150 - 1) * 4 + 2] = SwapLE16(15 - 1); // 539
	pTiles[(45 - 1) * 4 + 2] = SwapLE16(11 - 1); // 148
	pTiles[(45 - 1) * 4 + 3] = SwapLE16(12 - 1); // 149
	pTiles[(46 - 1) * 4 + 0] = SwapLE16(9 - 1); // 150
	//pTiles[(47 - 1) * 4 + 0] = SwapLE16(9 - 1); // 153
	//pTiles[(47 - 1) * 4 + 3] = SwapLE16(152 - 1); // 156
	//pTiles[(48 - 1) * 4 + 2] = SwapLE16(155 - 1); // 157
	//pTiles[(48 - 1) * 4 + 3] = SwapLE16(156 - 1); // 158
	//pTiles[(49 - 1) * 4 + 0] = SwapLE16(9 - 1); // 159
	//pTiles[(49 - 1) * 4 + 1] = SwapLE16(10 - 1); // 160
	pTiles[(68 - 1) * 4 + 1] = SwapLE16(10 - 1); // 235
	pTiles[(70 - 1) * 4 + 1] = SwapLE16(10 - 1); // 243
	pTiles[(71 - 1) * 4 + 0] = SwapLE16(9 - 1); // 246
	pTiles[(71 - 1) * 4 + 1] = SwapLE16(10 - 1); // 247
	pTiles[(77 - 1) * 4 + 3] = SwapLE16(12 - 1); // 268
	pTiles[(140 - 1) * 4 + 1] = SwapLE16(10 - 1); // 511
	pTiles[(140 - 1) * 4 + 3] = SwapLE16(162 - 1); // 513
	pTiles[(142 - 1) * 4 + 3] = SwapLE16(162 - 1); // 519
	// - doors
	pTiles[(4 - 1) * 4 + 0] = SwapLE16(537 - 1); // - to make 537 'accessible'
	pTiles[(5 - 1) * 4 + 0] = SwapLE16(539 - 1); // - to make 539 'accessible'
	pTiles[(54 - 1) * 4 + 0] = SwapLE16(173 - 1); // - to make 173 'accessible'
	pTiles[(58 - 1) * 4 + 0] = SwapLE16(172 - 1); // - to make 172 'accessible'
	pTiles[(58 - 1) * 4 + 1] = SwapLE16(179 - 1); // - to make 172 'accessible'
	pTiles[(58 - 1) * 4 + 2] = SwapLE16(180 - 1); // - to make 172 'accessible'
	pTiles[(58 - 1) * 4 + 3] = SwapLE16(181 - 1); // - to make 172 'accessible'
	// use common subtiles instead of minor alterations
	pTiles[(1 - 1) * 4 + 1] = SwapLE16(10 - 1);  // 2
	pTiles[(2 - 1) * 4 + 2] = SwapLE16(11 - 1);  // 7
	pTiles[(4 - 1) * 4 + 1] = SwapLE16(10 - 1);  // 14
	pTiles[(150 - 1) * 4 + 1] = SwapLE16(10 - 1);
	// pTiles[(152 - 1) * 4 + 1] = SwapLE16(10 - 1);
	pTiles[(5 - 1) * 4 + 2] = SwapLE16(11 - 1);  // 19
	pTiles[(151 - 1) * 4 + 2] = SwapLE16(11 - 1);
	// pTiles[(153 - 1) * 4 + 2] = SwapLE16(11 - 1);
	pTiles[(5 - 1) * 4 + 3] = SwapLE16(12 - 1); // 20
	pTiles[(151 - 1) * 4 + 3] = SwapLE16(12 - 1);
	// pTiles[(153 - 1) * 4 + 3] = SwapLE16(12 - 1);
	pTiles[(21 - 1) * 4 + 2] = SwapLE16(11 - 1);  // 70
	pTiles[(21 - 1) * 4 + 3] = SwapLE16(12 - 1); // 71
	pTiles[(31 - 1) * 4 + 3] = SwapLE16(12 - 1); // 106
	pTiles[(32 - 1) * 4 + 2] = SwapLE16(11 - 1);  // 109
	pTiles[(32 - 1) * 4 + 3] = SwapLE16(12 - 1); // 110
	pTiles[(42 - 1) * 4 + 3] = SwapLE16(12 - 1); // 142
	pTiles[(8 - 1) * 4 + 2] = SwapLE16(15 - 1);  // 31
	pTiles[(68 - 1) * 4 + 2] = SwapLE16(244 - 1); // 236
	pTiles[(73 - 1) * 4 + 2] = SwapLE16(11 - 1);  // 255
	pTiles[(73 - 1) * 4 + 3] = SwapLE16(12 - 1);  // 256
	pTiles[(75 - 1) * 4 + 3] = SwapLE16(12 - 1);  // 264
	// pTiles[(78 - 1) * 4 + 0] = SwapLE16(21 - 1); // 269
	pTiles[(106 - 1) * 4 + 1] = SwapLE16(10 - 1); // 376
	pTiles[(110 - 1) * 4 + 0] = SwapLE16(9 - 1); // 391
	pTiles[(111 - 1) * 4 + 0] = SwapLE16(9 - 1); // 395
	pTiles[(111 - 1) * 4 + 3] = SwapLE16(12 - 1); // 397
	pTiles[(121 - 1) * 4 + 0] = SwapLE16(9 - 1); // 434
	pTiles[(112 - 1) * 4 + 2] = SwapLE16(11 - 1); // 400
	pTiles[(138 - 1) * 4 + 2] = SwapLE16(11 - 1); // 504
	pTiles[(138 - 1) * 4 + 3] = SwapLE16(12 - 1); // 505
	pTiles[(139 - 1) * 4 + 1] = SwapLE16(10 - 1); // 507
	pTiles[(139 - 1) * 4 + 3] = SwapLE16(12 - 1); // 509
	pTiles[(134 - 1) * 4 + 1] = SwapLE16(10 - 1); // 487
	pTiles[(134 - 1) * 4 + 3] = SwapLE16(12 - 1); // 489
	pTiles[(135 - 1) * 4 + 1] = SwapLE16(10 - 1); // 491
	pTiles[(135 - 1) * 4 + 3] = SwapLE16(12 - 1); // 493
	pTiles[(51 - 1) * 4 + 2] = SwapLE16(155 - 1); // 168
	pTiles[(141 - 1) * 4 + 2] = SwapLE16(155 - 1); // 516
	pTiles[(141 - 1) * 4 + 3] = SwapLE16(169 - 1); // 517
	pTiles[(142 - 1) * 4 + 2] = SwapLE16(155 - 1); // 518

	// - reduce pointless bone-chamber complexity II.
	// -- bones
	pTiles[(54 - 1) * 4 + 1] = SwapLE16(181 - 1); // '179'
	pTiles[(54 - 1) * 4 + 3] = SwapLE16(185 - 1); // '181'
	pTiles[(53 - 1) * 4 + 1] = SwapLE16(179 - 1); // '175'
	pTiles[(53 - 1) * 4 + 3] = SwapLE16(189 - 1); // '177'
	pTiles[(56 - 1) * 4 + 2] = SwapLE16(179 - 1); // '188'
	pTiles[(56 - 1) * 4 + 3] = SwapLE16(181 - 1); // '189'
	pTiles[(57 - 1) * 4 + 2] = SwapLE16(177 - 1); // 192
	pTiles[(57 - 1) * 4 + 3] = SwapLE16(179 - 1); // 193

	pTiles[(59 - 1) * 4 + 0] = SwapLE16(177 - 1); // 198
	pTiles[(59 - 1) * 4 + 1] = SwapLE16(185 - 1); // 199
	pTiles[(59 - 1) * 4 + 2] = SwapLE16(199 - 1); // 200
	pTiles[(59 - 1) * 4 + 3] = SwapLE16(185 - 1); // 201

	pTiles[(60 - 1) * 4 + 0] = SwapLE16(188 - 1); // 202
	pTiles[(60 - 1) * 4 + 1] = SwapLE16(185 - 1); // 203
	pTiles[(60 - 1) * 4 + 2] = SwapLE16(185 - 1); // 204
	pTiles[(60 - 1) * 4 + 3] = SwapLE16(189 - 1); // 205

	pTiles[(62 - 1) * 4 + 0] = SwapLE16(175 - 1); // 210
	pTiles[(62 - 1) * 4 + 1] = SwapLE16(199 - 1); // 211
	pTiles[(62 - 1) * 4 + 2] = SwapLE16(179 - 1); // 212
	pTiles[(62 - 1) * 4 + 3] = SwapLE16(177 - 1); // 213

	pTiles[(63 - 1) * 4 + 0] = SwapLE16(189 - 1); // 214
	pTiles[(63 - 1) * 4 + 1] = SwapLE16(177 - 1); // 215
	pTiles[(63 - 1) * 4 + 2] = SwapLE16(185 - 1); // 216
	pTiles[(63 - 1) * 4 + 3] = SwapLE16(189 - 1); // 217
	// -- flat floor
	pTiles[(92 - 1) * 4 + 3] = SwapLE16(332 - 1); // 326
	pTiles[(94 - 1) * 4 + 2] = SwapLE16(323 - 1); // 333
	pTiles[(94 - 1) * 4 + 3] = SwapLE16(324 - 1); // 334
	pTiles[(97 - 1) * 4 + 0] = SwapLE16(324 - 1); // 339
	pTiles[(97 - 1) * 4 + 1] = SwapLE16(323 - 1); // 340
	pTiles[(97 - 1) * 4 + 2] = SwapLE16(332 - 1); // 341
	pTiles[(99 - 1) * 4 + 0] = SwapLE16(332 - 1); // 347
	pTiles[(99 - 1) * 4 + 2] = SwapLE16(324 - 1); // 349
	pTiles[(99 - 1) * 4 + 3] = SwapLE16(323 - 1); // 350
	// create separate pillar tile
	pTiles[(52 - 1) * 4 + 0] = SwapLE16(55 - 1);
	pTiles[(52 - 1) * 4 + 1] = SwapLE16(26 - 1);
	pTiles[(52 - 1) * 4 + 2] = SwapLE16(23 - 1);
	pTiles[(52 - 1) * 4 + 3] = SwapLE16(12 - 1);
	// create the new shadows
	// - horizontal door for a pillar
	pTiles[(17 - 1) * 4 + 0] = SwapLE16(540 - 1); // TODO: use 17 and update DRLG_L2DoorSubs?
	pTiles[(17 - 1) * 4 + 1] = SwapLE16(18 - 1);
	pTiles[(17 - 1) * 4 + 2] = SwapLE16(155 - 1);
	pTiles[(17 - 1) * 4 + 3] = SwapLE16(162 - 1);
	// - horizontal hallway for a pillar
	pTiles[(18 - 1) * 4 + 0] = SwapLE16(553 - 1);
	pTiles[(18 - 1) * 4 + 1] = SwapLE16(99 - 1);
	pTiles[(18 - 1) * 4 + 2] = SwapLE16(155 - 1);
	pTiles[(18 - 1) * 4 + 3] = SwapLE16(162 - 1);
	// - corner tile for a pillar
	pTiles[(34 - 1) * 4 + 0] = SwapLE16(21 - 1);
	pTiles[(34 - 1) * 4 + 1] = SwapLE16(26 - 1);
	pTiles[(34 - 1) * 4 + 2] = SwapLE16(148 - 1);
	pTiles[(34 - 1) * 4 + 3] = SwapLE16(169 - 1);
	// - vertical wall end for a horizontal arch
	pTiles[(35 - 1) * 4 + 0] = SwapLE16(25 - 1);
	pTiles[(35 - 1) * 4 + 1] = SwapLE16(26 - 1);
	pTiles[(35 - 1) * 4 + 2] = SwapLE16(512 - 1);
	pTiles[(35 - 1) * 4 + 3] = SwapLE16(162 - 1);
	// - horizontal wall end for a pillar
	pTiles[(36 - 1) * 4 + 0] = SwapLE16(33 - 1);
	pTiles[(36 - 1) * 4 + 1] = SwapLE16(34 - 1);
	pTiles[(36 - 1) * 4 + 2] = SwapLE16(148 - 1);
	pTiles[(36 - 1) * 4 + 3] = SwapLE16(162 - 1);
	// - horizontal wall end for a horizontal arch
	pTiles[(37 - 1) * 4 + 0] = SwapLE16(268 - 1);
	pTiles[(37 - 1) * 4 + 1] = SwapLE16(515 - 1);
	pTiles[(37 - 1) * 4 + 2] = SwapLE16(148 - 1);
	pTiles[(37 - 1) * 4 + 3] = SwapLE16(169 - 1);
	// - floor tile with vertical arch
	pTiles[(44 - 1) * 4 + 0] = SwapLE16(150 - 1);
	pTiles[(44 - 1) * 4 + 1] = SwapLE16(10 - 1);
	pTiles[(44 - 1) * 4 + 2] = SwapLE16(153 - 1);
	pTiles[(44 - 1) * 4 + 3] = SwapLE16(12 - 1);
	// - floor tile with shadow of a vertical arch + horizontal arch
	pTiles[(46 - 1) * 4 + 0] = SwapLE16(9 - 1);   // 150
	pTiles[(46 - 1) * 4 + 1] = SwapLE16(154 - 1);
	pTiles[(46 - 1) * 4 + 2] = SwapLE16(161 - 1); // '151'
	pTiles[(46 - 1) * 4 + 3] = SwapLE16(162 - 1); // 152
	// - floor tile with shadow of a pillar + vertical arch
	pTiles[(47 - 1) * 4 + 0] = SwapLE16(9 - 1);   // 153
	// pTiles[(47 - 1) * 4 + 1] = SwapLE16(154 - 1);
	// pTiles[(47 - 1) * 4 + 2] = SwapLE16(155 - 1);
	pTiles[(47 - 1) * 4 + 3] = SwapLE16(162 - 1); // 156
	// - floor tile with shadow of a pillar
	// pTiles[(48 - 1) * 4 + 0] = SwapLE16(9 - 1);
	// pTiles[(48 - 1) * 4 + 1] = SwapLE16(10 - 1);
	pTiles[(48 - 1) * 4 + 2] = SwapLE16(155 - 1); // 157
	pTiles[(48 - 1) * 4 + 3] = SwapLE16(162 - 1); // 158
	// - floor tile with shadow of a horizontal arch
	pTiles[(49 - 1) * 4 + 0] = SwapLE16(9 - 1);   // 159
	pTiles[(49 - 1) * 4 + 1] = SwapLE16(10 - 1);  // 160
	// pTiles[(49 - 1) * 4 + 2] = SwapLE16(161 - 1);
	// pTiles[(49 - 1) * 4 + 3] = SwapLE16(162 - 1);
	// - floor tile with shadow(45) with vertical arch
	pTiles[(95 - 1) * 4 + 0] = SwapLE16(158 - 1);
	pTiles[(95 - 1) * 4 + 1] = SwapLE16(165 - 1);
	pTiles[(95 - 1) * 4 + 2] = SwapLE16(155 - 1);
	pTiles[(95 - 1) * 4 + 3] = SwapLE16(162 - 1);
	// - floor tile with shadow(49) with vertical arch
	pTiles[(96 - 1) * 4 + 0] = SwapLE16(150 - 1);
	pTiles[(96 - 1) * 4 + 1] = SwapLE16(10 - 1);
	pTiles[(96 - 1) * 4 + 2] = SwapLE16(156 - 1);
	pTiles[(96 - 1) * 4 + 3] = SwapLE16(162 - 1);
	// - floor tile with shadow(51) with horizontal arch
	pTiles[(100 - 1) * 4 + 0] = SwapLE16(158 - 1);
	pTiles[(100 - 1) * 4 + 1] = SwapLE16(165 - 1);
	pTiles[(100 - 1) * 4 + 2] = SwapLE16(155 - 1);
	pTiles[(100 - 1) * 4 + 3] = SwapLE16(169 - 1);
	// - shadow for the separate pillar
	pTiles[(101 - 1) * 4 + 0] = SwapLE16(55 - 1);
	pTiles[(101 - 1) * 4 + 1] = SwapLE16(26 - 1);
	pTiles[(101 - 1) * 4 + 2] = SwapLE16(148 - 1);
	pTiles[(101 - 1) * 4 + 3] = SwapLE16(169 - 1);
	// fix graphical glitch
	pTiles[(157 - 1) * 4 + 1] = SwapLE16(99 - 1); // 548
	// fix the upstairs II.
	pTiles[(72 - 1) * 4 + 1] = SwapLE16(56 - 1); // 10 make the back of the stairs non-walkable
	pTiles[(72 - 1) * 4 + 0] = SwapLE16(9 - 1);  // 250
	pTiles[(72 - 1) * 4 + 2] = SwapLE16(11 - 1); // 251
	// pTiles[(76 - 1) * 4 + 1] = SwapLE16(10 - 1); // 265
	// pTiles[(158 - 1) * 4 + 0] = SwapLE16(9 - 1);  // 250
	// pTiles[(158 - 1) * 4 + 1] = SwapLE16(56 - 1); // 10 make the back of the stairs non-walkable
	// pTiles[(158 - 1) * 4 + 2] = SwapLE16(11 - 1); // 554
	// pTiles[(159 - 1) * 4 + 0] = SwapLE16(9 - 1);  // 555
	// pTiles[(159 - 1) * 4 + 1] = SwapLE16(10 - 1); // 556
	// pTiles[(159 - 1) * 4 + 2] = SwapLE16(11 - 1); // 557
	// eliminate subtiles of unused tiles
	const int unusedTiles[] = {
		61, 64, 65, 66, 67, 76, 93, 98, 102, 103, 104, 143, 144, 145, 146, 147, 148, 149, 152, 153, 154, 155, 158, 159, 160
	};
	constexpr int blankSubtile = 2;
	for (int n = 0; n < lengthof(unusedTiles); n++) {
		int tileId = unusedTiles[n];
		pTiles[(tileId - 1) * 4 + 0] = SwapLE16(blankSubtile - 1);
		pTiles[(tileId - 1) * 4 + 1] = SwapLE16(blankSubtile - 1);
		pTiles[(tileId - 1) * 4 + 2] = SwapLE16(blankSubtile - 1);
		pTiles[(tileId - 1) * 4 + 3] = SwapLE16(blankSubtile - 1);
	}
}

DEVILUTION_END_NAMESPACE
