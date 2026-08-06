/**
 * @file townp.cpp
 *
 * Implementation of the town level patching functionality.
 */
#include "all.h"
#include "engine/render/cel_render.h"
#include "engine/render/dun_render.h"
#include "towndat.h"

DEVILUTION_BEGIN_NAMESPACE

static void maskMicro(int idx, int x0, int x1, int y0, int y1, int TRANS_COLOR, int DRAW_HEIGHT)
{
	for (int x = x0; x < x1; x++) {
		for (int y = y0; y < y1; y++) {
			unsigned addr = x + MICRO_WIDTH * (idx / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (idx % DRAW_HEIGHT)) * BUFFER_WIDTH;
			gpBuffer[addr] = TRANS_COLOR;
		}
	}
}

static void moveMicroPixels(int src, int dst, int TRANS_COLOR, int DRAW_HEIGHT)
{
	for (int x = 0; x < MICRO_WIDTH; x++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			unsigned addr = x + MICRO_WIDTH * (src / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (src % DRAW_HEIGHT)) * BUFFER_WIDTH;
			BYTE color = gpBuffer[addr];
			if (color != TRANS_COLOR) {
				gpBuffer[addr] = TRANS_COLOR;
				gpBuffer[x + MICRO_WIDTH * (dst / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (dst % DRAW_HEIGHT)) * BUFFER_WIDTH] = color;
			}
		}
	}
}

static void moveUpperMicroPixels(int src, int dst, int TRANS_COLOR, int DRAW_HEIGHT)
{
	for (int x = 0; x < MICRO_WIDTH; x++) {
		for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
			unsigned addr = x + MICRO_WIDTH * (src / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (src % DRAW_HEIGHT)) * BUFFER_WIDTH;
			BYTE color = gpBuffer[addr];
			if (color != TRANS_COLOR) {
				gpBuffer[addr] = TRANS_COLOR;
				gpBuffer[x + MICRO_WIDTH * (dst / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (dst % DRAW_HEIGHT)) * BUFFER_WIDTH] = color;
			}
		}
	}
}

static void moveLowerMicroPixels(int src, int dst, int TRANS_COLOR, int DRAW_HEIGHT)
{
	for (int x = 0; x < MICRO_WIDTH; x++) {
		for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
			unsigned addr = x + MICRO_WIDTH * (src / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (src % DRAW_HEIGHT)) * BUFFER_WIDTH;
			BYTE color = gpBuffer[addr];
			if (color != TRANS_COLOR) {
				gpBuffer[addr] = TRANS_COLOR;
				gpBuffer[x + MICRO_WIDTH * (dst / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (dst % DRAW_HEIGHT)) * BUFFER_WIDTH] = color;
			}
		}
	}
}

static void moveLimitedMicroPixels(int src, int dst, int x0, int x1, int TRANS_COLOR, int DRAW_HEIGHT)
{
	for (int x = x0; x < x1; x++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			unsigned addr = x + MICRO_WIDTH * (src / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (src % DRAW_HEIGHT)) * BUFFER_WIDTH;
			BYTE color = gpBuffer[addr];
			if (color != TRANS_COLOR) {
				gpBuffer[addr] = TRANS_COLOR;
				gpBuffer[x + MICRO_WIDTH * (dst / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (dst % DRAW_HEIGHT)) * BUFFER_WIDTH] = color;
			}
		}
	}
}

static void moveLimitedUpperMicroPixels(int src, int dst, int x0, int x1, int TRANS_COLOR, int DRAW_HEIGHT)
{
	for (int x = x0; x < x1; x++) {
		for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
			unsigned addr = x + MICRO_WIDTH * (src / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (src % DRAW_HEIGHT)) * BUFFER_WIDTH;
			BYTE color = gpBuffer[addr];
			if (color != TRANS_COLOR) {
				gpBuffer[addr] = TRANS_COLOR;
				gpBuffer[x + MICRO_WIDTH * (dst / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (dst % DRAW_HEIGHT)) * BUFFER_WIDTH] = color;
			}
		}
	}
}

static void moveLimitedLowerMicroPixels(int src, int dst, int x0, int x1, int TRANS_COLOR, int DRAW_HEIGHT)
{
	for (int x = x0; x < x1; x++) {
		for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
			unsigned addr = x + MICRO_WIDTH * (src / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (src % DRAW_HEIGHT)) * BUFFER_WIDTH;
			BYTE color = gpBuffer[addr];
			if (color != TRANS_COLOR) {
				gpBuffer[addr] = TRANS_COLOR;
				gpBuffer[x + MICRO_WIDTH * (dst / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (dst % DRAW_HEIGHT)) * BUFFER_WIDTH] = color;
			}
		}
	}
}

static void shiftMicrosDown(int m0, int m1, int TRANS_COLOR, int DRAW_HEIGHT)
{
	for (int i = m0; i < m1; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			if (i != m0) {
				for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
					unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
					BYTE color = gpBuffer[addr];
					// if (color != TRANS_COLOR) {
						gpBuffer[addr] = TRANS_COLOR;
						gpBuffer[x + MICRO_WIDTH * ((i - 1) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i - 1) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color;
					// }
				}
			}
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				// if (color != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
					gpBuffer[x + MICRO_WIDTH * ((i - 0) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i - 0) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color;
				// }
			}
		}
	}
}

static void shiftMicrosUp(int m0, int m1, int TRANS_COLOR, int DRAW_HEIGHT)
{
	for (int i = m0; i < m1; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			if (i != m0) {
				for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
					unsigned addr = x + MICRO_WIDTH * ((i - 0) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 0) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					BYTE color = gpBuffer[addr];
					// if (color != TRANS_COLOR) {
						gpBuffer[addr] = TRANS_COLOR;
						gpBuffer[x + MICRO_WIDTH * ((i - 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i - 1) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color;
					// }
				}
			}
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * ((i - 0) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 0) % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				// if (color != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
					gpBuffer[x + MICRO_WIDTH * ((i - 0) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i - 0) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color;
				// }
			}
		}
	}
}

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

