/**
 * @file drlp_l3.cpp
 *
 * Implementation of the caves level patching functionality.
 */
#include "all.h"
#include "engine/render/dun_render.h"

DEVILUTION_BEGIN_NAMESPACE

static BYTE shadowColorCaves(BYTE color)
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
static BYTE* patchCavesFloorCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	typedef struct {
		int subtileIndex;
		unsigned microIndex;
		int res_encoding;
	} CelMicro;

	const CelMicro micros[] = {
/*  0 */{ 537 - 1, 2, -1 },              // used to block subsequent calls
/*  1 */{ 537 - 1, 0, MET_LTRIANGLE },   // fix/mask door
/*  2 */{ 537 - 1, 1, MET_RTRIANGLE },
/*  3 */{ 538 - 1, 0, MET_TRANSPARENT },
/*  4 */{ 538 - 1, 1, -1 },              // unused
/*  5 */{ 538 - 1, 2, MET_TRANSPARENT },
/*  6 */{ 538 - 1, 4, MET_TRANSPARENT },
/*  7 */{ 539 - 1, 0, MET_TRANSPARENT },
/*  8 */{ 540 - 1, 0, MET_LTRIANGLE },
/*  9 */{ 540 - 1, 1, MET_RTRIANGLE },
/* 10 */{ 541 - 1, 0, MET_LTRIANGLE },
/* 11 */{ 541 - 1, 1, MET_TRANSPARENT },
/* 12 */{ 541 - 1, 3, MET_TRANSPARENT },
/* 13 */{ 541 - 1, 5, MET_TRANSPARENT },
/* 14 */{ 542 - 1, 1, -1 }, // unused

/* 15 */{ 197 - 1, 1, MET_RTRIANGLE },   // change type
/* 16 */{ 501 - 1, 0, -1 },              // unused
/* 17 */{ 494 - 1, 0, -1 },
/* 18 */{ 496 - 1, 0, MET_LTRIANGLE },
/* 19 */{ 495 - 1, 1, MET_RTRIANGLE },

/* 20 */{ 100 - 1, 0, MET_LTRIANGLE },   // one-subtile islands
/* 21 */{ 126 - 1, 0, MET_LTRIANGLE },
/* 22 */{  83 - 1, 1, MET_RTRIANGLE },

/* 23 */{ 544 - 1, 1, -1 },             // new shadows
/* 24 */{ 202 - 1, 1, -1 },
/* 25 */{ 551 - 1, 1, MET_RTRIANGLE },

/* 26 */{ 506 - 1, 0, -1 },
/* 27 */{ 543 - 1, 0, MET_LTRIANGLE },
/* 28 */{ 528 - 1, 0, MET_LTRIANGLE },

/* 29 */{ 543 - 1, 1, -1 },
/* 30 */{ 506 - 1, 1, -1 },
/* 31 */{ 528 - 1, 1, MET_RTRIANGLE },

/* 32 */{ 476 - 1, 1, MET_RTRIANGLE }, // fix shadows
/* 33 */{ 484 - 1, 0, MET_LTRIANGLE },
/* 34 */{ 490 - 1, 0, MET_LTRIANGLE },
/* 35 */{ 492 - 1, 0, MET_TRANSPARENT },
/* 36 */{ 492 - 1, 1, MET_RTRIANGLE },
/* 37 */{ 497 - 1, 0, MET_LTRIANGLE },
/* 38 */{ 499 - 1, 0, MET_TRANSPARENT },
/* 39 */{ 499 - 1, 1, MET_RTRIANGLE },
/* 40 */{ 500 - 1, 0, MET_TRANSPARENT },
/* 41 */{ 500 - 1, 1, MET_RTRIANGLE },
/* 42 */{ 502 - 1, 0, MET_LTRIANGLE },
/* 43 */{ 502 - 1, 1, MET_TRANSPARENT },
/* 44 */{ 535 - 1, 0, MET_LTRIANGLE },
/* 45 */{ 535 - 1, 1, MET_RTRIANGLE },
/* 46 */{ 542 - 1, 0, MET_LTRIANGLE },
/* 47 */{ 542 - 1, 2, MET_TRANSPARENT },
/* 48 */{ 493 - 1, 0, MET_LTRIANGLE },
/* 49 */{ 493 - 1, 1, MET_RTRIANGLE },
/* 50 */{ 501 - 1, 1, MET_RTRIANGLE },
/* 51 */{ 504 - 1, 0, MET_LTRIANGLE },
/* 52 */{ 504 - 1, 1, MET_TRANSPARENT },
/* 53 */{ 479 - 1, 0, MET_TRANSPARENT },
/* 54 */{ 479 - 1, 1, MET_RTRIANGLE },
/* 55 */{ 488 - 1, 0, MET_LTRIANGLE },
/* 56 */{ 517 - 1, 0, MET_TRANSPARENT },
/* 57 */{ 517 - 1, 1, MET_TRANSPARENT },
/* 58 */{ 516 - 1, 0, MET_TRANSPARENT },
/* 59 */{ 507 - 1, 0, MET_TRANSPARENT },
/* 60 */{ 500 - 1, 2, MET_TRANSPARENT },
/* 61 */{ 473 - 1, 0, MET_LTRAPEZOID },
/* 62 */{ 544 - 1, 1, MET_RTRIANGLE },
/* 63 */{ 435 - 1, 1, MET_RTRIANGLE },

/* 64 */{  33 - 1, 3, MET_TRANSPARENT },
/* 65 */{ 449 - 1, 1, MET_TRANSPARENT },
/* 66 */{ 441 - 1, 2, MET_SQUARE },
/* 67 */{  32 - 1, 0, MET_LTRAPEZOID },
/* 68 */{ 457 - 1, 0, MET_TRANSPARENT },
/* 69 */{ 469 - 1, 2, MET_TRANSPARENT },
/* 70 */{  49 - 1, 7, MET_TRANSPARENT },

/* 71 */{  387 - 1, 0, -1 },
/* 72 */{  387 - 1, 1, -1 },
/* 73 */{  238 - 1, 0, MET_LTRIANGLE },
/* 74 */{  238 - 1, 1, MET_RTRIANGLE },
/* 75 */{  242 - 1, 0, MET_LTRIANGLE },

/* 76 */{  210 - 1, 0, MET_LTRIANGLE },
/* 77 */{  210 - 1, 1, MET_RTRIANGLE },

/* 78 */{  233 - 1, 0, MET_LTRIANGLE },

/* 79 */{  25 - 1, 1, MET_RTRIANGLE }, // improve connections
/* 80 */{  26 - 1, 0, MET_LTRIANGLE },
/* 81 */{  26 - 1, 1, MET_RTRIANGLE },
/* 82 */{ 205 - 1, 1, MET_RTRIANGLE },
/* 83 */{ 385 - 1, 0, MET_LTRIANGLE },
/* 84 */{ 385 - 1, 1, MET_RTRIANGLE },
/* 85 */{ 386 - 1, 1, MET_RTRIANGLE },
/* 86 */{ 387 - 1, 0, MET_LTRIANGLE },
/* 87 */{ 388 - 1, 1, MET_RTRIANGLE },
/* 88 */{ 390 - 1, 1, MET_RTRIANGLE },
/* 89 */{ 395 - 1, 0, MET_LTRIANGLE },
/* 90 */{ 395 - 1, 1, MET_RTRIANGLE },

/* 91 */{ 511 - 1, 5, MET_TRANSPARENT },

/* 92 */{ 171 - 1, 3, MET_TRANSPARENT },
	};

	const uint16_t* pSubtiles = (const uint16_t*)minBuf;
	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_L3;
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

	// add shadow 537[1]
	for (int i = 2; i < 3; i++) {
		for (int x = 2; x < 6; x++) {
			for (int y = 26; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR && (y != 26 || x == 2)) {
					gpBuffer[addr] = shadowColorCaves(color); // 537[1]
				}
			}
		}
	}

	// remove door 538[0]
	for (int i = 3; i < 4; i++) {
		for (int x = 21; x < 30; x++) {
			for (int y = 0; y < 9; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}
	// remove door 538[2]
	for (int i = 5; i < 6; i++) {
		for (int x = 21; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}
	// remove door 538[4]
	for (int i = 6; i < 7; i++) {
		for (int x = 21; x < MICRO_WIDTH; x++) {
			for (int y = 4; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y < 8 && x < 24) {
					continue;
				}
				if (y == 8 && x == 21) {
					continue;
				}
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}
	// remove door 540[1]
	for (int i = 9; i < 10; i++) {
		for (int x = 7; x < MICRO_WIDTH; x++) {
			for (int y = 13; y < 21; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y > 9 + (x + 1) / 2) {
					continue;
				}
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR && (color % 16) < 13) {
					gpBuffer[addr] = shadowColorCaves(color);
				}
			}
		}
	}
	// remove door 541[0]
	for (int i = 10; i < 11; i++) {
		for (int x = 22; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < 6; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR && (color % 16) < 13) {
					gpBuffer[addr] = shadowColorCaves(color);
				}
			}
		}
	}
	// remove door 541[1]
	for (int i = 11; i < 12; i++) {
		for (int x = 0; x < 11; x++) {
			for (int y = 0; y < 10; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (x < 4) {
					BYTE color = gpBuffer[addr];
					if (color != TRANS_COLOR && (color % 16) < 13) {
						gpBuffer[addr] = shadowColorCaves(color);
					}
				} else {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// remove door 541[3]
	for (int i = 12; i < 13; i++) {
		for (int x = 0; x < 11; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}
	// remove door 541[5]
	for (int i = 13; i < 14; i++) {
		for (int x = 0; x < 11; x++) {
			for (int y = 5; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y < 7 && x > 7) {
					continue;
				}
				if (y == 7 && x > 8) {
					continue;
				}
				gpBuffer[addr] = TRANS_COLOR;
			}
		}
	}
	// fix shadow 496[0] using 494[0]
	for (int i = 18; i < 19; i++) {
		for (int x = 15; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < 15; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[x + MICRO_WIDTH * ((i - 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 1) % DRAW_HEIGHT)) * BUFFER_WIDTH]; // 494[0]
				if (color == 0 || color == 47 || color == 60 || color == 93 || color == 125) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// create new shadow 551[1] using 202[1] and 544[1]
	for (int i = 25; i < 26; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color1 = gpBuffer[x + MICRO_WIDTH * ((i - 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 1) % DRAW_HEIGHT)) * BUFFER_WIDTH]; // 202[1]
				BYTE color2 = gpBuffer[x + MICRO_WIDTH * ((i - 2) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 2) % DRAW_HEIGHT)) * BUFFER_WIDTH]; // 544[1]
				BYTE color;
				if (x < 15 && y > 23) {
					color = color1;
					if (x == 15 && y == 24) {
						color = shadowColorCaves(color);
					}
				} else if (x < 5 && y < 10 - x) {
					color = color1;
				//} else if (x < 3 && y > 11 && y < 15 && (x != 2 || y != 12)) {
				} else if (x < 9 && y > 8 && y < 15 && y > 8 + x / 2) {
					color = color1;
				} else if (x < 4 && y > 16 && y < 24 && y > 16 + 2 * x) {
					color = color1;
				} else if ((y == 20 && x >= 16 && x <= 19) || (y == 21 && x >= 12 && x <= 17)
						|| (y == 22 && x >= 11 && x <= 15) || (y == 23 && x >=  8 && x <= 14)) {
					color = color1;
				} else {
					color = color2;
				}
				gpBuffer[addr] = color;
			}
		}
	}
	// add missing leg to 543[0] using 506[0]
	for (int i = 27; i < 28; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < 14; y++) {
				if (x > 26 && y < 8 && y < x - 22) {
					continue;
				}
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[x + MICRO_WIDTH * ((i - 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 1) % DRAW_HEIGHT)) * BUFFER_WIDTH]; // 506[0]
				gpBuffer[addr] = color;
			}
		}
	}

	// create new shadow 528[0] using 506[0] and 543[0]
	for (int i = 28; i < 29; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr2;
				if (x > 26 && y < 8 && y < x - 22) {
					addr2 = x + MICRO_WIDTH * ((i - 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 1) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 543[0]
				} else {
					addr2 = x + MICRO_WIDTH * ((i - 2) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 2) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 506[0]
				}
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
	}

	// create new shadow 528[1] using 506[1] and 543[1]
	for (int i = 31; i < 32; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr2;
				if (y > 25) {
					addr2 = x + MICRO_WIDTH * ((i - 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 1) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 506[1]
				} else {
					addr2 = x + MICRO_WIDTH * ((i - 2) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 2) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 543[1]
				}
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
	}
	// fix shadow of 497[0]
	for (int i = 37; i < 38; i++) {
		for (int x = 20; x < MICRO_WIDTH; x++) {
			for (int y = 21; y < 29; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (y > 10 + x / 2 && color != TRANS_COLOR && (color % 16) < 13) {
					gpBuffer[addr] = shadowColorCaves(color); // 497[0]
				}
			}
		}
	}
	// fix shadow of 500[1]
	for (int i = 41; i < 42; i++) {
		for (int x = 0; x < 20; x++) {
			for (int y = 11; y < 24; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				// reduce bottom-shadow
				if (x < 13 && y > 14 + x / 2 && (color == 0 || color == 60 || color == 122 || color == 123 || color == 125)) {
					gpBuffer[addr] = gpBuffer[addr + (6 - 2 * (y - (15 + x /2))) * BUFFER_WIDTH]; // 500[1]
				}
				// add top-shadow
				if (x > 15 && y > 2 * x - 22 && color != TRANS_COLOR && (color % 16) < 13) {
					gpBuffer[addr] = shadowColorCaves(color);
				}
			}
		}
	}

	// fix shadow of 542[0]
	for (int i = 46; i < 47; i++) {
		for (int x = 14; x < 25; x++) {
			for (int y = 9; y < 15; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (y < 22 - (x + 1) / 2 && color != TRANS_COLOR && (color % 16) < 13) {
					gpBuffer[addr] = shadowColorCaves(color); // 542[0]
				}
			}
		}
	}

	// fix 238[0], 238[1] using 387[0], 387[1]
	for (int i = 73; i < 75; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;              // 238
				unsigned addr2 = x + MICRO_WIDTH * ((i - 2) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 2) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 387
				if (y < 24) {
					gpBuffer[addr] = gpBuffer[addr2];
				}
			}
		}
	}

	// extend shadow of 242[0]
	for (int i = 75; i < 76; i++) {
		for (int x = 0; x < 12; x++) {
			for (int y = 0; y < 19; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR && (color % 16) < 13) {
					gpBuffer[addr] = shadowColorCaves(color); // 242[0]
				}
			}
		}
	}

	// fix artifacts
	{ // 537[0]
		int i = 1;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 30 + 0 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 31 + 0 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // 538[0]
		int i = 3;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 30 + 0 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 31 + 0 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // 539[0]
		int i = 7;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		// - trunk color
		gpBuffer[addr + 25 + 0 * BUFFER_WIDTH] = 60;
		gpBuffer[addr + 25 + 1 * BUFFER_WIDTH] = 93;
		gpBuffer[addr + 25 + 2 * BUFFER_WIDTH] = 124;
		// - overdraw
		gpBuffer[addr + 20 + 6 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 19 + 7 * BUFFER_WIDTH] = 123;
		gpBuffer[addr + 20 + 7 * BUFFER_WIDTH] = 124;
		gpBuffer[addr + 21 + 7 * BUFFER_WIDTH] = 126;
		gpBuffer[addr + 22 + 7 * BUFFER_WIDTH] = 123;
		gpBuffer[addr + 22 + 8 * BUFFER_WIDTH] = 124;
	}
	{ // 540[0]
		int i = 8;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 14 + 21 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 14 + 21 * BUFFER_WIDTH]);
		gpBuffer[addr + 15 + 20 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 15 + 20 * BUFFER_WIDTH]);
		gpBuffer[addr + 15 + 19 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 15 + 19 * BUFFER_WIDTH]);

		gpBuffer[addr + 13 + 20 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 13 + 21 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 14 + 21 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 14 + 16 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 14 + 15 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 15 + 15 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 14 + 14 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 15 + 14 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 16 + 14 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 17 + 14 * BUFFER_WIDTH] = 120;
		gpBuffer[addr + 15 + 13 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 16 + 13 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 17 + 13 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 17 + 12 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 18 + 12 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 18 + 13 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 19 + 13 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 20 + 13 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 18 + 14 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 19 + 14 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 20 + 14 * BUFFER_WIDTH] = 91;
		gpBuffer[addr + 21 + 14 * BUFFER_WIDTH] = 118;
		gpBuffer[addr + 20 + 15 * BUFFER_WIDTH] = 93;
		gpBuffer[addr + 21 + 15 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 22 + 15 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 23 + 15 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 22 + 16 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 24 + 16 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 25 + 15 * BUFFER_WIDTH] = 120;
		gpBuffer[addr + 26 + 15 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 27 + 15 * BUFFER_WIDTH] = 89;
		gpBuffer[addr + 25 + 14 * BUFFER_WIDTH] = 119;
		gpBuffer[addr + 28 + 14 * BUFFER_WIDTH] = 120;
		gpBuffer[addr + 29 + 14 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 27 + 13 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 28 + 13 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 29 + 13 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 30 + 13 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 31 + 13 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 30 + 12 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 31 + 12 * BUFFER_WIDTH] = 70;
	}
	{ // 540[1]
		int i = 9;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 19 + 21 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 20 + 21 * BUFFER_WIDTH] = 72;
		gpBuffer[addr + 21 + 21 * BUFFER_WIDTH] = 75;
	}
	{ // 541[0]
		int i = 10;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 22 +  6 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 23 +  6 * BUFFER_WIDTH] = 74;
	}
	{ // 541[1]
		int i = 11;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  0 + 0 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  1 + 0 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  2 + 0 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  2 + 1 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  3 + 0 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  3 + 1 * BUFFER_WIDTH] = TRANS_COLOR;

		gpBuffer[addr +  5 + 14 * BUFFER_WIDTH] = 123;
		gpBuffer[addr +  6 + 14 * BUFFER_WIDTH] = 72;
		gpBuffer[addr +  5 + 15 * BUFFER_WIDTH] = 69;
		gpBuffer[addr +  6 + 15 * BUFFER_WIDTH] = 71;
		gpBuffer[addr +  7 + 15 * BUFFER_WIDTH] = 119;
	}
	{ // 496[0]
		int i = 18;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 14 + 13 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 14 + 13 * BUFFER_WIDTH]);
		gpBuffer[addr + 14 + 12 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 14 + 12 * BUFFER_WIDTH]);
		gpBuffer[addr + 15 + 12 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 15 + 12 * BUFFER_WIDTH]);
		gpBuffer[addr + 16 + 12 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 16 + 12 * BUFFER_WIDTH]);
		gpBuffer[addr + 14 + 11 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 14 + 11 * BUFFER_WIDTH]);
		gpBuffer[addr + 15 + 11 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 15 + 11 * BUFFER_WIDTH]);
		gpBuffer[addr + 16 + 11 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 16 + 11 * BUFFER_WIDTH]);
		gpBuffer[addr + 17 + 11 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 17 + 11 * BUFFER_WIDTH]);
		gpBuffer[addr + 18 + 11 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 18 + 11 * BUFFER_WIDTH]);
		gpBuffer[addr + 17 + 10 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 17 + 10 * BUFFER_WIDTH]);
		gpBuffer[addr + 18 + 10 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 18 + 10 * BUFFER_WIDTH]);
		gpBuffer[addr + 19 + 10 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 19 + 10 * BUFFER_WIDTH]);
		gpBuffer[addr + 20 + 10 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 20 + 10 * BUFFER_WIDTH]);
		gpBuffer[addr + 21 +  9 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 21 +  9 * BUFFER_WIDTH]);
		gpBuffer[addr + 22 +  9 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 22 +  9 * BUFFER_WIDTH]);
	}
	{ // 495[1]
		int i = 19;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 18 + 21 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 19 + 21 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 20 + 21 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 14 + 22 * BUFFER_WIDTH] = 115;
		gpBuffer[addr + 15 + 22 * BUFFER_WIDTH] = 83;
		gpBuffer[addr + 16 + 22 * BUFFER_WIDTH] = 118;
		gpBuffer[addr + 17 + 22 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 18 + 22 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 19 + 22 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 14 + 23 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 15 + 23 * BUFFER_WIDTH] = 74;
		gpBuffer[addr + 16 + 23 * BUFFER_WIDTH] = 72;
		gpBuffer[addr + 17 + 23 * BUFFER_WIDTH] = 89;
		gpBuffer[addr + 14 + 24 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 15 + 24 * BUFFER_WIDTH] = 91;

		gpBuffer[addr +  0 +  9 * BUFFER_WIDTH] = 116;
		gpBuffer[addr +  1 +  9 * BUFFER_WIDTH] = 71;
		gpBuffer[addr +  2 +  9 * BUFFER_WIDTH] = 73;
		gpBuffer[addr +  3 +  9 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  0 + 10 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  1 + 10 * BUFFER_WIDTH] = 69;
		gpBuffer[addr +  2 + 10 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  3 + 10 * BUFFER_WIDTH] = 70;
		gpBuffer[addr +  0 + 11 * BUFFER_WIDTH] = 73;
		gpBuffer[addr +  1 + 11 * BUFFER_WIDTH] = 75;

		gpBuffer[addr +  0 + 12 * BUFFER_WIDTH] = 125;
		gpBuffer[addr +  1 + 12 * BUFFER_WIDTH] = 123;
		gpBuffer[addr +  2 + 11 * BUFFER_WIDTH] = 122;
		gpBuffer[addr +  3 + 11 * BUFFER_WIDTH] = 123;
		gpBuffer[addr +  4 + 11 * BUFFER_WIDTH] = 125;
		gpBuffer[addr +  5 + 10 * BUFFER_WIDTH] = 125;
		// eliminate ugly leftovers(?)
		gpBuffer[addr + 31 + 16 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 30 + 16 * BUFFER_WIDTH] = 66;
		gpBuffer[addr + 29 + 16 * BUFFER_WIDTH] = 118;
		gpBuffer[addr + 29 + 15 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 28 + 16 * BUFFER_WIDTH] = 120;
		gpBuffer[addr + 28 + 15 * BUFFER_WIDTH] = 117;
		gpBuffer[addr + 27 + 15 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 27 + 14 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 26 + 15 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 26 + 14 * BUFFER_WIDTH] = 118;
		gpBuffer[addr + 25 + 14 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 25 + 13 * BUFFER_WIDTH] = 116;
		gpBuffer[addr + 24 + 14 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 24 + 13 * BUFFER_WIDTH] = 66;
		gpBuffer[addr + 23 + 13 * BUFFER_WIDTH] = 119;
		gpBuffer[addr + 23 + 12 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 22 + 12 * BUFFER_WIDTH] = 119;

		gpBuffer[addr + 20 + 21 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 22 + 20 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 21 + 21 * BUFFER_WIDTH] = 120;
		gpBuffer[addr + 23 + 20 * BUFFER_WIDTH] = 121;
		gpBuffer[addr + 25 + 19 * BUFFER_WIDTH] = 120;
	}
	{ // 100[0]
		int i = 20;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 14 + 23 * BUFFER_WIDTH] = 119;
	}
	{ // 126[0]
		int i = 21;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 12 + 22 * BUFFER_WIDTH] = 13;
		gpBuffer[addr + 13 + 22 * BUFFER_WIDTH] = 86;
		gpBuffer[addr + 18 + 25 * BUFFER_WIDTH] = 15;
		gpBuffer[addr + 16 + 24 * BUFFER_WIDTH] = 61;
	}
	{ // 83[1]
		int i = 22;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 11 +  8 * BUFFER_WIDTH] = 28;
		gpBuffer[addr + 12 +  8 * BUFFER_WIDTH] = 25;
	}
	{ // 551[1]
		int i = 25;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 18 + 21 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 19 + 21 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 16 + 22 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 17 + 22 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 18 + 22 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 15 + 23 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 16 + 23 * BUFFER_WIDTH] = 73;
	}
	{ // 476[1]
		int i = 32;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 26 + 15 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 26 + 15 * BUFFER_WIDTH]);
		gpBuffer[addr + 27 + 16 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 27 + 16 * BUFFER_WIDTH]);
		gpBuffer[addr + 28 + 16 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 28 + 16 * BUFFER_WIDTH]);
		gpBuffer[addr + 29 + 16 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 29 + 16 * BUFFER_WIDTH]);
		gpBuffer[addr + 29 + 17 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 29 + 17 * BUFFER_WIDTH]);
		gpBuffer[addr + 24 + 17 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 24 + 17 * BUFFER_WIDTH]);
		gpBuffer[addr + 25 + 18 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 25 + 18 * BUFFER_WIDTH]);
		gpBuffer[addr + 25 + 19 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 25 + 19 * BUFFER_WIDTH]);

		gpBuffer[addr +  0 + 10 * BUFFER_WIDTH] = 73; // make it reusable in 484
	}
	{ // 484[0] - after reuse
		int i = 33;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 30 +  9 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 31 +  9 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 31 + 10 * BUFFER_WIDTH] = 68;
	}
	{ // 490[0]
		int i = 34;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 18 + 25 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 18 + 25 * BUFFER_WIDTH]);
		gpBuffer[addr + 19 + 25 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 19 + 25 * BUFFER_WIDTH]);
		gpBuffer[addr + 20 + 26 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 20 + 26 * BUFFER_WIDTH]);
		gpBuffer[addr + 21 + 26 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 21 + 26 * BUFFER_WIDTH]);
		gpBuffer[addr + 22 + 27 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 22 + 27 * BUFFER_WIDTH]);
		gpBuffer[addr + 23 + 27 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 23 + 27 * BUFFER_WIDTH]);
		gpBuffer[addr + 24 + 28 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 24 + 28 * BUFFER_WIDTH]);
		gpBuffer[addr + 26 + 29 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 26 + 29 * BUFFER_WIDTH]);

		gpBuffer[addr + 10 + 15 * BUFFER_WIDTH] = 121;
		gpBuffer[addr + 10 + 16 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 11 + 15 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 11 + 16 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 12 + 14 * BUFFER_WIDTH] = 66;
		gpBuffer[addr + 13 + 14 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 12 + 15 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 13 + 15 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 14 + 15 * BUFFER_WIDTH] = 115;
		gpBuffer[addr + 15 + 15 * BUFFER_WIDTH] = 113;
		gpBuffer[addr + 12 + 16 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 13 + 16 * BUFFER_WIDTH] = 65;
		gpBuffer[addr + 14 + 16 * BUFFER_WIDTH] = 66;
		gpBuffer[addr + 15 + 16 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 16 + 16 * BUFFER_WIDTH] = 66;
		gpBuffer[addr + 17 + 16 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 12 + 17 * BUFFER_WIDTH] = 119;
		gpBuffer[addr + 13 + 17 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 14 + 17 * BUFFER_WIDTH] = 66;
		gpBuffer[addr + 15 + 17 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 16 + 17 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 17 + 17 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 18 + 17 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 17 + 18 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 18 + 18 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 19 + 18 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 18 + 19 * BUFFER_WIDTH] = 65;
		gpBuffer[addr + 19 + 19 * BUFFER_WIDTH] = 66;
		gpBuffer[addr + 20 + 19 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 21 + 19 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 20 + 20 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 21 + 20 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 22 + 20 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 23 + 20 * BUFFER_WIDTH] = 119;
		gpBuffer[addr + 24 + 20 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 25 + 20 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 22 + 21 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 23 + 21 * BUFFER_WIDTH] = 66;
		gpBuffer[addr + 24 + 21 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 25 + 21 * BUFFER_WIDTH] = 66;
		gpBuffer[addr + 26 + 21 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 27 + 21 * BUFFER_WIDTH] = 65;
		gpBuffer[addr + 25 + 22 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 26 + 22 * BUFFER_WIDTH] = 114;
		gpBuffer[addr + 27 + 22 * BUFFER_WIDTH] = 66;
		gpBuffer[addr + 28 + 22 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 27 + 23 * BUFFER_WIDTH] = 65;
		gpBuffer[addr + 28 + 23 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 29 + 23 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 31 + 24 * BUFFER_WIDTH] = 70;
	}
	{ // 492[0]
		int i = 35;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 27 +  4 * BUFFER_WIDTH] = 121;

		gpBuffer[addr + 28 +  2 * BUFFER_WIDTH] = 60;
		gpBuffer[addr + 28 +  3 * BUFFER_WIDTH] = 119;

		gpBuffer[addr + 30 +  3 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 31 +  3 * BUFFER_WIDTH] = 70;
	}
	{ // 492[1]
		int i = 36;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  3 +  4 * BUFFER_WIDTH] = 125;
		gpBuffer[addr +  4 +  4 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  4 +  5 * BUFFER_WIDTH] = 125;

		gpBuffer[addr + 25 + 18 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 25 + 18 * BUFFER_WIDTH]);
		gpBuffer[addr + 25 + 19 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 25 + 19 * BUFFER_WIDTH]);

		gpBuffer[addr + 16 +  9 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 17 +  9 * BUFFER_WIDTH] = 127;
		gpBuffer[addr + 17 + 10 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 17 + 10 * BUFFER_WIDTH]);
		gpBuffer[addr + 18 + 10 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 18 + 10 * BUFFER_WIDTH]);
		gpBuffer[addr + 19 + 10 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 19 + 10 * BUFFER_WIDTH]);
		gpBuffer[addr + 19 + 11 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 19 + 11 * BUFFER_WIDTH]);
		gpBuffer[addr + 20 + 11 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 20 + 11 * BUFFER_WIDTH]);
		gpBuffer[addr + 21 + 11 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 21 + 11 * BUFFER_WIDTH]);
		gpBuffer[addr + 20 + 12 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 20 + 12 * BUFFER_WIDTH]);
		gpBuffer[addr + 21 + 12 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 21 + 12 * BUFFER_WIDTH]);
		gpBuffer[addr + 22 + 12 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 22 + 12 * BUFFER_WIDTH]);
		gpBuffer[addr + 23 + 12 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 23 + 12 * BUFFER_WIDTH]);
		gpBuffer[addr + 22 + 13 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 22 + 13 * BUFFER_WIDTH]);
		gpBuffer[addr + 23 + 13 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 23 + 13 * BUFFER_WIDTH]);
		gpBuffer[addr + 24 + 13 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 24 + 13 * BUFFER_WIDTH]);
		gpBuffer[addr + 25 + 13 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 25 + 13 * BUFFER_WIDTH]);
		gpBuffer[addr + 24 + 14 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 24 + 14 * BUFFER_WIDTH]);
		gpBuffer[addr + 25 + 14 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 25 + 14 * BUFFER_WIDTH]);
		gpBuffer[addr + 26 + 14 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 26 + 14 * BUFFER_WIDTH]);
		gpBuffer[addr + 27 + 14 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 27 + 14 * BUFFER_WIDTH]);
		gpBuffer[addr + 26 + 15 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 26 + 15 * BUFFER_WIDTH]);
		gpBuffer[addr + 27 + 15 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 27 + 15 * BUFFER_WIDTH]);
		gpBuffer[addr + 28 + 15 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 28 + 15 * BUFFER_WIDTH]);
		gpBuffer[addr + 27 + 16 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 27 + 16 * BUFFER_WIDTH]);
		gpBuffer[addr + 28 + 16 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 28 + 16 * BUFFER_WIDTH]);
		gpBuffer[addr + 29 + 16 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 29 + 16 * BUFFER_WIDTH]);
		gpBuffer[addr + 29 + 17 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 29 + 17 * BUFFER_WIDTH]);

		gpBuffer[addr + 16 + 11 * BUFFER_WIDTH] = 120;
		gpBuffer[addr + 17 + 11 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 17 + 12 * BUFFER_WIDTH] = 67;
	}
	{ // 499[0]
		int i = 38;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 28 +  2 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 29 +  2 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 29 +  3 * BUFFER_WIDTH] = 119;
		gpBuffer[addr + 30 +  3 * BUFFER_WIDTH] = 71;

		gpBuffer[addr + 31 +  4 * BUFFER_WIDTH] = 70;
	}
	{ // 499[1]
		int i = 39;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  0 + 10 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr +  0 + 10 * BUFFER_WIDTH]);
		gpBuffer[addr +  1 + 10 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr +  1 + 10 * BUFFER_WIDTH]);
		gpBuffer[addr +  4 + 11 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr +  4 + 11 * BUFFER_WIDTH]);
		gpBuffer[addr +  8 + 12 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr +  8 + 12 * BUFFER_WIDTH]);
		gpBuffer[addr + 20 + 11 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 20 + 11 * BUFFER_WIDTH]);
		gpBuffer[addr + 21 + 11 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 21 + 11 * BUFFER_WIDTH]);
	}
	{ // 500[0]
		int i = 40;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 27 + 13 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 28 + 14 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 29 + 14 * BUFFER_WIDTH] = 119;
		gpBuffer[addr + 30 + 14 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 30 + 15 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 31 + 15 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 31 + 16 * BUFFER_WIDTH] = 70;

		gpBuffer[addr + 25 + 15 * BUFFER_WIDTH] = 121;
		gpBuffer[addr + 25 + 16 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 25 + 17 * BUFFER_WIDTH] = 70;

		gpBuffer[addr + 26 + 16 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 26 + 17 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 26 + 18 * BUFFER_WIDTH] = 70;

		gpBuffer[addr + 27 + 17 * BUFFER_WIDTH] = 114;
		gpBuffer[addr + 27 + 18 * BUFFER_WIDTH] = 70;

		gpBuffer[addr + 28 + 17 * BUFFER_WIDTH] = 71;
	}
	{ // 500[1]
		int i = 41;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  5 + 12 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr +  5 + 12 * BUFFER_WIDTH]);
		gpBuffer[addr +  6 + 13 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr +  6 + 13 * BUFFER_WIDTH]);

		gpBuffer[addr +  7 + 11 * BUFFER_WIDTH] = 73;
		gpBuffer[addr +  7 + 12 * BUFFER_WIDTH] = 71;

		gpBuffer[addr + 12 + 22 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 12 + 21 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 13 + 20 * BUFFER_WIDTH] = 123;
		gpBuffer[addr + 13 + 21 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 13 + 22 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 14 + 22 * BUFFER_WIDTH] = 119;
		gpBuffer[addr + 14 + 23 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 15 + 23 * BUFFER_WIDTH] = 73;

		gpBuffer[addr + 25 + 16 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 25 + 17 * BUFFER_WIDTH] = 115;
		gpBuffer[addr + 26 + 16 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 26 + 17 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 26 + 18 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 27 + 17 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 27 + 18 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 28 + 17 * BUFFER_WIDTH] = 73;

		gpBuffer[addr +  0 + 17 * BUFFER_WIDTH] = 117;
	}
	{ // 502[0]
		int i = 42;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  3 + 17 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  4 + 17 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  5 + 17 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  4 + 18 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  5 + 18 * BUFFER_WIDTH] = 0;

		gpBuffer[addr + 23 +  5 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 29 +  8 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 31 +  7 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 25 + 10 * BUFFER_WIDTH] = 120;
		gpBuffer[addr + 26 + 10 * BUFFER_WIDTH] = 71;
	}
	{ // 502[1]
		int i = 43;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  4 +  5 * BUFFER_WIDTH] = 124;
		gpBuffer[addr +  1 +  6 * BUFFER_WIDTH] = 71;
		gpBuffer[addr +  3 +  6 * BUFFER_WIDTH] = 69;
		gpBuffer[addr +  2 +  6 * BUFFER_WIDTH] = 117;
	}
	{ // 535[0]
		int i = 44;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;

		gpBuffer[addr + 14 + 20 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 15 + 20 * BUFFER_WIDTH] = 121;
		gpBuffer[addr + 15 + 19 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 16 + 19 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 17 + 19 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 15 + 18 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 16 + 18 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 17 + 18 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 18 + 18 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 19 + 18 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 20 + 17 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 21 + 17 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 22 + 16 * BUFFER_WIDTH] = 72;
		gpBuffer[addr + 23 + 16 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 24 + 15 * BUFFER_WIDTH] = 73;

		gpBuffer[addr + 24 + 11 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 24 + 10 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 24 +  9 * BUFFER_WIDTH] = 72;
		gpBuffer[addr + 25 +  9 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 26 +  8 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 27 +  8 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 28 +  8 * BUFFER_WIDTH] = 71;

		gpBuffer[addr + 30 +  9 * BUFFER_WIDTH] = 119;
		gpBuffer[addr + 31 +  9 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 31 + 10 * BUFFER_WIDTH] = 122;
	}
	{ // 535[1]
		int i = 45;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  0 +  9 * BUFFER_WIDTH] = 69;
		gpBuffer[addr +  1 +  9 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  2 +  9 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  3 +  9 * BUFFER_WIDTH] = 69;
		gpBuffer[addr +  0 + 10 * BUFFER_WIDTH] = 71;
		gpBuffer[addr +  1 + 10 * BUFFER_WIDTH] = 70;
		gpBuffer[addr +  2 + 10 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  3 + 10 * BUFFER_WIDTH] = 69;

		gpBuffer[addr +  0 + 11 * BUFFER_WIDTH] = 71;
		gpBuffer[addr +  1 + 11 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  2 + 11 * BUFFER_WIDTH] = 70;

		gpBuffer[addr +  0 + 12 * BUFFER_WIDTH] = 125;
		gpBuffer[addr +  1 + 12 * BUFFER_WIDTH] = 123;
		gpBuffer[addr +  2 + 12 * BUFFER_WIDTH] = 91;
		gpBuffer[addr +  3 + 12 * BUFFER_WIDTH] = 125;

		gpBuffer[addr + 14 + 24 * BUFFER_WIDTH] = 119;
		gpBuffer[addr + 14 + 23 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 15 + 23 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 16 + 23 * BUFFER_WIDTH] = 70;

		gpBuffer[addr + 18 + 22 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 19 + 22 * BUFFER_WIDTH] = 69;

		gpBuffer[addr + 21 + 13 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 21 + 13 * BUFFER_WIDTH]);
		gpBuffer[addr + 22 + 13 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 22 + 13 * BUFFER_WIDTH]);
		gpBuffer[addr + 22 + 14 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 22 + 14 * BUFFER_WIDTH]);
		gpBuffer[addr + 22 + 15 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 22 + 15 * BUFFER_WIDTH]);
		gpBuffer[addr + 23 + 15 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 23 + 15 * BUFFER_WIDTH]);
		gpBuffer[addr + 23 + 16 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 23 + 16 * BUFFER_WIDTH]);
		gpBuffer[addr + 24 + 16 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 24 + 16 * BUFFER_WIDTH]);
		gpBuffer[addr + 24 + 17 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 24 + 17 * BUFFER_WIDTH]);
		gpBuffer[addr + 24 + 18 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 24 + 18 * BUFFER_WIDTH]);
		gpBuffer[addr + 24 + 19 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 24 + 19 * BUFFER_WIDTH]);
		gpBuffer[addr + 23 + 20 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 23 + 20 * BUFFER_WIDTH]);
	}
	{ // 542[0]
		int i = 46;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 14 + 15 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 14 + 15 * BUFFER_WIDTH]);
		gpBuffer[addr + 15 + 14 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 15 + 14 * BUFFER_WIDTH]);
		gpBuffer[addr + 17 + 13 * BUFFER_WIDTH] = shadowColorCaves(gpBuffer[addr + 17 + 13 * BUFFER_WIDTH]);
	}
	{ // 542[2]
		int i = 47;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 21 + 28 * BUFFER_WIDTH] = 124;
	}
	{ // 493[0]
		int i = 48;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 13 + 20 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 13 + 21 * BUFFER_WIDTH] = 106;
		gpBuffer[addr + 13 + 22 * BUFFER_WIDTH] = 89;

		gpBuffer[addr + 14 + 19 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 14 + 20 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 14 + 21 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 15 + 18 * BUFFER_WIDTH] = 121;
		gpBuffer[addr + 15 + 19 * BUFFER_WIDTH] = 117;
		gpBuffer[addr + 15 + 20 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 15 + 21 * BUFFER_WIDTH] = 121;

		gpBuffer[addr + 16 + 20 * BUFFER_WIDTH] = 90;
		gpBuffer[addr + 17 + 20 * BUFFER_WIDTH] = 123;

		gpBuffer[addr + 16 + 19 * BUFFER_WIDTH] = 88;
		gpBuffer[addr + 17 + 19 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 18 + 19 * BUFFER_WIDTH] = 120;

		gpBuffer[addr + 16 + 18 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 17 + 18 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 18 + 18 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 19 + 18 * BUFFER_WIDTH] = 120;
		gpBuffer[addr + 20 + 18 * BUFFER_WIDTH] = 72;

		gpBuffer[addr + 16 + 17 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 17 + 17 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 18 + 17 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 19 + 17 * BUFFER_WIDTH] = 74;
		gpBuffer[addr + 20 + 17 * BUFFER_WIDTH] = 74;
		gpBuffer[addr + 21 + 17 * BUFFER_WIDTH] = 88;
		gpBuffer[addr + 22 + 17 * BUFFER_WIDTH] = 90;

		gpBuffer[addr + 16 + 16 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 17 + 16 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 18 + 16 * BUFFER_WIDTH] = 72;
		gpBuffer[addr + 19 + 16 * BUFFER_WIDTH] = 72;
		gpBuffer[addr + 20 + 16 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 21 + 16 * BUFFER_WIDTH] = 74;
		gpBuffer[addr + 22 + 16 * BUFFER_WIDTH] = 72;
		gpBuffer[addr + 23 + 16 * BUFFER_WIDTH] = 120;
		gpBuffer[addr + 24 + 16 * BUFFER_WIDTH] = 90;
		gpBuffer[addr + 25 + 16 * BUFFER_WIDTH] = 123;

		gpBuffer[addr + 19 + 15 * BUFFER_WIDTH] = 85;
		gpBuffer[addr + 20 + 15 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 21 + 15 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 22 + 15 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 23 + 15 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 21 + 14 * BUFFER_WIDTH] = 121;
		gpBuffer[addr + 22 + 14 * BUFFER_WIDTH] = 72;
		gpBuffer[addr + 23 + 14 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 24 + 14 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 25 + 14 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 24 + 15 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 25 + 15 * BUFFER_WIDTH] = 121;

		gpBuffer[addr + 23 + 13 * BUFFER_WIDTH] = 123;
		gpBuffer[addr + 24 + 13 * BUFFER_WIDTH] = 121;
		gpBuffer[addr + 23 + 11 * BUFFER_WIDTH] = 89;

		gpBuffer[addr + 26 +  6 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 24 +  8 * BUFFER_WIDTH] = 72;
		gpBuffer[addr + 25 +  7 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 26 +  7 * BUFFER_WIDTH] = 117;
		gpBuffer[addr + 27 +  7 * BUFFER_WIDTH] = 115;

		gpBuffer[addr + 25 +  8 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 26 +  8 * BUFFER_WIDTH] = 118;
		gpBuffer[addr + 27 +  8 * BUFFER_WIDTH] = 119;
		gpBuffer[addr + 28 +  8 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 29 +  8 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 24 +  9 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 29 +  9 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 30 +  9 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 31 +  9 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 30 + 10 * BUFFER_WIDTH] = 121;
		gpBuffer[addr + 31 + 10 * BUFFER_WIDTH] = 119;
		gpBuffer[addr + 31 + 11 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 31 + 12 * BUFFER_WIDTH] = 125;

		gpBuffer[addr + 24 + 10 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 25 +  9 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 27 +  9 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 28 +  9 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 25 + 10 * BUFFER_WIDTH] = 115;
		gpBuffer[addr + 26 + 10 * BUFFER_WIDTH] = 117;
		gpBuffer[addr + 25 + 12 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 25 + 13 * BUFFER_WIDTH] = 89;
		gpBuffer[addr + 26 + 13 * BUFFER_WIDTH] = 87;
		gpBuffer[addr + 27 + 13 * BUFFER_WIDTH] = 123;
		gpBuffer[addr + 27 + 10 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 27 + 14 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 26 +  9 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 26 + 11 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 26 + 12 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 26 + 14 * BUFFER_WIDTH] = 86;
		gpBuffer[addr + 26 + 15 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 25 + 11 * BUFFER_WIDTH] = 68;
		// eliminate ugly leftovers(?)
		gpBuffer[addr + 23 + 27 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 24 + 26 * BUFFER_WIDTH] = 119;
		gpBuffer[addr + 24 + 27 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 24 + 28 * BUFFER_WIDTH] = 119;
		gpBuffer[addr + 25 + 27 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 25 + 28 * BUFFER_WIDTH] = 66;
		gpBuffer[addr + 26 + 28 * BUFFER_WIDTH] = 114;
		gpBuffer[addr + 26 + 29 * BUFFER_WIDTH] = 67;
		gpBuffer[addr + 27 + 28 * BUFFER_WIDTH] = 66;
		gpBuffer[addr + 27 + 29 * BUFFER_WIDTH] = 66;
		gpBuffer[addr + 28 + 29 * BUFFER_WIDTH] = 116;
		gpBuffer[addr + 28 + 30 * BUFFER_WIDTH] = 117;
		gpBuffer[addr + 29 + 30 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 30 + 31 * BUFFER_WIDTH] = 120;
	}
	{ // 493[1]
		int i = 49;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  0 + 10 * BUFFER_WIDTH] = 71;
		gpBuffer[addr +  0 + 11 * BUFFER_WIDTH] = 123;
		gpBuffer[addr +  1 +  9 * BUFFER_WIDTH] = 71;
		gpBuffer[addr +  1 + 10 * BUFFER_WIDTH] = 73;
		gpBuffer[addr +  1 + 11 * BUFFER_WIDTH] = 120;
		gpBuffer[addr +  2 +  9 * BUFFER_WIDTH] = 73;
		gpBuffer[addr +  2 + 10 * BUFFER_WIDTH] = 88;
		gpBuffer[addr +  3 + 10 * BUFFER_WIDTH] = 122;
		gpBuffer[addr +  4 +  9 * BUFFER_WIDTH] = 120;
		gpBuffer[addr +  4 + 10 * BUFFER_WIDTH] = 123;
		gpBuffer[addr +  5 +  9 * BUFFER_WIDTH] = 71;
		gpBuffer[addr +  6 +  8 * BUFFER_WIDTH] = 74;
		gpBuffer[addr +  6 +  9 * BUFFER_WIDTH] = 121;
	}
	{ // 501[1]
		int i = 50;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  2 +  8 * BUFFER_WIDTH] = 121;
		gpBuffer[addr +  3 +  8 * BUFFER_WIDTH] = 72;

		gpBuffer[addr +  0 +  9 * BUFFER_WIDTH] = 119;
		gpBuffer[addr +  1 +  9 * BUFFER_WIDTH] = 117;
		gpBuffer[addr +  2 +  9 * BUFFER_WIDTH] = 123;
		gpBuffer[addr +  3 +  9 * BUFFER_WIDTH] = 121;

		gpBuffer[addr +  0 + 10 * BUFFER_WIDTH] = 121;
		gpBuffer[addr +  1 + 10 * BUFFER_WIDTH] = 122;
		gpBuffer[addr +  2 + 10 * BUFFER_WIDTH] = 73;
		gpBuffer[addr +  3 + 10 * BUFFER_WIDTH] = 75;

		gpBuffer[addr +  0 + 11 * BUFFER_WIDTH] = 73;
		gpBuffer[addr +  1 + 11 * BUFFER_WIDTH] = 71;

		gpBuffer[addr +  3 + 22 * BUFFER_WIDTH] = 125;
		gpBuffer[addr +  4 + 19 * BUFFER_WIDTH] = 71;
		gpBuffer[addr +  4 + 20 * BUFFER_WIDTH] = 73;
		gpBuffer[addr +  4 + 21 * BUFFER_WIDTH] = 91;
		gpBuffer[addr +  4 + 22 * BUFFER_WIDTH] = 120;
		gpBuffer[addr +  4 + 23 * BUFFER_WIDTH] = 123;

		gpBuffer[addr +  5 + 23 * BUFFER_WIDTH] = 75;
		gpBuffer[addr +  5 + 24 * BUFFER_WIDTH] = 73;
		gpBuffer[addr +  5 + 25 * BUFFER_WIDTH] = 90;
		gpBuffer[addr +  5 + 26 * BUFFER_WIDTH] = 73;
		gpBuffer[addr +  5 + 27 * BUFFER_WIDTH] = 90;
		gpBuffer[addr +  5 + 28 * BUFFER_WIDTH] = 125;
	}
	{ // 504[0]
		int i = 51;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 16 + 13 * BUFFER_WIDTH] = 120;
		gpBuffer[addr + 15 + 13 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 14 + 13 * BUFFER_WIDTH] = 124;
		gpBuffer[addr + 14 + 14 * BUFFER_WIDTH] = 123;

		gpBuffer[addr + 26 +  3 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 24 +  4 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 25 +  4 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 22 +  5 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 23 +  5 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 20 +  6 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 21 +  6 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 22 +  6 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 20 +  7 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 21 +  7 * BUFFER_WIDTH] = 0;
	}
	{ // 504[1]
		int i = 52;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  2 +  2 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  3 +  2 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  2 +  3 * BUFFER_WIDTH] = 125;
		gpBuffer[addr +  3 +  3 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  4 +  3 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  5 +  3 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  3 +  4 * BUFFER_WIDTH] = 124;
		gpBuffer[addr +  4 +  4 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  5 +  4 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  6 +  4 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  4 +  5 * BUFFER_WIDTH] = 125;
		gpBuffer[addr +  5 +  5 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  5 +  6 * BUFFER_WIDTH] = 124;
		gpBuffer[addr +  6 +  5 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  6 +  6 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  6 +  7 * BUFFER_WIDTH] = 125;
		gpBuffer[addr +  6 +  8 * BUFFER_WIDTH] = 123;
		gpBuffer[addr +  7 +  9 * BUFFER_WIDTH] = 120;
	}
	{ // 479[0]
		int i = 53;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 31 + 26 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 31 + 27 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 31 + 28 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 31 + 29 * BUFFER_WIDTH] = 123;
		gpBuffer[addr + 31 + 30 * BUFFER_WIDTH] = 73;
	}
	{ // 479[1]
		int i = 54;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  1 + 31 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  0 + 29 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  0 + 30 * BUFFER_WIDTH] = 60;
		gpBuffer[addr +  0 + 31 * BUFFER_WIDTH] = 126;
	}
	{ // 488[0]
		int i = 55;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  6 + 19 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  7 + 19 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  7 + 18 * BUFFER_WIDTH] = 0;
	}
	{ // 517[0]
		int i = 56;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 14 + 11 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 16 + 11 * BUFFER_WIDTH] = 119;

		gpBuffer[addr +  6 + 13 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  6 + 14 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  6 + 15 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  6 + 16 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  6 + 17 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  6 + 18 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  6 + 19 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  7 + 15 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  7 + 16 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  7 + 17 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  7 + 18 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  7 + 19 * BUFFER_WIDTH] = 0;
	}
	{ // 517[1]
		int i = 57;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 25 + 16 * BUFFER_WIDTH] = 87;
		gpBuffer[addr + 25 + 17 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 26 + 16 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 26 + 17 * BUFFER_WIDTH] = 116;
		gpBuffer[addr + 26 + 18 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 27 + 17 * BUFFER_WIDTH] = 72;
		gpBuffer[addr + 27 + 18 * BUFFER_WIDTH] = 68;
	}
	{ // 516[0]
		int i = 58;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 17 +  8 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 18 +  7 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 18 +  8 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 18 +  9 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 19 +  7 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 19 +  8 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 19 +  9 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 19 + 10 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 19 + 11 * BUFFER_WIDTH] = 123;
		gpBuffer[addr + 20 +  8 * BUFFER_WIDTH] = 0;

		gpBuffer[addr + 19 +  9 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 18 + 10 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 19 + 10 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 19 + 11 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 20 + 13 * BUFFER_WIDTH] = 121;
		gpBuffer[addr + 20 + 15 * BUFFER_WIDTH] = 119;
		gpBuffer[addr + 20 + 16 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 19 + 17 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 20 + 17 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 19 + 18 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 16 + 19 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 17 + 18 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 17 + 19 * BUFFER_WIDTH] = 119;
		gpBuffer[addr + 18 + 18 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 15 + 20 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 16 + 20 * BUFFER_WIDTH] = 123;

		gpBuffer[addr + 28 +  8 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 29 +  9 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 29 + 10 * BUFFER_WIDTH] = 74;

		gpBuffer[addr + 27 +  8 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 27 +  9 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 27 + 10 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 27 + 11 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 28 + 10 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 28 + 11 * BUFFER_WIDTH] = 0;
	}
	{ // 507[0]
		int i = 59;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 16 +  8 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 16 +  9 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 17 +  8 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 17 +  9 * BUFFER_WIDTH] = 123;
		gpBuffer[addr + 17 + 10 * BUFFER_WIDTH] = 120;

		gpBuffer[addr + 18 +  7 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 18 +  8 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 18 +  9 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 18 + 10 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 18 + 11 * BUFFER_WIDTH] = 123;
		gpBuffer[addr + 19 +  8 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 19 +  9 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 19 + 10 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 19 + 11 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 19 + 12 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 19 + 13 * BUFFER_WIDTH] = 123;
		gpBuffer[addr + 20 +  9 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 20 + 10 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 20 + 11 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 20 + 12 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 20 + 13 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 20 + 14 * BUFFER_WIDTH] = 123;
		gpBuffer[addr + 20 + 15 * BUFFER_WIDTH] = 120;

		gpBuffer[addr + 21 + 11 * BUFFER_WIDTH] = 60;
		gpBuffer[addr + 21 + 12 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 21 + 13 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 21 + 14 * BUFFER_WIDTH] = 123;
		gpBuffer[addr + 21 + 15 * BUFFER_WIDTH] = 125;

		gpBuffer[addr + 22 + 11 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 22 + 12 * BUFFER_WIDTH] = 60;
		gpBuffer[addr + 22 + 13 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 22 + 14 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 22 + 15 * BUFFER_WIDTH] = 123;

		gpBuffer[addr + 23 + 12 * BUFFER_WIDTH] = 60;
		gpBuffer[addr + 23 + 13 * BUFFER_WIDTH] = 123;
		gpBuffer[addr + 23 + 14 * BUFFER_WIDTH] = 92;
		gpBuffer[addr + 23 + 15 * BUFFER_WIDTH] = 123;
		gpBuffer[addr + 23 + 16 * BUFFER_WIDTH] = 92;

		gpBuffer[addr + 24 + 12 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 24 + 13 * BUFFER_WIDTH] = 92;
		gpBuffer[addr + 24 + 14 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 24 + 15 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 24 + 16 * BUFFER_WIDTH] = 125;

		gpBuffer[addr + 25 + 12 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 25 + 13 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 25 + 14 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 25 + 15 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 25 + 16 * BUFFER_WIDTH] = 75;

		gpBuffer[addr + 26 + 12 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 26 + 13 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 26 + 14 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 26 + 15 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 26 + 16 * BUFFER_WIDTH] = 75;

		gpBuffer[addr + 27 + 11 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 27 + 12 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 27 + 13 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 27 + 14 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 27 + 15 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 27 + 16 * BUFFER_WIDTH] = 122;

		gpBuffer[addr + 28 + 11 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 28 + 12 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 28 + 13 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 28 + 14 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 28 + 15 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 28 + 16 * BUFFER_WIDTH] = 89;

		gpBuffer[addr + 29 +  9 * BUFFER_WIDTH] = 92;
		gpBuffer[addr + 29 + 10 * BUFFER_WIDTH] = 91;
		gpBuffer[addr + 29 + 11 * BUFFER_WIDTH] = 91;
		gpBuffer[addr + 29 + 12 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 29 + 13 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 29 + 14 * BUFFER_WIDTH] = 93;
		gpBuffer[addr + 29 + 15 * BUFFER_WIDTH] = 119;
	}
	{ // 500[2]
		int i = 60;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  4 + 11 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  5 + 11 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  6 + 11 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  6 + 12 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  6 + 13 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  7 + 12 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  7 + 13 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  7 + 14 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  8 + 14 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  8 + 15 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  8 + 16 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  9 + 17 * BUFFER_WIDTH] = 0;
		gpBuffer[addr +  9 + 18 * BUFFER_WIDTH] = 125;
	}
	{ // 473[0]
		int i = 61;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 25 + 19 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 26 + 20 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 27 + 20 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 27 + 21 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 27 + 22 * BUFFER_WIDTH] = 0;

		gpBuffer[addr + 28 + 21 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 28 + 22 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 28 + 23 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 29 + 21 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 29 + 22 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 29 + 23 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 29 + 24 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 29 + 25 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 30 + 22 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 30 + 23 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 30 + 24 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 30 + 25 * BUFFER_WIDTH] = 0;

		gpBuffer[addr + 29 + 28 * BUFFER_WIDTH] = 90;
		gpBuffer[addr + 30 + 28 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 31 + 28 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 30 + 29 * BUFFER_WIDTH] = 70;
	}
	{ // 544[1]
		int i = 62;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  3 + 22 * BUFFER_WIDTH] = 120;
		gpBuffer[addr +  4 + 24 * BUFFER_WIDTH] = 73;
	}
	{ // 33[3]
		int i = 64;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  4 + 31 * BUFFER_WIDTH] = 71;
	}
	{ // 449[1]
		int i = 65;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 30 + 14 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 31 + 13 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 31 + 14 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 31 + 15 * BUFFER_WIDTH] = 68;
	}
	{ // 441[2]
		int i = 66;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 22 + 16 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 23 + 16 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 24 + 16 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 23 + 15 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 24 + 15 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 24 + 14 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 25 + 14 * BUFFER_WIDTH] = 0;
	}
	{ // 32[2]
		int i = 67;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 33 + 19 * BUFFER_WIDTH] = 72;
		gpBuffer[addr + 33 + 20 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 33 + 21 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 33 + 22 * BUFFER_WIDTH] = 66;
		gpBuffer[addr + 33 + 23 * BUFFER_WIDTH] = 67;
		gpBuffer[addr + 33 + 24 * BUFFER_WIDTH] = 112;
		gpBuffer[addr + 33 + 25 * BUFFER_WIDTH] = 117;
		gpBuffer[addr + 33 + 26 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 33 + 27 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 33 + 28 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 33 + 29 * BUFFER_WIDTH] = 67;
		gpBuffer[addr + 33 + 30 * BUFFER_WIDTH] = 69;
	}
	{ // 457[0] - fix glitch after reuse of 465[2]
		int i = 68;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 16 +  0 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 17 +  0 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 18 +  0 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 19 +  0 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 20 +  0 * BUFFER_WIDTH] = 72;
		gpBuffer[addr + 21 +  0 * BUFFER_WIDTH] = 73;
	}
	{ // 469[2] - fix glitch after reuse of 465[4]
		int i = 69;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 18 + 0 * BUFFER_WIDTH] = 57;
		gpBuffer[addr + 18 + 1 * BUFFER_WIDTH] = 58;
		gpBuffer[addr + 18 + 2 * BUFFER_WIDTH] = 91;
	}
	{ // 49[7] - fix glitch
		int i = 70;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 20 + 23 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // 210[0] - fix shadow
		int i = 76;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 27 +  6 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 28 +  7 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 31 +  9 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 30 + 21 * BUFFER_WIDTH] = 123;
		gpBuffer[addr + 31 + 21 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 31 + 21 * BUFFER_WIDTH] = 71;
	}
	{ // 210[1] - fix shadow
		int i = 77;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 0 + 10 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 0 + 15 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 0 + 16 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 0 + 17 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 1 + 11 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 1 + 12 * BUFFER_WIDTH] = 123;
		gpBuffer[addr + 1 + 13 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 1 + 14 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 1 + 15 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 1 + 16 * BUFFER_WIDTH] = 91;
		gpBuffer[addr + 1 + 17 * BUFFER_WIDTH] = 122;
	}
	{ // 233[0] - fix shadow
		int i = 78;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 17 +  8 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 17 +  9 * BUFFER_WIDTH] = 123;
		gpBuffer[addr + 18 +  8 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 19 +  8 * BUFFER_WIDTH] = 123;
		gpBuffer[addr + 20 +  7 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 20 +  8 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 24 +  7 * BUFFER_WIDTH] = 123;
		gpBuffer[addr + 25 +  6 * BUFFER_WIDTH] = 123;
		gpBuffer[addr + 25 +  5 * BUFFER_WIDTH] = 120;
	}
	// improve connections
	{ // 25[1]
		int i = 79;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 18 + 22 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 15 + 24 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 30 + 16 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 29 + 16 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 16 + 23 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 22 + 20 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 21 + 20 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 21 + 19 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 25 + 19 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 24 + 19 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 23 + 20 * BUFFER_WIDTH] = 68;

		gpBuffer[addr +  8 + 27 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  9 + 27 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 10 + 26 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 11 + 26 * BUFFER_WIDTH] = 67;
		gpBuffer[addr +  9 + 26 * BUFFER_WIDTH] = 69;
		gpBuffer[addr +  7 + 27 * BUFFER_WIDTH] = 70;

		gpBuffer[addr + 12 +  7 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 11 +  6 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  8 +  5 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 14 +  8 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 18 + 10 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  5 +  3 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  2 +  2 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 16 +  9 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 15 +  8 * BUFFER_WIDTH] = 67;
		gpBuffer[addr + 13 +  7 * BUFFER_WIDTH] = 67;
	}
	{ // 26[0]
		int i = 80;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 28 +  4 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 29 +  3 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 25 +  9 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 26 +  9 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 28 +  9 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 29 +  9 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 29 + 10 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 30 + 10 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 30 + 11 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 31 + 10 * BUFFER_WIDTH] = 70;

		gpBuffer[addr + 25 +  7 * BUFFER_WIDTH] = 118;
		gpBuffer[addr + 26 +  7 * BUFFER_WIDTH] = 120;
		gpBuffer[addr + 27 +  8 * BUFFER_WIDTH] = 117;
		gpBuffer[addr + 27 +  7 * BUFFER_WIDTH] = 119;
		gpBuffer[addr + 29 +  7 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 28 +  7 * BUFFER_WIDTH] = 120;
		gpBuffer[addr + 29 +  8 * BUFFER_WIDTH] = 87;
		gpBuffer[addr + 30 +  8 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 20 +  6 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 18 +  7 * BUFFER_WIDTH] = 117;
		gpBuffer[addr + 16 +  8 * BUFFER_WIDTH] = 117;
		gpBuffer[addr + 12 + 10 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 11 + 11 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 12 + 12 * BUFFER_WIDTH] = 69;

		gpBuffer[addr +  5 + 14 * BUFFER_WIDTH] = 69;
		gpBuffer[addr +  4 + 15 * BUFFER_WIDTH] = 69;
	}
	{ // 26[1]
		int i = 81;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  0 + 10 * BUFFER_WIDTH] = 71;
		gpBuffer[addr +  0 + 11 * BUFFER_WIDTH] = 71;
		gpBuffer[addr +  3 +  4 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  4 +  4 * BUFFER_WIDTH] = 69;
		gpBuffer[addr +  5 +  5 * BUFFER_WIDTH] = 70;
		gpBuffer[addr +  1 +  7 * BUFFER_WIDTH] = 115;
		gpBuffer[addr +  3 +  6 * BUFFER_WIDTH] = 70;
		gpBuffer[addr +  2 +  7 * BUFFER_WIDTH] = 71;
		gpBuffer[addr +  4 +  7 * BUFFER_WIDTH] = 71;
		gpBuffer[addr +  6 +  7 * BUFFER_WIDTH] = 69;
		gpBuffer[addr +  3 +  8 * BUFFER_WIDTH] = 87;
		gpBuffer[addr +  4 +  8 * BUFFER_WIDTH] = 88;
		gpBuffer[addr +  6 +  8 * BUFFER_WIDTH] = 53;
		gpBuffer[addr +  1 +  8 * BUFFER_WIDTH] = 69;
		gpBuffer[addr +  2 +  8 * BUFFER_WIDTH] = 69;
		gpBuffer[addr +  1 +  9 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  2 +  9 * BUFFER_WIDTH] = 67;
		gpBuffer[addr +  5 +  9 * BUFFER_WIDTH] = 70;
		gpBuffer[addr +  6 +  9 * BUFFER_WIDTH] = 69;
		gpBuffer[addr +  1 + 10 * BUFFER_WIDTH] = 67;
		gpBuffer[addr +  2 + 10 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  1 + 11 * BUFFER_WIDTH] = 68;
	}
	{ // 205[1]
		int i = 82;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 17 +  9 * BUFFER_WIDTH] = 67;
		gpBuffer[addr + 13 +  7 * BUFFER_WIDTH] = 67;
		gpBuffer[addr + 10 +  6 * BUFFER_WIDTH] = 67;
		gpBuffer[addr +  7 +  4 * BUFFER_WIDTH] = 67;
		gpBuffer[addr + 21 + 11 * BUFFER_WIDTH] = 67;
		gpBuffer[addr + 25 + 13 * BUFFER_WIDTH] = 67;
		gpBuffer[addr + 24 + 13 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 15 +  8 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 12 +  7 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 15 +  9 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 11 +  7 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 11 +  6 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 12 +  8 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 16 + 10 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 20 + 13 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 21 + 13 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 20 + 12 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 20 + 11 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 19 + 12 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 14 + 24 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 14 + 23 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 13 + 25 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 13 + 24 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 13 + 23 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 12 + 23 * BUFFER_WIDTH] = 68;
	}
	{ // 385[0]
		int i = 83;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 24 +  4 * BUFFER_WIDTH] = 68;
	}
	{ // 385[1]
		int i = 84;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 14 +  8 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 13 +  8 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 14 +  9 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 15 +  9 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 12 +  8 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 13 +  9 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 11 +  8 * BUFFER_WIDTH] = 68;
	}
	{ // 386[1]
		int i = 85;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 25 + 13 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 27 + 14 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 25 + 14 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 14 + 13 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 23 + 13 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 23 + 12 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 26 + 14 * BUFFER_WIDTH] = 67;
		gpBuffer[addr + 26 + 15 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 10 +  6 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  9 +  5 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 11 +  6 * BUFFER_WIDTH] = 67;
		gpBuffer[addr +  7 + 28 * BUFFER_WIDTH] = 69;
		gpBuffer[addr +  6 + 28 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  5 + 28 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 15 + 24 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 11 +  7 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 11 +  8 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 12 +  7 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 13 +  7 * BUFFER_WIDTH] = 68;
	}
	{ // 387[0]
		int i = 86;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 26 +  3 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 21 +  6 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 16 +  8 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 12 + 10 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  8 + 12 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 18 +  7 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  4 + 14 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 22 +  5 * BUFFER_WIDTH] = 67;
		gpBuffer[addr + 27 +  3 * BUFFER_WIDTH] = 67;
		gpBuffer[addr + 28 +  2 * BUFFER_WIDTH] = 67;
		gpBuffer[addr + 15 +  9 * BUFFER_WIDTH] = 67;
		gpBuffer[addr + 20 +  6 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 24 +  4 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 24 +  5 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 28 +  4 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 29 +  2 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 29 +  3 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 14 +  9 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 14 + 10 * BUFFER_WIDTH] = 69;
		gpBuffer[addr +  3 + 17 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 23 +  5 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 24 +  5 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 24 +  6 * BUFFER_WIDTH] = 67;
		gpBuffer[addr + 23 +  6 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 19 +  7 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 24 +  7 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 25 +  6 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 26 +  6 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 20 +  7 * BUFFER_WIDTH] = 68;
	}
	{ // 388[1]
		int i = 87;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  4 + 29 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  5 + 29 * BUFFER_WIDTH] = 69;
	}
	{ // 390[1]
		int i = 88;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 27 + 14 * BUFFER_WIDTH] = 68;

		gpBuffer[addr + 26 + 14 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 27 + 18 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 26 + 18 * BUFFER_WIDTH] = 67;
		gpBuffer[addr +  7 + 28 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  9 + 27 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  8 + 26 * BUFFER_WIDTH] = 69;
		gpBuffer[addr +  6 + 27 * BUFFER_WIDTH] = 69;
		gpBuffer[addr +  7 + 27 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  8 + 27 * BUFFER_WIDTH] = 67;
		gpBuffer[addr + 11 +  6 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 10 +  6 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  9 +  5 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 13 +  7 * BUFFER_WIDTH] = 68;
	}
	{ // 395[0]
		int i = 89;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  5 + 14 * BUFFER_WIDTH] = 69;
		gpBuffer[addr +  3 + 15 * BUFFER_WIDTH] = 69;
	}
	{ // 395[1]
		int i = 90;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  0 + 10 * BUFFER_WIDTH] = 71;
		gpBuffer[addr +  0 + 11 * BUFFER_WIDTH] = 71;
		gpBuffer[addr +  3 +  4 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  4 +  4 * BUFFER_WIDTH] = 69;
		gpBuffer[addr +  5 +  5 * BUFFER_WIDTH] = 70;
		gpBuffer[addr +  1 +  7 * BUFFER_WIDTH] = 115;
		gpBuffer[addr +  3 +  6 * BUFFER_WIDTH] = 70;
		gpBuffer[addr +  2 +  7 * BUFFER_WIDTH] = 71;
		gpBuffer[addr +  4 +  7 * BUFFER_WIDTH] = 71;
		gpBuffer[addr +  6 +  7 * BUFFER_WIDTH] = 69;
		gpBuffer[addr +  3 +  8 * BUFFER_WIDTH] = 87;
		gpBuffer[addr +  4 +  8 * BUFFER_WIDTH] = 88;
		gpBuffer[addr +  6 +  8 * BUFFER_WIDTH] = 53;
		gpBuffer[addr +  1 +  8 * BUFFER_WIDTH] = 69;
		gpBuffer[addr +  2 +  8 * BUFFER_WIDTH] = 69;
		gpBuffer[addr +  1 +  9 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  2 +  9 * BUFFER_WIDTH] = 67;
		gpBuffer[addr +  5 +  9 * BUFFER_WIDTH] = 70;
		gpBuffer[addr +  6 +  9 * BUFFER_WIDTH] = 69;
		gpBuffer[addr +  1 + 10 * BUFFER_WIDTH] = 67;
		gpBuffer[addr +  2 + 10 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  1 + 11 * BUFFER_WIDTH] = 68;
	}
	{ // fix 511[5] after reuse
		int i = 91;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		for (int y = 15; y < MICRO_HEIGHT; y++) {
			gpBuffer[addr + 15 + y * BUFFER_WIDTH] = TRANS_COLOR;
		}
		/*gpBuffer[addr +  8 + 24 * BUFFER_WIDTH] = 125;
		gpBuffer[addr +  8 + 25 * BUFFER_WIDTH] = 125;
		gpBuffer[addr +  8 + 26 * BUFFER_WIDTH] = 60;
		gpBuffer[addr +  8 + 27 * BUFFER_WIDTH] = 125;
		gpBuffer[addr +  8 + 28 * BUFFER_WIDTH] = 123;
		gpBuffer[addr +  8 + 29 * BUFFER_WIDTH] = 125;
		gpBuffer[addr +  8 + 30 * BUFFER_WIDTH] = 94;
		gpBuffer[addr +  8 + 31 * BUFFER_WIDTH] = 60;
		gpBuffer[addr +  9 + 31 * BUFFER_WIDTH] = 59;*/
	}
	{ // move pixels to 171[3]
		int i = 92;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 31 + 14 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 31 + 15 * BUFFER_WIDTH] = 93;
	}

	// create new shadow 435[1] using 544[1] and 551[1]
	for (int i = 63; i < 64; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (x < 9 && y > 18) {
					addr2 = x + MICRO_WIDTH * ((i - 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 1) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 544[1]
				} else {
					addr2 = x + MICRO_WIDTH * ((i - 38) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i - 38) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 551[1]
				}
				gpBuffer[addr] = gpBuffer[addr2];
			}
		}
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