// patch subtiles around the pot of Adria to prevent graphical glitch when a player passes it I.
static BYTE* patchTownPotCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	typedef struct {
		int subtileIndex;
		unsigned microIndex;
		int res_encoding;
	} CelMicro;
	const CelMicro micros[] = {
/*  0 */{ 553 - 1, 5, MET_TRANSPARENT }, // 376
/*  1 */{ 553 - 1, 3, MET_TRANSPARENT },
/*  2 */{ 553 - 1, 1, MET_RTRIANGLE },
/*  3 */{ 554 - 1, 0, MET_TRANSPARENT }, // 377
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

	// copy part of 553[1] to 554[0]
	moveLimitedLowerMicroPixels(2, 3, 18, 28, TRANS_COLOR, DRAW_HEIGHT);
	// shift 553[3..] up by half
	shiftMicrosUp(0, 2, TRANS_COLOR, DRAW_HEIGHT);
	// copy part of 553[1] to 553[3]
	moveLimitedUpperMicroPixels(2, 1, 18, 28, TRANS_COLOR, DRAW_HEIGHT);

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
		int res_encoding;
	} CelMicro;
	const CelMicro micros[] = {
/*  0 */{ 807 - 1, 12, MET_TRANSPARENT },
/*  1 */{ 805 - 1, 12, MET_SQUARE }, // 544
/*  2 */{ 805 - 1, 13, MET_SQUARE }, // 544
/*  3 */{ 832 - 1, 10, MET_TRANSPARENT }, // 571

/*  4 */{ 727 - 1, 9, MET_SQUARE }, // 492[9]
/*  5 */{ 811 - 1, 0, MET_TRANSPARENT }, // 550[0]
/*  6 */{ 811 - 1, 2, MET_TRANSPARENT }, // 550[2]
/*  7 */{ 811 - 1, 4, MET_TRANSPARENT }, // 550[4]
/*  8 */{ 811 - 1, 6, -1 }, // 550[6]

/*  9 */{ 716 - 1, 13, MET_SQUARE }, // 481[13]
/* 10 */{ 716 - 1, 11, MET_SQUARE }, // 481[11]
/* 11 */{ 728 - 1, 9, -1 },
/* 12 */{ 728 - 1, 7, -1 },
/* 13 */{ 811 - 1, 1, MET_SQUARE }, // 550[1]
/* 14 */{ 812 - 1, 0, -1 }, // 551[0]
/* 15 */{ 809 - 1, 0, MET_SQUARE }, // 548[0]
/* 16 */{ 809 - 1, 2, MET_SQUARE }, // 548[2]
/* 17 */{ 809 - 1, 4, MET_TRANSPARENT }, // 548[4]
/* 18 */{ 809 - 1, 6, -1 }, // 548[6]

/* 19 */{ 719 - 1, 13, MET_SQUARE }, // 484[13]
/* 20 */{ 818 - 1, 0, MET_SQUARE }, // 557[0]
/* 21 */{ 818 - 1, 2, MET_SQUARE }, // 557[2]
/* 22 */{ 818 - 1, 4, MET_SQUARE }, // 557[4]
/* 23 */{ 818 - 1, 6, MET_SQUARE }, // 557[6]
/* 24 */{ 818 - 1, 8, MET_TRANSPARENT }, // 557[8]
/* 25 */{ 818 - 1, 10, MET_TRANSPARENT }, // 557[10]
/* 26 */{ 818 - 1, 12, -1 }, // 557[12]

/* 27 */{ 719 - 1, 12, MET_SQUARE }, // 484[12]
/* 28 */{ 810 - 1, 1, MET_SQUARE }, // 549[1]
/* 29 */{ 810 - 1, 3, MET_SQUARE }, // 549[3]
/* 30 */{ 810 - 1, 5, MET_SQUARE }, // 549[0]
/* 31 */{ 810 - 1, 7, MET_SQUARE }, // 549[0]
/* 32 */{ 810 - 1, 9, MET_TRANSPARENT }, // 549[0]

/* 33 */{ 721 - 1, 12, MET_SQUARE }, // 491[0]
/* 34 */{ 810 - 1, 0, MET_SQUARE }, // 549[0]
/* 35 */{ 812 - 1, 1, -1 }, // 551[1]
/* 36 */{ 809 - 1, 1, MET_SQUARE }, // 548[1]
/* 37 */{ 809 - 1, 3, MET_SQUARE }, // 548[2]
/* 38 */{ 809 - 1, 5, MET_TRANSPARENT }, // 548[4]
/* 39 */{ 809 - 1, 7, MET_TRANSPARENT }, // 548[6]

/* 40 */{ 819 - 1, 0, MET_LTRAPEZOID }, // 558[0]
/* 41 */{ 816 - 1, 0, MET_SQUARE }, // 555[0]
/* 42 */{ 818 - 1, 1, -1 }, // 557[1]

/* 43 */{ 819 - 1, 1, MET_RTRAPEZOID }, // 558[1]
/* 44 */{ 816 - 1, 1, MET_SQUARE }, // 555[1]
/* 45 */{ 817 - 1, 0, -1 }, // 556[0]

/* 46 */{ 816 - 1, 12, MET_SQUARE }, // 555[12]
/* 47 */{ 846 - 1, 0, MET_SQUARE }, // 585[0]
/* 48 */{ 837 - 1, 1, MET_SQUARE }, // 576[1]
/* 49 */{ 837 - 1, 3, MET_SQUARE }, // 576[3]
/* 50 */{ 837 - 1, 5, -1 }, // 576[5]
/* 51 */{ 843 - 1, 0, MET_SQUARE }, // 582[0]
/* 52 */{ 845 - 1, 1, -1 }, // 584[1]

/* 53 */{ 816 - 1, 13, MET_SQUARE }, // 555[13]
/* 54 */{ 846 - 1, 1, MET_SQUARE }, // 585[1]
/* 55 */{ 843 - 1, 1, MET_SQUARE }, // 582[1]
/* 56 */{ 844 - 1, 0, -1 }, // 583[0]
/* 57 */{ 841 - 1, 0, MET_SQUARE }, // 580[0]
/* 58 */{ 841 - 1, 2, MET_SQUARE }, // 580[2]
/* 59 */{ 841 - 1, 4, -1 }, // 580[4]

/* 60 */{ 838 - 1, 1, MET_TRANSPARENT }, // 577[1]
/* 61 */{ 836 - 1, 1, MET_TRANSPARENT }, // 575[1]
/* 62 */{ 836 - 1, 5, MET_TRANSPARENT }, // 575[5]
/* 63 */{ 837 - 1, 0, -1 }, // 576[0]
/* 64 */{ 845 - 1, 0, MET_TRANSPARENT }, // 584[0]
/* 65 */{ 845 - 1, 2, MET_TRANSPARENT }, // 584[2]
/* 66 */{ 845 - 1, 4, MET_TRANSPARENT }, // 584[4]
/* 67 */{ 845 - 1, 6, MET_TRANSPARENT }, // 584[6]
/* 68 */{ 845 - 1, 8, -1 }, // 584[8]

/* 69 */{ 839 - 1, 4, MET_SQUARE }, // 578[4]
/* 70 */{ 844 - 1, 1, MET_SQUARE }, // 583[1]
/* 71 */{ 844 - 1, 3, MET_SQUARE }, // 583[3]
/* 72 */{ 844 - 1, 5, MET_SQUARE }, // 583[5]
/* 73 */{ 844 - 1, 7, MET_SQUARE }, // 583[7]
/* 74 */{ 844 - 1, 9, MET_TRANSPARENT }, // 583[9]
/* 75 */{ 844 - 1, 11, MET_TRANSPARENT }, // 583[11]

/* 76 */{ 839 - 1, 0, MET_SQUARE }, // 578[1]
/* 77 */{ 842 - 1, 0, MET_SQUARE }, // 581[0]
/* 78 */{ 841 - 1, 1, -1 }, // 580[1]
/* 79 */{ 817 - 1, 1, MET_SQUARE }, // 556[1]
/* 80 */{ 817 - 1, 3, MET_SQUARE }, // 556[3]
/* 81 */{ 817 - 1, 5, MET_SQUARE }, // 556[5]
/* 82 */{ 817 - 1, 7, MET_SQUARE }, // 556[7]
/* 83 */{ 817 - 1, 9, MET_SQUARE }, // 556[9]
/* 84 */{ 817 - 1, 11, MET_SQUARE }, // 556[11]
/* 85 */{ 817 - 1, 13, -1 }, // 556[13]

/* 86 */{ 842 - 1, 1, MET_SQUARE }, // 581[1]
/* 87 */{ 839 - 1, 1, MET_SQUARE }, // 578[1]
/* 88 */{ 840 - 1, 0, -1 }, // 579[0]
/* 89 */{ 839 - 1, 5, MET_SQUARE }, // 578[5]
/* 90 */{ 848 - 1, 0, MET_SQUARE }, // 587[0]
/* 91 */{ 848 - 1, 2, MET_SQUARE }, // 587[0]
/* 92 */{ 848 - 1, 4, MET_SQUARE }, // 587[0]
/* 93 */{ 848 - 1, 6, MET_TRANSPARENT }, // 587[0]
/* 94 */{ 848 - 1, 8, MET_TRANSPARENT }, // 587[8]
/* 95 */{ 848 - 1, 10, -1 }, // 587[10]

/* 96 */{ 849 - 1, 0, MET_TRANSPARENT }, // 588[0]
/* 97 */{ 847 - 1, 0, MET_TRANSPARENT }, // 586[0]
/* 98 */{ 848 - 1, 1, -1 }, // 587[1]
/* 99 */{ 820 - 1, 12, MET_TRANSPARENT }, // 559[12]
/*100 */{ 840 - 1, 1, MET_TRANSPARENT }, // 579[1]
/*101 */{ 840 - 1, 3, MET_TRANSPARENT }, // 579[3]
/*102 */{ 840 - 1, 5, -1 }, // 579[5]

/*103 */{ 822 - 1, 0, MET_SQUARE }, // 561[12]
/*104 */{ 822 - 1, 2, MET_SQUARE }, // 561[12]
/*105 */{ 822 - 1, 4, MET_SQUARE }, // 561[12]
/*106 */{ 822 - 1, 6, MET_SQUARE }, // 561[12]
/*107 */{ 822 - 1, 8, MET_SQUARE }, // 561[12]
/*108 */{ 822 - 1, 10, MET_SQUARE }, // 561[12]
/*109 */{ 822 - 1, 12, -1 }, // 561[12]

/*110 */{ 823 - 1, 1, MET_SQUARE }, // 562
/*111 */{ 820 - 1, 1, MET_SQUARE }, // 559
/*112 */{ 821 - 1, 0, -1 }, // 560
/*113 */{ 826 - 1, 0, MET_SQUARE }, // 565
/*114 */{ 826 - 1, 2, MET_SQUARE },
/*115 */{ 826 - 1, 4, MET_SQUARE },
/*116 */{ 826 - 1, 6, MET_SQUARE },
/*117 */{ 826 - 1, 8, MET_SQUARE },
/*118 */{ 826 - 1, 10, MET_TRANSPARENT },
/*119 */{ 826 - 1, 12, -1 },

/*120 */{ 823 - 1, 0, MET_SQUARE }, // 562
/*121 */{ 820 - 1, 0, MET_SQUARE }, // 559
/*122 */{ 822 - 1, 1, -1 }, // 561
/*123 */{ 781 - 1, 0, MET_LTRAPEZOID }, // 528

/*124 */{ 806 - 1, 1, MET_SQUARE }, // 545
/*125 */{ 806 - 1, 3, MET_SQUARE },
/*126 */{ 806 - 1, 5, MET_SQUARE },
/*127 */{ 806 - 1, 7, MET_SQUARE },
/*128 */{ 806 - 1, 9, MET_SQUARE },
/*129 */{ 806 - 1, 11, MET_SQUARE },
/*130 */{ 806 - 1, 13, -1 },

/*131 */{ 828 - 1, 12, MET_TRANSPARENT }, // 567
/*132 */{ 781 - 1, 1, MET_RTRAPEZOID }, // 528
/*133 */{ 787 - 1, 0, MET_SQUARE }, // 534
/*134 */{ 787 - 1, 2, MET_SQUARE },
/*135 */{ 787 - 1, 4, MET_SQUARE },
/*136 */{ 787 - 1, 6, MET_SQUARE },
/*137 */{ 787 - 1, 8, MET_SQUARE },
/*138 */{ 787 - 1, 10, MET_SQUARE },
/*139 */{ 787 - 1, 12, -1 },

/*140 */{ 827 - 1, 0, MET_SQUARE }, // 566
/*141 */{ 824 - 1, 0, MET_SQUARE }, // 563
/*142 */{ 826 - 1, 1, -1 }, // 565
/*143 */{ 785 - 1, 12, MET_SQUARE }, // 532
/*144 */{ 821 - 1, 1, -1 }, // 560
/*145 */{ 827 - 1, 1, MET_SQUARE }, // 566
/*146 */{ 824 - 1, 1, MET_SQUARE }, // 563
/*147 */{ 825 - 1, 0, -1 }, // 564
/*148 */{ 785 - 1, 1, MET_RTRAPEZOID }, // 532
/*149 */{ 791 - 1, 0, MET_SQUARE }, // 537
/*150 */{ 791 - 1, 2, MET_SQUARE },
/*151 */{ 791 - 1, 4, MET_SQUARE },
/*152 */{ 791 - 1, 6, MET_SQUARE },
/*153 */{ 791 - 1, 8, MET_SQUARE },
/*154 */{ 791 - 1, 10, MET_SQUARE },
/*155 */{ 791 - 1, 12, MET_TRANSPARENT },

/*156 */{ 830 - 1, 0, MET_SQUARE }, // 569
/*157 */{ 830 - 1, 2, MET_SQUARE },
/*158 */{ 830 - 1, 4, MET_SQUARE },
/*159 */{ 830 - 1, 6, MET_SQUARE },
/*160 */{ 830 - 1, 8, MET_SQUARE },
/*161 */{ 830 - 1, 10, MET_TRANSPARENT },
/*162 */{ 830 - 1, 12, -1 },

/*163 */{ 789 - 1, 12, MET_SQUARE }, // 536
/*164 */{ 831 - 1, 0, MET_SQUARE }, // 570
/*165 */{ 825 - 1, 1, -1 }, // 564
/*166 */{ 828 - 1, 0, MET_SQUARE }, // 567
/*167 */{ 830 - 1, 1, -1 }, // 569

/*168 */{ 831 - 1, 1, MET_SQUARE }, // 570
/*169 */{ 828 - 1, 1, MET_SQUARE }, // 567
/*170 */{ 829 - 1, 0, -1 }, // 568
/*171 */{ 834 - 1, 0, MET_SQUARE }, // 573
/*172 */{ 834 - 1, 2, MET_SQUARE },
/*173 */{ 834 - 1, 4, MET_SQUARE },
/*174 */{ 834 - 1, 6, MET_SQUARE },
/*175 */{ 834 - 1, 8, MET_SQUARE },
/*176 */{ 834 - 1, 10, MET_TRANSPARENT },
/*177 */{ 834 - 1, 12, -1 },

/*178 */{ 835 - 1, 0, MET_SQUARE }, // 574
/*179 */{ 832 - 1, 0, MET_SQUARE }, // 571
/*180 */{ 834 - 1, 1, -1 }, // 573
/*181 */{ 793 - 1, 12, MET_SQUARE }, // 538
/*182 */{ 829 - 1, 1, -1 }, // 568
/*183 */{ 789 - 1, 1, MET_RTRAPEZOID }, // 536
/*184 */{ 795 - 1, 0, MET_SQUARE }, // 540
/*185 */{ 795 - 1, 2, MET_SQUARE },
/*186 */{ 795 - 1, 4, MET_SQUARE },
/*187 */{ 795 - 1, 6, MET_SQUARE },
/*188 */{ 795 - 1, 8, MET_SQUARE },
/*189 */{ 795 - 1, 10, MET_SQUARE },
/*190 */{ 795 - 1, 12, MET_TRANSPARENT },

/*191 */{ 835 - 1, 1, MET_SQUARE }, // 574
/*192 */{ 832 - 1, 1, MET_SQUARE }, // 571
/*193 */{ 833 - 1, 0, -1 }, // 572
/*194 */{ 797 - 1, 12, MET_SQUARE }, // 541
/*195 */{ 833 - 1, 1, MET_SQUARE }, // 572
/*196 */{ 833 - 1, 3, MET_TRANSPARENT },
/*197 */{ 833 - 1, 5, MET_TRANSPARENT },
/*198 */{ 833 - 1, 7, MET_TRANSPARENT },
/*199 */{ 833 - 1, 9, MET_TRANSPARENT },

/*200 */{ 793 - 1, 1, MET_RTRAPEZOID }, // 538
/*201 */{ 799 - 1, 0, MET_SQUARE }, // 542
/*202 */{ 799 - 1, 2, MET_SQUARE },
/*203 */{ 799 - 1, 4, MET_SQUARE },
/*204 */{ 799 - 1, 6, MET_SQUARE },
/*205 */{ 799 - 1, 8, MET_SQUARE },
/*206 */{ 799 - 1, 10, MET_SQUARE },
/*207 */{ 799 - 1, 12, MET_TRANSPARENT },

/*208 */{ 797 - 1, 1, MET_TRANSPARENT }, // 541
/*209 */{ 815 - 1, 1, MET_TRANSPARENT }, // 554
/*210 */{ 803 - 1, 0, -1 }, // 543

/*211 */{ 814 - 1, 0, MET_TRANSPARENT }, // 553
/*212 */{ 813 - 1, 1, MET_TRANSPARENT }, // 552
/*213 */{ 813 - 1, 3, MET_TRANSPARENT },
/*214 */{ 813 - 1, 5, MET_TRANSPARENT },

/*215 */{ 808 - 1, 1, MET_RTRAPEZOID }, // 547
/*216 */{ 805 - 1, 1, MET_SQUARE }, // 544
/*217 */{ 806 - 1, 0, -1 }, // 545

/*218 */{ 808 - 1, 0, MET_LTRAPEZOID }, // 547
/*219 */{ 805 - 1, 0, MET_SQUARE }, // 544
/*220 */{ 807 - 1, 1, -1 }, // 546
	};

	const uint16_t* pSubtiles = (const uint16_t*)minBuf;
	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_TOWN;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		if (micro.subtileIndex < 0) {
			continue;
		}
		unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
		if ((SwapLE16(pSubtiles[index]) & 0xFFF) == 0) {
			// mem_free_dbg(celBuf);
			// app_warn("Invalid (empty) cathedral subtile (%d).", micro.subtileIndex + 1);
			return celBuf; // frame is empty -> assume it is already done
		}
	}

	// TODO: check celLen
	// draw the micros to the back-buffer
	pMicrosCel = celBuf;
	constexpr BYTE TRANS_COLOR = 128;
	constexpr int DRAW_HEIGHT = 16;
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

	// draw extra line to each frame
	{ // 807[12]
		int i = 0;
		for (int sx = 0; sx < MICRO_WIDTH; sx++) {
			int y = MICRO_HEIGHT / 2 - sx / 2 + MICRO_HEIGHT * (i % DRAW_HEIGHT);
			int x = sx + MICRO_WIDTH * (i / DRAW_HEIGHT);
			BYTE color = gpBuffer[x + (y + 6) * BUFFER_WIDTH];
			gpBuffer[x + (y) * BUFFER_WIDTH] = color;
		}
	}
	{ // 805[12]
		int i = 1;
		for (int sx = 0; sx < MICRO_WIDTH - 4; sx++) {
			int y = MICRO_HEIGHT / 2 - sx / 2 + MICRO_HEIGHT * (i % DRAW_HEIGHT);
			int x = sx + MICRO_WIDTH * (i / DRAW_HEIGHT);
			BYTE color = gpBuffer[x + 4 + (y + 4) * BUFFER_WIDTH];
			gpBuffer[x + (y) * BUFFER_WIDTH] = color;
		}
		for (int sx = MICRO_WIDTH - 4; sx < MICRO_WIDTH; sx++) {
			int y = MICRO_HEIGHT / 2 - sx / 2 + MICRO_HEIGHT * (i % DRAW_HEIGHT);
			int x = sx + MICRO_WIDTH * (i / DRAW_HEIGHT);
			BYTE color = gpBuffer[x + 0 + (y + 2) * BUFFER_WIDTH];
			gpBuffer[x + (y) * BUFFER_WIDTH] = color;
		}
	}
	{ // 805[13]
		int i = 2;
		for (int sx = 0; sx < 20; sx++) {
			int y = 1 + sx / 2 + MICRO_HEIGHT * (i % DRAW_HEIGHT);
			int x = sx + MICRO_WIDTH * (i / DRAW_HEIGHT);
			BYTE color = gpBuffer[x + 0 + (y + 1) * BUFFER_WIDTH];
			gpBuffer[x + (y) * BUFFER_WIDTH] = color;
		}
		for (int sx = 20; sx < MICRO_WIDTH; sx++) {
			int y = 1 + sx / 2 + MICRO_HEIGHT * (i % DRAW_HEIGHT);
			int x = sx + MICRO_WIDTH * (i / DRAW_HEIGHT);
			BYTE color = gpBuffer[x - 12 + (y - 6) * BUFFER_WIDTH];
			gpBuffer[x + (y) * BUFFER_WIDTH] = color;
		}
	}
	{ // 806[13]
		int i = 130;
		for (int x = 0; x < MICRO_WIDTH; x++) {
			int y = 1 + x / 2;
			BYTE color = gpBuffer[x + 0 + MICRO_WIDTH * ((i - 1) / DRAW_HEIGHT) + (y + 0 + MICRO_HEIGHT * ((i - 1) % DRAW_HEIGHT)) * BUFFER_WIDTH];
			gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH] = color;
		}
	}
	{ // 832[10]
		int i = 3;
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
	{ // 828[12]
		int i = 131;
		unsigned addr = 30 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = TRANS_COLOR;
		gpBuffer[addr + 1] = TRANS_COLOR;

		unsigned addr1 = 29 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (17 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		unsigned addr2 = 27 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (18 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr1] = gpBuffer[addr2 + 1];
		gpBuffer[addr1 + 1] = gpBuffer[addr2 + 2];
		gpBuffer[addr2] = gpBuffer[addr2 + 3];
		gpBuffer[addr2 - 3 + 1 * BUFFER_WIDTH] = gpBuffer[addr2 + 3];
	}
	// copy lower half 811[0] to 727[9]
	moveLowerMicroPixels(5, 4, TRANS_COLOR, DRAW_HEIGHT);
	// shift 811[2..] by half
	shiftMicrosDown(5, 9, TRANS_COLOR, DRAW_HEIGHT);
	// copy 728[9] to 716[13]
	// copy 728[7] to 716[11]
	for (int i = 11; i < 13; i++) {
		moveMicroPixels(i, i - 2, TRANS_COLOR, DRAW_HEIGHT);
	}
	// copy lower half of 812[0] to 716[13]
	moveLowerMicroPixels(14, 9, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 812[0] to 811[1]
	moveUpperMicroPixels(14, 13, TRANS_COLOR, DRAW_HEIGHT);

	// copy lower half of 809[0] to 811[1]
	moveLowerMicroPixels(15, 13, TRANS_COLOR, DRAW_HEIGHT);
	// shift 809[2..] by half
	shiftMicrosDown(15, 19, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 818[0] to 719[13]
	moveLowerMicroPixels(20, 19, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 818[12] to 838[1]
	moveUpperMicroPixels(26, 60, TRANS_COLOR, DRAW_HEIGHT);
	// shift 818[2..] by half
	shiftMicrosDown(20, 27, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 810[1] to 719[12]
	moveLowerMicroPixels(28, 27, TRANS_COLOR, DRAW_HEIGHT);
	// shift 810[3..] by half
	shiftMicrosDown(28, 33, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 812[1] to 721[12]
	moveLowerMicroPixels(35, 33, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 812[1] to 810[0]
	moveUpperMicroPixels(35, 34, TRANS_COLOR, DRAW_HEIGHT);

	// copy lower half of 809[1] to 810[0]
	moveLowerMicroPixels(36, 34, TRANS_COLOR, DRAW_HEIGHT);
	// shift 809[3..] by half
	shiftMicrosDown(36, 40, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 818[1] to 819[0]
	moveLowerMicroPixels(42, 40, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 818[1] to 816[0]
	moveUpperMicroPixels(42, 41, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 817[0] to 819[1]
	moveLowerMicroPixels(45, 43, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 817[0] to 816[1]
	moveUpperMicroPixels(45, 44, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 837[1] to 816[12]
	moveLowerMicroPixels(48, 46, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 837[5] to 846[0]
	moveUpperMicroPixels(50, 47, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 845[1] to 846[0]
	moveLowerMicroPixels(52, 47, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 845[1] to 843[0]
	moveUpperMicroPixels(52, 51, TRANS_COLOR, DRAW_HEIGHT);
	// shift 837[3..] by half
	shiftMicrosDown(48, 51, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 841[0] to 816[13]
	moveLowerMicroPixels(57, 53, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 844[0] to 846[1]
	moveLowerMicroPixels(56, 54, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 844[0] to 843[1]
	moveUpperMicroPixels(56, 55, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 841[4] to 846[1]
	moveUpperMicroPixels(59, 54, TRANS_COLOR, DRAW_HEIGHT);
	// shift 841[2..] by half
	shiftMicrosDown(57, 60, TRANS_COLOR, DRAW_HEIGHT);
	// mask 836[1] and 838[1]
	maskMicro(60, 0, 10, 0, MICRO_HEIGHT, TRANS_COLOR, DRAW_HEIGHT);
	maskMicro(61, 0, 10, 0, MICRO_HEIGHT, TRANS_COLOR, DRAW_HEIGHT);
	// copy part of lower half of 837[0] to 838[1]
	moveLimitedLowerMicroPixels(63, 60, 10, MICRO_WIDTH, TRANS_COLOR, DRAW_HEIGHT);
	// copy part of upper half of 837[0] to 836[1]
	moveLimitedUpperMicroPixels(63, 61, 10, MICRO_WIDTH, TRANS_COLOR, DRAW_HEIGHT);
	// copy part of lower half of 845[0] to 836[5]
	moveLimitedLowerMicroPixels(64, 62, 10, MICRO_WIDTH, TRANS_COLOR, DRAW_HEIGHT);
	// mask 845[0]
	maskMicro(64, 0, 10, 0, MICRO_HEIGHT, TRANS_COLOR, DRAW_HEIGHT);
	// mask 845[4]
	maskMicro(66, 0, 10, 0, MICRO_HEIGHT, TRANS_COLOR, DRAW_HEIGHT);
	// shift 845[2..] by half
	shiftMicrosDown(64, 69, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 844[1] to 839[4]
	moveLowerMicroPixels(70, 69, TRANS_COLOR, DRAW_HEIGHT);
	// shift 844[3..] by half
	shiftMicrosDown(70, 76, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 817[1] to 805[12]
	moveLowerMicroPixels(79, 1, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 841[1] to 842[0]
	moveLowerMicroPixels(78, 77, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 841[1] to 839[0]
	moveUpperMicroPixels(78, 76, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 817[13] to 842[0]
	moveUpperMicroPixels(85, 77, TRANS_COLOR, DRAW_HEIGHT);
	// shift 817[3..] by half
	shiftMicrosDown(79, 86, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 840[0] to 842[1]
	moveLowerMicroPixels(88, 86, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 840[0] to 839[1]
	moveUpperMicroPixels(88, 87, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 848[0] to 839[5]
	moveLowerMicroPixels(90, 89, TRANS_COLOR, DRAW_HEIGHT);
	// shift 848[2..] by half
	shiftMicrosDown(90, 96, TRANS_COLOR, DRAW_HEIGHT);
	// mask 847[0] and 849[0]
	maskMicro(96, 30, MICRO_WIDTH, 0, MICRO_HEIGHT, TRANS_COLOR, DRAW_HEIGHT);
	maskMicro(97, 30, MICRO_WIDTH, 0, MICRO_HEIGHT, TRANS_COLOR, DRAW_HEIGHT);
	// copy part of lower half of 848[1] to 849[0]
	moveLimitedLowerMicroPixels(98, 96, 0, 30, TRANS_COLOR, DRAW_HEIGHT);
	// copy part of upper half of 848[1] to 847[0]
	moveLimitedUpperMicroPixels(98, 97, 0, 30, TRANS_COLOR, DRAW_HEIGHT);

	// copy lower half of 840[0] to 559[12]
	moveLowerMicroPixels(100, 99, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 840[5] to 849[0]
	moveUpperMicroPixels(102, 96, TRANS_COLOR, DRAW_HEIGHT);
	// shift 840[3..] by half
	shiftMicrosDown(100, 103, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 822[12] to 842[1]
	moveUpperMicroPixels(109, 86, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 822[0] to 805[13]
	moveLowerMicroPixels(103, 2, TRANS_COLOR, DRAW_HEIGHT);
	// shift 822[2..] by half
	shiftMicrosDown(103, 110, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 821[0] to 823[1]
	moveLowerMicroPixels(112, 110, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 821[0] to 820[1]
	moveUpperMicroPixels(112, 111, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 826[0] to 820[1]
	moveLowerMicroPixels(113, 111, TRANS_COLOR, DRAW_HEIGHT);
	// shift 826[2..] by half
	shiftMicrosDown(113, 120, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 822[1] to 823[0]
	moveLowerMicroPixels(122, 120, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 822[1] to 820[0]
	moveUpperMicroPixels(122, 121, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 806[13] to 823[0]
	moveUpperMicroPixels(130, 120, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 806[1] to 781[0]
	moveLowerMicroPixels(124, 123, TRANS_COLOR, DRAW_HEIGHT);
	// shift 806[3..] by half
	shiftMicrosDown(124, 131, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 787[12] to 823[1]
	moveUpperMicroPixels(139, 110, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 787[0] to 781[1]
	moveLowerMicroPixels(133, 132, TRANS_COLOR, DRAW_HEIGHT);
	// shift 787[2..] by half
	shiftMicrosDown(133, 140, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 826[1] to 827[0]
	moveLowerMicroPixels(142, 140, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 826[1] to 824[0]
	moveUpperMicroPixels(142, 141, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 821[1] to 785[12]
	moveLowerMicroPixels(144, 143, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 821[1] to 827[0]
	moveUpperMicroPixels(144, 140, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 825[0] to 827[1]
	moveLowerMicroPixels(147, 145, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 825[0] to 824[1]
	moveUpperMicroPixels(147, 146, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 791[12] to 827[1]
	moveUpperMicroPixels(155, 145, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 791[0] to 785[1]
	moveLowerMicroPixels(149, 148, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 830[0] to 824[1]
	moveLowerMicroPixels(156, 146, TRANS_COLOR, DRAW_HEIGHT);
	// shift 791[2..] by half
	shiftMicrosDown(149, 156, TRANS_COLOR, DRAW_HEIGHT);
	// shift 830[2..] by half
	shiftMicrosDown(156, 163, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 825[1] to 789[12]
	moveLowerMicroPixels(165, 163, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 825[1] to 831[0]
	moveUpperMicroPixels(165, 164, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 830[1] to 831[0]
	moveLowerMicroPixels(167, 164, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 830[1] to 828[0]
	moveUpperMicroPixels(167, 166, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 829[0] to 831[1]
	moveLowerMicroPixels(170, 168, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 829[0] to 828[1]
	moveUpperMicroPixels(170, 169, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 834[0] to 828[1]
	moveLowerMicroPixels(171, 169, TRANS_COLOR, DRAW_HEIGHT);
	// shift 834[2..] by half
	shiftMicrosDown(171, 178, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 834[1] to 835[0]
	moveLowerMicroPixels(180, 178, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 834[1] to 832[0]
	moveUpperMicroPixels(180, 179, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 829[1] to 793[12]
	moveLowerMicroPixels(182, 181, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 829[1] to 835[0]
	moveUpperMicroPixels(182, 178, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 795[12] to 831[1]
	moveUpperMicroPixels(190, 168, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 795[0] to 789[1]
	moveLowerMicroPixels(184, 183, TRANS_COLOR, DRAW_HEIGHT);
	// shift 795[2..] by half
	shiftMicrosDown(184, 191, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 833[0] to 835[1]
	moveLowerMicroPixels(193, 191, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 833[0] to 832[1]
	moveUpperMicroPixels(193, 192, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 833[1] to 797[12]
	moveLowerMicroPixels(195, 194, TRANS_COLOR, DRAW_HEIGHT);
	// shift 833[3..] by half
	shiftMicrosDown(195, 200, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 799[12] to 835[1]
	moveUpperMicroPixels(207, 191, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 799[0] to 793[1]
	moveLowerMicroPixels(201, 200, TRANS_COLOR, DRAW_HEIGHT);
	// shift 799[2..] by half
	shiftMicrosDown(201, 208, TRANS_COLOR, DRAW_HEIGHT);

	// copy lower half of 806[0] to 808[1]
	moveLowerMicroPixels(217, 215, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 806[0] to 805[1]
	moveUpperMicroPixels(217, 216, TRANS_COLOR, DRAW_HEIGHT);
	// copy lower half of 807[1] to 808[0]
	moveLowerMicroPixels(220, 218, TRANS_COLOR, DRAW_HEIGHT);
	// copy upper half of 807[1] to 805[0]
	moveUpperMicroPixels(220, 219, TRANS_COLOR, DRAW_HEIGHT);
	// copy part of lower half of 803[0] to 797[1]
	moveLimitedLowerMicroPixels(210, 208, 0, 17, TRANS_COLOR, DRAW_HEIGHT);
	// copy part of upper half of 803[0] to 815[1]
	moveLimitedUpperMicroPixels(210, 209, 0, 17, TRANS_COLOR, DRAW_HEIGHT);

	// copy part of lower half of 814[0] to 815[1]
	moveLimitedLowerMicroPixels(211, 209, 0, 16, TRANS_COLOR, DRAW_HEIGHT);
	// copy part of upper half of 814[0] to 813[1]
	moveLimitedUpperMicroPixels(211, 212, 0, 16, TRANS_COLOR, DRAW_HEIGHT);

	// copy part of lower half of 813[1] to 814[0]
	moveLimitedLowerMicroPixels(212, 211, 16, MICRO_WIDTH, TRANS_COLOR, DRAW_HEIGHT);
	// copy part of upper half of 813[1] to 799[12]
	for (int i = 212; i < 213; i++) {
		for (int x = 16; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				if (y <= 21 - x)
					continue;
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
					gpBuffer[x + MICRO_WIDTH * ((i - 5) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i - 5) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color;
				}
			}
		}
	}
	// copy part of lower half of 813[3] to 799[12]
	// copy part of upper half of 813[3] to 795[12]
	for (int i = 213; i < 214; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				bool replace = false;
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (y <= 8 + x) {
					replace = y <= 3 + x || x > 20;
					if (y > 3 + x) {
						replace = !(color == 13 || color == 20 || color == 22 || color == 26 || color == 31 || color == 40 || color == 43 || color == 48 || color == 52 || color == 59 || color == 75 || color == 162);
					}
				}
				if (replace) {
					gpBuffer[addr] = TRANS_COLOR;
					if (y >= MICRO_HEIGHT / 2)
						gpBuffer[x + MICRO_WIDTH * ((i - 6) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i - 6) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color;
					else
						gpBuffer[x + MICRO_WIDTH * ((i - 23) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i - 23) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color;
				}
			}
		}
	}

	// copy lower half of 813[5] to 795[12]
	moveLowerMicroPixels(214, 190, TRANS_COLOR, DRAW_HEIGHT);

	// copy part of 815[1] to 791[12]
	moveLimitedMicroPixels(209, 155, 17, MICRO_WIDTH, TRANS_COLOR, DRAW_HEIGHT);

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

static BYTE* patchTownFloorCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	typedef struct {
		int subtileIndex;
		unsigned microIndex;
		int res_encoding;
	} CelMicro;
	const CelMicro micros[] = {
/*  0 */{ 731 - 1, 9, MET_TRANSPARENT }, // move micro 495
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
/* 15 */{  845 - 1, 4, -1 }, //MET_TRANSPARENT },
/* 16 */{  128 - 1, 0, MET_LTRIANGLE }, // mask micros
/* 17 */{  128 - 1, 1, MET_RTRIANGLE },
/* 18 */{  156 - 1, 1, MET_RTRIANGLE },
/* 19 */{  212 - 1, 1, MET_RTRIANGLE },

/* 20 */{  783 - 1, 0, MET_LTRIANGLE }, // fix grass (530)
/* 21 */{  783 - 1, 1, MET_RTRIANGLE },

/* 22 */{  138 - 1, 0, MET_LTRIANGLE }, // merge 94 and 86

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
		shiftMicrosUp(0, 1, TRANS_COLOR, DRAW_HEIGHT);
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
		}
		// move 220[1] to 221[0]
		moveLowerMicroPixels(4, 5, TRANS_COLOR, DRAW_HEIGHT);
		// 220[1]
		shiftMicrosDown(4, 5, TRANS_COLOR, DRAW_HEIGHT);
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
		}
		// 962[1]
		shiftMicrosDown(6, 7, TRANS_COLOR, DRAW_HEIGHT);
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
		}
		// move 218[0] to 219[1]
		moveLowerMicroPixels(7, 8, TRANS_COLOR, DRAW_HEIGHT);
		// 218[0]
		shiftMicrosDown(7, 8, TRANS_COLOR, DRAW_HEIGHT);
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
		}
		// move 1166[0] to 1167[1]
		moveLowerMicroPixels(9, 10, TRANS_COLOR, DRAW_HEIGHT);
		// 1166[0]
		shiftMicrosDown(9, 10, TRANS_COLOR, DRAW_HEIGHT);
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
	// copy 128[0] to 138[0]
	for (int i = 22; i < 23; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 14; y < MICRO_HEIGHT; y++) {
				if (x >= 13 && x <= 22) continue;
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * ((i - 6) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 6) % DRAW_HEIGHT)) * BUFFER_WIDTH;
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
	// fix grass 783[0]
	{
		int i = 20;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 21 + 20 * BUFFER_WIDTH] = 85;
		gpBuffer[addr + 30 + 20 * BUFFER_WIDTH] = 102;
		gpBuffer[addr + 13 + 22 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 14 + 22 * BUFFER_WIDTH] = 85;
		gpBuffer[addr + 14 + 23 * BUFFER_WIDTH] = 85;
	}
	// fix grass 783[1]
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
/*  4 */{ 715 - 1, 11, -1 }, // 480
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
/* 16 */{ 725 - 1, 4, MET_TRANSPARENT }, // 490
/* 17 */{ 725 - 1, 2, MET_TRANSPARENT },
/* 18 */{ 725 - 1, 0, MET_TRANSPARENT },

/* 19 */{ 428 - 1, 4, -1 }, // 286
/* 20 */{ 428 - 1, 2, -1 },
/* 21 */{ 428 - 1, 0, -1 },
/* 22 */{ 418 - 1, 5, MET_SQUARE },
/* 23 */{ 418 - 1, 3, MET_SQUARE },
/* 24 */{ 418 - 1, 1, MET_RTRAPEZOID },
/* 25 */{ 426 - 1, 2, -1 },
/* 26 */{ 426 - 1, 0, -1 },
/* 27 */{ 428 - 1, 1, -1 },
/* 28 */{ 429 - 1, 0, -1 },
/* 29 */{ 419 - 1, 5, MET_SQUARE }, // 277
/* 30 */{ 419 - 1, 3, MET_SQUARE },
/* 31 */{ 419 - 1, 1, MET_RTRAPEZOID },

/* 32 */{ 911 - 1, 9, -1 },
/* 33 */{ 911 - 1, 7, -1 },
/* 34 */{ 911 - 1, 5, -1 },
/* 35 */{ 931 - 1, 5, MET_SQUARE },
/* 36 */{ 931 - 1, 3, MET_SQUARE },
/* 37 */{ 931 - 1, 1, MET_RTRAPEZOID },
/* 38 */{ 402 - 1, 0, -1 }, // 264
/* 39 */{ 954 - 1, 2, -1 },
/* 40 */{ 919 - 1, 9, -1 },
/* 41 */{ 919 - 1, 5, -1 },
/* 42 */{ 927 - 1, 5, MET_SQUARE }, // 637
/* 43 */{ 927 - 1, 1, MET_RTRAPEZOID },
/* 44 */{ 956 - 1, 0, MET_LTRIANGLE }, // unused
		// { 956 - 1, 2, -1 },
/* 45 */{ 954 - 1, 0, MET_LTRIANGLE }, // unused
/* 46 */{ 919 - 1, 7, MET_SQUARE },
/* 47 */{ 918 - 1, 9, -1 },
/* 48 */{ 926 - 1, 5, MET_SQUARE },
/* 49 */{ 927 - 1, 0, -1 }, // 637
/* 50 */{ 918 - 1, 3, -1 },
/* 51 */{ 918 - 1, 2, -1 },
/* 52 */{ 918 - 1, 5, MET_SQUARE },
/* 53 */{ 929 - 1, 0, MET_LTRAPEZOID }, // 639
/* 54 */{ 929 - 1, 1, MET_RTRAPEZOID },
/* 55 */{ 918 - 1, 8, -1 },
/* 56 */{ 926 - 1, 4, MET_SQUARE },
/* 57 */{ 928 - 1, 4, -1 },
/* 58 */{ 920 - 1, 8, MET_SQUARE },
/* 59 */{ 551 - 1, 0, -1 },
/* 60 */{ 552 - 1, 1, -1 },
/* 61 */{ 519 - 1, 0, -1 },
/* 62 */{ 509 - 1, 5, MET_SQUARE }, // 339
/* 63 */{ 509 - 1, 3, MET_SQUARE },
/* 64 */{ 509 - 1, 1, MET_RTRAPEZOID },

/* 65 */{ 510 - 1, 7, -1 },
/* 66 */{ 510 - 1, 5, -1 },
/* 67 */{ 551 - 1, 3, MET_SQUARE },
/* 68 */{ 551 - 1, 1, MET_SQUARE },

/* 69 */{ /*728*/ - 1, 9, -1 },
/* 70 */{ /*728*/ - 1, 7, -1 },
/* 71 */{ /*716*/ - 1, 13, -1 }, // MET_TRANSPARENT },
/* 72 */{ /*716*/ - 1, 11, -1}, // MET_SQUARE },

/* 73 */{ 910 - 1, 9, -1 },
/* 74 */{ 910 - 1, 7, -1 },
/* 75 */{ 930 - 1, 5, MET_TRANSPARENT },
/* 76 */{ 930 - 1, 3, MET_TRANSPARENT },

/* 77 */{ 537 - 1, 0, -1 },
/* 78 */{ 539 - 1, 0, -1 },
/* 79 */{ 529 - 1, 4, MET_SQUARE },
/* 80 */{ 531 - 1, 4, MET_SQUARE },

/* 81 */{ 478 - 1, 0, -1 },             // 317
/* 82 */{ 477 - 1, 1, MET_SQUARE },     // 316
/* 83 */{ 480 - 1, 1, MET_RTRAPEZOID }, // 319
/* 84 */{ 479 - 1, 1, -1 },             // 318
/* 85 */{ 477 - 1, 0, MET_SQUARE },     // 316
/* 86 */{ 480 - 1, 0, MET_LTRAPEZOID }, // 319

/* 87 */{ 517 - 1, 0, -1 },             // move micros for better light propagation
/* 88 */{ 519 - 1, 1, MET_RTRAPEZOID },
	};

	const uint16_t* pSubtiles = (const uint16_t*)minBuf;
	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_TOWN;
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

	// copy 724[0] to 721[2]
	moveMicroPixels(0, 11, TRANS_COLOR, DRAW_HEIGHT);

	// copy 724[1] to 719[2]
	moveUpperMicroPixels(1, 13, TRANS_COLOR, DRAW_HEIGHT);

	// copy 724[3] to 719[4]
	moveUpperMicroPixels(2, 12, TRANS_COLOR, DRAW_HEIGHT);
	// copy 724[3] to 719[2]
	moveLowerMicroPixels(2, 13, TRANS_COLOR, DRAW_HEIGHT);

	// copy 723[1] to 721[4]
	moveUpperMicroPixels(3, 10, TRANS_COLOR, DRAW_HEIGHT);
	// copy 723[1] to 721[2]
	moveLowerMicroPixels(3, 11, TRANS_COLOR, DRAW_HEIGHT);

	// copy 715[11] to 727[7]
	moveMicroPixels(4, 14, TRANS_COLOR, DRAW_HEIGHT);

	// copy 715[9] to 727[5]
	moveMicroPixels(5, 15, TRANS_COLOR, DRAW_HEIGHT);

	// copy part of 715[3] to 725[2]
	moveLimitedUpperMicroPixels(8, 17, 9, 24, TRANS_COLOR, DRAW_HEIGHT);
	// copy part of 715[3] to 725[0]
	moveLimitedLowerMicroPixels(8, 18, 9, 24, TRANS_COLOR, DRAW_HEIGHT);

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
	moveLowerMicroPixels(19, 22, TRANS_COLOR, DRAW_HEIGHT);

	// copy 428[2] to 418[5]
	moveUpperMicroPixels(20, 22, TRANS_COLOR, DRAW_HEIGHT);
	// copy 428[2] to 418[3]
	moveLowerMicroPixels(20, 23, TRANS_COLOR, DRAW_HEIGHT);

	// copy part of 428[0] to 418[3]
	moveLimitedUpperMicroPixels(21, 23, 8, MICRO_WIDTH, TRANS_COLOR, DRAW_HEIGHT);
	// copy part of 428[0] to 418[1]
	moveLimitedLowerMicroPixels(21, 24, 8, MICRO_WIDTH, TRANS_COLOR, DRAW_HEIGHT);

	// copy 426[2] to 419[5]
	moveLowerMicroPixels(25, 29, TRANS_COLOR, DRAW_HEIGHT);

	// copy part of 426[0] to 419[5]
	moveLimitedUpperMicroPixels(26, 29, 0, 9, TRANS_COLOR, DRAW_HEIGHT);
	// copy part of 426[0] to 419[3]
	moveLimitedLowerMicroPixels(26, 30, 0, 9, TRANS_COLOR, DRAW_HEIGHT);

	// mask 428[1]
	maskMicro(27, 9, MICRO_WIDTH, 0, MICRO_HEIGHT, TRANS_COLOR, DRAW_HEIGHT);
	// copy 428[1] to 419[3]
	moveMicroPixels(27, 30, TRANS_COLOR, DRAW_HEIGHT);

	// copy part of 429[0] to 419[3]
	moveLimitedUpperMicroPixels(28, 30, 0, 9, TRANS_COLOR, DRAW_HEIGHT);
	// copy part of 429[0] to 419[1]
	moveLimitedLowerMicroPixels(28, 31, 0, 9, TRANS_COLOR, DRAW_HEIGHT);

	// copy 911[9] to 931[5]
	// copy 911[7] to 931[3]
	// copy 911[5] to 931[1]
	for (int i = 32; i < 35; i++) {
		moveMicroPixels(i, i + 3, TRANS_COLOR, DRAW_HEIGHT);
	}
	// copy 919[9] to 927[5]
	// copy 919[5] to 927[1]
	for (int i = 40; i < 42; i++) {
		moveMicroPixels(i, i + 2, TRANS_COLOR, DRAW_HEIGHT);
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
	// copy 954[2] to 919[7] (-> 927[3])
	moveUpperMicroPixels(39, 46, TRANS_COLOR, DRAW_HEIGHT);
	// copy 954[2] to 927[1]
	moveLowerMicroPixels(39, 43, TRANS_COLOR, DRAW_HEIGHT);

	// copy 918[9] to 926[5]
	moveMicroPixels(47, 48, TRANS_COLOR, DRAW_HEIGHT);
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
	moveMicroPixels(50, 54, TRANS_COLOR, DRAW_HEIGHT);
	// copy 918[2] to 929[0]
	moveMicroPixels(51, 53, TRANS_COLOR, DRAW_HEIGHT);
	// copy 918[8] to 926[4]
	moveMicroPixels(55, 56, TRANS_COLOR, DRAW_HEIGHT);
	// copy 928[4] to 920[8]
	moveMicroPixels(57, 58, TRANS_COLOR, DRAW_HEIGHT);

	// copy 551[0] to 509[5]
	moveUpperMicroPixels(59, 62, TRANS_COLOR, DRAW_HEIGHT);
	// copy 551[0] to 509[3]
	moveLowerMicroPixels(59, 63, TRANS_COLOR, DRAW_HEIGHT);

	// copy 552[1] to 509[3]
	moveMicroPixels(60, 63, TRANS_COLOR, DRAW_HEIGHT);

	// copy 519[0] to 509[3]
	moveUpperMicroPixels(61, 63, TRANS_COLOR, DRAW_HEIGHT);
	// copy 519[0] to 509[1]
	moveLowerMicroPixels(61, 64, TRANS_COLOR, DRAW_HEIGHT);

	// copy 510[7] to 551[3]
	// copy 510[5] to 551[1]
	for (int i = 65; i < 67; i++) {
		moveMicroPixels(i, i + 2, TRANS_COLOR, DRAW_HEIGHT);
	}
	// copy 910[9] to 930[5]
	// copy 910[7] to 930[3]
	for (int i = 73; i < 75; i++) {
		moveMicroPixels(i, i + 2, TRANS_COLOR, DRAW_HEIGHT);
	}
	// copy 537[0] to 529[4]
	// copy 539[0] to 531[4]
	for (int i = 77; i < 79; i++) {
		moveMicroPixels(i, i + 2, TRANS_COLOR, DRAW_HEIGHT);
	}
	// copy 478[0] to 477[1]
	moveUpperMicroPixels(81, 82, TRANS_COLOR, DRAW_HEIGHT);
	// copy 478[0] to 480[1]
	moveLowerMicroPixels(81, 83, TRANS_COLOR, DRAW_HEIGHT);

	// copy 479[1] to 477[0]
	moveUpperMicroPixels(84, 85, TRANS_COLOR, DRAW_HEIGHT);
	// copy 479[1] to 480[0]
	moveLowerMicroPixels(84, 86, TRANS_COLOR, DRAW_HEIGHT);

	// copy 517[0] to 551[1]
	moveUpperMicroPixels(87, 68, TRANS_COLOR, DRAW_HEIGHT);
	// copy 517[0] to 519[1]
	moveLowerMicroPixels(87, 88, TRANS_COLOR, DRAW_HEIGHT);

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
/* 19 */{ 432 - 1, 0, MET_SQUARE }, // 290
/* 20 */{ 432 - 1, 2, MET_SQUARE },
/* 21 */{ 432 - 1, 4, MET_TRANSPARENT },
/* 22 */{ 432 - 1, 6, MET_TRANSPARENT },
/* 23 */{ 432 - 1, 8, MET_TRANSPARENT },

/* 24 */{ 433 - 1, 0, -1 },  // 291
/* 25 */{ 387 - 1, 9, MET_SQUARE },
/* 26 */{ 432 - 1, 1, MET_SQUARE },
/* 27 */{ 430 - 1, 0, MET_SQUARE }, // 288
/* 28 */{ 430 - 1, 2, MET_SQUARE },
/* 29 */{ 430 - 1, 4, MET_TRANSPARENT },

/* 30 */{ 433 - 1, 1, -1 },
/* 31 */{ 424 - 1, 8, MET_SQUARE },
/* 32 */{ 431 - 1, 0, MET_SQUARE }, // 289
/* 33 */{ 430 - 1, 1, MET_SQUARE },
/* 34 */{ 430 - 1, 3, MET_SQUARE },
/* 35 */{ 430 - 1, 5, MET_TRANSPARENT },
/* 36 */{ 430 - 1, 7, MET_TRANSPARENT },
/* 37 */{ 430 - 1, 9, MET_TRANSPARENT },

/* 38 */{ 422 - 1, 8, MET_SQUARE },
/* 39 */{ 431 - 1, 1, MET_SQUARE },
/* 40 */{ 431 - 1, 3, MET_SQUARE },
/* 41 */{ 431 - 1, 5, MET_SQUARE },
/* 42 */{ 431 - 1, 7, MET_TRANSPARENT },
/* 43 */{ 431 - 1, 9, MET_TRANSPARENT },
/* 44 */{ 431 - 1, 11, MET_TRANSPARENT },

/* 45 */{ 422 - 1, 9, MET_SQUARE },
/* 46 */{ 436 - 1, 0, MET_SQUARE },
/* 47 */{ 436 - 1, 2, MET_SQUARE },
/* 48 */{ 436 - 1, 4, MET_SQUARE },
/* 49 */{ 436 - 1, 6, MET_TRANSPARENT },

/* 50 */{ 437 - 1, 0, -1 },
/* 51 */{ 423 - 1, 9, MET_SQUARE },
/* 52 */{ 436 - 1, 1, MET_SQUARE },
/* 53 */{ 434 - 1, 0, MET_SQUARE },
/* 54 */{ 434 - 1, 2, MET_SQUARE },
/* 55 */{ 434 - 1, 4, MET_TRANSPARENT },

/* 56 */{ 437 - 1, 1, -1 },
/* 57 */{ 435 - 1, 0, -1 },
/* 58 */{ 418 - 1, 11, MET_SQUARE },
/* 59 */{ 434 - 1, 1, MET_SQUARE },

/* 60 */{ 435 - 1, 1, -1 },
/* 61 */{ 419 - 1, 13, MET_SQUARE }, // 277

/* 62 */{ 440 - 1, 0, -1 },
/* 63 */{ 408 - 1, 12, MET_SQUARE },

/* 64 */{ 440 - 1, 1, -1 },
/* 65 */{ 441 - 1, 0, -1 },          // 299
/* 66 */{ 406 - 1, 10, MET_SQUARE }, // 268
/* 67 */{ 438 - 1, 0, MET_SQUARE },  // 296

/* 68 */{ 441 - 1, 1, -1 },
/* 69 */{ 412 - 1, 8, MET_SQUARE },
/* 70 */{ 439 - 1, 0, MET_SQUARE },
/* 71 */{ 438 - 1, 1, MET_SQUARE }, // 296
/* 72 */{ 438 - 1, 3, MET_SQUARE },
/* 73 */{ 438 - 1, 5, MET_TRANSPARENT },
/* 74 */{ 438 - 1, 7, MET_TRANSPARENT },
/* 75 */{ 438 - 1, 9, MET_TRANSPARENT },

/* 76 */{ 410 - 1, 8, MET_SQUARE },
/* 77 */{ 439 - 1, 1, MET_SQUARE }, // 297
/* 78 */{ 439 - 1, 3, MET_TRANSPARENT },
/* 79 */{ 439 - 1, 5, MET_TRANSPARENT },
/* 80 */{ 439 - 1, 7, -1 },

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
	moveMicroPixels(1, 2, TRANS_COLOR, DRAW_HEIGHT);
	// copy 522[0] to 524[1]
	moveLowerMicroPixels(3, 5, TRANS_COLOR, DRAW_HEIGHT);
	// copy 522[0] to 521[1]
	moveUpperMicroPixels(3, 6, TRANS_COLOR, DRAW_HEIGHT);

	// copy 517[0] to 551[1] - done in patchTownDoorCel
	// copy 517[0] to 519[1] - done in patchTownDoorCel

	// copy 523[1] to 521[0]
	moveUpperMicroPixels(7, 9, TRANS_COLOR, DRAW_HEIGHT);
	// copy 523[1] to 524[0]
	moveLowerMicroPixels(7, 10, TRANS_COLOR, DRAW_HEIGHT);
	// copy 509[6] to 524[0]
	moveMicroPixels(4, 10, TRANS_COLOR, DRAW_HEIGHT);
	// copy 509[7] to 524[1]
	moveMicroPixels(8, 5, TRANS_COLOR, DRAW_HEIGHT);
	// copy 523[0] to 513[5]
	moveLowerMicroPixels(12, 11, TRANS_COLOR, DRAW_HEIGHT);
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
	moveLowerMicroPixels(19, 18, TRANS_COLOR, DRAW_HEIGHT);
	// shift 432[2..] by half
	shiftMicrosDown(19, 24, TRANS_COLOR, DRAW_HEIGHT);

	// copy 433[0] to 387[9]
	moveLowerMicroPixels(24, 25, TRANS_COLOR, DRAW_HEIGHT);
	// copy 433[0] to 432[1]
	moveUpperMicroPixels(24, 26, TRANS_COLOR, DRAW_HEIGHT);
	// copy 430[0] to 432[1]
	moveLowerMicroPixels(27, 26, TRANS_COLOR, DRAW_HEIGHT);
	// move 430[0 2] down MICRO_HEIGHT / 2
	for (int i = 27; i < 29; i++) {
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
	maskMicro(29, 0, MICRO_WIDTH, MICRO_HEIGHT / 2, MICRO_HEIGHT, TRANS_COLOR, DRAW_HEIGHT);

	// copy 433[1] to 424[8]
	moveLowerMicroPixels(30, 31, TRANS_COLOR, DRAW_HEIGHT);
	// copy 433[1] to 431[0]
	moveUpperMicroPixels(30, 32, TRANS_COLOR, DRAW_HEIGHT);
	// copy 430[1] to 431[0]
	moveLowerMicroPixels(33, 32, TRANS_COLOR, DRAW_HEIGHT);
	// shift 430[3..] by half
	shiftMicrosDown(33, 38, TRANS_COLOR, DRAW_HEIGHT);

	// copy 431[1] to 422[8]
	moveLowerMicroPixels(39, 38, TRANS_COLOR, DRAW_HEIGHT);
	// shift 431[3..] by half
	shiftMicrosDown(39, 45, TRANS_COLOR, DRAW_HEIGHT);

	// copy 436[0] to 422[9]
	moveLowerMicroPixels(46, 45, TRANS_COLOR, DRAW_HEIGHT);
	// move 436[0 2 4] down MICRO_HEIGHT / 2
	for (int i = 46; i < 49; i++) {
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
	maskMicro(49, 0, MICRO_WIDTH, MICRO_HEIGHT / 2, MICRO_HEIGHT, TRANS_COLOR, DRAW_HEIGHT);

	// copy 437[0] to 423[9]
	moveLowerMicroPixels(50, 51, TRANS_COLOR, DRAW_HEIGHT);
	// copy 437[0] to 436[1]
	moveUpperMicroPixels(50, 52, TRANS_COLOR, DRAW_HEIGHT);
	// copy 434[0] to 436[1]
	moveLowerMicroPixels(53, 52, TRANS_COLOR, DRAW_HEIGHT);
	// move 434[0 2] down MICRO_HEIGHT / 2
	for (int i = 53; i < 55; i++) {
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
	maskMicro(55, 0, MICRO_WIDTH, MICRO_HEIGHT / 2, MICRO_HEIGHT, TRANS_COLOR, DRAW_HEIGHT);
	// copy 435[0] to 418[11]
	moveLowerMicroPixels(57, 58, TRANS_COLOR, DRAW_HEIGHT);
	// copy 435[0] to 434[1]
	moveUpperMicroPixels(57, 59, TRANS_COLOR, DRAW_HEIGHT);
	// copy 437[1] to 418[11]
	moveMicroPixels(56, 58, TRANS_COLOR, DRAW_HEIGHT);

	// copy 435[1] to 419[13]
	moveMicroPixels(60, 61, TRANS_COLOR, DRAW_HEIGHT);
	// copy 440[0] to 408[12]
	moveMicroPixels(62, 63, TRANS_COLOR, DRAW_HEIGHT);
	// copy 440[1] to 406[10]
	moveLowerMicroPixels(64, 66, TRANS_COLOR, DRAW_HEIGHT);
	// copy 440[1] to 438[0]
	moveUpperMicroPixels(64, 67, TRANS_COLOR, DRAW_HEIGHT);
	// copy 441[0] to 406[10]
	moveMicroPixels(65, 66, TRANS_COLOR, DRAW_HEIGHT);
	// copy 441[1] to 412[8]
	moveLowerMicroPixels(68, 69, TRANS_COLOR, DRAW_HEIGHT);
	// copy 441[1] to 439[0]
	moveUpperMicroPixels(68, 70, TRANS_COLOR, DRAW_HEIGHT);
	// copy 438[1] to 439[0]
	moveLowerMicroPixels(71, 70, TRANS_COLOR, DRAW_HEIGHT);
	// shift 438[3..] by half
	shiftMicrosDown(71, 76, TRANS_COLOR, DRAW_HEIGHT);

	// copy 439[1] to 410[8]
	moveLowerMicroPixels(77, 76, TRANS_COLOR, DRAW_HEIGHT);
	// shift 439[3..] by half
	shiftMicrosDown(77, 81, TRANS_COLOR, DRAW_HEIGHT);

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

/* 10 */{ /*832*/ - 1, 10, MET_TRANSPARENT },

/* 11 */{ /*834*/ - 1, 10, MET_SQUARE },
/* 12 */{ /*834*/ - 1, 12, MET_TRANSPARENT },
/* 13 */{ /*828*/ - 1, 12, MET_TRANSPARENT },

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
		if (micro.subtileIndex < 0) {
			continue;
		}
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

	// fix bad artifacts
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
	size_t maxCelSize = *celLen + newEntries * MICRO_WIDTH * MICRO_HEIGHT;
	BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
	memset(resCelBuf, 0, maxCelSize);

	CelFrameEntry entries[newEntries];
	xx = 0, yy = MICRO_HEIGHT - 1;
	int idx = 0;
	for (int i = 0; i < newEntries; i++) {
		const CelMicro &micro = micros[i];
		if (micro.subtileIndex >= 0 && micro.res_encoding >= 0) {
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
	if (celBuf == NULL) {
		return NULL;
	}
	// patch subtiles of the cathedral to fix graphical glitch
	celBuf = patchTownCathedralCel(minBuf, minLen, celBuf, celLen);
	return celBuf;
}

BYTE* Town_PatchCelFrames(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	const uint16_t* pSubtiles = (const uint16_t*)minBuf;
	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_TOWN;
	unsigned xx, yy;
	constexpr BYTE TRANS_COLOR = 128;
	constexpr int DRAW_HEIGHT = 8;
	const int batchEntries = (DEFAULT_WIDTH / MICRO_WIDTH) * DRAW_HEIGHT;
	CelFrameEntry entries[batchEntries];

	int idx = 0;
	// patch frames to reduce graininess
	for (int i = 0; i < lengthof(deltaGrain); ) {
		if ((idx % batchEntries) == 0) {
			if (idx != 0) {
				// recreate the new CEL file with the new batch
				size_t maxCelSize = *celLen; // + batchEntries * MICRO_WIDTH * MICRO_HEIGHT;
				BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
				memset(resCelBuf, 0, maxCelSize);

				*celLen = encodeCelMicros(entries, idx, resCelBuf, celBuf, TRANS_COLOR);

				mem_free_dbg(celBuf);
				celBuf = resCelBuf;
			}
			// (re)start the process
			idx = 0;
			pMicrosCel = celBuf;
			memset(&gpBuffer[0], TRANS_COLOR, DRAW_HEIGHT * BUFFER_WIDTH * MICRO_HEIGHT);
			xx = 0; yy = MICRO_HEIGHT - 1;
		}

		// draw the micro to the back-buffer
		const DeltaFrameData* frame = &deltaGrain[i];
		int frameNum = frame->dfFrameNum;
		unsigned index = 0;
		for ( ; index < minLen / sizeof(uint16_t); index++) {
			const uint16_t levelCelBlock = SwapLE16(pSubtiles[index]);
			if ((levelCelBlock & 0xFFF) == frameNum) {
				// assert(idx < batchEntries);
				entries[idx].encoding = levelCelBlock >> 12;
				entries[idx].frameRef = levelCelBlock & 0xFFF;
				entries[idx].frameSrc = &gpBuffer[xx + yy * BUFFER_WIDTH];
				RenderMicro(&gpBuffer[xx + yy * BUFFER_WIDTH], levelCelBlock, DMT_NONE);
				idx++;
				break;
			}
		}
		if (index >= minLen / sizeof(uint16_t)) {
			app_fatal("Town-frame %d missing from .MIN", frameNum);
		}
		// patch the frame (if necessary)
		bool change = false;
		while (i < lengthof(deltaGrain)) {
			frame = &deltaGrain[i];
			if (frame->dfFrameNum != frameNum) {
				break;
			}
			unsigned addr = (xx + frame->dfx) + (yy - (MICRO_HEIGHT - 1 - frame->dfy)) * BUFFER_WIDTH;
			if (gpBuffer[addr] != frame->color) {
				gpBuffer[addr] = frame->color;
				change = true;
			}
			i++;
		}
		if (!change) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				for (int x = 0; x < MICRO_WIDTH; x++) {
					unsigned addr = (xx + x) + (yy - (MICRO_HEIGHT - 1 - y)) * BUFFER_WIDTH;
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
			idx--;
			continue;
		}

		// move to the next micro
		yy += MICRO_HEIGHT;
		if (yy == (DRAW_HEIGHT + 1) * MICRO_HEIGHT - 1) {
			yy = MICRO_HEIGHT - 1;
			xx += MICRO_WIDTH;
		}
	}
	// write the last batch to the CEL file (necessary because encodeCelMicros requires ordered entries)
	if (idx != 0) {
		// create the new CEL file
		size_t maxCelSize = *celLen; // + batchEntries * MICRO_WIDTH * MICRO_HEIGHT;
		BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
		memset(resCelBuf, 0, maxCelSize);

		*celLen = encodeCelMicros(entries, idx, resCelBuf, celBuf, TRANS_COLOR);

		mem_free_dbg(celBuf);
		celBuf = resCelBuf;
		idx = 0;
	}
	// change pixels with color 127 to color 0
	for (int i = 0; ; i++) {
		if ((idx % batchEntries) == 0) {
			if (idx != 0) {
				// recreate the new CEL file with the new batch
				size_t maxCelSize = *celLen; // + batchEntries * MICRO_WIDTH * MICRO_HEIGHT;
				BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
				memset(resCelBuf, 0, maxCelSize);

				*celLen = encodeCelMicros(entries, idx, resCelBuf, celBuf, TRANS_COLOR);

				mem_free_dbg(celBuf);
				celBuf = resCelBuf;
			}
			// (re)start the process
			idx = 0;
			pMicrosCel = celBuf;
			memset(&gpBuffer[0], TRANS_COLOR, DRAW_HEIGHT * BUFFER_WIDTH * MICRO_HEIGHT);
			xx = 0; yy = MICRO_HEIGHT - 1;
		}

		// draw the micro to the back-buffer
		int frameNum = i + 1;
		unsigned index = 0;
		for ( ; index < minLen / sizeof(uint16_t); index++) {
			const uint16_t levelCelBlock = SwapLE16(pSubtiles[index]);
			if ((levelCelBlock & 0xFFF) == frameNum) {
				// assert(idx < batchEntries);
				entries[idx].encoding = levelCelBlock >> 12;
				entries[idx].frameRef = levelCelBlock & 0xFFF;
				entries[idx].frameSrc = &gpBuffer[xx + yy * BUFFER_WIDTH];
				RenderMicro(&gpBuffer[xx + yy * BUFFER_WIDTH], levelCelBlock, DMT_NONE);
				idx++;
				break;
			}
		}
		if (index >= minLen / sizeof(uint16_t)) {
			break;
		}
		// patch the frame if necessary
		bool change = false;
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 0; x < MICRO_WIDTH; x++) {
				unsigned addr = (xx + x) + (yy - (MICRO_HEIGHT - 1 - y)) * BUFFER_WIDTH;
				if (gpBuffer[addr] == 127) {
					gpBuffer[addr] = 0;
					change = true;
				}
			}
		}
		if (!change) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				for (int x = 0; x < MICRO_WIDTH; x++) {
					unsigned addr = (xx + x) + (yy - (MICRO_HEIGHT - 1 - y)) * BUFFER_WIDTH;
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
			idx--;
			continue;
		}

		// move to the next micro
		yy += MICRO_HEIGHT;
		if (yy == (DRAW_HEIGHT + 1) * MICRO_HEIGHT - 1) {
			yy = MICRO_HEIGHT - 1;
			xx += MICRO_WIDTH;
		}
	}
	// write the last batch to the CEL file
	if (idx != 0) {
		// create the new CEL file
		size_t maxCelSize = *celLen; // + batchEntries * MICRO_WIDTH * MICRO_HEIGHT;
		BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
		memset(resCelBuf, 0, maxCelSize);

		*celLen = encodeCelMicros(entries, idx, resCelBuf, celBuf, TRANS_COLOR);

		mem_free_dbg(celBuf);
		celBuf = resCelBuf;
	}
	return celBuf;
}

BYTE* Town_PatchSpec(const BYTE* minBuf, size_t minLen, const BYTE* celBuf, size_t celLen, BYTE* sCelBuf, size_t* sCelLen)
{
	typedef struct {
		int subtileIndex;
		unsigned microIndex;
		// int res_encoding;
	} CelMicro;

	const CelMicro micros[] = {
/*  0 */{ 1171  - 1, 0 }, // 745 -- catacombs
/*  1 */{ 1171  - 1, 2 }, // 745
/*  2 */{ 1171  - 1, 1 }, // 745
/*  3 */{ 1171  - 1, 3 }, // 745
/*  4 */{ 1172  - 1, 0 }, // 746
/*  5 */{ 1173  - 1, 1 }, // 747
/*  6 */{ 1174  - 1, 0 }, // 748
/*  7 */{ 1174  - 1, 1 }, // 748
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
/* 22 */{ 359  - 1, 1 }, // 236 -- trees
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
	constexpr uint16_t SUB_HEADER_SIZE = 14;
	constexpr int FRAME_WIDTH = TILE_WIDTH;
	constexpr int FRAME_HEIGHT = 7 * TILE_HEIGHT;

	DWORD* srcHeaderCursor = (DWORD*)sCelBuf;
	int srcCelEntries = SwapLE32(srcHeaderCursor[0]);
	srcHeaderCursor++;

	if (srcCelEntries != 18) {
		return sCelBuf; // assume it is already done
	}

	// calculate the number of frames in the result
	int resCelEntries = 18 + 5;
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

	bool clippedSource = *(WORD*)(sCelBuf + 4 * (srcCelEntries + 2)) == SwapLE16(SUB_HEADER_SIZE);
	CelImageBuf* sCelBuff = (CelImageBuf*)sCelBuf;
	if (!clippedSource) {
		sCelBuff->ciWidth = FRAME_WIDTH;
	}

	// create the new CEL file
	size_t maxCelSize = (2 * *sCelLen) + (resCelEntries - srcCelEntries) * FRAME_HEIGHT * FRAME_WIDTH;
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
		if (i != 3 - 1 && i != 4 - 1 && i != 5 - 1 && i != 9 - 1 && i != 18 - 1 && i < srcCelEntries) {
			if (clippedSource)
				CelClippedDraw(0, FRAME_HEIGHT - 1, sCelBuf, i + 1, FRAME_WIDTH);
			else
				CelDraw(0, FRAME_HEIGHT - 1, sCelBuff, i + 1);
		}

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
			if (clippedSource)
				CelClippedDraw(FRAME_WIDTH, FRAME_HEIGHT - 1, sCelBuf, 9, FRAME_WIDTH);
			else
				CelDraw(FRAME_WIDTH, FRAME_HEIGHT - 1, sCelBuff, 9);
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
			if (clippedSource)
				CelClippedDraw(FRAME_WIDTH, FRAME_HEIGHT - 1, sCelBuf, 12, FRAME_WIDTH);
			else
				CelDraw(FRAME_WIDTH, FRAME_HEIGHT - 1, sCelBuff, 12);
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
			if (clippedSource)
				CelClippedDraw(FRAME_WIDTH, FRAME_HEIGHT - 1, sCelBuf, 16, FRAME_WIDTH);
			else
				CelDraw(FRAME_WIDTH, FRAME_HEIGHT - 1, sCelBuff, 16);
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
			if (clippedSource)
				CelClippedDraw(FRAME_WIDTH, FRAME_HEIGHT - 1, sCelBuf, 18, FRAME_WIDTH);
			else
				CelDraw(FRAME_WIDTH, FRAME_HEIGHT - 1, sCelBuff, 18);
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
		// create new frame from 416 shifted by (WIDTH/2; -HEIGHT -HEIGHT/2) and 418 shifted by (0; -HEIGHT) to add to 398
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
		// catacombs
		// create new frame from 745 747 to add to 747
		if (i == 22 - 1) {
			// 745
			for (int j = 0; j < 2; j++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 0; x < MICRO_WIDTH; x++) {
						bool grate = false;
						// vertical grates
						if ((x >= 7 && x <= 9) || (x >= 15 && x <= 17) || (x >= 23 && x <= 25) || x >= 31) {
							// if (j == 0 || y > 2 + x / 2)
							if (j == 0 || (y > 6 + 2 * ((x - 7) / 4)))
								grate = true;
						}
						// horizontal grates
						if (j == 1 && x >= 2 && y >= 15 + x / 2 && y <= 17 + x / 2)
							grate = true;
						if (j == 0 && x >= 2) {
							if (y >= 4 + x / 2 && y <= 6 + x / 2)
								grate = true;
							if (y >= -17 + x / 2 && y <= -15 + x / 2)
								grate = true;
						}
						BYTE color = gpBuffer[(x + ((0 + j) / DRAW_HEIGHT) * MICRO_WIDTH) + (y + ((0 + j) % DRAW_HEIGHT) * MICRO_HEIGHT + FRAME_HEIGHT) * BUFFER_WIDTH];
						if (grate)
							gpBuffer[(x + MICRO_WIDTH) + (y + FRAME_HEIGHT - TILE_HEIGHT - MICRO_HEIGHT / 2 - MICRO_HEIGHT * (j + 0)) * BUFFER_WIDTH] = color;
					}
				}
			}
			// 747
			for (int j = 0; j < 1; j++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 0; x < MICRO_WIDTH; x++) {
						bool grate = false;
						// vertical grates
						if ((x >= 7 && x <= 9) || (x >= 15 && x <= 17) || (x >= 23 && x <= 25) || x >= 31) {
							// if (y <= 6 + x / 2)
							if (y <= 9 + 2 * ((x - 7) / 4))
								grate = true;
						}
						BYTE color = gpBuffer[(x + ((5 + j) / DRAW_HEIGHT) * MICRO_WIDTH) + (y + ((5 + j) % DRAW_HEIGHT) * MICRO_HEIGHT + FRAME_HEIGHT) * BUFFER_WIDTH];
						if (grate && color != TRANS_COLOR)
							gpBuffer[(x + MICRO_WIDTH) + (y + FRAME_HEIGHT - MICRO_HEIGHT - (MICRO_HEIGHT / 2) * (j + 0)) * BUFFER_WIDTH] = color;
					}
				}
			}
		}

		// create new frame from 745 746 748 to add to 748
		if (i == 23 - 1) {
			// 745
			for (int j = 0; j < 2; j++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 0; x < MICRO_WIDTH; x++) {
						bool grate = false;
						// vertical grates
						if ((x >= 0 && x <= 1) || (x >= 8 && x <= 10) || (x >= 16 && x <= 18)) {
							if (j == 0 || y > 18 + x / 2)
								grate = true;
						}
						// horizontal grates
						if (j == 0 && x <= 20) {
							if (y >= -1 + x / 2 && y <= 1 + x / 2)
								grate = true;
							if (y >= 20 + x / 2 && y <= 22 + x / 2)
								grate = true;
						}
						BYTE color = gpBuffer[(x + ((2 + j) / DRAW_HEIGHT) * MICRO_WIDTH) + (y + ((2 + j) % DRAW_HEIGHT) * MICRO_HEIGHT + FRAME_HEIGHT) * BUFFER_WIDTH];
						if (grate)
							gpBuffer[(x + MICRO_WIDTH) + (y + FRAME_HEIGHT - TILE_HEIGHT - MICRO_HEIGHT - MICRO_HEIGHT * (j + 0)) * BUFFER_WIDTH] = color;
					}
				}
			}
			// 746
			for (int j = 0; j < 1; j++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 0; x < MICRO_WIDTH; x++) {
						bool grate = false;
						// vertical grates
						if ((x >= 0 && x <= 1) || (x >= 8 && x <= 10) || (x >= 16 && x <= 18)) {
							grate = true;
						}
						// horizontal grates
						if (j == 0 && y >= 4 + x / 2 && y <= 6 + x / 2)
							grate = true;
						BYTE color = gpBuffer[(x + ((4 + j) / DRAW_HEIGHT) * MICRO_WIDTH) + (y + ((4 + j) % DRAW_HEIGHT) * MICRO_HEIGHT + FRAME_HEIGHT) * BUFFER_WIDTH];
						if (grate && color != TRANS_COLOR && color != 107)
							gpBuffer[(x + MICRO_WIDTH) + (y + FRAME_HEIGHT - MICRO_HEIGHT - (MICRO_HEIGHT / 2) * (j + 1)) * BUFFER_WIDTH] = color;
					}
				}
			}
			// 748
			for (int j = 0; j < 1; j++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 0; x < MICRO_WIDTH; x++) {
						bool grate = false;
						// vertical grates
						if (x >= 31 && y <= 5) {
							grate = true;
						}
						BYTE color = gpBuffer[(x + ((6 + j) / DRAW_HEIGHT) * MICRO_WIDTH) + (y + ((6 + j) % DRAW_HEIGHT) * MICRO_HEIGHT + FRAME_HEIGHT) * BUFFER_WIDTH];
						if (grate && color != TRANS_COLOR)
							gpBuffer[(x + 0) + (y + FRAME_HEIGHT - MICRO_HEIGHT - (MICRO_HEIGHT / 2) * (j + 0)) * BUFFER_WIDTH] = color;
					}
				}
			}
			// 748
			for (int j = 0; j < 1; j++) {
				for (int y = 0; y < MICRO_HEIGHT; y++) {
					for (int x = 0; x < MICRO_WIDTH; x++) {
						bool grate = false;
						// vertical grates
						//if ((x >= 0 && x <= 1) || (x >= 8 && x <= 10) || (x >= 16 && x <= 18)) {
						//	if (y <= 5 + x / 2) {
						if (((x >= 0 && x <= 1) && y <= 5) || ((x >= 8 && x <= 10) && y <= 9) || ((x >= 16 && x <= 18) && y <= 13)) {
								grate = true;
						}
						BYTE color = gpBuffer[(x + ((7 + j) / DRAW_HEIGHT) * MICRO_WIDTH) + (y + ((7 + j) % DRAW_HEIGHT) * MICRO_HEIGHT + FRAME_HEIGHT) * BUFFER_WIDTH];
						if (grate && color != TRANS_COLOR)
							gpBuffer[(x + MICRO_WIDTH) + (y + FRAME_HEIGHT - MICRO_HEIGHT - (MICRO_HEIGHT / 2) * (j + 0)) * BUFFER_WIDTH] = color;
					}
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

	mem_free_dbg(sCelBuf);

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
	// use the micros created by patchTownPotCel
	if (pSubtiles[MICRO_IDX(553 - 1, blockSize, 3)] != 0) {
		SetFrameType(553, 1, MET_RTRIANGLE);
		SetFrameType(554, 0, MET_TRANSPARENT);

		MoveMcr(554, 2, 553, 3); // 376[3] -> 377[2]
		MoveMcr(554, 4, 553, 5);
	}
	// eliminate micros after patchTownChopCel
	{
		Blk2Mcr(362, 11);
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
		MoveMcr(732, 8, 731, 9); // 495 -> 496
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

		Blk2Mcr(522, 1);
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

		MoveMcr(480, 4, 480, 0); // 319 +2
		MoveMcr(480, 5, 480, 1);
		MoveMcr(480, 6, 477, 0); // 316 -> 319
		MoveMcr(480, 7, 477, 1);
		MoveMcr(480, 8, 477, 2);
		MoveMcr(480, 9, 477, 3);
		MoveMcr(480, 10, 477, 4);
		MoveMcr(480, 11, 477, 5);
		MoveMcr(480, 12, 477, 6);
		MoveMcr(480, 13, 477, 7);
		MoveMcr(480, 14, 477, 8);

		// patchTownLightCel
		SetFrameType(524, 0, MET_SQUARE);
		SetFrameType(524, 1, MET_SQUARE);
		SetFrameType(521, 0, MET_SQUARE);
		SetFrameType(521, 1, MET_SQUARE);

		Blk2Mcr(509, 6);
		Blk2Mcr(509, 7);
		MoveMcr(509,  6, 524, 0);
		MoveMcr(509,  7, 524, 1);
		MoveMcr(509,  8, 521, 0);
		MoveMcr(509,  9, 521, 1);
		MoveMcr(509, 10, 521, 2);
		MoveMcr(509, 11, 521, 3);
		MoveMcr(509, 12, 521, 4);
		MoveMcr(509, 13, 521, 5);
		MoveMcr(509, 14, 521, 6);
		MoveMcr(509, 15, 521, 7);
		Blk2Mcr(522, 0);

		MoveMcr(531, 6, 539, 2);
		MoveMcr(531, 8, 539, 4);
		MoveMcr(531, 10, 539, 6);
		MoveMcr(531, 12, 539, 8);
		MoveMcr(531, 14, 539, 10);

		MoveMcr(519, 5, 551, 3); // 374[3] -> 349[5]
		MoveMcr(519, 7, 510, 9); // -> 349[7]
		MoveMcr(519, 9, 522, 3); // 352 -> 349
		MoveMcr(519, 11, 522, 5);
		MoveMcr(519, 13, 522, 7);
		MoveMcr(519, 15, 522, 9);

		// SetFrameType(551, 1, MET_SQUARE);
		SetFrameType(519, 1, MET_RTRAPEZOID);
		Blk2Mcr(517, 0);
		MoveMcr(519, 3, 551, 1); // 374[1] -> 349[3]
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

		MoveMcr(432, 15, 432, 6); // 290

		SetFrameType(424, 8, MET_SQUARE);
		SetFrameType(431, 0, MET_SQUARE);
		SetFrameType(430, 1, MET_SQUARE);
		// SetFrameType(430, 3, MET_SQUARE);
		SetFrameType(430, 5, MET_TRANSPARENT);
		MoveMcr(424, 10, 431, 0);
		MoveMcr(424, 12, 430, 1);
		MoveMcr(424, 14, 430, 3);
		Blk2Mcr(433, 1);

		MoveMcr(433, 12, 430, 5); // 288 -> 291
		MoveMcr(433, 14, 430, 7);
		Blk2Mcr(430, 9);

		MoveMcr(430, 14, 430, 6); // 288
		MoveMcr(430, 12, 430, 4);

		SetFrameType(422, 8, MET_SQUARE);
		SetFrameType(431, 1, MET_SQUARE);
		// SetFrameType(431, 3, MET_SQUARE);
		// SetFrameType(431, 5, MET_SQUARE);
		SetFrameType(431, 7, MET_TRANSPARENT);
		MoveMcr(422, 10, 431, 1); // 289 ->
		MoveMcr(422, 12, 431, 3);
		MoveMcr(422, 14, 431, 5);

		MoveMcr(431, 12, 431, 7); // 289
		MoveMcr(431, 14, 431, 9);

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

		MoveMcr(439, 10, 438, 5); // 296[5..9] -> 297
		MoveMcr(439, 12, 438, 7);
		MoveMcr(439, 14, 438, 9);

		SetFrameType(410, 8, MET_SQUARE);
		SetFrameType(439, 1, MET_SQUARE);
		SetFrameType(439, 3, MET_TRANSPARENT);
		// SetFrameType(439, 5, MET_TRANSPARENT);
		MoveMcr(410, 10, 439, 1);
		MoveMcr(410, 12, 439, 3);
		MoveMcr(410, 14, 439, 5);
		Blk2Mcr(439, 7);

		// MoveMcr(?, 14, 440, 4);
		MoveMcr(923, 4, 920, 6);
		MoveMcr(923, 6, 920, 8);

		MoveMcr(478, 11, 478, 7); // 317 +2
		MoveMcr(478, 9, 478, 5);
		MoveMcr(478, 7, 478, 3);
		MoveMcr(478, 5, 478, 1);

		MoveMcr(479, 14, 479, 10); // 318 +2
		MoveMcr(479, 12, 479, 8);
		MoveMcr(479, 10, 479, 6);
		MoveMcr(479, 8, 479, 4);
		MoveMcr(479, 6, 479, 2);
		MoveMcr(479, 4, 479, 0);

		// TODO: eliminate unused subtiles 551 (374), 539 (366), 537 (365), 524 (354), 523 (353), 477 (316), 441 (299), 437 (295), 429 (287), 426 (284)
	}
	// use micros created by patchTownCathedralCel
	if (pSubtiles[MICRO_IDX(847 - 1, blockSize, 2)] != 0) {
		SetFrameType(727, 9, MET_SQUARE); // 492[9]
		SetFrameType(811, 0, MET_TRANSPARENT); // 550[0]
		SetFrameType(811, 2, MET_TRANSPARENT); // 550[2]
		SetFrameType(811, 4, MET_TRANSPARENT); // 550[4]

		SetFrameType(716, 13, MET_SQUARE); // 481[13]
		SetFrameType(716, 11, MET_SQUARE); // 481[11]
		SetFrameType(811, 1, MET_SQUARE); // 550[1]
		SetFrameType(809, 0, MET_SQUARE); // 548[0]
		// SetFrameType(809, 2, MET_SQUARE); // 548[2]
		SetFrameType(809, 4, MET_TRANSPARENT); // 548[4]

		SetFrameType(719, 13, MET_SQUARE); // 484[13]
		SetFrameType(818, 0, MET_SQUARE); // 557[0]
		// SetFrameType(818, 2, MET_SQUARE); // 557[2]
		// SetFrameType(818, 4, MET_SQUARE); // 557[4]
		// SetFrameType(818, 6, MET_SQUARE); // 557[6]
		// SetFrameType(818, 8, MET_TRANSPARENT); // 557[8]
		// SetFrameType(818, 10, MET_TRANSPARENT); // 557[10]

		SetFrameType(719, 12, MET_SQUARE); // 484[12]
		SetFrameType(810, 1, MET_SQUARE); // 549[1]
		SetFrameType(810, 3, MET_SQUARE);  // 549[3]
		SetFrameType(810, 5, MET_SQUARE);  // 549[0]
		SetFrameType(810, 7, MET_SQUARE);  // 549[0]
		SetFrameType(810, 9, MET_TRANSPARENT); // 549[0]

		// SetFrameType(812, 1, MET_RTRAPEZOID); // 551[1]
		// SetFrameType(809, 1, MET_SQUARE); // 548[1]

		SetFrameType(721, 12, MET_SQUARE); // 491[0]
		SetFrameType(810, 0, MET_SQUARE); // 549[0]
		SetFrameType(809, 1, MET_SQUARE); // 548[1]
		// SetFrameType(809, 3, MET_SQUARE); // 548[2]
		SetFrameType(809, 5, MET_TRANSPARENT); // 548[4]
		//SetFrameType(809, 7, MET_TRANSPARENT); // 548[6]

		SetFrameType(819, 0, MET_LTRAPEZOID); // 558[0]
		SetFrameType(816, 0, MET_SQUARE); // 555[0]

		SetFrameType(819, 1, MET_RTRAPEZOID); // 558[1]
		SetFrameType(816, 1, MET_SQUARE); // 555[1]

		SetFrameType(816, 12, MET_SQUARE); // 555[12]
		SetFrameType(846, 0, MET_SQUARE); // 585[0]
		SetFrameType(837, 1, MET_SQUARE); // 576[1]
		// SetFrameType(837, 3, MET_SQUARE); // 576[3]
		SetFrameType(843, 0, MET_SQUARE); // 582[0]

		SetFrameType(816, 13, MET_SQUARE); // 555[13]
		SetFrameType(846, 1, MET_SQUARE); // 585[1]
		SetFrameType(843, 1, MET_SQUARE); // 582[1]

		SetFrameType(841, 0, MET_SQUARE); // 580[0]
		// SetFrameType(841, 2, MET_SQUARE); // 580[2]
//		//SetFrameType(841, 4, MET_TRANSPARENT); // 580[4]

		SetFrameType(838, 1, MET_TRANSPARENT); // 577[1]
		// SetFrameType(836, 1, MET_TRANSPARENT); // 575[1]
		// SetFrameType(836, 5, MET_TRANSPARENT); // 575[5]

		//SetFrameType(845, 0, MET_TRANSPARENT); // 584[0]
		//SetFrameType(845, 2, MET_TRANSPARENT); // 584[2]
		//SetFrameType(845, 4, MET_TRANSPARENT); // 584[4]
		//SetFrameType(845, 6, MET_TRANSPARENT); // 584[6]

		SetFrameType(839, 4, MET_SQUARE); // 578[4]
		SetFrameType(844, 1, MET_SQUARE); // 583[1]
		// SetFrameType(844, 3, MET_SQUARE); // 583[3]
		// SetFrameType(844, 5, MET_SQUARE); // 583[5]
		// SetFrameType(844, 7, MET_SQUARE); // 583[7]
		SetFrameType(844, 9, MET_TRANSPARENT); // 583[9]
		SetFrameType(844, 11, MET_TRANSPARENT); // 583[11]

		SetFrameType(805, 12, MET_SQUARE); // 544[12]
		SetFrameType(839, 0, MET_SQUARE); // 578[1]
		SetFrameType(842, 0, MET_SQUARE); // 581[0]
		SetFrameType(817, 1, MET_SQUARE); // 556[1]
		//SetFrameType(817, 3, MET_SQUARE); // 556[3]
		//SetFrameType(817, 5, MET_SQUARE); // 556[5]
		//SetFrameType(817, 7, MET_SQUARE); // 556[7]
		//SetFrameType(817, 9, MET_SQUARE); // 556[9]
		//SetFrameType(817, 11, MET_SQUARE); // 556[11]

		SetFrameType(842, 1, MET_SQUARE); // 581[1]
		SetFrameType(839, 1, MET_SQUARE); // 578[1]
		SetFrameType(839, 5, MET_SQUARE); // 578[5]
		SetFrameType(848, 0, MET_SQUARE); // 587[0]
		//SetFrameType(848, 2, MET_SQUARE); // 587[0]
		//SetFrameType(848, 4, MET_SQUARE); // 587[0]
		SetFrameType(848, 6, MET_TRANSPARENT); // 587[0]
		//SetFrameType(848, 8, MET_TRANSPARENT); // 587[8]

		SetFrameType(849, 0, MET_TRANSPARENT); // 588[0]
		SetFrameType(847, 0, MET_TRANSPARENT); // 586[0]
		SetFrameType(820, 12, MET_TRANSPARENT); // 559[12]
		// SetFrameType(840, 1, MET_TRANSPARENT); // 579[1]
		// SetFrameType(840, 3, MET_TRANSPARENT); // 579[3]

		SetFrameType(805, 13, MET_SQUARE); // 544[13]
		SetFrameType(822, 0, MET_SQUARE); // 561[12]
		// SetFrameType(822, 2, MET_SQUARE); // 561[12]
		// SetFrameType(822, 4, MET_SQUARE); // 561[12]
		// SetFrameType(822, 6, MET_SQUARE); // 561[12]
		// SetFrameType(822, 8, MET_SQUARE); // 561[12]
		// SetFrameType(822, 10, MET_SQUARE); // 561[12]

		// SetFrameType(823, 1, MET_RTRAPEZOID); // 562
		SetFrameType(820, 1, MET_SQUARE); // 559[12]
		SetFrameType(826, 0, MET_SQUARE); // 565
		SetFrameType(826, 2, MET_SQUARE);
		SetFrameType(826, 4, MET_SQUARE);
		SetFrameType(826, 6, MET_SQUARE);
		SetFrameType(826, 8, MET_SQUARE);
		SetFrameType(826, 10, MET_TRANSPARENT);

		SetFrameType(823, 0, MET_SQUARE); // 562
		SetFrameType(820, 0, MET_SQUARE); // 559[12]
		SetFrameType(781, 0, MET_LTRAPEZOID); // 528

		SetFrameType(806, 1, MET_SQUARE); // 545
		SetFrameType(806, 3, MET_SQUARE);
		SetFrameType(806, 5, MET_SQUARE);
		SetFrameType(806, 7, MET_SQUARE);
		SetFrameType(806, 9, MET_SQUARE);
		SetFrameType(806, 11, MET_SQUARE);

		SetFrameType(823, 1, MET_SQUARE); // 562
		SetFrameType(781, 1, MET_RTRAPEZOID); // 528
		SetFrameType(787, 0, MET_SQUARE); // 534
		SetFrameType(787, 2, MET_SQUARE);
		SetFrameType(787, 4, MET_SQUARE);
		SetFrameType(787, 6, MET_SQUARE);
		SetFrameType(787, 8, MET_SQUARE);
		SetFrameType(787, 10, MET_SQUARE);
		SetFrameType(827, 0, MET_SQUARE); // 566
		SetFrameType(824, 0, MET_SQUARE); // 563
		SetFrameType(785, 12, MET_SQUARE); // 532
		SetFrameType(827, 1, MET_SQUARE); // 566
		SetFrameType(824, 1, MET_SQUARE); // 563
		SetFrameType(785, 1, MET_RTRAPEZOID); // 532
		SetFrameType(791, 0, MET_SQUARE); // 537
		SetFrameType(791, 2, MET_SQUARE);
		SetFrameType(791, 4, MET_SQUARE);
		SetFrameType(791, 6, MET_SQUARE);
		SetFrameType(791, 8, MET_SQUARE);
		SetFrameType(791, 10, MET_SQUARE);
		SetFrameType(791, 12, MET_TRANSPARENT);

		SetFrameType(830, 0, MET_SQUARE); // 569
		SetFrameType(830, 2, MET_SQUARE);
		SetFrameType(830, 4, MET_SQUARE);
		SetFrameType(830, 6, MET_SQUARE);
		SetFrameType(830, 8, MET_SQUARE);
		SetFrameType(830, 10, MET_TRANSPARENT);
		SetFrameType(789, 12, MET_SQUARE); // 536
		SetFrameType(831, 0, MET_SQUARE); // 570
		SetFrameType(828, 0, MET_SQUARE); // 567

		SetFrameType(831, 1, MET_SQUARE); // 570
		SetFrameType(828, 1, MET_SQUARE); // 567
		SetFrameType(834, 0, MET_SQUARE); // 573
		SetFrameType(834, 2, MET_SQUARE);
		SetFrameType(834, 4, MET_SQUARE);
		SetFrameType(834, 6, MET_SQUARE);
		SetFrameType(834, 8, MET_SQUARE);
		SetFrameType(834, 10, MET_TRANSPARENT);

		SetFrameType(835, 0, MET_SQUARE); // 574
		SetFrameType(832, 0, MET_SQUARE); // 571
		SetFrameType(793, 12, MET_SQUARE); // 538
		SetFrameType(789, 1, MET_RTRAPEZOID); // 536
		SetFrameType(795, 0, MET_SQUARE); // 540
		SetFrameType(795, 2, MET_SQUARE);
		SetFrameType(795, 4, MET_SQUARE);
		SetFrameType(795, 6, MET_SQUARE);
		SetFrameType(795, 8, MET_SQUARE);
		SetFrameType(795, 10, MET_SQUARE);
		SetFrameType(795, 12, MET_TRANSPARENT);
		SetFrameType(835, 1, MET_SQUARE); // 574
		SetFrameType(832, 1, MET_SQUARE); // 571
		SetFrameType(797, 12, MET_SQUARE); // 541
		SetFrameType(833, 1, MET_SQUARE); // 572
		SetFrameType(833, 3, MET_TRANSPARENT);
		//SetFrameType(833, 5, MET_TRANSPARENT);
		//SetFrameType(833, 7, MET_TRANSPARENT);
		//SetFrameType(833, 9, MET_TRANSPARENT);
		SetFrameType(793, 1, MET_RTRAPEZOID); // 538
		SetFrameType(799, 0, MET_SQUARE); // 542
		SetFrameType(799, 2, MET_SQUARE);
		SetFrameType(799, 4, MET_SQUARE);
		SetFrameType(799, 6, MET_SQUARE);
		SetFrameType(799, 8, MET_SQUARE);
		SetFrameType(799, 10, MET_SQUARE)
		SetFrameType(797, 1, MET_TRANSPARENT); // 541
		SetFrameType(815, 1, MET_TRANSPARENT); // 554
		// SetFrameType(803, 0, MET_TRANSPARENT); // 543

		SetFrameType(814, 0, MET_TRANSPARENT); // 553
		SetFrameType(813, 1, MET_TRANSPARENT); // 552
		SetFrameType(813, 3, MET_TRANSPARENT);
		SetFrameType(813, 5, MET_TRANSPARENT);

		SetFrameType(808, 1, MET_RTRAPEZOID); // 547
		SetFrameType(805, 1, MET_SQUARE); // 544

		SetFrameType(808, 0, MET_LTRAPEZOID); // 547
		SetFrameType(805, 0, MET_SQUARE); // 544

		Blk2Mcr(822, 1); // 561
		Blk2Mcr(806, 13); // 545
		Blk2Mcr(787, 12); // 534
		Blk2Mcr(826, 1); // 565
		Blk2Mcr(821, 1); // 560
		Blk2Mcr(825, 0); // 564

		Blk2Mcr(830, 12); // 569
		Blk2Mcr(825, 1); // 564
		Blk2Mcr(830, 1); // 569
		Blk2Mcr(829, 0); // 568
		Blk2Mcr(834, 12); // 573

		Blk2Mcr(834, 1); // 573
		Blk2Mcr(829, 1); // 568
		Blk2Mcr(833, 0); // 572
		Blk2Mcr(806, 0); // 545
		Blk2Mcr(807, 1); // 546

		MoveMcr(727, 11, 811, 0); // 550 -> 492
		MoveMcr(727, 13, 811, 2);
		MoveMcr(727, 15, 811, 4);

		Blk2Mcr(728, 9);
		MoveMcr(728, 9, 716, 13); // 481[13] -> 493[9]
		MoveMcr(728, 11, 811, 1); // 550[1] -> 493[11]
		Blk2Mcr(728, 7);

		MoveMcr(719, 15, 818, 0);

		Blk2Mcr(812, 1); // 551[1]

		MoveMcr(719, 14, 810, 1); // 549[1] -> 484
		MoveMcr(810, 8, 810, 3); // 549[3]
		MoveMcr(810, 10, 810, 5);
		MoveMcr(810, 12, 810, 7);
		MoveMcr(810, 14, 810, 9);

		Blk2Mcr(818, 1);
		MoveMcr(810, 9, 818, 2); // 557[2] -> 549
		MoveMcr(810, 11, 818, 4); // 557[4]
		MoveMcr(810, 13, 818, 6);
		MoveMcr(810, 15, 818, 8);
		MoveMcr(818, 9, 818, 10);
		Blk2Mcr(818, 12);
		MoveMcr(818, 11, 838, 1);
		MoveMcr(818, 13, 836, 1); // 575[] -> 557[15]
		MoveMcr(818, 15, 836, 3); // 575[3] -> 557[15]

		Blk2Mcr(817, 0);
		MoveMcr(817, 4, 817, 1); // 556[1]
		MoveMcr(817, 6, 817, 3); // 556[3]
		MoveMcr(817, 8, 817, 5);
		MoveMcr(817, 10, 817, 7);
		MoveMcr(817, 12, 817, 9);
		MoveMcr(817, 14, 817, 11);
		Blk2Mcr(817, 13);

		Blk2Mcr(812, 0); // 551[0]
		Blk2Mcr(837, 0); // 576

		MoveMcr(837, 12, 837, 3); // 576

		MoveMcr(837, 10, 837, 1); // 576

		MoveMcr(837, 11, 841, 0); // 580[0] -> 576[11]

		Blk2Mcr(837, 5); // 576
		MoveMcr(837, 14, 846, 0); // 585[0] -> 576[14]
		MoveMcr(837, 13, 841, 2); // 580[2] -> 576[13]
		MoveMcr(837, 15, 846, 1); // 585[1] -> 576[15] 555 816
		Blk2Mcr(841, 4); // 580
		Blk2Mcr(841, 1); // 580
		Blk2Mcr(840, 0); // 579
		Blk2Mcr(840, 5); // 579[5]
		Blk2Mcr(821, 0); // 560
		Blk2Mcr(826, 12); // 565

		MoveMcr(819, 2, 816, 0); // 555 -> 558
		MoveMcr(819, 3, 816, 1);
		MoveMcr(819, 4, 816, 2);
		MoveMcr(819, 5, 816, 3);
		MoveMcr(819, 6, 816, 4);
		MoveMcr(819, 7, 816, 5);
		MoveMcr(819, 8, 816, 6);
		MoveMcr(819, 9, 816, 7);
		MoveMcr(819, 10, 816, 8);
		MoveMcr(819, 11, 816, 9);
		MoveMcr(819, 12, 816, 10);
		MoveMcr(819, 13, 816, 11);
		MoveMcr(819, 14, 816, 12);
		MoveMcr(819, 15, 816, 13);

		MoveMcr(808, 2, 805, 0); // 544 -> 547
		MoveMcr(808, 3, 805, 1);
		MoveMcr(808, 4, 805, 2);
		MoveMcr(808, 5, 805, 3);
		MoveMcr(808, 6, 805, 4);
		MoveMcr(808, 7, 805, 5);
		MoveMcr(808, 8, 805, 6);
		MoveMcr(808, 9, 805, 7);
		MoveMcr(808, 10, 805, 8);
		MoveMcr(808, 11, 805, 9);
		MoveMcr(808, 12, 805, 10);
		MoveMcr(808, 13, 805, 11);
		MoveMcr(808, 14, 805, 12);
		MoveMcr(808, 15, 805, 13);

		// MoveMcr(843, 14, 843, 10); // 582
		MoveMcr(843, 12, 843, 8);
		MoveMcr(843, 10, 843, 6);
		MoveMcr(843, 8, 843, 4);
		MoveMcr(843, 6, 843, 2);
		MoveMcr(843, 4, 843, 0);
		MoveMcr(843, 15, 843, 11);
		MoveMcr(843, 13, 843, 9);
		MoveMcr(843, 11, 843, 7);
		MoveMcr(843, 9, 843, 5);
		MoveMcr(843, 7, 843, 3);
		MoveMcr(843, 5, 843, 1);

		MoveMcr(839, 13, 839, 5); // 578[13]
		MoveMcr(839, 12, 839, 4); // 578[12]
		MoveMcr(839, 11, 839, 3); // 578[11]
		MoveMcr(839, 10, 839, 2); // 578[10]
		MoveMcr(839,  9, 839, 1); // 578[9]
		MoveMcr(839,  8, 839, 0); // 578[8]

		MoveMcr(839,  6, 842, 0); // 581[0] -> 578[6]
		MoveMcr(839,  7, 842, 1); // 581[1] -> 578[7]

		Blk2Mcr(844, 0);
		MoveMcr(839, 14, 844, 1); // 583[1] -> 578[14]
		MoveMcr(844, 6, 844, 3); // 583[3]
		MoveMcr(844, 8, 844, 5);
		MoveMcr(844, 10, 844, 7);
		MoveMcr(844, 12, 844, 9);
		MoveMcr(844, 14, 844, 11);

		Blk2Mcr(845, 1); // 584
		Blk2Mcr(845, 8); // 584
		MoveMcr(838, 15, 845, 6); // 584[6] -> 577
		MoveMcr(838, 13, 845, 4); // 584[4] -> 577
		MoveMcr(838, 11, 845, 2); // 584[2] -> 577
		MoveMcr(838, 9, 845, 0); // 584[0] -> 577

		MoveMcr(838, 14, 836, 12); // 575[12] -> 577
		MoveMcr(838, 7, 836, 5); // 575[5] -> 577

		MoveMcr(847, 15, 847, 7); // 586[15]
		MoveMcr(847, 14, 847, 6); // 586[14]
		MoveMcr(847, 13, 847, 5); // 586[13]
		MoveMcr(847, 12, 847, 4); // 586[12]
		// MoveMcr(847, 11, 847, 3); // 586[11]
		MoveMcr(847, 10, 847, 2); // 586[10]
		HideMcr(847, 1); // 586[1]
		MoveMcr(847,  8, 847, 0); // 586[8]
		MoveMcr(847, 6, 849, 0); // 588[0] -> 586[6]

		MoveMcr(839, 15, 848, 0); // 587[0] -> 578[15]
		MoveMcr(844, 7, 848, 2); // 587[2] -> 583
		MoveMcr(844, 9, 848, 4); // 587[4] -> 583
		MoveMcr(844, 11, 848, 6); // 587[6] -> 583
		MoveMcr(844, 13, 848, 8); // 587[8] -> 583
		Blk2Mcr(848, 10); // 587[10]
		Blk2Mcr(848, 1); // 587[1]

		MoveMcr(817, 5, 822, 0); // 561 -> 556
		MoveMcr(817, 7, 822, 2);
		MoveMcr(817, 9, 822, 4);
		MoveMcr(817, 11, 822, 6);
		MoveMcr(817, 13, 822, 8);
		MoveMcr(817, 15, 822, 10);
		Blk2Mcr(822, 12); // 561[12]

		MoveMcr(840, 14, 840, 3); // 579[3] -> 579[14]
		MoveMcr(840, 12, 840, 1); // 579[1] -> 579[12]
		MoveMcr(840, 10, 820, 12); // 559[12] -> 579[10]

		MoveMcr(820, 14, 820, 10); // 559 + 2
		MoveMcr(820, 12, 820, 8);
		MoveMcr(820, 10, 820, 6);
		MoveMcr(820, 8, 820, 4);
		MoveMcr(820, 6, 820, 2);
		MoveMcr(820, 4, 820, 0);
		MoveMcr(820, 5, 820, 1);
		MoveMcr(820, 2, 823, 0); // 562 -> 559
		MoveMcr(820, 3, 823, 1);
		MoveMcr(820, 7, 826, 0); // 565 -> 559
		MoveMcr(820, 9, 826, 2);
		MoveMcr(820, 11, 826, 4);
		MoveMcr(820, 13, 826, 6);
		MoveMcr(820, 15, 826, 8);
		MoveMcr(840, 11, 826, 10); // 565[10] -> 579[11]

		Blk2Mcr(811, 6); // 550
		Blk2Mcr(809, 6); // 548
		MoveMcr(811, 7, 809, 4); // 548[4] -> 550
		MoveMcr(811, 5, 809, 2); // 548[2]
		MoveMcr(811, 3, 809, 0); // 548[0]

		MoveMcr(809, 14, 809, 7); // 548[7]
		MoveMcr(809, 12, 809, 5); // 548[5]
		MoveMcr(809, 10, 809, 3); // 548[3]
		MoveMcr(809, 8, 809, 1); // 548[1]
		MoveMcr(809, 6, 810, 0); // 549[0] -> 548[7]

		// MoveMcr(781, 14, 806, 13); // 545 -> 528
		MoveMcr(781, 12, 806, 11);
		MoveMcr(781, 10, 806, 9);
		MoveMcr(781, 8, 806, 7);
		MoveMcr(781, 6, 806, 5);
		MoveMcr(781, 4, 806, 3);
		MoveMcr(781, 2, 806, 1);

		// MoveMcr(781, 15, 787, 12); // 534 -> 528
		MoveMcr(781, 13, 787, 10);
		MoveMcr(781, 11, 787, 8);
		MoveMcr(781, 9, 787, 6);
		MoveMcr(781, 7, 787, 4);
		MoveMcr(781, 5, 787, 2);
		MoveMcr(781, 3, 787, 0);

		MoveMcr(785, 13, 791, 10); // 537 -> 532
		MoveMcr(785, 11, 791, 8);
		MoveMcr(785, 9, 791, 6);
		MoveMcr(785, 7, 791, 4);
		MoveMcr(785, 5, 791, 2);
		MoveMcr(785, 3, 791, 0);

		MoveMcr(824, 15, 830, 10); // 569 -> 563
		MoveMcr(824, 13, 830, 8);
		MoveMcr(824, 11, 830, 6);
		MoveMcr(824, 9, 830, 4);
		MoveMcr(824, 7, 830, 2);
		MoveMcr(824, 5, 830, 0);
		MoveMcr(824, 3, 824, 1); // 563 + 1
		MoveMcr(824, 14, 824, 12);
		MoveMcr(824, 12, 824, 10);
		MoveMcr(824, 10, 824, 8);
		MoveMcr(824, 8, 824, 6);
		MoveMcr(824, 6, 824, 4);
		MoveMcr(824, 4, 824, 2);
		MoveMcr(824, 2, 824, 0);
		MoveMcr(824, 0, 827, 0); // 566 -> 563
		MoveMcr(824, 1, 827, 1);

		MoveMcr(828, 15, 834, 10); // 573 -> 567
		MoveMcr(828, 13, 834, 8);
		MoveMcr(828, 11, 834, 6);
		MoveMcr(828, 9, 834, 4);
		MoveMcr(828, 7, 834, 2);
		MoveMcr(828, 5, 834, 0);
		MoveMcr(828, 3, 828, 1) // 567 + 1
		MoveMcr(828, 14, 828, 12);
		MoveMcr(828, 12, 828, 10);
		MoveMcr(828, 10, 828, 8);
		MoveMcr(828, 8, 828, 6);
		MoveMcr(828, 6, 828, 4);
		MoveMcr(828, 4, 828, 2);
		MoveMcr(828, 2, 828, 0);
		MoveMcr(828, 0, 831, 0); // 570 -> 567
		MoveMcr(828, 1, 831, 1);

		MoveMcr(789, 13, 795, 10); // 540 -> 536
		MoveMcr(789, 11, 795, 8);
		MoveMcr(789, 9, 795, 6);
		MoveMcr(789, 7, 795, 4);
		MoveMcr(789, 5, 795, 2);
		MoveMcr(789, 3, 795, 0);

		MoveMcr(793, 13, 799, 10); // 542 -> 538
		MoveMcr(793, 11, 799, 8);
		MoveMcr(793, 9, 799, 6);
		MoveMcr(793, 7, 799, 4);
		MoveMcr(793, 5, 799, 2);
		MoveMcr(793, 3, 799, 0);

		MoveMcr(833, 14, 833, 9); // 572
		MoveMcr(833, 12, 833, 7);
		MoveMcr(833, 10, 833, 5);
		MoveMcr(833, 8, 833, 3);
		MoveMcr(833, 6, 833, 1);

		MoveMcr(832, 14, 832, 10); // 571 + 2
		ReplaceMcr(832, 12, 832, 8);
		MoveMcr(832, 10, 832, 6);
		MoveMcr(832, 8, 832, 4);
		MoveMcr(832, 6, 832, 2);
		MoveMcr(832, 4, 832, 0);
		MoveMcr(832, 15, 832, 11);
		MoveMcr(832, 13, 832, 9);
		MoveMcr(832, 11, 832, 7);
		MoveMcr(832, 9, 832, 5);
		MoveMcr(832, 7, 832, 3);
		MoveMcr(832, 5, 832, 1);

		MoveMcr(832, 2, 835, 0); // 574 -> 571
		MoveMcr(832, 3, 835, 1);

		MoveMcr(814, 2, 799, 12); // 542[12] -> 553[2]
		MoveMcr(814, 4, 795, 12); // 540[12] -> 553[4]
		MoveMcr(797, 3, 815, 1); // 554[1] -> 541[3]
		MoveMcr(797, 5, 813, 1); // 552[1] -> 541[5]
		MoveMcr(797, 7, 813, 3); // 552[3] -> 541[7]
		MoveMcr(815, 1, 791, 12); // 537[12] -> 554[1]

		MoveMcr(813, 8, 797, 12); // 541[12] -> 552[8]
		MoveMcr(813, 6, 797, 10); // 541[10] -> 552[6]
		MoveMcr(813, 4, 797, 8); // 541[8] -> 552[4]

		// add floor micros
		SetMcr(847, 0, 1094, 0); // 726[0] -> 586[0]
		SetMcr(847, 1, 1094, 1); // 726[1] -> 586[1]

		SetMcr(838, 0, 1027, 0); // 696[0] -> 577[0]
		SetMcr(838, 1, 1027, 1); // 696[1] -> 577[1]
		SetMcr(838, 3, 1027, 3); // 696[3] -> 577[3]
		SetMcr(836, 0, 1094, 0); // 726[0] -> 575[0]
		SetMcr(836, 1, 1094, 1); // 726[1] -> 575[1]

		Blk2Mcr(803, 0); // 543
		Blk2Mcr(803, 1);

		// TODO: eliminate unused subtiles 846 (585), 845 (584), 848 (587) 812 (551) 811 (550) 841 (580), 849 (588), 816 (555), 842 (581), 822 (561), 826 (565), 821 (560),
		// 825 (564), 830 (569), 829 (568), 834 (573), 835 (574), 827 (566), 831 (570), 823 (562), 803 (543), 806 (545), 805 (544), 724 (489)
	}
	// prepare new subtiles for Town_PatchSpec
	{
		// catacombs
		ReplaceMcr(1171, 0, 1175, 0); // 745 <- 749
		ReplaceMcr(1171, 1, 1175, 1);
		ReplaceMcr(1171, 2, 1175, 2);
		ReplaceMcr(1171, 3, 1175, 3);
		ReplaceMcr(1172, 0, 1176, 0); // 746 <- 750
		ReplaceMcr(1173, 1, 1177, 1); // 747 <- 751
		ReplaceMcr(1174, 0, 1178, 0); // 748 <- 752
		ReplaceMcr(1174, 1, 1178, 1);

		// trees
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

		// restore 801
		MoveMcr(774, 9, 801, 9); // 521
		MoveMcr(774, 11, 801, 11);
		ReplaceMcr(774, 0, 19, 0);
		ReplaceMcr(774, 1, 19, 1);
		// TODO: eliminate unused subtiles 773 (520), 775 (522), 779 (526), 1171 (745), 1172 (746)
		Blk2Mcr(773, 0);
		Blk2Mcr(773, 1);
		Blk2Mcr(775, 0);
		Blk2Mcr(775, 1);
		Blk2Mcr(779, 0);
		Blk2Mcr(779, 1);
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
	ReplaceMcr(795, 1, 13, 1); // lost details
	ReplaceMcr(850, 1, 9, 1);  // lost details
	ReplaceMcr(892, 0, 92, 0);    // lost details
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