static BYTE* patchCavesStairsCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	typedef struct {
		int subtileIndex;
		unsigned microIndex;
		int res_encoding;
	} CelMicro;

	const CelMicro micros[] = {
/*  0 */{ 180 - 1, 2, -1 },              // sync stairs (used to block subsequent calls)
/*  1 */{ 180 - 1, 4, -1 },
/*  2 */{ 171 - 1, 0, MET_LTRIANGLE },
/*  3 */{ 171 - 1, 1, MET_RTRAPEZOID },
/*  4 */{ 171 - 1, 3, MET_SQUARE },
/*  5 */{ 173 - 1, 1, MET_RTRIANGLE },
/*  6 */{ 174 - 1, 0, MET_LTRIANGLE },
/*  7 */{ 174 - 1, 2, -1 },
/*  8 */{ 174 - 1, 5, MET_TRANSPARENT },
/*  9 */{ 174 - 1, 7, MET_TRANSPARENT },
/* 10 */{ 176 - 1, 0, MET_LTRIANGLE },
/* 11 */{ 176 - 1, 2, -1 },

/* 12 */{ 163 - 1, 1, MET_RTRIANGLE },
	};

	const uint16_t* pSubtiles = (const uint16_t*)minBuf;
	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_L3;
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

	// move pixels to 171[0] from 176[2]
	for (int i = 2; i < 3; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y > 16 + x / 2 || y < 16 - x / 2) {
					continue;
				}
				unsigned addr2 = x + MICRO_WIDTH * (11 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (11 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 176[2]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}

	// move pixels to 171[1] from 174[0] and 174[2]
	for (int i = 3; i < 4; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y > 31 - x / 2) {
					continue;
				}
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * (7 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (7 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 174[2]
				} else {
					addr2 = x + MICRO_WIDTH * (6 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (6 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 174[0]
				}
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// move pixels to 171[3] from 174[2]
	for (int i = 4; i < 5; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (7 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (7 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 174[2]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// move pixels to 173[1] from 176[0] and 176[2]
	for (int i = 5; i < 6; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y > 31 - x / 2 || y < 1 + x / 2) {
					continue;
				}
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * (11 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (11 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 176[2]
				} else {
					addr2 = x + MICRO_WIDTH * (10 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (10 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 176[0]
				}
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// move pixels to 174[5] from 180[4] and 180[2]
	for (int i = 8; i < 9; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * (1 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (1 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 180[4]
				} else {
					addr2 = x + MICRO_WIDTH * (0 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (0 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 180[2]
				}
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
	}
	// move pixels to 174[7] from 180[4]
	for (int i = 9; i < 10; i++) {
		for (int x = 0; x < MICRO_WIDTH / 2; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (1 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (1 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 180[4]
				BYTE color = gpBuffer[addr2];
				if (color != TRANS_COLOR) {
					gpBuffer[addr] = color;
				}
			}
		}
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

static BYTE* patchCavesWall1Cel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	typedef struct {
		int subtileIndex;
		unsigned microIndex;
		int res_encoding;
	} CelMicro;

	const CelMicro micros[] = {
/*  0 */{ 446 - 1, 1, -1 },              // mask walls leading to north east
/*  1 */{ 446 - 1, 3, -1 },
/*  2 */{  35 - 1, 0, MET_TRANSPARENT },
/*  3 */{  34 - 1, 1, MET_TRANSPARENT },
/*  4 */{  43 - 1, 0, MET_TRANSPARENT },
/*  5 */{  42 - 1, 1, MET_TRANSPARENT },
/*  6 */{  55 - 1, 0, MET_TRANSPARENT },
/*  7 */{  54 - 1, 1, MET_TRANSPARENT },
/*  8 */{  91 - 1, 0, MET_TRANSPARENT },
/*  9 */{  90 - 1, 1, MET_TRANSPARENT },
/* 10 */{ 165 - 1, 0, MET_TRANSPARENT },
/* 11 */{ 164 - 1, 1, MET_TRANSPARENT },
/* 12 */{ /*251*/ - 1, 0, -1 },
/* 13 */{ /*250*/ - 1, 1, -1 },
/* 14 */{ 316 - 1, 0, MET_TRANSPARENT },
/* 15 */{ 315 - 1, 1, MET_TRANSPARENT },
/* 16 */{ 322 - 1, 0, MET_TRANSPARENT },
/* 17 */{ 321 - 1, 1, MET_TRANSPARENT },
/* 18 */{ /*336*/ - 1, 0, -1 },
/* 19 */{ 335 - 1, 1, MET_TRANSPARENT },
/* 20 */{ 343 - 1, 0, MET_TRANSPARENT },
/* 21 */{ 342 - 1, 1, MET_TRANSPARENT },
/* 22 */{ 400 - 1, 0, MET_TRANSPARENT },
/* 23 */{ 399 - 1, 1, MET_TRANSPARENT },
/* 24 */{ 444 - 1, 0, MET_TRANSPARENT },
/* 25 */{ 443 - 1, 1, MET_TRANSPARENT },
/* 26 */{ 476 - 1, 0, MET_TRANSPARENT },
/* 27 */{ 475 - 1, 1, MET_TRANSPARENT },

/* 28 */{ 440 - 1, 0, MET_TRANSPARENT },
/* 29 */{ 439 - 1, 1, MET_TRANSPARENT },
/* 30 */{ /*448*/ - 1, 0, -1 },
/* 31 */{ 447 - 1, 1, MET_TRANSPARENT },
/* 32 */{ /*452*/ - 1, 0, -1 },
/* 33 */{ 451 - 1, 1, MET_TRANSPARENT },
/* 34 */{ 456 - 1, 0, MET_TRANSPARENT },
/* 35 */{ 455 - 1, 1, MET_TRANSPARENT },
/* 36 */{ 484 - 1, 0, MET_TRANSPARENT },
/* 37 */{ 483 - 1, 1, MET_TRANSPARENT },

/* 38 */{  32 - 1, 1, -1 },
/* 39 */{  32 - 1, 3, -1 },
/* 40 */{  32 - 1, 5, -1 },
/* 41 */{  32 - 1, 7, -1 },
/* 42 */{ /*1*/ - 1, 1, -1 },
/* 43 */{ /*1*/ - 1, 3, -1 },
/* 44 */{ /*1*/ - 1, 5, -1 },
/* 45 */{   1 - 1, 7, MET_TRANSPARENT },
/* 46 */{  44 - 1, 1, MET_TRANSPARENT },
/* 47 */{  44 - 1, 3, MET_TRANSPARENT },
/* 48 */{  44 - 1, 5, MET_TRANSPARENT },
/* 49 */{ /*44*/ - 1, 7, -1 },
/* 50 */{  34 - 1, 0, MET_TRANSPARENT },
/* 51 */{  34 - 1, 2, MET_TRANSPARENT },
/* 52 */{  34 - 1, 4, MET_TRANSPARENT },
/* 53 */{  34 - 1, 6, MET_TRANSPARENT },
/* 54 */{  42 - 1, 0, MET_TRANSPARENT },
/* 55 */{  42 - 1, 2, MET_TRANSPARENT },
/* 56 */{  42 - 1, 4, MET_TRANSPARENT },
/* 57 */{  42 - 1, 6, MET_TRANSPARENT },
/* 58 */{  54 - 1, 0, MET_TRANSPARENT },
/* 59 */{  54 - 1, 2, MET_TRANSPARENT },
/* 60 */{  54 - 1, 4, MET_TRANSPARENT },
/* 61 */{  54 - 1, 6, MET_TRANSPARENT },
/* 62 */{  90 - 1, 0, MET_TRANSPARENT },
/* 63 */{  90 - 1, 2, MET_TRANSPARENT },
/* 64 */{  90 - 1, 4, MET_TRANSPARENT },
/* 65 */{  90 - 1, 6, MET_TRANSPARENT },
/* 66 */{ 164 - 1, 0, MET_TRANSPARENT },
/* 67 */{ 164 - 1, 2, MET_TRANSPARENT },
/* 68 */{ /*164*/ - 1, 4, -1 },
/* 69 */{ /*164*/ - 1, 6, -1 },
/* 70 */{ /*250*/ - 1, 0, -1 },
/* 71 */{ /*250*/ - 1, 2, -1 },
/* 72 */{ /*250*/ - 1, 4, -1 },
/* 73 */{ /*250*/ - 1, 6, -1 },
/* 74 */{ 315 - 1, 0, MET_TRANSPARENT },
/* 75 */{ 315 - 1, 2, MET_TRANSPARENT },
/* 76 */{ 315 - 1, 4, MET_TRANSPARENT },
/* 77 */{ 315 - 1, 6, MET_TRANSPARENT },
/* 78 */{ 321 - 1, 0, MET_TRANSPARENT },
/* 79 */{ 321 - 1, 2, MET_TRANSPARENT },
/* 80 */{ 321 - 1, 4, MET_TRANSPARENT },
/* 81 */{ 321 - 1, 6, MET_TRANSPARENT },
/* 82 */{ 335 - 1, 0, MET_TRANSPARENT },
/* 83 */{ 335 - 1, 2, MET_TRANSPARENT },
/* 84 */{ 335 - 1, 4, MET_TRANSPARENT },
/* 85 */{ 335 - 1, 6, MET_TRANSPARENT },
/* 86 */{ 342 - 1, 0, MET_TRANSPARENT },
/* 87 */{ 342 - 1, 2, MET_TRANSPARENT },
/* 88 */{ 342 - 1, 4, MET_TRANSPARENT },
/* 89 */{ /*342*/ - 1, 6, -1 },
/* 90 */{ 399 - 1, 0, MET_TRANSPARENT },
/* 91 */{ 399 - 1, 2, MET_TRANSPARENT },
/* 92 */{ 399 - 1, 4, MET_TRANSPARENT },
/* 93 */{ 399 - 1, 6, MET_TRANSPARENT },
/* 94 */{ 443 - 1, 0, MET_TRANSPARENT },
/* 95 */{ 443 - 1, 2, MET_TRANSPARENT },
/* 96 */{ 443 - 1, 4, MET_TRANSPARENT },
/* 97 */{ 443 - 1, 6, MET_TRANSPARENT },
/* 98 */{ /*475*/ - 1, 0, -1 },
/* 99 */{ 475 - 1, 2, MET_TRANSPARENT },
/*100 */{ 475 - 1, 4, MET_TRANSPARENT },
/*101 */{ /*475*/ - 1, 6, -1 },

/*102 */{ 441 - 1, 1, -1 },
/*103 */{ 441 - 1, 3, MET_TRANSPARENT },
/*104 */{ 441 - 1, 5, MET_TRANSPARENT },
/*105 */{ 441 - 1, 7, MET_TRANSPARENT },
/*106 */{ 439 - 1, 0, MET_TRANSPARENT },
/*107 */{ 439 - 1, 2, MET_TRANSPARENT },
/*108 */{ 439 - 1, 4, MET_TRANSPARENT },
/*109 */{ 439 - 1, 6, MET_TRANSPARENT },
/*110 */{ 483 - 1, 0, MET_TRANSPARENT },
/*111 */{ 483 - 1, 2, MET_TRANSPARENT },
/*112 */{ 483 - 1, 4, MET_TRANSPARENT },
/*113 */{ /*483*/ - 1, 6, -1 },

/*114 */{ 162 - 1, 1, MET_RTRIANGLE },
/*115 */{  89 - 1, 1, MET_TRANSPARENT },
/*116 */{  88 - 1, 1, MET_TRANSPARENT },
/*117 */{  88 - 1, 3, MET_TRANSPARENT },
/*118 */{  88 - 1, 5, MET_TRANSPARENT },

/*119 */{   8 - 1, 5, MET_TRANSPARENT },
/*120 */{ 373 - 1, 5, MET_TRANSPARENT },
/*121 */{  20 - 1, 5, MET_TRANSPARENT },
/*122 */{ /*20*/ - 1, 4, -1 },
/*123 */{  16 - 1, 4, MET_TRANSPARENT },

/*124 */{  52 - 1, 0, -1 },
/*125 */{ /*2*/ - 1, 1, -1 },
/*126 */{ /*6*/ - 1, 1, -1 },
/*127 */{ /*18*/ - 1, 1, -1 },
/*128 */{ 510 - 1, 1, MET_TRANSPARENT },

/*129 */{  48 - 1, 1, -1 },
/*130 */{  11 - 1, 0, MET_TRANSPARENT },
/*131 */{ /*15*/ - 1, 0, -1 },
/*132 */{  19 - 1, 0, MET_TRANSPARENT },
/*133 */{ 376 - 1, 0, MET_TRANSPARENT },
/*134 */{ /*380*/ - 1, 0, -1 },
/*135 */{ 513 - 1, 0, MET_TRANSPARENT },

/*136 */{  49 - 1, 3, MET_TRANSPARENT },

/*137 */{  511 - 1, 5, MET_TRANSPARENT },

/*138 */{  450 - 1, 1, MET_TRANSPARENT },
/*139 */{  482 - 1, 1, MET_TRANSPARENT },
/*140 */{ /*438*/ - 1, 1, -1 },
	};

	const uint16_t* pSubtiles = (const uint16_t*)minBuf;
	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_L3;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		if (micro.subtileIndex < 0) {
			continue;
		}
		unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
		if ((SwapLE16(pSubtiles[index]) & 0xFFF) == 0) {
			mem_free_dbg(celBuf);
			app_warn("Invalid (empty) caves subtile (%d).", micro.subtileIndex + 1);
			return NULL; // frame is empty -> abort
		}
	}

	// TODO: check celLen
	// draw the micros to the back-buffer
	pMicrosCel = celBuf;
	constexpr BYTE TRANS_COLOR = 128;
	constexpr int DRAW_HEIGHT = 12;
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

	// mask walls leading to north east [1]
	for (int i = 2; i < 38; i += 2) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * (1 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (1 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 446[3]
				} else {
					addr2 = x + MICRO_WIDTH * (0 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (0 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 446[1]
				}
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	for (int i = 3; i < 38; i += 2) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (1 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (1 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 446[3]
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask walls leading to north east [0]
	// mask 1[1] 44[1]
	for (int i = 42; i < 50; i += 4) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (38 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (38 % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (gpBuffer[addr2] == TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 1[3] 44[3]
	for (int i = 43; i < 50; i += 4) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (39 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (39 % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (gpBuffer[addr2] == TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 1[5] 44[5]
	for (int i = 44; i < 50; i += 4) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (40 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (40 % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (gpBuffer[addr2] == TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 1[7] 44[7]
	for (int i = 45; i < 50; i += 4) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (41 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (41 % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (gpBuffer[addr2] == TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}

	// mask 34[0] 42[0] 54[0] 90[0] 164[0] 250[0] 315[0] 321[0] 335[0] 342[0] 399[0] 443[0] 475[0]
	for (int i = 50; i < 102; i += 4) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * (39 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (39 % DRAW_HEIGHT)) * BUFFER_WIDTH;
				} else {
					addr2 = x + MICRO_WIDTH * (38 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (38 % DRAW_HEIGHT)) * BUFFER_WIDTH;
				}
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 34[2] 42[2] 54[2] 90[2] 164[2] 250[2] 315[2] 321[2] 335[2] 342[2] 399[2] 443[2] 475[2]
	for (int i = 51; i < 102; i += 4) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * (40 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (40 % DRAW_HEIGHT)) * BUFFER_WIDTH;
				} else {
					addr2 = x + MICRO_WIDTH * (39 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (39 % DRAW_HEIGHT)) * BUFFER_WIDTH;
				}
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 34[4] 42[4] 54[4] 90[4] 164[4] 250[4] 315[4] 321[4] 335[4] 342[4] 399[4] 443[4] 475[4]
	for (int i = 52; i < 102; i += 4) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * (41 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (41 % DRAW_HEIGHT)) * BUFFER_WIDTH;
				} else {
					addr2 = x + MICRO_WIDTH * (40 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (40 % DRAW_HEIGHT)) * BUFFER_WIDTH;
				}
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 34[6] 42[6] 54[6] 90[6] 164[6] 250[6] 315[6] 321[6] 335[6] 342[6] 399[6] 443[6] 475[6]
	for (int i = 53; i < 102; i += 4) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (41 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (41 % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}

	{ // prepare mask 441[3]
		int i = 103;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 13 +  1 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 13 +  2 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // prepare mask 441[5]
		int i = 104;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 12 + 29 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 12 + 30 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 12 + 31 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // prepare mask 441[7]
		int i = 105;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  5 + 11 * BUFFER_WIDTH] = TRANS_COLOR;

		gpBuffer[addr +  5 + 19 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  5 + 20 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  5 + 21 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  5 + 22 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  5 + 23 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  5 + 24 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  5 + 25 * BUFFER_WIDTH] = TRANS_COLOR;
	}

	// mask 439[0] 483[0] using 441[3] and 441[1]
	for (int i = 106; i < 114; i += 4) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * (103 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (103 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 441[3]
				} else {
					addr2 = x + MICRO_WIDTH * (102 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (102 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 441[1]
				}
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 439[2] 483[2] using 441[5] and 441[3]
	for (int i = 107; i < 114; i += 4) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * (104 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (104 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 441[5]
				} else {
					addr2 = x + MICRO_WIDTH * (103 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (103 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 441[3]
				}
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 439[4] 483[4] using 441[7] and 441[5]
	for (int i = 108; i < 114; i += 4) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * (105 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (105 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 441[7]
				} else {
					addr2 = x + MICRO_WIDTH * (104 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (104 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 441[5]
				}
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 439[6] using 441[7]
	for (int i = 109; i < 114; i += 4) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (105 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (105 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 441[7]
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}

	// fix 88[5] using 32[5]
	for (int i = 118; i < 119; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (40 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (40 % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (gpBuffer[addr] == TRANS_COLOR && gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr] = gpBuffer[addr2];
				}
			}
		}
	}

	// mask 8[5], 373[5] 20[5]
	for (int i = 119; i < 122; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y > 7 - x / 2) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}

	// mask 20[4], 16[4]
	for (int i = 122; i < 124; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				if (y > x / 2 - 8) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}

	// mask 2[1], 6[1], 18[1], 510[1] with 52[0]
	for (int i = 125; i < 129; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (124 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (124 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 52[0]
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 11[0], 15[0], 19[0], 376[0], 380[0], 513[0] with 48[1]
	for (int i = 130; i < 136; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (129 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (129 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 48[1]
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}

	// mask 511[5] using 54[4] and 54[6]
	for (int i = 137; i < 138; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * (61 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (61 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 54[6]
				} else {
					addr2 = x + MICRO_WIDTH * (60 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (60 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 54[4]
				}
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
				if (x < 7 && y > 11) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 1[1] using 54[2] and 54[4]
	for (int i = 42; i < 43; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * (60 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (60 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 54[4]
				} else {
					addr2 = x + MICRO_WIDTH * (59 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (59 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 54[2]
				}
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 1[3] using 54[4] and 54[6] 
	for (int i = 43; i < 44; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * (61 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (61 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 54[6]
				} else {
					addr2 = x + MICRO_WIDTH * (60 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (60 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 54[4]
				}
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}

	// fix artifacts
	/*{ // 456[0]
		int i = 34;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 12 + 17 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 13 + 18 * BUFFER_WIDTH] = 67;
		gpBuffer[addr + 14 + 19 * BUFFER_WIDTH] = 68;
	}*/
	{ // 89[1]
		int i = 115;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 24 + 12 * BUFFER_WIDTH] = 72;
	}
	{ // 88[1]
		int i = 116;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 28 + 0 * BUFFER_WIDTH] = 70;
	}
	{ // 88[3]
		int i = 117;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 25 + 28 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 24 + 27 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 23 + 25 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 22 + 24 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 22 + 23 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 21 + 22 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 20 + 20 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 18 + 17 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 17 + 15 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 16 + 14 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 16 + 13 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 16 + 12 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 15 + 11 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 15 + 10 * BUFFER_WIDTH] = 88;
		gpBuffer[addr + 15 +  9 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 14 +  7 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 14 +  6 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 14 +  5 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 13 +  4 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 13 +  3 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 13 +  2 * BUFFER_WIDTH] = 72;
		gpBuffer[addr + 13 +  1 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 12 +  1 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 12 +  0 * BUFFER_WIDTH] = 69;
	}
	{ // 54[2]
		int i = 59;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 30 + 19 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 31 + 19 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 28 + 18 * BUFFER_WIDTH] = 89;
		gpBuffer[addr + 29 + 18 * BUFFER_WIDTH] = 120;
		gpBuffer[addr + 30 + 18 * BUFFER_WIDTH] = 88;
		gpBuffer[addr + 31 + 18 * BUFFER_WIDTH] = 71;
	}
	{ // 16[4]
		int i = 123;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 24 +  4 * BUFFER_WIDTH] = 73;
	}
	{ // 49[3]
		int i = 136;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  0 + 19 * BUFFER_WIDTH] = 73;
		gpBuffer[addr +  3 + 18 * BUFFER_WIDTH] = 73;
	}
	{ // 450[1]
		int i = 138;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 12 +  1 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 13 +  2 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 14 +  3 * BUFFER_WIDTH] = 70;

		gpBuffer[addr + 18 +  6 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 19 +  7 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 20 +  8 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 21 +  9 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 22 +  9 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 22 + 10 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 23 + 10 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 23 + 11 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 24 + 11 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 25 + 12 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 26 + 12 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 26 + 13 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 27 + 13 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 28 + 14 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 29 + 14 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 30 + 15 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // 482[1]
		int i = 139;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 10 +  0 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 11 +  1 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 12 +  2 * BUFFER_WIDTH] = 70;

		gpBuffer[addr + 12 +  1 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 13 +  2 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 14 +  3 * BUFFER_WIDTH] = 70;

		gpBuffer[addr + 15 +  4 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 16 +  5 * BUFFER_WIDTH] = 70;

		gpBuffer[addr + 21 +  9 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 22 + 10 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 23 + 11 * BUFFER_WIDTH] = TRANS_COLOR;

		gpBuffer[addr + 25 + 12 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 26 + 13 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr + 28 + 14 * BUFFER_WIDTH] = TRANS_COLOR;
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

static BYTE* patchCavesWall2Cel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	typedef struct {
		int subtileIndex;
		unsigned microIndex;
		int res_encoding;
	} CelMicro;

	const CelMicro micros[] = {
/*  0 */{   38 - 1, 0, -1 },               // mask walls leading to north west
/*  1 */{   38 - 1, 2, -1 },
/*  2 */{  175 - 1, 0, MET_TRANSPARENT },
/*  3 */{  177 - 1, 1, MET_TRANSPARENT },
/*  4 */{  320 - 1, 0, MET_TRANSPARENT },
/*  5 */{  322 - 1, 1, MET_TRANSPARENT },
/*  6 */{  324 - 1, 0, MET_TRANSPARENT },
/*  7 */{  326 - 1, 1, MET_TRANSPARENT },
/*  8 */{  341 - 1, 0, MET_TRANSPARENT },
/*  9 */{  343 - 1, 1, MET_TRANSPARENT },
/* 10 */{  345 - 1, 0, MET_TRANSPARENT },
/* 11 */{  347 - 1, 1, MET_TRANSPARENT },
/* 12 */{  402 - 1, 0, MET_TRANSPARENT },
/* 13 */{  404 - 1, 1, MET_TRANSPARENT },
/* 14 */{   37 - 1, 0, MET_TRANSPARENT },
/* 15 */{   39 - 1, 1, MET_TRANSPARENT },
/* 16 */{   41 - 1, 0, MET_TRANSPARENT },
/* 17 */{   43 - 1, 1, MET_TRANSPARENT },
/* 18 */{   49 - 1, 0, MET_TRANSPARENT },
/* 19 */{   51 - 1, 1, MET_TRANSPARENT },
/* 20 */{   93 - 1, 0, MET_TRANSPARENT },
/* 21 */{   95 - 1, 1, MET_TRANSPARENT },
/* 22 */{  466 - 1, 0, MET_TRANSPARENT },
/* 23 */{  468 - 1, 1, MET_TRANSPARENT },
/* 24 */{  478 - 1, 0, MET_TRANSPARENT },
/* 25 */{  /*480*/ - 1, 1, -1 },

/* 26 */{  36 - 1, 0, -1 },
/* 27 */{  36 - 1, 2, -1 },
/* 28 */{  36 - 1, 4, -1 },
/* 29 */{  36 - 1, 6, -1 },
/* 30 */{ 175 - 1, 1, MET_TRANSPARENT },
/* 31 */{ 175 - 1, 3, MET_TRANSPARENT },
/* 32 */{ 175 - 1, 5, MET_TRANSPARENT },
/* 33 */{ 320 - 1, 1, MET_TRANSPARENT },
/* 34 */{ 320 - 1, 3, MET_TRANSPARENT },
/* 35 */{ 320 - 1, 5, MET_TRANSPARENT },
/* 36 */{ 324 - 1, 1, MET_TRANSPARENT },
/* 37 */{ 324 - 1, 3, MET_TRANSPARENT },
/* 38 */{ 324 - 1, 5, MET_TRANSPARENT },
/* 39 */{ 341 - 1, 1, MET_TRANSPARENT },
/* 40 */{ 341 - 1, 3, MET_TRANSPARENT },
/* 41 */{ 341 - 1, 5, MET_TRANSPARENT },
/* 42 */{ 345 - 1, 1, MET_TRANSPARENT },
/* 43 */{ 345 - 1, 3, MET_TRANSPARENT },
/* 44 */{ 345 - 1, 5, MET_TRANSPARENT },
/* 45 */{ 402 - 1, 1, MET_TRANSPARENT },
/* 46 */{ 402 - 1, 3, MET_TRANSPARENT },
/* 47 */{ 402 - 1, 5, MET_TRANSPARENT },
/* 48 */{  37 - 1, 1, MET_TRANSPARENT },
/* 49 */{  37 - 1, 3, MET_TRANSPARENT },
/* 50 */{  37 - 1, 5, MET_TRANSPARENT },
/* 51 */{  41 - 1, 1, MET_TRANSPARENT },
/* 52 */{  41 - 1, 3, MET_TRANSPARENT },
/* 53 */{  41 - 1, 5, MET_TRANSPARENT },
/* 54 */{  49 - 1, 1, MET_TRANSPARENT },
/* 55 */{  49 - 1, 3, MET_TRANSPARENT },
/* 56 */{  49 - 1, 5, MET_TRANSPARENT },
/* 57 */{  93 - 1, 1, MET_TRANSPARENT },
/* 58 */{  93 - 1, 3, MET_TRANSPARENT },
/* 59 */{  93 - 1, 5, MET_TRANSPARENT },
/* 60 */{ 466 - 1, 1, MET_TRANSPARENT },
/* 61 */{ 466 - 1, 3, MET_TRANSPARENT },
/* 62 */{ 466 - 1, 5, MET_TRANSPARENT },
/* 63 */{ /*478*/ - 1, 1, -1 },
/* 64 */{ 478 - 1, 3, MET_TRANSPARENT },
/* 65 */{ 478 - 1, 5, MET_TRANSPARENT },

/* 66 */{ 471 - 1, 0, -1 },
/* 67 */{ /*471*/ - 1, 2, -1 },
/* 68 */{ 486 - 1, 0, MET_TRANSPARENT },
/* 69 */{ 488 - 1, 1, MET_TRANSPARENT },
/* 70 */{ 470 - 1, 0, MET_TRANSPARENT },
/* 71 */{ 472 - 1, 1, MET_TRANSPARENT },
/* 72 */{ 462 - 1, 0, MET_TRANSPARENT },
/* 73 */{ 464 - 1, 1, MET_TRANSPARENT },
/* 74 */{ 454 - 1, 0, MET_TRANSPARENT },
/* 75 */{ 456 - 1, 1, MET_TRANSPARENT },

/* 76 */{ 470 - 1, 1, -1 },
/* 77 */{ 470 - 1, 3, -1 },
/* 78 */{ 470 - 1, 5, -1 },
/* 79 */{ 470 - 1, 7, -1 },
/* 80 */{ 486 - 1, 1, MET_TRANSPARENT },
/* 81 */{ 486 - 1, 3, MET_TRANSPARENT },
/* 82 */{ 486 - 1, 5, MET_TRANSPARENT },
/* 83 */{ /*486*/ - 1, 7, -1 },

/* 84 */{ 467 - 1, 0, MET_TRANSPARENT }, // sync overlapping micros
/* 85 */{ 459 - 1, 0, MET_TRANSPARENT },

/* 86 */{ 9 - 1, 0, MET_TRANSPARENT }, // fix micros after masking
/* 87 */{ 11 - 1, 0, MET_TRANSPARENT },
/* 88 */{ 92 - 1, 0, MET_TRANSPARENT },
/* 89 */{ 92 - 1, 2, MET_TRANSPARENT },
/* 90 */{ 94 - 1, 0, MET_TRANSPARENT },
/* 91 */{ 252 - 1, 2, MET_TRANSPARENT },
/* 92 */{ 252 - 1, 4, MET_TRANSPARENT },
/* 93 */{ 252 - 1, 6, MET_TRANSPARENT },
/* 94 */{ 376 - 1, 0, MET_TRANSPARENT },
/* 95 */{ 374 - 1, 0, MET_TRANSPARENT },
/* 96 */{ 374 - 1, 6, MET_TRANSPARENT },
	};

	const uint16_t* pSubtiles = (const uint16_t*)minBuf;
	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_L3;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		if (micro.subtileIndex < 0) {
			continue;
		}
		unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
		if ((SwapLE16(pSubtiles[index]) & 0xFFF) == 0) {
			mem_free_dbg(celBuf);
			app_warn("Invalid (empty) caves subtile (%d).", micro.subtileIndex + 1);
			return NULL; // frame is empty -> abort
		}
	}

	// TODO: check celLen
	// draw the micros to the back-buffer
	pMicrosCel = celBuf;
	constexpr BYTE TRANS_COLOR = 128;
	constexpr int DRAW_HEIGHT = 12;
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

	// mask walls leading to north west
	// mask 175[0], 320[0], 324[0], 341[0], 345[0], 402[0] + 37[0], 41[0], 49[0], 93[0], 466[0], 478[0] using 38[2]
	for (int i = 2; i < 26; i += 2) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (1 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (1 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 38[2]
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 177[1], 322[1], 326[1], 343[1], 347[1], 404[1] + 39[1], 43[1], 51[1], 95[1], 468[1] using 38[0] and 38[2]
	for (int i = 3; i < 26; i += 2) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * (1 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (1 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 38[2]
				} else {
					addr2 = x + MICRO_WIDTH * (0 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (0 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 38[0]
				}
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}

	// mask 175[1], 320[1], 324[1], 341[1], 345[1], 402[1] + 37[1], 41[1], 49[1], 93[1], 466[1], 478[1] using 36[0] and 36[2]
	for (int i = 30; i < 66; i += 3) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * (27 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (27 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 36[2]
				} else {
					addr2 = x + MICRO_WIDTH * (26 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (26 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 36[0]
				}
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 175[3], 320[3], 324[3], 341[3], 345[3], 402[3] + 37[3], 41[3], 49[3], 93[3], 466[3], 478[3] using 36[2] and 36[4]
	for (int i = 31; i < 66; i += 3) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * (28 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (28 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 36[4]
				} else {
					addr2 = x + MICRO_WIDTH * (27 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (27 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 36[2]
				}
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 175[5], 320[5], 324[5], 341[5], 345[5], 402[5] + 37[5], 41[5], 49[5], 93[5], 466[5], 478[5] using 36[4] and 36[6]
	for (int i = 32; i < 66; i += 3) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * (29 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (29 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 36[6]
				} else {
					addr2 = x + MICRO_WIDTH * (28 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (28 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 36[4]
				}
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}

	// mask 486[0], 470[0], 462[0], 454[0] using 38[2]
	for (int i = 68; i < 76; i += 2) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (1 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (1 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 38[2]
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 488[1], 472[1], 464[1], 456[1] using 471[0] and 38[2]
	for (int i = 69; i < 76; i += 2) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2;
				if (y < MICRO_HEIGHT / 2) {
					addr2 = x + MICRO_WIDTH * (1 / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * (1 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 38[2]
				} else {
					addr2 = x + MICRO_WIDTH * (66 / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * (66 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 471[0]
				}
				if (gpBuffer[addr2] != TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}

	// mask 486[1] using 470[1]
	for (int i = 80; i < 84; i += 4) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (76 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (76 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 470[1]
				if (gpBuffer[addr2] == TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 486[3] using 470[3]
	for (int i = 81; i < 84; i += 4) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (77 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (77 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 470[3]
				if (gpBuffer[addr2] == TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 486[5] using 470[5]
	for (int i = 82; i < 84; i += 4) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (78 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (78 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 470[5]
				if (gpBuffer[addr2] == TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}
	// mask 486[7] using 470[7]
	for (int i = 83; i < 84; i += 4) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (79 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (79 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 470[7]
				if (gpBuffer[addr2] == TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}

	// sync overlapping micros 467[0], 459[0] with 471[0]
	for (int i = 84; i < 86; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				unsigned addr2 = x + MICRO_WIDTH * (66 / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (66 % DRAW_HEIGHT)) * BUFFER_WIDTH; // 471[0]
				if (gpBuffer[addr2] == TRANS_COLOR) {
					gpBuffer[addr] = TRANS_COLOR;
				}
			}
		}
	}

	// fix micros after masking
	{ // 9[0]
		int i = 86;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  1 +  2 * BUFFER_WIDTH] = 70;
		gpBuffer[addr +  2 +  1 * BUFFER_WIDTH] = 73;
	}
	{ // 11[0]
		int i = 87;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 12 +  7 * BUFFER_WIDTH] = 69;
		gpBuffer[addr +  5 + 13 * BUFFER_WIDTH] = 70;
	}
	{ // 92[0]
		int i = 88;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  0 +  4 * BUFFER_WIDTH] = 119;
		gpBuffer[addr +  1 +  2 * BUFFER_WIDTH] = 70;
		gpBuffer[addr +  1 +  3 * BUFFER_WIDTH] = 70;
		gpBuffer[addr +  2 +  1 * BUFFER_WIDTH] = 73;
		gpBuffer[addr +  2 +  2 * BUFFER_WIDTH] = 119;
		gpBuffer[addr +  3 +  0 * BUFFER_WIDTH] = 120;
	}
	{ // 92[2]
		int i = 89;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  4 + 30 * BUFFER_WIDTH] = 71;
		gpBuffer[addr +  4 + 31 * BUFFER_WIDTH] = 71;
		gpBuffer[addr +  5 + 30 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  5 + 29 * BUFFER_WIDTH] = 68;
		gpBuffer[addr +  6 + 28 * BUFFER_WIDTH] = 70;
		gpBuffer[addr +  7 + 27 * BUFFER_WIDTH] = 119;
		gpBuffer[addr +  7 + 26 * BUFFER_WIDTH] = 71;
		gpBuffer[addr +  8 + 26 * BUFFER_WIDTH] = 71;
		gpBuffer[addr +  8 + 25 * BUFFER_WIDTH] = 71;
		gpBuffer[addr +  8 + 24 * BUFFER_WIDTH] = 71;
		gpBuffer[addr +  9 + 24 * BUFFER_WIDTH] = 67;
		gpBuffer[addr +  9 + 23 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 10 + 23 * BUFFER_WIDTH] = 121;
		gpBuffer[addr + 10 + 22 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 10 + 21 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 11 + 21 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 11 + 20 * BUFFER_WIDTH] = 117;
		gpBuffer[addr + 11 + 19 * BUFFER_WIDTH] = 71;

		gpBuffer[addr + 12 + 19 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 12 + 18 * BUFFER_WIDTH] = 117;
		gpBuffer[addr + 13 + 17 * BUFFER_WIDTH] = 117;
		gpBuffer[addr + 13 + 16 * BUFFER_WIDTH] = 68;

		gpBuffer[addr + 14 + 16 * BUFFER_WIDTH] = 67;
		gpBuffer[addr + 14 + 15 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 14 + 14 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 15 + 14 * BUFFER_WIDTH] = 68;
		gpBuffer[addr + 15 + 13 * BUFFER_WIDTH] = 121;
		gpBuffer[addr + 15 + 12 * BUFFER_WIDTH] = 91;
		gpBuffer[addr + 15 + 11 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 16 + 12 * BUFFER_WIDTH] = 120;
		gpBuffer[addr + 16 + 11 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 16 + 10 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 16 +  9 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 16 +  8 * BUFFER_WIDTH] = 88;
		gpBuffer[addr + 17 +  9 * BUFFER_WIDTH] = 74;
		gpBuffer[addr + 17 +  8 * BUFFER_WIDTH] = 88;
		gpBuffer[addr + 17 +  7 * BUFFER_WIDTH] = 123;
		gpBuffer[addr + 17 +  6 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 17 +  5 * BUFFER_WIDTH] = 120;
		gpBuffer[addr + 18 +  5 * BUFFER_WIDTH] = 120;
		gpBuffer[addr + 18 +  4 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 18 +  3 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 18 +  2 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 18 +  1 * BUFFER_WIDTH] = 60;
		gpBuffer[addr + 19 +  2 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 19 +  1 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 19 +  0 * BUFFER_WIDTH] = 88;
	}
	{ // 94[0]
		int i = 90;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  7 + 12 * BUFFER_WIDTH] = 70;
		gpBuffer[addr +  8 + 11 * BUFFER_WIDTH] = 67;
		gpBuffer[addr +  9 + 10 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 10 +  9 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 11 +  8 * BUFFER_WIDTH] = 69;
		gpBuffer[addr + 12 +  7 * BUFFER_WIDTH] = 69;

		gpBuffer[addr + 16 +  4 * BUFFER_WIDTH] = 71;
		gpBuffer[addr + 17 +  3 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 18 +  2 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 19 +  1 * BUFFER_WIDTH] = 70;
		gpBuffer[addr + 20 +  0 * BUFFER_WIDTH] = 71;
		gpBuffer[addr +  5 + 13 * BUFFER_WIDTH] = 70;
	}
	{ // 252[2]
		int i = 91;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 11 + 19 * BUFFER_WIDTH] = 119;
		gpBuffer[addr + 15 + 11 * BUFFER_WIDTH] = 91;
	}
	{ // 252[4]
		int i = 92;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 19 + 29 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 20 + 24 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 20 + 25 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 21 + 17 * BUFFER_WIDTH] = 0;
		gpBuffer[addr + 21 + 18 * BUFFER_WIDTH] = 0;
	}
	{ // 252[6]
		int i = 93;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 25 + 20 * BUFFER_WIDTH] = 123;
		gpBuffer[addr + 25 + 21 * BUFFER_WIDTH] = 75;
		gpBuffer[addr + 25 + 22 * BUFFER_WIDTH] = 120;
		gpBuffer[addr + 25 + 23 * BUFFER_WIDTH] = 73;
		gpBuffer[addr + 26 + 20 * BUFFER_WIDTH] = 125;
		gpBuffer[addr + 26 + 21 * BUFFER_WIDTH] = 60;
		gpBuffer[addr + 26 + 22 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 26 + 23 * BUFFER_WIDTH] = 121;
	}
	{ // 376[0]
		int i = 94;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 12 +  7 * BUFFER_WIDTH] = 69;
		gpBuffer[addr +  5 + 13 * BUFFER_WIDTH] = 70;
	}
	{ // 374[0]
		int i = 95;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  1 +  2 * BUFFER_WIDTH] = 70;
		gpBuffer[addr +  2 +  1 * BUFFER_WIDTH] = 73;
	}
	{ // 374[6]
		int i = 96;
		unsigned addr = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 24 + 26 * BUFFER_WIDTH] = 89;
		gpBuffer[addr + 24 + 27 * BUFFER_WIDTH] = 121;
		gpBuffer[addr + 24 + 28 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 24 + 29 * BUFFER_WIDTH] = 93;
		gpBuffer[addr + 24 + 30 * BUFFER_WIDTH] = 123;
		gpBuffer[addr + 24 + 31 * BUFFER_WIDTH] = 122;
		gpBuffer[addr + 25 + 20 * BUFFER_WIDTH] = 121;
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

BYTE* DRLP_L3_PatchCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	celBuf = patchCavesFloorCel(minBuf, minLen, celBuf, celLen);
	if (celBuf == NULL) {
		return NULL;
	}
	celBuf = patchCavesStairsCel(minBuf, minLen, celBuf, celLen);
	if (celBuf == NULL) {
		return NULL;
	}
	celBuf = patchCavesWall1Cel(minBuf, minLen, celBuf, celLen);
	if (celBuf == NULL) {
		return NULL;
	}
	celBuf = patchCavesWall2Cel(minBuf, minLen, celBuf, celLen);
	return celBuf;
}

void DRLP_L3_PatchMin(BYTE* buf)
{
	uint16_t* pSubtiles = (uint16_t*)buf;
	constexpr int blockSize = BLOCK_SIZE_L3;

	// adjust the frame types
	// - after patchCavesFloorCel
	SetFrameType(537, 0, MET_LTRIANGLE);
	SetFrameType(540, 1, MET_RTRIANGLE);
	SetFrameType(197, 1, MET_RTRIANGLE);
	SetFrameType(551, 1, MET_RTRIANGLE);

	// - after patchCavesStairsCel
	SetFrameType(171, 0, MET_LTRIANGLE);
	SetFrameType(171, 1, MET_RTRAPEZOID);
	SetFrameType(171, 3, MET_SQUARE);
	SetFrameType(173, 1, MET_RTRIANGLE);
	SetFrameType(174, 0, MET_LTRIANGLE);
	SetFrameType(176, 0, MET_LTRIANGLE);
	SetFrameType(163, 1, MET_RTRIANGLE);

	// use the new shadows
	SetMcr(551, 0, 26, 0);

	SetMcr(536, 0, 520, 0);
	ReplaceMcr(536, 1, 508, 1);
	ReplaceMcr(536, 2, 520, 2);
	ReplaceMcr(536, 3, 545, 3);
	SetMcr(536, 4, 508, 4);
	ReplaceMcr(536, 5, 545, 5);
	SetMcr(536, 6, 508, 6);
	ReplaceMcr(536, 7, 545, 7);

	ReplaceMcr(532, 0, 508, 0);
	SetMcr(532, 1, 517, 1);
	ReplaceMcr(532, 2, 545, 2);
	SetMcr(532, 3, 517, 3);
	ReplaceMcr(532, 4, 545, 4);
	SetMcr(532, 5, 517, 5);
	ReplaceMcr(532, 6, 545, 6);
	SetMcr(532, 7, 517, 7);

	// fix shadow
	ReplaceMcr(501, 0, 493, 0);
	ReplaceMcr(512, 0, 530, 0);
	ReplaceMcr(544, 0, 495, 0);
	SetMcr(544, 1, 435, 1);

	// fix shine
	ReplaceMcr(325, 0, 346, 0);
	ReplaceMcr(325, 2, 38, 2);

	// fix bad artifact
	Blk2Mcr(82, 4);

	// pointless door micros (re-drawn by dSpecial or the object)
	// - vertical doors	
	Blk2Mcr(540, 3);
	Blk2Mcr(540, 5);
	ReplaceMcr(534, 0, 541, 0);
	ReplaceMcr(534, 1, 541, 1);
	Blk2Mcr(534, 2);
	ReplaceMcr(534, 3, 541, 3);
	Blk2Mcr(534, 4);
	ReplaceMcr(534, 5, 541, 5);
	ReplaceMcr(534, 7, 541, 7);
	// - horizontal doors
	Blk2Mcr(537, 2);
	Blk2Mcr(537, 4);
	ReplaceMcr(531, 0, 538, 0);
	ReplaceMcr(531, 1, 538, 1);
	ReplaceMcr(531, 2, 538, 2);
	Blk2Mcr(531, 3);
	ReplaceMcr(531, 4, 538, 4);
	Blk2Mcr(531, 5);
	// pointless pixels
	Blk2Mcr(7, 7);
	Blk2Mcr(14, 6);
	Blk2Mcr(82, 4);
	Blk2Mcr(382, 6);
	Blk2Mcr(4, 1);
	Blk2Mcr(4, 3);
	Blk2Mcr(8, 1);
	Blk2Mcr(8, 3);
	Blk2Mcr(12, 1);
	Blk2Mcr(12, 3);
	Blk2Mcr(20, 1);
	Blk2Mcr(20, 3);
	Blk2Mcr(373, 1);
	Blk2Mcr(373, 3);
	Blk2Mcr(511, 1);
	Blk2Mcr(16, 0);
	Blk2Mcr(16, 2);
	Blk2Mcr(20, 0);
	Blk2Mcr(20, 2);
	Blk2Mcr(377, 0);
	Blk2Mcr(377, 2);
	Blk2Mcr(381, 0);
	Blk2Mcr(381, 2);
	Blk2Mcr(514, 0);
	// - moved pixels
	HideMcr(146, 2);
	Blk2Mcr(146, 3);
	Blk2Mcr(146, 4);
	HideMcr(146, 5);
	Blk2Mcr(150, 2);
	HideMcr(150, 3);
	Blk2Mcr(150, 5);
	Blk2Mcr(174, 4);
	// -  by patchCavesStairsCel
	Blk2Mcr(180, 2);
	ReplaceMcr(180, 4, 30, 4);
	Blk2Mcr(174, 2);
	Blk2Mcr(176, 2);
	Blk2Mcr(163, 3);

	// mask walls
	// after patchCavesWall1Cel
	SetFrameType(476, 0, MET_TRANSPARENT);
	SetFrameType(475, 1, MET_TRANSPARENT);
	SetFrameType(484, 0, MET_TRANSPARENT);
	SetFrameType(483, 1, MET_TRANSPARENT);
	// SetFrameType(475, 0, MET_TRANSPARENT);
	SetFrameType(475, 2, MET_TRANSPARENT);
	SetFrameType(475, 4, MET_TRANSPARENT);

	SetFrameType(483, 0, MET_TRANSPARENT);
	SetFrameType(483, 2, MET_TRANSPARENT);
	SetFrameType(483, 4, MET_TRANSPARENT);
	// SetFrameType(483, 6, MET_TRANSPARENT);

	SetFrameType(6, 1, MET_TRANSPARENT);
	SetFrameType(371, 1, MET_TRANSPARENT);
	SetFrameType(18, 1, MET_TRANSPARENT);
	SetFrameType(15, 0, MET_TRANSPARENT);
	SetFrameType(19, 0, MET_TRANSPARENT);
	SetFrameType(380, 0, MET_TRANSPARENT);

	SetFrameType(162, 1, MET_RTRIANGLE);
	Blk2Mcr(162, 3);
	Blk2Mcr(162, 5);
	Blk2Mcr(162, 7);
	// after patchCavesWall2Cel
	SetFrameType(478, 0, MET_TRANSPARENT);
	// SetFrameType(478, 1, MET_TRANSPARENT);
	SetFrameType(478, 3, MET_TRANSPARENT);
	SetFrameType(478, 5, MET_TRANSPARENT);
	// SetFrameType(478, 7, MET_TRANSPARENT);
	SetFrameType(486, 0, MET_TRANSPARENT);
	SetFrameType(488, 1, MET_TRANSPARENT);

	SetFrameType(486, 1, MET_TRANSPARENT);
	SetFrameType(486, 3, MET_TRANSPARENT);
	SetFrameType(486, 5, MET_TRANSPARENT);
	// SetFrameType(486, 7, MET_TRANSPARENT);

	// reuse subtiles
	// ReplaceMcr(209, 1, 25, 1);

	ReplaceMcr(481, 6, 437, 6);
	ReplaceMcr(481, 7, 437, 7);
	ReplaceMcr(482, 3, 438, 3);
	ReplaceMcr(484, 1, 476, 1); // lost details

	ReplaceMcr(516, 2, 507, 2);
	ReplaceMcr(516, 4, 507, 4);
	ReplaceMcr(516, 6, 507, 6);

	ReplaceMcr(520, 1, 508, 1);
	ReplaceMcr(520, 3, 508, 3);
	ReplaceMcr(517, 4, 508, 4);
	ReplaceMcr(520, 4, 508, 4);
	ReplaceMcr(546, 4, 508, 4);
	ReplaceMcr(517, 6, 508, 6);
	ReplaceMcr(520, 6, 508, 6);
	ReplaceMcr(546, 6, 508, 6);
	ReplaceMcr(546, 2, 520, 2);
	ReplaceMcr(547, 5, 517, 5);

	ReplaceMcr(44, 7, 1, 7);
	ReplaceMcr(1, 5, 44, 5);

	ReplaceMcr(166, 7, 32, 7);
	ReplaceMcr(166, 6, 32, 6);
	ReplaceMcr(313, 7, 32, 7);
	ReplaceMcr(333, 7, 32, 7);
	ReplaceMcr(473, 7, 32, 7);
	ReplaceMcr(473, 6, 32, 6);
	ReplaceMcr(473, 5, 32, 5);

	ReplaceMcr(401, 7, 36, 7);
	ReplaceMcr(401, 6, 36, 6);
	ReplaceMcr(477, 7, 36, 7);
	ReplaceMcr(477, 6, 36, 6);
	ReplaceMcr(477, 4, 36, 4);

	ReplaceMcr(252, 3, 36, 3);
	ReplaceMcr(252, 1, 36, 1);

	ReplaceMcr(478, 1, 37, 1);
	ReplaceMcr(478, 7, 37, 7);
	ReplaceMcr(480, 1, 39, 1);

	ReplaceMcr(457, 6, 465, 6);
	ReplaceMcr(469, 6, 465, 6);
	ReplaceMcr(457, 4, 465, 4);
	ReplaceMcr(469, 4, 465, 4);
	ReplaceMcr(457, 2, 465, 2);

	ReplaceMcr(485, 6, 465, 6);
	ReplaceMcr(485, 4, 465, 4);
	ReplaceMcr(485, 7, 457, 7);
	// ReplaceMcr(487, 2, 38, 2);

	ReplaceMcr(486, 7, 458, 7);

	ReplaceMcr(88, 7, 32, 7);
	ReplaceMcr(397, 6, 32, 6);
	ReplaceMcr(397, 7, 32, 7);
	ReplaceMcr(397, 5, 32, 5);
	ReplaceMcr(313, 5, 88, 5);

	ReplaceMcr(342, 6, 42, 6); // lost details

	ReplaceMcr(437, 7, 441, 7);
	ReplaceMcr(437, 5, 441, 5);
	ReplaceMcr(481, 7, 441, 7);
	ReplaceMcr(481, 5, 441, 5);
	ReplaceMcr(483, 6, 439, 6);

	ReplaceMcr(164, 6, 34, 6);
	ReplaceMcr(164, 4, 34, 4);
	ReplaceMcr(475, 6, 34, 6);
	ReplaceMcr(475, 0, 34, 0);

	ReplaceMcr(92, 6, 374, 6);
	ReplaceMcr(92, 4, 461, 4);
	ReplaceMcr(344, 6, 461, 6);
	ReplaceMcr(374, 4, 461, 4);
	ReplaceMcr(374, 2, 461, 2);
	ReplaceMcr(44, 6, 461, 6);
	ReplaceMcr(44, 4, 461, 4);
	ReplaceMcr(9, 6, 461, 6);
	ReplaceMcr(9, 4, 461, 4);
	ReplaceMcr(9, 2, 36, 2);

	ReplaceMcr(438, 3, 446, 3);
	ReplaceMcr(45, 3, 446, 3);
	ReplaceMcr(450, 3, 446, 3);
	ReplaceMcr(482, 3, 446, 3);
	ReplaceMcr(89, 3, 446, 3);
	// ReplaceMcr(2, 3, 446, 3);
	ReplaceMcr(334, 3, 446, 3);

	// ReplaceMcr(292, 4, 254, 4);
	// ReplaceMcr(163, 3, 33, 3);
	ReplaceMcr(167, 3, 33, 3);
	ReplaceMcr(249, 3, 33, 3);
	ReplaceMcr(398, 3, 33, 3);
	ReplaceMcr(474, 3, 33, 3);

	ReplaceMcr(11, 2, 38, 2);
	ReplaceMcr(467, 2, 38, 2);
	ReplaceMcr(471, 2, 38, 2);
	ReplaceMcr(459, 2, 38, 2);
	ReplaceMcr(479, 2, 38, 2);
	ReplaceMcr(46, 2, 38, 2);
	ReplaceMcr(94, 2, 38, 2);
	ReplaceMcr(172, 2, 38, 2);
	ReplaceMcr(346, 2, 38, 2);
	ReplaceMcr(376, 2, 38, 2);
	ReplaceMcr(403, 2, 38, 2);
	ReplaceMcr(463, 2, 38, 2);

	// ReplaceMcr(146, 4, 8, 6);
	// ReplaceMcr(146, 3, 8, 5);
	ReplaceMcr(146, 1, 104, 1);

	// ReplaceMcr(150, 5, 8, 6);
	// ReplaceMcr(150, 2, 8, 5);
	ReplaceMcr(150, 0, 56, 0); // or 140

	ReplaceMcr(4, 5, 8, 5);

	ReplaceMcr(511, 3, 496, 3);
	ReplaceMcr(509, 1, 493, 1);

	ReplaceMcr(438, 1, 450, 1);
	ReplaceMcr(336, 0, 35, 0);
	ReplaceMcr(452, 0, 440, 0); // lost details

	ReplaceMcr(12, 4, 16, 4);
	ReplaceMcr(20, 4, 16, 4);
	ReplaceMcr(381, 4, 16, 4);
	ReplaceMcr(377, 4, 16, 4); // lost details
	ReplaceMcr(12, 5, 16, 5);
	ReplaceMcr(381, 5, 16, 5);

	ReplaceMcr(13, 0, 47, 0); // lost details
	ReplaceMcr(378, 0, 17, 0); // lost details

	ReplaceMcr(24, 4, 28, 2);
	ReplaceMcr(24, 5, 28, 3);
	ReplaceMcr(3, 4, 30, 4);
	ReplaceMcr(3, 6, 30, 6);
	ReplaceMcr(183, 6, 30, 6);
	ReplaceMcr(350, 6, 30, 6);
	ReplaceMcr(359, 4, 30, 4);
	ReplaceMcr(362, 4, 30, 4);

	ReplaceMcr(10, 7, 29, 7);
	ReplaceMcr(10, 5, 29, 5);
	ReplaceMcr(375, 7, 29, 7);
	ReplaceMcr(375, 5, 29, 5);
	ReplaceMcr(355, 5, 29, 5);

	ReplaceMcr(40, 7, 28, 7);
	ReplaceMcr(453, 7, 28, 7);

	ReplaceMcr(317, 3, 28, 3); // lost details
	// ReplaceMcr(317, 7, 28, 7); // lost details + adjust 317[5]?
	ReplaceMcr(327, 2, 28, 2);
	ReplaceMcr(327, 3, 28, 3);
	ReplaceMcr(327, 5, 28, 5); // lost details

	ReplaceMcr(337, 3, 28, 3);
	ReplaceMcr(337, 7, 28, 7);

	ReplaceMcr(348, 2, 28, 2); // lost details
	ReplaceMcr(348, 3, 28, 3);
	ReplaceMcr(348, 6, 28, 6);
	ReplaceMcr(348, 7, 28, 7);

	ReplaceMcr(351, 2, 28, 2);
	ReplaceMcr(351, 3, 28, 3);
	ReplaceMcr(351, 6, 28, 6);
	ReplaceMcr(351, 7, 28, 7);

	ReplaceMcr(357, 2, 28, 2);
	ReplaceMcr(357, 3, 28, 3);
	ReplaceMcr(357, 5, 28, 5);

	ReplaceMcr(360, 2, 28, 2);
	ReplaceMcr(360, 3, 28, 3);
	ReplaceMcr(360, 5, 28, 5);
	ReplaceMcr(360, 7, 28, 7);

	ReplaceMcr(363, 2, 28, 2);
	ReplaceMcr(363, 3, 28, 3);

	ReplaceMcr(354, 2, 28, 2);
	ReplaceMcr(354, 3, 28, 3);
	ReplaceMcr(354, 6, 28, 6);

	ReplaceMcr(178, 2, 28, 2);
	ReplaceMcr(178, 3, 28, 3);
	ReplaceMcr(178, 5, 28, 5);

	ReplaceMcr(181, 6, 28, 6);
	ReplaceMcr(384, 4, 28, 2);
	ReplaceMcr(384, 5, 28, 3);

	ReplaceMcr(311, 7, 29, 7);
	ReplaceMcr(310, 2, 28, 2);
	ReplaceMcr(310, 6, 28, 6);
	ReplaceMcr(310, 7, 28, 7);

	ReplaceMcr(330, 2, 28, 2);
	ReplaceMcr(330, 3, 28, 3);
	ReplaceMcr(330, 4, 28, 4);
	ReplaceMcr(330, 6, 28, 6);

	ReplaceMcr(202, 0, 26, 0);
	ReplaceMcr(234, 0, 391, 0);
	ReplaceMcr(219, 0, 392, 0);
	ReplaceMcr(242, 1, 391, 1);

	ReplaceMcr(53, 1, 386, 1); // lost details
	ReplaceMcr(189, 0, 213, 0); // lost details
	ReplaceMcr(189, 1, 213, 1); // lost details
	ReplaceMcr(189, 3, 213, 3); // lost details
	ReplaceMcr(194, 0, 230, 0); // lost details
	ReplaceMcr(194, 1, 230, 1); // lost details
	ReplaceMcr(194, 2, 230, 2); // lost details
	ReplaceMcr(229, 0, 193, 0);
	ReplaceMcr(229, 1, 193, 1);
	ReplaceMcr(229, 3, 193, 3); // lost details
	ReplaceMcr(228, 0, 192, 0); // lost details
	ReplaceMcr(228, 1, 192, 1); // lost details
	ReplaceMcr(228, 3, 192, 3); // lost details

	// ignore invisible parts
	ReplaceMcr(1, 1, 333, 1); // lost details
	ReplaceMcr(1, 3, 333, 3); // lost details
	ReplaceMcr(5, 1, 52, 1); // lost details
	ReplaceMcr(17, 1, 52, 1); // lost details
	// ReplaceMcr(370, 1, 52, 1); // lost details

	// - one-subtile islands
	ReplaceMcr(81, 0, 97, 0);

	// eliminate micros of unused subtiles
	// Blk2Mcr(148, 151,  ...),
	Blk2Mcr(102, 1);
	Blk2Mcr(128, 1);
	Blk2Mcr(283, 0);
	Blk2Mcr(353, 4);
	Blk2Mcr(370, 1);
	Blk2Mcr(405, 1);
	Blk2Mcr(405, 3);
	Blk2Mcr(405, 5);
	Blk2Mcr(409, 1);
	Blk2Mcr(436, 1);
	Blk2Mcr(436, 3);
	Blk2Mcr(436, 5);
	Blk2Mcr(519, 0);
	Blk2Mcr(548, 2);
	Blk2Mcr(548, 5);
	Blk2Mcr(549, 0);
	Blk2Mcr(549, 1);
	Blk2Mcr(549, 3);
	Blk2Mcr(435, 0);
	// reused for the new shadows
	// Blk2Mcr(435, 1);
	// Blk2Mcr(551, 1);
	// Blk2Mcr(528, 0 .. 1);
	// Blk2Mcr(532, 0);
	// Blk2Mcr(532, 2);
	// Blk2Mcr(532, 4);
	// Blk2Mcr(532, 6);
	// Blk2Mcr(536, 1);
	// Blk2Mcr(536, 2);
	// Blk2Mcr(536, 3);
	// Blk2Mcr(536, 5);
	// Blk2Mcr(536, 7);
	Blk2Mcr(552, 0);
	Blk2Mcr(552, 2);
	Blk2Mcr(552, 3);
	Blk2Mcr(552, 4);
	Blk2Mcr(552, 5);
	Blk2Mcr(553, 5);
	Blk2Mcr(555, 1);
	Blk2Mcr(556, 2);
	Blk2Mcr(556, 3);
	Blk2Mcr(556, 5);
	Blk2Mcr(556, 6);
	Blk2Mcr(559, 4);
	Blk2Mcr(559, 5);
	Blk2Mcr(559, 6);
	Blk2Mcr(559, 7);

	const int unusedSubtiles[] = {
		2, 6, 15, 18, 21, 147, 149, 152, 153, 155, 157, 160, 161, 179, 195, 204, 205, 208, 209, 211, 218, 220, 221, 222, 224, 225, 226, 227, 240, 241, 243, 250, 251, 253, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 312, 314, 328, 332, 338, 339, 349, 356, 358, 361, 364, 365, 366, 367, 368, 369, 371, 380, 406, 407, 408, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 421, 422, 423, 424, 425, 426, 427, 428, 429, 430, 431, 432, 433, 434, 442, 448, 487, 503, 521, 522, 523, 524, 525, 526, 527, 529, 533, 537, 550, 554, 557, 558, 560
	};

	for (int n = 0; n < lengthof(unusedSubtiles); n++) {
		for (int i = 0; i < blockSize; i++) {
			Blk2Mcr(unusedSubtiles[n], i);
		}
	}
}

void DRLP_L3_PatchTil(BYTE* buf)
{
	uint16_t* pTiles = (uint16_t*)buf;

	// fix shadow
	pTiles[(4 - 1) * 4 + 2] = SwapLE16(19 - 1);  // 15
	pTiles[(104 - 1) * 4 + 2] = SwapLE16(19 - 1);  // 380
	pTiles[(133 - 1) * 4 + 2] = SwapLE16(479 - 1); // 487
	pTiles[(137 - 1) * 4 + 2] = SwapLE16(495 - 1); // 503
	// fix shine
	pTiles[(85 - 1) * 4 + 1] = SwapLE16(398 - 1); // 314

	// useless pixels
	pTiles[(42 - 1) * 4 + 3] = SwapLE16(8 - 1); // 149
	pTiles[(43 - 1) * 4 + 3] = SwapLE16(16 - 1); // 153
	// use common subtiles
	pTiles[(42 - 1) * 4 + 2] = SwapLE16(7 - 1); // 148
	pTiles[(43 - 1) * 4 + 1] = SwapLE16(14 - 1); // 151
	pTiles[(57 - 1) * 4 + 0] = SwapLE16(393 - 1); // 204
	pTiles[(102 - 1) * 4 + 0] = SwapLE16(5 - 1); // 370
	// pTiles[(102 - 1) * 4 + 1] = SwapLE16(6 - 1); // 371
	pTiles[(143 - 1) * 4 + 1] = SwapLE16(506 - 1); // 519
	pTiles[(122 - 1) * 4 + 1] = SwapLE16(446 - 1); // 442
	// - doors
	pTiles[(146 - 1) * 4 + 0] = SwapLE16(490 - 1); // 529
	pTiles[(146 - 1) * 4 + 3] = SwapLE16(539 - 1); // 532

	pTiles[(147 - 1) * 4 + 0] = SwapLE16(540 - 1); // 533
	pTiles[(147 - 1) * 4 + 3] = SwapLE16(542 - 1); // 536

	pTiles[(148 - 1) * 4 + 0] = SwapLE16(490 - 1); // 537

	// use common subtiles instead of minor alterations
	pTiles[(6 - 1) * 4 + 0] = SwapLE16(393 - 1); // 21 TODO: invisible subtiles(6, 105, 112)?
	pTiles[(7 - 1) * 4 + 0] = SwapLE16(393 - 1);
	pTiles[(105 - 1) * 4 + 0] = SwapLE16(393 - 1);
	pTiles[(42 - 1) * 4 + 1] = SwapLE16(69 - 1); // 147 (another option: 108)
	pTiles[(43 - 1) * 4 + 2] = SwapLE16(86 - 1); // 152
	pTiles[(54 - 1) * 4 + 3] = SwapLE16(231 - 1); // 195
	// pTiles[(66 - 1) * 4 + 0] = SwapLE16(224 - 1); // 240
	// pTiles[(66 - 1) * 4 + 1] = SwapLE16(225 - 1); // 241
	// pTiles[(66 - 1) * 4 + 3] = SwapLE16(227 - 1); // 243
	pTiles[(68 - 1) * 4 + 2] = SwapLE16(34 - 1); // 250
	pTiles[(68 - 1) * 4 + 3] = SwapLE16(35 - 1); // 251
	pTiles[(69 - 1) * 4 + 1] = SwapLE16(37 - 1); // 253
	pTiles[(69 - 1) * 4 + 3] = SwapLE16(39 - 1); // 255
	pTiles[(50 - 1) * 4 + 1] = SwapLE16(29 - 1); // 179
	pTiles[(89 - 1) * 4 + 1] = SwapLE16(29 - 1); // 328
	pTiles[(92 - 1) * 4 + 1] = SwapLE16(29 - 1); // 338
	pTiles[(95 - 1) * 4 + 1] = SwapLE16(29 - 1); // 349
	pTiles[(98 - 1) * 4 + 1] = SwapLE16(29 - 1); // 358
	pTiles[(99 - 1) * 4 + 1] = SwapLE16(29 - 1); // 361
	pTiles[(100 - 1) * 4 + 1] = SwapLE16(29 - 1); // 364
	pTiles[(84 - 1) * 4 + 2] = SwapLE16(30 - 1); // 312
	pTiles[(90 - 1) * 4 + 2] = SwapLE16(30 - 1); // 332
	pTiles[(92 - 1) * 4 + 2] = SwapLE16(30 - 1); // 339
	pTiles[(96 - 1) * 4 + 2] = SwapLE16(30 - 1); // 353
	pTiles[(97 - 1) * 4 + 2] = SwapLE16(30 - 1); // 356
	pTiles[(100 - 1) * 4 + 2] = SwapLE16(30 - 1); // 365
	pTiles[(123 - 1) * 4 + 3] = SwapLE16(440 - 1); // 448
	// ignore invisible parts
	pTiles[(1 - 1) * 4 + 1] = SwapLE16(334 - 1); // 2
	// pTiles[(101 - 1) * 4 + 1] = SwapLE16(334 - 1); // 367
	pTiles[(2 - 1) * 4 + 1] = SwapLE16(53 - 1); // 6
	pTiles[(5 - 1) * 4 + 1] = SwapLE16(53 - 1); // 18
	pTiles[(102 - 1) * 4 + 1] = SwapLE16(53 - 1); // 371

	// - shadows
	// pTiles[(57 - 1) * 4 + 0] = SwapLE16(385 - 1);
	pTiles[(57 - 1) * 4 + 1] = SwapLE16(386 - 1); // 205
	pTiles[(61 - 1) * 4 + 0] = SwapLE16(385 - 1); // 220
	pTiles[(61 - 1) * 4 + 1] = SwapLE16(386 - 1); // 221
	pTiles[(58 - 1) * 4 + 0] = SwapLE16(389 - 1); // 208
	pTiles[(58 - 1) * 4 + 1] = SwapLE16(390 - 1); // 209
	pTiles[(58 - 1) * 4 + 3] = SwapLE16(392 - 1); // 211
	pTiles[(60 - 1) * 4 + 2] = SwapLE16(391 - 1); // 218
	pTiles[(62 - 1) * 4 + 0] = SwapLE16(389 - 1); // 224
	pTiles[(62 - 1) * 4 + 1] = SwapLE16(390 - 1); // 225
	pTiles[(62 - 1) * 4 + 2] = SwapLE16(242 - 1); // 226 - after patchCavesFloorCel
	pTiles[(62 - 1) * 4 + 3] = SwapLE16(392 - 1); // 227
	pTiles[(61 - 1) * 4 + 2] = SwapLE16(238 - 1); // 222 - after patchCavesFloorCel
	pTiles[(66 - 1) * 4 + 0] = SwapLE16(389 - 1); // 240
	pTiles[(66 - 1) * 4 + 1] = SwapLE16(390 - 1); // 241
	pTiles[(66 - 1) * 4 + 3] = SwapLE16(392 - 1); // 243

	// - lava
	pTiles[(44 - 1) * 4 + 1] = SwapLE16(57 - 1); // 155
	pTiles[(44 - 1) * 4 + 3] = SwapLE16(63 - 1); // 157
	pTiles[(45 - 1) * 4 + 2] = SwapLE16(74 - 1); // 160
	pTiles[(45 - 1) * 4 + 3] = SwapLE16(71 - 1); // 161
	// - one-subtile islands
	pTiles[(26 - 1) * 4 + 2] = SwapLE16(117 - 1); // 102
	pTiles[(34 - 1) * 4 + 2] = SwapLE16(117 - 1); // 128

	// create new fences
	pTiles[(144 - 1) * 4 + 0] = SwapLE16(505 - 1);
	pTiles[(144 - 1) * 4 + 1] = SwapLE16(528 - 1);
	pTiles[(144 - 1) * 4 + 2] = SwapLE16(516 - 1);
	pTiles[(144 - 1) * 4 + 3] = SwapLE16(536 - 1);
	pTiles[(145 - 1) * 4 + 0] = SwapLE16(505 - 1);
	pTiles[(145 - 1) * 4 + 1] = SwapLE16(515 - 1);
	pTiles[(145 - 1) * 4 + 2] = SwapLE16(551 - 1);
	pTiles[(145 - 1) * 4 + 3] = SwapLE16(532 - 1);

	// eliminate subtiles of unused tiles
	const int unusedTiles[] = {
		70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 101, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 153, 154, 155, 156,
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
