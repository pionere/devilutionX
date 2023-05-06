#include <string>
#include <fstream>

#include "diabloui.h"
#include "selok.h"
#include "utils/paths.h"
#include "utils/file_util.h"
#include "engine/render/cl2_render.h"
#include "engine/render/dun_render.h"

DEVILUTION_BEGIN_NAMESPACE

static unsigned workProgress;
static unsigned workPhase;
static int hashCount;
static constexpr int RETURN_ERROR = 101;
static constexpr int RETURN_DONE = 100;

// base mapflags set in the corresponding .AMP file (only the lower byte is used)
#define MAPFLAG_TYPE      0x00FF
#define MAPFLAG_VERTDOOR  0x0100
#define MAPFLAG_HORZDOOR  0x0200
#define MAPFLAG_VERTARCH  0x0400
#define MAPFLAG_HORZARCH  0x0800
#define MAPFLAG_VERTGRATE 0x1000
#define MAPFLAG_HORZGRATE 0x2000

typedef enum filenames {
	FILE_TOWN_MIN,
	FILE_TOWN_CEL,
	FILE_CATHEDRAL_MIN,
	FILE_CATHEDRAL_SOL,
	FILE_CATACOMBS_TIL,
	FILE_CATACOMBS_MIN,
	FILE_CATACOMBS_SOL,
	FILE_CATACOMBS_AMP,
	FILE_CAVES_MIN,
	FILE_CAVES_SOL,
	FILE_HELL_CEL,
	FILE_HELL_MIN,
	FILE_HELL_TIL,
	FILE_HELL_SOL,
	FILE_HELL_AMP,
	FILE_BHSM_TRN,
	FILE_BSM_TRN,
	FILE_ACIDB_TRN,
	FILE_ACIDBLK_TRN,
	FILE_BLKKNTBE_TRN,
	FILE_DARK_TRN,
	FILE_BLUE_TRN,
	FILE_FATB_TRN,
	FILE_GARGB_TRN,
	FILE_GARGBR_TRN,
	FILE_GRAY_TRN,
	FILE_CNSELBK_TRN,
	FILE_CNSELG_TRN,
	FILE_GUARD_TRN,
	FILE_VTEXL_TRN,
	FILE_RHINOB_TRN,
	FILE_BLACK_TRN,
	FILE_WHITE_TRN,
	FILE_THINV1_TRN,
	FILE_GREY_TRN,
	FILE_PLR_WHBAT,
	FILE_PLR_WLBAT,
	FILE_PLR_WMBAT,
#ifdef HELLFIRE
	FILE_NTOWN_CEL,
	FILE_NTOWN_MIN,
	FILE_CRYPT_TIL,
	FILE_CRYPT_MIN,
	FILE_CRYPT_SOL,
	FILE_NEST_MIN,
	FILE_NEST_SOL,
	FILE_OBJCURS_CEL,
#endif
	NUM_FILENAMES
} filenames;

static const char* const filesToPatch[NUM_FILENAMES] = {
/*FILE_TOWN_MIN*/      "Levels\\TownData\\Town.MIN",
/*FILE_TOWN_CEL*/      "Levels\\TownData\\Town.CEL",
/*FILE_CATHEDRAL_MIN*/ "Levels\\L1Data\\L1.MIN",
/*FILE_CATHEDRAL_SOL*/ "Levels\\L1Data\\L1.SOL",
/*FILE_CATACOMBS_TIL*/ "Levels\\L2Data\\L2.TIL",
/*FILE_CATACOMBS_MIN*/ "Levels\\L2Data\\L2.MIN",
/*FILE_CATACOMBS_SOL*/ "Levels\\L2Data\\L2.SOL",
/*FILE_CATACOMBS_AMP*/ "Levels\\L2Data\\L2.AMP",
/*FILE_CAVES_MIN*/     "Levels\\L3Data\\L3.MIN",
/*FILE_CAVES_SOL*/     "Levels\\L3Data\\L3.SOL",
/*FILE_HELL_CEL*/      "Levels\\L4Data\\L4.CEL",
/*FILE_HELL_MIN*/      "Levels\\L4Data\\L4.MIN",
/*FILE_HELL_TIL*/      "Levels\\L4Data\\L4.TIL",
/*FILE_HELL_SOL*/      "Levels\\L4Data\\L4.SOL",
/*FILE_HELL_AMP*/      "Levels\\L4Data\\L4.AMP",
/*FILE_BHSM_TRN*/      "Monsters\\Monsters\\BHSM.TRN",
/*FILE_BSM_TRN*/       "Monsters\\Monsters\\BSM.TRN",
/*FILE_ACIDB_TRN*/     "Monsters\\Acid\\AcidB.TRN",
/*FILE_ACIDBLK_TRN*/   "Monsters\\Acid\\AcidBlk.TRN",
/*FILE_BLKKNTBE_TRN*/  "Monsters\\Black\\BlkKntBe.TRN",
/*FILE_DARK_TRN*/      "Monsters\\FalSpear\\Dark.TRN",
/*FILE_BLUE_TRN*/      "Monsters\\Fat\\Blue.TRN",
/*FILE_FATB_TRN*/      "Monsters\\Fat\\FatB.TRN",
/*FILE_GARGB_TRN*/     "Monsters\\Gargoyle\\GargB.TRN",
/*FILE_GARGBR_TRN*/    "Monsters\\Gargoyle\\GargBr.TRN",
/*FILE_GRAY_TRN*/      "Monsters\\GoatMace\\Gray.TRN",
/*FILE_CNSELBK_TRN*/   "Monsters\\Mage\\Cnselbk.TRN",
/*FILE_CNSELG_TRN*/    "Monsters\\Mage\\Cnselg.TRN",
/*FILE_GUARD_TRN*/     "Monsters\\Mega\\Guard.TRN",
/*FILE_VTEXL_TRN*/     "Monsters\\Mega\\Vtexl.TRN",
/*FILE_RHINOB_TRN*/    "Monsters\\Rhino\\RhinoB.TRN",
/*FILE_BLACK_TRN*/     "Monsters\\SkelSd\\Black.TRN",
/*FILE_WHITE_TRN*/     "Monsters\\SkelSd\\White.TRN",
/*FILE_THINV1_TRN*/    "Monsters\\Thin\\Thinv1.TRN",
/*FILE_GREY_TRN*/      "Monsters\\Zombie\\Grey.TRN",
/*FILE_PLR_WHBAT*/     "PlrGFX\\Warrior\\WHB\\WHBAT.CL2",
/*FILE_PLR_WLBAT*/     "PlrGFX\\Warrior\\WLB\\WLBAT.CL2",
/*FILE_PLR_WMBAT*/     "PlrGFX\\Warrior\\WMB\\WMBAT.CL2",
#ifdef HELLFIRE
/*FILE_NTOWN_CEL*/     "NLevels\\TownData\\Town.CEL",
/*FILE_NTOWN_MIN*/     "NLevels\\TownData\\Town.MIN",
/*FILE_CRYPT_TIL*/     "NLevels\\L5Data\\L5.TIL",
/*FILE_CRYPT_MIN*/     "NLevels\\L5Data\\L5.MIN",
/*FILE_CRYPT_SOL*/     "NLevels\\L5Data\\L5.SOL",
/*FILE_NEST_MIN*/      "NLevels\\L6Data\\L6.MIN",
/*FILE_NEST_SOL*/      "NLevels\\L6Data\\L6.SOL",
/*FILE_OBJCURS_CEL*/   "Data\\Inv\\Objcurs.CEL",
#endif
};

#define DESCRIPTION_WIDTH (SELGAME_LPANEL_WIDTH - 2 * 10)

#define MICRO_IDX(subtile, blockSize, microIndex) ((subtile) * (blockSize) + (blockSize) - (2 + ((microIndex) & ~1)) + ((microIndex) & 1))
/*#define blkMicro(subtileRef, blockSize, microIndex, value) \
{ \
	assert((pSubtiles[MICRO_IDX(subtileRef - 1, blockSize, microIndex)] & 0xFFF) == SwapLE16(value) || pSubtiles[MICRO_IDX(subtileRef - 1, blockSize, microIndex)] == SwapLE16(0)); \
	pSubtiles[MICRO_IDX(subtileRef - 1, blockSize, microIndex)] = 0; \
}*/
#define blkMicro(subtileRef, microIndex) \
{ \
	pSubtiles[MICRO_IDX(subtileRef - 1, blockSize, microIndex)] = 0; \
}

#define nSolidTable(pn, v) \
if (v) { \
   buf[pn - 1] |= PFLAG_BLOCK_PATH; \
} else { \
   buf[pn - 1] &= ~PFLAG_BLOCK_PATH; \
}

#define nMissileTable(pn, v) \
if (v) { \
   buf[pn - 1] |= PFLAG_BLOCK_MISSILE; \
} else { \
   buf[pn - 1] &= ~PFLAG_BLOCK_MISSILE; \
}

#define nBlockTable(pn, v) \
if (v) { \
   buf[pn - 1] |= PFLAG_BLOCK_LIGHT; \
} else { \
   buf[pn - 1] &= ~PFLAG_BLOCK_LIGHT; \
}

BYTE* WriteSquare(BYTE* pDst, const BYTE* pSrc, BYTE transparentPixel)
{
	int x, y;
	// int length = MICRO_WIDTH * MICRO_HEIGHT;

	// add opaque pixels
	for (y = MICRO_HEIGHT - 1; y >= 0; y--) {
		for (x = 0; x < MICRO_WIDTH; ++x, pSrc++) {
			BYTE pixel = *pSrc;
			if (pixel == transparentPixel) {
				pixel = 0;
			}
			*pDst = pixel;
			++pDst;
		}
		pSrc -= BUFFER_WIDTH + MICRO_WIDTH;
	}
	return pDst;
}

BYTE* WriteTransparentSquare(BYTE* pDst, const BYTE* pSrc, BYTE transparentPixel)
{
	int x, y;
	// int length = MICRO_WIDTH * MICRO_HEIGHT;
	bool hasColor = false;
	BYTE* pStart = pDst;
	BYTE* pHead = pDst;
	pDst++;
	for (y = MICRO_HEIGHT - 1; y >= 0; y--) {
		bool alpha = false;
		for (x = 0; x < MICRO_WIDTH; x++, pSrc++) {
			BYTE pixel = *pSrc;
			if (pixel == transparentPixel) {
				// add transparent pixel
				if ((char)(*pHead) > 0) {
					pHead = pDst;
					pDst++;
				}
				--*pHead;
				alpha = true;
			} else {
				// add opaque pixel
				if (alpha) {
					alpha = false;
					pHead = pDst;
					pDst++;
				}
				*pDst = pixel;
				pDst++;
				++*pHead;
				hasColor = true;
			}
		}
		pSrc -= BUFFER_WIDTH + MICRO_WIDTH;
		pHead = pDst;
		pDst++;
	}
	// if (!hasColor) {
	//     qDebug() << "Empty transparent frame"; -- TODO: log empty frame?
	// }
	// preserve 4-byte alignment
	pHead = pStart + (((size_t)pHead - (size_t)pStart + 3) & ~3);
	return pHead;
}

BYTE* WriteLeftTriangle(BYTE* pDst, const BYTE* pSrc, BYTE transparentPixel)
{
	int i, x, y;
	// int length = MICRO_WIDTH * MICRO_HEIGHT / 2 + MICRO_HEIGHT;

	// memset(pDst, 0, length); -- unnecessary for the game, and the current user did this anyway
	y = MICRO_HEIGHT - 1;
	for (i = MICRO_HEIGHT - 2; i >= 0; i -= 2, y--) {
		// check transparent pixels
		for (x = 0; x < i; x++, pSrc++) {
			BYTE pixel = *pSrc;
			/*if (pixel != transparentPixel) {
				dProgressErr() << QApplication::tr("Invalid non-transparent pixel in the bottom part of the Left Triangle frame.");
				// return pDst;
			}*/
		}
		pDst += i & 2;
		// add opaque pixels
		for (x = i; x < MICRO_WIDTH; x++, pSrc++) {
			BYTE pixel = *pSrc;
			if (pixel == transparentPixel) {
				pixel = 0;
			}
			*pDst = pixel;
			++pDst;
		}
		pSrc -= BUFFER_WIDTH + MICRO_WIDTH;
	}

	for (i = 2; i != MICRO_HEIGHT; i += 2, y--) {
		// check transparent pixels
		for (x = 0; x < i; x++, pSrc++) {
			BYTE pixel = *pSrc;
			/*if (pixel != transparentPixel) {
				dProgressErr() << QApplication::tr("Invalid non-transparent pixel in the top part of the Left Triangle frame.");
				// return pDst;
			}*/
		}
		pDst += i & 2;
		// add opaque pixels
		for (x = i; x < MICRO_WIDTH; ++x, pSrc++) {
			BYTE pixel = *pSrc;
			if (pixel == transparentPixel) {
				pixel = 0;
			}
			*pDst = pixel;
			++pDst;
		}
		pSrc -= BUFFER_WIDTH + MICRO_WIDTH;
	}
	return pDst;
}

BYTE* WriteRightTriangle(BYTE* pDst, const BYTE* pSrc, BYTE transparentPixel)
{
	int i, x, y;
	// int length = MICRO_WIDTH * MICRO_HEIGHT / 2 + MICRO_HEIGHT;

	// memset(pDst, 0, length); -- unnecessary for the game, and the current user did this anyway
	y = MICRO_HEIGHT - 1;
	for (i = MICRO_HEIGHT - 2; i >= 0; i -= 2, y--) {
		// add opaque pixels
		for (x = 0; x < (MICRO_WIDTH - i); x++, pSrc++) {
			BYTE pixel = *pSrc;
			if (pixel == transparentPixel) {
				pixel = 0;
			}
			*pDst = pixel;
			++pDst;
		}
		pDst += i & 2;
		// check transparent pixels
		for (x = MICRO_WIDTH - i; x < MICRO_WIDTH; x++, pSrc++) {
			BYTE pixel = *pSrc;
			/*if (pixel != transparentPixel) {
				dProgressErr() << QApplication::tr("Invalid non-transparent pixel in the bottom part of the Right Triangle frame.");
				// return pDst;
			}*/
		}
		pSrc -= BUFFER_WIDTH + MICRO_WIDTH;
	}

	for (i = 2; i != MICRO_HEIGHT; i += 2, y--) {
		// add opaque pixels
		for (x = 0; x < (MICRO_WIDTH - i); x++, pSrc++) {
			BYTE pixel = *pSrc;
			if (pixel == transparentPixel) {
				pixel = 0;
			}
			*pDst = pixel;
			++pDst;
		}
		pDst += i & 2;
		// check transparent pixels
		for (x = MICRO_WIDTH - i; x < MICRO_WIDTH; x++, pSrc++) {
			BYTE pixel = *pSrc;
			/*if (pixel != transparentPixel) {
				dProgressErr() << QApplication::tr("Invalid non-transparent pixel in the top part of the Right Triangle frame.");
				// return pDst;
			}*/
		}
		pSrc -= BUFFER_WIDTH + MICRO_WIDTH;
	}
	return pDst;
}

BYTE* WriteLeftTrapezoid(BYTE* pDst, const BYTE* pSrc, BYTE transparentPixel)
{
	int i, x, y;
	// int length = (MICRO_WIDTH * MICRO_HEIGHT) / 2 + MICRO_HEIGHT * (2 + MICRO_HEIGHT) / 4 + MICRO_HEIGHT / 2;

	// memset(pDst, 0, length); -- unnecessary for the game, and the current user did this anyway
	y = MICRO_HEIGHT - 1;
	for (i = MICRO_HEIGHT - 2; i >= 0; i -= 2, y--) {
		// check transparent pixels
		for (x = 0; x < i; x++, pSrc++) {
			BYTE pixel = *pSrc;
			/*if (pixel != transparentPixel) {
				dProgressErr() << QApplication::tr("Invalid non-transparent pixel in the bottom part of the Left Trapezoid frame.");
				// return pDst;
			}*/
		}
		pDst += i & 2;
		// add opaque pixels
		for (x = i; x < MICRO_WIDTH; x++, pSrc++) {
			BYTE pixel = *pSrc;
			if (pixel == transparentPixel) {
				pixel = 0;
			}
			*pDst = pixel;
			++pDst;
		}
		pSrc -= BUFFER_WIDTH + MICRO_WIDTH;
	}
	// add opaque pixels
	for (i = MICRO_HEIGHT / 2; i != 0; i--, y--) {
		for (x = 0; x < MICRO_WIDTH; x++, pSrc++) {
			BYTE pixel = *pSrc;
			if (pixel == transparentPixel) {
				pixel = 0;
			}
			*pDst = pixel;
			++pDst;
		}
		pSrc -= BUFFER_WIDTH + MICRO_WIDTH;
	}
	return pDst;
}

BYTE* WriteRightTrapezoid(BYTE* pDst, BYTE* pSrc, BYTE transparentPixel)
{
	int i, x, y;
	// int length = (MICRO_WIDTH * MICRO_HEIGHT) / 2 + MICRO_HEIGHT * (2 + MICRO_HEIGHT) / 4 + MICRO_HEIGHT / 2;

	// memset(pDst, 0, length); -- unnecessary for the game, and the current user did this anyway
	y = MICRO_HEIGHT - 1;
	for (i = MICRO_HEIGHT - 2; i >= 0; i -= 2, y--) {
		// add opaque pixels
		for (x = 0; x < (MICRO_WIDTH - i); x++, pSrc++) {
			BYTE pixel = *pSrc;
			if (pixel == transparentPixel) {
				pixel = 0;
			}
			*pDst = pixel;
			++pDst;
		}
		pDst += i & 2;
		// check transparent pixels
		for (x = MICRO_WIDTH - i; x < MICRO_WIDTH; x++, pSrc++) {
			BYTE pixel = *pSrc;
			/*if (!pixel.isTransparent()) {
				dProgressErr() << QApplication::tr("Invalid non-transparent pixel in the bottom part of the Right Trapezoid frame.");
				// return pDst;
			}*/
		}
		pSrc -= BUFFER_WIDTH + MICRO_WIDTH;
	}
	// add opaque pixels
	for (i = MICRO_HEIGHT / 2; i != 0; i--, y--) {
		for (x = 0; x < MICRO_WIDTH; x++, pSrc++) {
			BYTE pixel = *pSrc;
			if (pixel == transparentPixel) {
				pixel = 0;
			}
			*pDst = pixel;
			++pDst;
		}
		pSrc -= BUFFER_WIDTH + MICRO_WIDTH;
	}
	return pDst;
}

BYTE* EncodeMicro(int encoding, BYTE* pDst, BYTE* pSrc, BYTE transparentPixel)
{
	switch (encoding) {
	case MET_LTRIANGLE:
		pDst = WriteLeftTriangle(pDst, pSrc, transparentPixel);
		break;
	case MET_RTRIANGLE:
		pDst = WriteRightTriangle(pDst, pSrc, transparentPixel);
		break;
	case MET_LTRAPEZOID:
		pDst = WriteLeftTrapezoid(pDst, pSrc, transparentPixel);
		break;
	case MET_RTRAPEZOID:
		pDst = WriteRightTrapezoid(pDst, pSrc, transparentPixel);
		break;
	case MET_SQUARE:
		pDst = WriteSquare(pDst, pSrc, transparentPixel);
		break;
	case MET_TRANSPARENT:
		pDst = WriteTransparentSquare(pDst, pSrc, transparentPixel);
		break;
	}
	return pDst;
}

static void patchTownPotMin(uint16_t* pSubtiles, int potLeftSubtileRef, int potRightSubtileRef)
{
    const unsigned blockSize = 16;
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

static BYTE* patchTownPotCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen, int potLeftSubtileRef, int potRightSubtileRef)
{
	const uint16_t* pSubtiles = (const uint16_t*)minBuf;

	// TODO: check minLen
    const unsigned blockSize = 16;
	unsigned leftIndex0 = MICRO_IDX(potLeftSubtileRef - 1, blockSize, 1);
    unsigned leftFrameRef0 = pSubtiles[leftIndex0] & 0xFFF;
    unsigned leftIndex1 = MICRO_IDX(potLeftSubtileRef - 1, blockSize, 3);
    unsigned leftFrameRef1 = pSubtiles[leftIndex1] & 0xFFF;
    unsigned leftIndex2 = MICRO_IDX(potLeftSubtileRef - 1, blockSize, 5);
    unsigned leftFrameRef2 = pSubtiles[leftIndex2] & 0xFFF;

    if (leftFrameRef1 == 0 || leftFrameRef2 == 0) {
        // TODO: report error if not empty both? + additional checks
        return celBuf; // left frames are empty -> assume it is already done
    }
    if (leftFrameRef0 == 0) {
		mem_free_dbg(celBuf);
        app_warn("Invalid (empty) pot floor subtile (%1).", potLeftSubtileRef);
        return NULL;
    }

    unsigned rightIndex0 = MICRO_IDX(potRightSubtileRef - 1, blockSize, 0);
    unsigned rightFrameRef0 = pSubtiles[rightIndex0] & 0xFFF;
    unsigned rightIndex1 = MICRO_IDX(potRightSubtileRef - 1, blockSize, 2);
    unsigned rightFrameRef1 = pSubtiles[rightIndex1] & 0xFFF;
    unsigned rightIndex2 = MICRO_IDX(potRightSubtileRef - 1, blockSize, 4);
    unsigned rightFrameRef2 = pSubtiles[rightIndex2] & 0xFFF;

    if (rightFrameRef1 != 0 || rightFrameRef2 != 0) {
        // TODO: report error if not empty both? + additional checks
        return celBuf; // right frames are not empty -> assume it is already done
    }
    if (rightFrameRef0 == 0) {
		mem_free_dbg(celBuf);
        app_warn("Invalid (empty) pot floor subtile (%1).", potRightSubtileRef);
        return NULL;
    }

	// TODO: check celLen
	// draw the micros to the back-buffer
	pMicrosCel = celBuf;
	constexpr BYTE TRANS_COLOR = 128;
	memset(&gpBuffer[0], TRANS_COLOR, 3 * BUFFER_WIDTH * MICRO_HEIGHT);

	RenderMicro(&gpBuffer[0 + (MICRO_HEIGHT * 3 - 1) * BUFFER_WIDTH], pSubtiles[leftIndex0], DMT_NONE);
	RenderMicro(&gpBuffer[0 + (MICRO_HEIGHT * 2 - 1) * BUFFER_WIDTH], pSubtiles[leftIndex1], DMT_NONE);
	RenderMicro(&gpBuffer[0 + (MICRO_HEIGHT * 1 - 1) * BUFFER_WIDTH], pSubtiles[leftIndex2], DMT_NONE);

	RenderMicro(&gpBuffer[MICRO_WIDTH + (MICRO_HEIGHT * 3 - 1) * BUFFER_WIDTH], pSubtiles[rightIndex0], DMT_NONE);

	// move the image from left to the right 'subtile'
    for (int x = MICRO_WIDTH / 2; x < MICRO_WIDTH; x++) {
        for (int y = MICRO_HEIGHT / 2; y < 2 * MICRO_HEIGHT; y++) {
			gpBuffer[x + MICRO_WIDTH + (y - MICRO_HEIGHT / 2) * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
			gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
		}
	}
    for (int x = MICRO_WIDTH / 2 + 2; x < MICRO_WIDTH - 4; x++) {
        for (int y = 2 * MICRO_HEIGHT; y < 2 * MICRO_HEIGHT + MICRO_HEIGHT / 2 + 8; y++) {
			if (gpBuffer[x + y * BUFFER_WIDTH] == TRANS_COLOR)
				continue;
			gpBuffer[x + MICRO_WIDTH + (y - MICRO_HEIGHT / 2) * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
			gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
		}
	}

	// create the new CEL file
	BYTE* resCelBuf = DiabloAllocPtr(*celLen + 4 * MICRO_WIDTH * MICRO_HEIGHT);

	typedef struct {
		int type;
		unsigned frameRef;
	} CelFrameEntry;
	CelFrameEntry entries[4];
	entries[0].type = 0; // left 0
	entries[0].frameRef = leftFrameRef0;
	entries[1].type = 1; // left 1
	entries[1].frameRef = leftFrameRef1;
	entries[2].type = 2; // left 2
	entries[2].frameRef = leftFrameRef2;
	entries[3].type = 3; // right 0
	entries[3].frameRef = rightFrameRef0;
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
		int numEntries = entries[next].frameRef - ((size_t)srcHeaderCursor - (size_t)celBuf) / 4;
		for (int i = 0; i < numEntries; i++) {
			dstHeaderCursor[0] = SwapLE32((size_t)dstDataCursor - (size_t)resCelBuf);
			dstHeaderCursor++;
			DWORD len = srcHeaderCursor[1] - srcHeaderCursor[0];
			memcpy(dstDataCursor, celBuf + srcHeaderCursor[0], len);
			dstDataCursor += len;
			srcHeaderCursor++;
		}
		// add the next frame
		dstHeaderCursor[0] = SwapLE32((size_t)dstDataCursor - (size_t)resCelBuf);
		dstHeaderCursor++;
		
		BYTE* frameSrc;
		int encoding = MET_TRANSPARENT;
		switch (entries[next].type) {
		case 0: // left 0
			frameSrc = &gpBuffer[0 + (MICRO_HEIGHT * 3 - 1) * BUFFER_WIDTH];
			encoding = MET_RTRIANGLE;
			break;
		case 1: // left -> right 1
			frameSrc = &gpBuffer[MICRO_WIDTH + (MICRO_HEIGHT * 2 - 1) * BUFFER_WIDTH];
			break;
		case 2: // left -> right 2
			frameSrc = &gpBuffer[MICRO_WIDTH + (MICRO_HEIGHT * 1 - 1) * BUFFER_WIDTH];
			break;
		case 3: // right 0
			frameSrc = &gpBuffer[MICRO_WIDTH + (MICRO_HEIGHT * 3 - 1) * BUFFER_WIDTH];
			break;
		}
		dstDataCursor = EncodeMicro(encoding, dstDataCursor, frameSrc, TRANS_COLOR);

		// skip the original frame
		srcHeaderCursor++;

		// remove entry
		entries[next].frameRef = 0;
	}
	// add remaining entries
	int numEntries = celEntries + 1 - ((size_t)srcHeaderCursor - (size_t)celBuf) / 4;
	for (int i = 0; i < numEntries; i++) {
		dstHeaderCursor[0] = SwapLE32((size_t)dstDataCursor - (size_t)resCelBuf);
		dstHeaderCursor++;
		DWORD len = srcHeaderCursor[1] - srcHeaderCursor[0];
		memcpy(dstDataCursor, celBuf + srcHeaderCursor[0], len);
		dstDataCursor += len;
		srcHeaderCursor++;
	}
	// add file-size
	dstHeaderCursor[0] = SwapLE32((size_t)dstDataCursor - (size_t)resCelBuf);

	*celLen = SwapLE32(dstHeaderCursor[0]);

	mem_free_dbg(celBuf);

	return resCelBuf;
}

static void patchTownFile(BYTE* buf)
{
	// pointless tree micros (re-drawn by dSpecial)
	uint16_t *pSubtiles = (uint16_t*)buf;
	constexpr int blockSize = 16;
	blkMicro(117, 3);
	blkMicro(117, 5);
	blkMicro(128, 2);
	blkMicro(128, 3);
	blkMicro(128, 4);
	blkMicro(128, 5);
	blkMicro(128, 6);
	blkMicro(128, 7);
	blkMicro(129, 3);
	blkMicro(129, 5);
	blkMicro(129, 7);
	blkMicro(130, 2);
	blkMicro(130, 4);
	blkMicro(130, 6);
	blkMicro(156, 2);
	blkMicro(156, 3);
	blkMicro(156, 4);
	blkMicro(156, 5);
	blkMicro(156, 6);
	blkMicro(156, 7);
	blkMicro(156, 8);
	blkMicro(156, 9);
	blkMicro(156, 10);
	blkMicro(156, 11);
	blkMicro(157, 3);
	blkMicro(157, 5);
	blkMicro(157, 7);
	blkMicro(157, 9);
	blkMicro(157, 11);
	blkMicro(158, 2);
	blkMicro(158, 4);
	blkMicro(160, 2);
	blkMicro(160, 3);
	blkMicro(160, 4);
	blkMicro(160, 5);
	blkMicro(160, 6);
	blkMicro(160, 7);
	blkMicro(160, 8);
	blkMicro(160, 9);
	blkMicro(162, 2);
	blkMicro(162, 4);
	blkMicro(162, 6);
	blkMicro(162, 8);
	blkMicro(162, 10);
	blkMicro(212, 3);
	blkMicro(212, 4);
	blkMicro(212, 5);
	blkMicro(212, 6);
	blkMicro(212, 7);
	blkMicro(212, 8);
	blkMicro(212, 9);
	blkMicro(212, 10);
	blkMicro(212, 11);
	blkMicro(214, 4); // optional
	blkMicro(214, 6); // optional
	blkMicro(216, 2);
	blkMicro(216, 4);
	blkMicro(216, 6);
	blkMicro(217, 4);  // optional
	blkMicro(217, 6);  // optional
	blkMicro(217, 8);  // optional
	blkMicro(358, 4);  // optional
	blkMicro(358, 5);  // optional
	blkMicro(358, 6);  // optional
	blkMicro(358, 7);  // optional
	blkMicro(358, 8);  // optional
	blkMicro(358, 9);  // optional
	blkMicro(358, 10); // optional
	blkMicro(358, 11); // optional
	blkMicro(358, 12); // optional
	blkMicro(358, 13); // optional
	blkMicro(360, 4);  // optional
	blkMicro(360, 6);  // optional
	blkMicro(360, 8);  // optional
	blkMicro(360, 10); // optional
	// fix bad artifact
	blkMicro(233, 6);
	// useless black micros
	blkMicro(426, 1);
	blkMicro(427, 0);
	blkMicro(427, 1);
	blkMicro(429, 1);
	// fix bad artifacts
	blkMicro(828, 12);
	blkMicro(828, 13);
	blkMicro(1018, 2);
	// useless black micros
	blkMicro(1143, 0);
	blkMicro(1145, 0);
	blkMicro(1145, 1);
	blkMicro(1146, 0);
	blkMicro(1153, 0);
	blkMicro(1155, 1);
	blkMicro(1156, 0);
	blkMicro(1169, 1);
	blkMicro(1170, 0);
	blkMicro(1170, 1);
	blkMicro(1172, 1);
	blkMicro(1176, 1);
	blkMicro(1199, 1);
	blkMicro(1200, 0);
	blkMicro(1200, 1);
	blkMicro(1202, 1);
	blkMicro(1203, 1);
	blkMicro(1205, 1);
	blkMicro(1212, 0);
	blkMicro(1219, 0);

	pSubtiles[MICRO_IDX(237 - 1, blockSize, 0)] = pSubtiles[MICRO_IDX(402 - 1, blockSize, 0)];
	pSubtiles[MICRO_IDX(237 - 1, blockSize, 1)] = pSubtiles[MICRO_IDX(402 - 1, blockSize, 1)];
	// patch subtiles around the pot of Adria to prevent graphical glitch when a player passes it I.
	patchTownPotMin(pSubtiles, 553, 554);
}

static BYTE* patchHellCel(const BYTE* tilBuf, size_t tilLen, const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen, int exitTileRef)
{
	const uint16_t* pTiles = (const uint16_t*)tilBuf;
	// TODO: check tilLen
	int topLeftSubtileIndex = SwapLE16(pTiles[(exitTileRef - 1) * 4 + 0]);
	int topRightSubtileIndex = SwapLE16(pTiles[(exitTileRef - 1) * 4 + 1]);
	int bottomLeftSubtileIndex = SwapLE16(pTiles[(exitTileRef - 1) * 4 + 2]);
	int bottomRightSubtileIndex = SwapLE16(pTiles[(exitTileRef - 1) * 4 + 3]);

	if (topLeftSubtileIndex != (137 - 1) || topRightSubtileIndex != (138 - 1) || bottomLeftSubtileIndex != (139 - 1) || bottomRightSubtileIndex != (140 - 1)) {
		return celBuf; // non-original subtiles -> assume it is already done
	}

	const uint16_t* pSubtiles = (const uint16_t*)minBuf;

	// TODO: check minLen
	const unsigned blockSize = 16;
	unsigned topLeft_LeftIndex0 = MICRO_IDX(topLeftSubtileIndex, blockSize, 0);
	unsigned topLeft_LeftFrameRef0 = pSubtiles[topLeft_LeftIndex0] & 0xFFF; // 368
	unsigned topLeft_RightIndex0 = MICRO_IDX(topLeftSubtileIndex, blockSize, 1);
	unsigned topLeft_RightFrameRef0 = pSubtiles[topLeft_RightIndex0] & 0xFFF; // 369
	unsigned topLeft_LeftIndex1 = MICRO_IDX(topLeftSubtileIndex, blockSize, 2);
	unsigned topLeft_LeftFrameRef1 = pSubtiles[topLeft_LeftIndex1] & 0xFFF; // 367
	unsigned topRight_LeftIndex0 = MICRO_IDX(topRightSubtileIndex, blockSize, 0);
	unsigned topRight_LeftFrameRef0 = pSubtiles[topRight_LeftIndex0] & 0xFFF; // 370
	unsigned bottomLeft_RightIndex0 = MICRO_IDX(bottomLeftSubtileIndex, blockSize, 1);
	unsigned bottomLeft_RightFrameRef0 = pSubtiles[bottomLeft_RightIndex0] & 0xFFF; // 375
	unsigned bottomRight_RightIndex0 = MICRO_IDX(bottomRightSubtileIndex, blockSize, 1);
	unsigned bottomRight_RightFrameRef0 = pSubtiles[bottomRight_RightIndex0] & 0xFFF; // 377
	unsigned bottomRight_LeftIndex0 = MICRO_IDX(bottomRightSubtileIndex, blockSize, 0);
	unsigned bottomRight_LeftFrameRef0 = pSubtiles[bottomRight_LeftIndex0] & 0xFFF; // 376

	if (topLeft_LeftFrameRef0 == 0 || topLeft_RightFrameRef0 == 0 || topLeft_LeftFrameRef1 == 0 || topRight_LeftFrameRef0 == 0 || bottomLeft_RightFrameRef0 == 0) {
		// TODO: report error if not empty both? + additional checks
		return celBuf; // left frames are empty -> assume it is already done
	}
	if (bottomRight_RightFrameRef0 == 0 || bottomRight_LeftFrameRef0 == 0) {
		mem_free_dbg(celBuf);
		app_warn("Invalid (empty) floor on tile (%1).", exitTileRef);
		return NULL;
	}

	// TODO: check celLen
	// draw the micros to the back-buffer
	pMicrosCel = celBuf;
	constexpr BYTE TRANS_COLOR = 128;
	memset(&gpBuffer[0], TRANS_COLOR, 3 * BUFFER_WIDTH * MICRO_HEIGHT);

	RenderMicro(&gpBuffer[0 + (MICRO_HEIGHT * 1 - 1) * BUFFER_WIDTH], pSubtiles[topLeft_LeftIndex1], DMT_NONE); // 367
	RenderMicro(&gpBuffer[0 + (MICRO_HEIGHT * 2 - 1) * BUFFER_WIDTH], pSubtiles[topLeft_LeftIndex0], DMT_NONE); // 368
	RenderMicro(&gpBuffer[MICRO_WIDTH + (MICRO_HEIGHT * 2 - 1) * BUFFER_WIDTH], pSubtiles[topLeft_RightIndex0], DMT_NONE); // 369
	RenderMicro(&gpBuffer[2 * MICRO_WIDTH + (MICRO_HEIGHT * 3 - 1) * BUFFER_WIDTH], pSubtiles[topRight_LeftIndex0], DMT_NONE); // 370
	// RenderMicro(&gpBuffer[2 * MICRO_WIDTH + (MICRO_HEIGHT * 3 - MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[topRight_LeftIndex0], DMT_NONE); // 370
	RenderMicro(&gpBuffer[0 + (MICRO_HEIGHT * 2 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[bottomLeft_RightIndex0], DMT_NONE); // 375
	RenderMicro(&gpBuffer[0 + (MICRO_HEIGHT * 3 - 1) * BUFFER_WIDTH], pSubtiles[bottomRight_LeftIndex0], DMT_NONE); // 376
	RenderMicro(&gpBuffer[MICRO_WIDTH + (MICRO_HEIGHT * 3 - 1) * BUFFER_WIDTH], pSubtiles[bottomRight_RightIndex0], DMT_NONE); // 377

	// mask floor bits of 369, 370
	// - 369
	for (int x = 0; x < MICRO_WIDTH; x++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			BYTE pixel = gpBuffer[MICRO_WIDTH + x + (MICRO_HEIGHT * 2 - y - 1) * BUFFER_WIDTH];
			if (pixel == TRANS_COLOR)
				continue;
			if (x >= 15 || gpBuffer[MICRO_WIDTH + x + (MICRO_HEIGHT * 2 - y - 1) * BUFFER_WIDTH] < 80) {
				gpBuffer[MICRO_WIDTH + x + (MICRO_HEIGHT * 2 - y - 1) * BUFFER_WIDTH] = TRANS_COLOR;
			}
		}
	}
	// - 370
	for (int x = 0; x < MICRO_WIDTH; x++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			BYTE pixel = gpBuffer[2 * MICRO_WIDTH + x + (MICRO_HEIGHT * 3 - y - 1) * BUFFER_WIDTH];
			if (pixel == TRANS_COLOR)
				continue;
			if (x >= 15 || gpBuffer[2 * MICRO_WIDTH + x + (MICRO_HEIGHT * 3 - y - 1) * BUFFER_WIDTH] < 96) {
				gpBuffer[2 * MICRO_WIDTH + x + (MICRO_HEIGHT * 3 - y - 1) * BUFFER_WIDTH] = TRANS_COLOR;
			}
		}
	}

	// fix bad artifacts
	gpBuffer[MICRO_WIDTH + 13 + (MICRO_HEIGHT * 2 - (MICRO_HEIGHT -1 - 22) - 1) * BUFFER_WIDTH] = TRANS_COLOR;     // 369
	gpBuffer[MICRO_WIDTH + 7 + (MICRO_HEIGHT * 2 - (MICRO_HEIGHT - 1 - 7) - 1) * BUFFER_WIDTH] = TRANS_COLOR;      // 369
	gpBuffer[2 * MICRO_WIDTH + 12 + (MICRO_HEIGHT * 3 - (MICRO_HEIGHT / 2 + 2) - 1) * BUFFER_WIDTH] = 122;         // 370
	gpBuffer[2 * MICRO_WIDTH + 14 + (MICRO_HEIGHT * 3 - (MICRO_HEIGHT / 2 - 1) - 1) * BUFFER_WIDTH] = TRANS_COLOR; // 370

	// copy the frame 370 to its position
	for (int x = 0; x < MICRO_WIDTH; x++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			BYTE pixel = gpBuffer[2 * MICRO_WIDTH + x + (MICRO_HEIGHT * 3 - y - 1) * BUFFER_WIDTH];
			if (pixel == TRANS_COLOR)
				continue;
			gpBuffer[x + MICRO_WIDTH + (MICRO_HEIGHT * 3 - y - MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH] = pixel;
		}
	}

	// create the new CEL file
	BYTE* resCelBuf = DiabloAllocPtr(*celLen + 4 * MICRO_WIDTH * MICRO_HEIGHT);

	typedef struct {
		int type;
		unsigned frameRef;
	} CelFrameEntry;
	CelFrameEntry entries[5];
	entries[0].type = 0; // 367
	// assert(topLeft_LeftFrameRef1 == 367);
	entries[0].frameRef = topLeft_LeftFrameRef1;
	entries[1].type = 1; // 368
	// assert(topLeft_LeftFrameRef0 == 368);
	entries[1].frameRef = topLeft_LeftFrameRef0;
	entries[2].type = 2; // 369
	// assert(topLeft_RightFrameRef0 == 369);
	entries[2].frameRef = topLeft_RightFrameRef0;
	entries[3].type = 3; // 376
	// assert(bottomRight_LeftFrameRef0 == 376);
	entries[3].frameRef = bottomRight_LeftFrameRef0;
	entries[4].type = 4; // 377
	// assert(bottomRight_RightFrameRef0 == 377);
	entries[4].frameRef = bottomRight_RightFrameRef0;
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
		int numEntries = entries[next].frameRef - ((size_t)srcHeaderCursor - (size_t)celBuf) / 4;
		for (int i = 0; i < numEntries; i++) {
			dstHeaderCursor[0] = SwapLE32((size_t)dstDataCursor - (size_t)resCelBuf);
			dstHeaderCursor++;
			DWORD len = srcHeaderCursor[1] - srcHeaderCursor[0];
			memcpy(dstDataCursor, celBuf + srcHeaderCursor[0], len);
			dstDataCursor += len;
			srcHeaderCursor++;
		}
		// add the next frame
		dstHeaderCursor[0] = SwapLE32((size_t)dstDataCursor - (size_t)resCelBuf);
		dstHeaderCursor++;
		
		BYTE* frameSrc;
		int encoding = MET_TRANSPARENT;
		switch (entries[next].type) {
		case 0: // 367
			frameSrc = &gpBuffer[0 + (MICRO_HEIGHT * 1 - 1) * BUFFER_WIDTH];
			break;
		case 1: // 368
			frameSrc = &gpBuffer[0 + (MICRO_HEIGHT * 2 - 1) * BUFFER_WIDTH];
			encoding = MET_SQUARE;
			break;
		case 2: // 369
			frameSrc = &gpBuffer[MICRO_WIDTH + (MICRO_HEIGHT * 2 - 1) * BUFFER_WIDTH];
			break;
		case 3: // 376
			frameSrc = &gpBuffer[0 + (MICRO_HEIGHT * 3 - 1) * BUFFER_WIDTH];
			encoding = MET_LTRAPEZOID;
			break;
		case 4: // 377
			frameSrc = &gpBuffer[MICRO_WIDTH + (MICRO_HEIGHT * 3 - 1) * BUFFER_WIDTH];
			break;
		}
		dstDataCursor = EncodeMicro(encoding, dstDataCursor, frameSrc, TRANS_COLOR);

		// skip the original frame
		srcHeaderCursor++;

		// remove entry
		entries[next].frameRef = 0;
	}
	// add remaining entries
	int numEntries = celEntries + 1 - ((size_t)srcHeaderCursor - (size_t)celBuf) / 4;
	for (int i = 0; i < numEntries; i++) {
		dstHeaderCursor[0] = SwapLE32((size_t)dstDataCursor - (size_t)resCelBuf);
		dstHeaderCursor++;
		DWORD len = srcHeaderCursor[1] - srcHeaderCursor[0];
		memcpy(dstDataCursor, celBuf + srcHeaderCursor[0], len);
		dstDataCursor += len;
		srcHeaderCursor++;
	}
	// add file-size
	dstHeaderCursor[0] = SwapLE32((size_t)dstDataCursor - (size_t)resCelBuf);

	*celLen = SwapLE32(dstHeaderCursor[0]);

	mem_free_dbg(celBuf);

	return resCelBuf;
}

static BYTE* EncodeCl2(BYTE* pBuf, const BYTE* pSrc, int width, int height, BYTE transparentPixel)
{
	const int RLE_LEN = 4; // number of matching colors to switch from bmp encoding to RLE

	int subHeaderSize = CEL_FRAME_HEADER_SIZE;
	int hs = (height - 1) / CEL_BLOCK_HEIGHT;
	hs = (hs + 1) * sizeof(WORD);
	subHeaderSize = std::max(subHeaderSize, hs);

	bool clipped = true; // frame->isClipped();
	// convert one image to cl2-data
	BYTE* pHeader = pBuf;
	if (clipped) {
		// add CL2 FRAME HEADER
		*(WORD*)&pBuf[0] = SwapLE16(subHeaderSize); // SUB_HEADER_SIZE
		*(DWORD*)&pBuf[2] = 0;
		*(DWORD*)&pBuf[6] = 0;
		pBuf += subHeaderSize;
	}

	BYTE* pHead = pBuf;
	BYTE col, lastCol;
	BYTE colMatches = 0;
	bool alpha = false;
	bool first = true;
	for (int i = 1; i <= height; i++) {
		if (clipped && (i % CEL_BLOCK_HEIGHT) == 1 /*&& (i / CEL_BLOCK_HEIGHT) * 2 < SUB_HEADER_SIZE*/) {
			pHead = pBuf;
			*(WORD*)(&pHeader[(i / CEL_BLOCK_HEIGHT) * 2]) = SwapLE16(pHead - pHeader); // pHead - buf - SUB_HEADER_SIZE;

			colMatches = 0;
			alpha = false;
			// first = true;
		}
		first = true;
		for (int j = 0; j < width; j++, pSrc++) {
			BYTE pixel = *pSrc; // frame->getPixel(j, height - i);
			if (pixel != transparentPixel) {
				// add opaque pixel
				col = pixel;
				if (alpha || first || col != lastCol)
					colMatches = 1;
				else
					colMatches++;
				if (colMatches < RLE_LEN || (char)*pHead <= -127) {
					// bmp encoding
					if (alpha || (char)*pHead <= -65 || first) {
						pHead = pBuf;
						pBuf++;
						colMatches = 1;
					}
					*pBuf = col;
					pBuf++;
				} else {
					// RLE encoding
					if (colMatches == RLE_LEN) {
						memset(pBuf - (RLE_LEN - 1), 0, RLE_LEN - 1);
						*pHead += RLE_LEN - 1;
						if (*pHead != 0) {
							pHead = pBuf - (RLE_LEN - 1);
						}
						*pHead = -65 - (RLE_LEN - 1);
						pBuf = pHead + 1;
						*pBuf = col;
						pBuf++;
					}
				}
				--*pHead;

				lastCol = col;
				alpha = false;
			} else {
				// add transparent pixel
				if (!alpha || (char)*pHead >= 127) {
					pHead = pBuf;
					pBuf++;
				}
				++*pHead;
				alpha = true;
			}
			first = false;
		}
		pSrc -= BUFFER_WIDTH + width;
	}
	return pBuf;
}


static BYTE* ReEncodeCL2(BYTE* cl2Buf, size_t *dwLen, int numGroups, int frameCount, int height, int width)
{
	constexpr BYTE TRANS_COLOR = 1;

	BYTE* resCl2Buf = DiabloAllocPtr(2 * *dwLen);
	memset(resCl2Buf, 0, 2 * *dwLen);

	bool groupped = true;
	int headerSize = 0;
	for (int i = 0; i < numGroups; i++) {
		int ni = frameCount;
		headerSize += 4 + 4 * (ni + 1);
	}
	if (groupped) {
		headerSize += sizeof(DWORD) * numGroups;
	}

	DWORD* hdr = (DWORD*)resCl2Buf;
	if (groupped) {
		// add optional {CL2 GROUP HEADER}
		int offset = numGroups * 4;
		for (int i = 0; i < numGroups; i++, hdr++) {
			hdr[0] = offset;
			int ni = frameCount;
			offset += 4 + 4 * (ni + 1);
		}
	}

	BYTE* pBuf = &resCl2Buf[headerSize];
	for (int ii = 0; ii < numGroups; ii++) {
		int ni = frameCount;
		hdr[0] = SwapLE32(ni);
		hdr[1] = SwapLE32((size_t)pBuf - (size_t)hdr);

		const BYTE* frameBuf = CelGetFrameStart(cl2Buf, ii);

		for (int n = 1; n <= ni; n++) {
			memset(&gpBuffer[0], TRANS_COLOR, BUFFER_WIDTH * height);

			Cl2DrawLightTbl(0, height - 1, frameBuf, n, width, 0);
			BYTE* frameSrc = &gpBuffer[0 + (height - 1) * BUFFER_WIDTH];

			pBuf = EncodeCl2(pBuf, frameSrc, width, height, TRANS_COLOR);
			hdr[n + 1] = SwapLE32((size_t)pBuf - (size_t)hdr);
		}
		hdr += ni + 2;
	}

	*dwLen = (size_t)pBuf - (size_t)resCl2Buf;

	return resCl2Buf;
}

static BYTE* patchFile(int index, size_t *dwLen)
{
	BYTE* buf = LoadFileInMem(filesToPatch[index], dwLen);
	if (buf == NULL) {
		app_warn("Unable to open file %s in the mpq.", filesToPatch[index]);
		return NULL;
	}

	switch (index) {
	case FILE_TOWN_MIN:
	{	// patch dMiniTiles - Town.MIN
#if ASSET_MPL == 1
		if (*dwLen < MICRO_IDX(1219 - 1, 16, 0) * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		patchTownFile(buf);
#endif
	} break;
	case FILE_TOWN_CEL:
#ifdef HELLFIRE
	case FILE_NTOWN_CEL:
#endif
	{
#if ASSET_MPL == 1
		// patch dMicroCels - TOWN.CEL
		// patch subtiles around the pot of Adria to prevent graphical glitch when a player passes it II.
#ifdef HELLFIRE
		index = index == FILE_TOWN_CEL ? FILE_TOWN_MIN : FILE_NTOWN_MIN;
#else
		index = FILE_TOWN_MIN;
#endif
		size_t minLen;
		BYTE* minBuf = LoadFileInMem(filesToPatch[index], &minLen);
		if (minBuf == NULL) {
			mem_free_dbg(buf);
			app_warn("Unable to open file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		buf = patchTownPotCel(minBuf, minLen, buf, dwLen, 553, 554);
		mem_free_dbg(minBuf);
#endif
	} break;
	case FILE_CATHEDRAL_MIN:
	{	// patch dMiniTiles - L1.MIN
#if ASSET_MPL == 1
		if (*dwLen < MICRO_IDX(140 - 1, 10, 1) * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		uint16_t *pSubtiles = (uint16_t*)buf;
		constexpr int blockSize = 10;
		// useless black micros
		blkMicro(107, 0);
		blkMicro(107, 1);
		blkMicro(109, 1);
		blkMicro(137, 1);
		blkMicro(138, 0);
		blkMicro(138, 1);
		blkMicro(140, 1);
#endif /* ASSET_MPL == 1 */
	} break;
	case FILE_CATHEDRAL_SOL:
	{	// patch dSolidTable - L1.SOL
		if (*dwLen <= 8) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		nMissileTable(8, false); // the only column which was blocking missiles
	} break;
	case FILE_CATACOMBS_TIL:
	{	// patch dMegaTiles - L2.TIL
		uint16_t *pTiles = (uint16_t*)buf;
		// reuse subtiles
		assert(pTiles[(41 - 1) * 4 + 1] == SwapLE16(139 - 1) || pTiles[(41 - 1) * 4 + 1] == SwapLE16(135 - 1));
		pTiles[(41 - 1) * 4 + 1] = SwapLE16(135 - 1);
		// add separate tiles and subtiles for the arches I.
		if (*dwLen < 164 * 4 * 2) {
			if (*dwLen != 160 * 4 * 2) {
				mem_free_dbg(buf);
				app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
				return NULL;
			}
			pTiles = (uint16_t*)DiabloAllocPtr(*dwLen + 4 * 4 * 2);
			memcpy(pTiles, buf, *dwLen);
			mem_free_dbg(buf);
			buf = (BYTE*)pTiles;
			*dwLen += 4 * 4 * 2;
		}

		// - floor tile(3) with vertical arch
		pTiles[(161 - 1) * 4 + 0] = SwapLE16(560 - 1);
		pTiles[(161 - 1) * 4 + 1] = SwapLE16(10 - 1);
		pTiles[(161 - 1) * 4 + 2] = SwapLE16(561 - 1);
		pTiles[(161 - 1) * 4 + 3] = SwapLE16(12 - 1);
		// - floor tile(3) with horizontal arch
		pTiles[(162 - 1) * 4 + 0] = SwapLE16(562 - 1);
		pTiles[(162 - 1) * 4 + 1] = SwapLE16(563 - 1);
		pTiles[(162 - 1) * 4 + 2] = SwapLE16(11 - 1);
		pTiles[(162 - 1) * 4 + 3] = SwapLE16(12 - 1);
		// - floor tile with shadow(49) with vertical arch
		pTiles[(163 - 1) * 4 + 0] = SwapLE16(564 - 1); // - 159
		pTiles[(163 - 1) * 4 + 1] = SwapLE16(160 - 1);
		pTiles[(163 - 1) * 4 + 2] = SwapLE16(565 - 1); // - 161
		pTiles[(163 - 1) * 4 + 3] = SwapLE16(162 - 1);
		// - floor tile with shadow(51) with horizontal arch
		pTiles[(164 - 1) * 4 + 0] = SwapLE16(566 - 1); // - 166
		pTiles[(164 - 1) * 4 + 1] = SwapLE16(567 - 1); // - 167
		pTiles[(164 - 1) * 4 + 2] = SwapLE16(168 - 1);
		pTiles[(164 - 1) * 4 + 3] = SwapLE16(169 - 1);
		// make the back of the stairs non-walkable I.
		pTiles[(72 - 1) * 4 + 1] = SwapLE16(56 - 1);
	} break;
	case FILE_CATACOMBS_MIN:
	{	// patch dMiniTiles - L2.MIN
		// add separate tiles and subtiles for the arches II.
		constexpr int blockSize = 10;
		uint16_t *pSubtiles = (uint16_t*)buf;
		if (*dwLen < 567 * blockSize * 2) {
			if (*dwLen != 559 * blockSize * 2) {
				mem_free_dbg(buf);
				app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
				return NULL;
			}
			pSubtiles = (uint16_t*)DiabloAllocPtr(*dwLen + 8 * blockSize * 2);
			memcpy(pSubtiles, buf, *dwLen);
			mem_free_dbg(buf);
			buf = (BYTE*)pSubtiles;
			memset(buf + *dwLen, 0, 8 * blockSize * 2);
			*dwLen += 8 * blockSize * 2;
		}

		pSubtiles[MICRO_IDX(560 - 1, blockSize, 0)] = pSubtiles[MICRO_IDX(9 - 1, blockSize, 0)];
		pSubtiles[MICRO_IDX(560 - 1, blockSize, 1)] = pSubtiles[MICRO_IDX(9 - 1, blockSize, 1)];
		pSubtiles[MICRO_IDX(561 - 1, blockSize, 0)] = pSubtiles[MICRO_IDX(11 - 1, blockSize, 0)];
		pSubtiles[MICRO_IDX(561 - 1, blockSize, 1)] = pSubtiles[MICRO_IDX(11 - 1, blockSize, 1)];
		pSubtiles[MICRO_IDX(562 - 1, blockSize, 0)] = pSubtiles[MICRO_IDX(9 - 1, blockSize, 0)];
		pSubtiles[MICRO_IDX(562 - 1, blockSize, 1)] = pSubtiles[MICRO_IDX(9 - 1, blockSize, 1)];
		pSubtiles[MICRO_IDX(563 - 1, blockSize, 0)] = pSubtiles[MICRO_IDX(10 - 1, blockSize, 0)];
		pSubtiles[MICRO_IDX(563 - 1, blockSize, 1)] = pSubtiles[MICRO_IDX(10 - 1, blockSize, 1)];
		pSubtiles[MICRO_IDX(564 - 1, blockSize, 0)] = pSubtiles[MICRO_IDX(159 - 1, blockSize, 0)];
		pSubtiles[MICRO_IDX(564 - 1, blockSize, 1)] = pSubtiles[MICRO_IDX(159 - 1, blockSize, 1)];
		pSubtiles[MICRO_IDX(565 - 1, blockSize, 0)] = pSubtiles[MICRO_IDX(161 - 1, blockSize, 0)];
		pSubtiles[MICRO_IDX(565 - 1, blockSize, 1)] = pSubtiles[MICRO_IDX(161 - 1, blockSize, 1)];
		pSubtiles[MICRO_IDX(566 - 1, blockSize, 0)] = pSubtiles[MICRO_IDX(166 - 1, blockSize, 0)];
		pSubtiles[MICRO_IDX(566 - 1, blockSize, 1)] = pSubtiles[MICRO_IDX(166 - 1, blockSize, 1)];
		pSubtiles[MICRO_IDX(567 - 1, blockSize, 0)] = pSubtiles[MICRO_IDX(167 - 1, blockSize, 0)];
		pSubtiles[MICRO_IDX(567 - 1, blockSize, 1)] = pSubtiles[MICRO_IDX(167 - 1, blockSize, 1)];
	} break;
	case FILE_CATACOMBS_SOL:
	{	// patch dSolidTable - L2.SOL
		// add separate tiles and subtiles for the arches III.
		if (*dwLen < 567) {
			if (*dwLen != 559) {
				mem_free_dbg(buf);
				app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
				return NULL;
			}
			BYTE *tmp = DiabloAllocPtr(*dwLen + 8);
			memcpy(tmp, buf, *dwLen);
			mem_free_dbg(buf);
			buf = (BYTE*)tmp;
			*dwLen += 8;
		}
		// reset flags of the 'new' floor tiles with arches
		memset(buf + 559, 0, 8);
		// make the back of the stairs non-walkable II.
		nSolidTable(252, true);
		nBlockTable(252, true);
		nMissileTable(252, true);
	} break;
	case FILE_CATACOMBS_AMP:
	{	// patch dAutomapData - L2.AMP
		if (*dwLen < 157 * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		uint16_t *automaptype = (uint16_t*)buf;
		automaptype[42 - 1] &= SwapLE16(~MAPFLAG_HORZARCH);
		automaptype[156 - 1] &= SwapLE16(~(MAPFLAG_VERTDOOR | MAPFLAG_TYPE));
		automaptype[157 - 1] &= SwapLE16(~(MAPFLAG_HORZDOOR | MAPFLAG_TYPE));
		// add separate tiles and subtiles for the arches IV.
		if (*dwLen < 164 * 2) {
			if (*dwLen != 160 * 2) {
				mem_free_dbg(buf);
				app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
				return NULL;
			}
			automaptype = (uint16_t*)DiabloAllocPtr(*dwLen + 4 * 2);
			memcpy(automaptype, buf, *dwLen);
			mem_free_dbg(buf);
			buf = (BYTE*)automaptype;
			// memset(buf + *dwLen, 0, 4 * 2);
			*dwLen += 4 * 2;
		}
		// - floor tile(3) with vertical arch
		automaptype[161 - 1] = automaptype[(3 - 1)];
		// - floor tile(3) with horizontal arch
		automaptype[162 - 1] = automaptype[(3 - 1)];
		// - floor tile with shadow(49) with vertical arch
		automaptype[163 - 1] = automaptype[(3 - 1)];
		// - floor tile with shadow(51) with horizontal arch
		automaptype[164 - 1] = automaptype[(3 - 1)];
	} break;
	case FILE_CAVES_MIN:
	{	// patch dMiniTiles - L3.MIN
#if ASSET_MPL == 1
		if (*dwLen < MICRO_IDX(82 - 1, 10, 4) * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		uint16_t *pSubtiles = (uint16_t*)buf;
		constexpr int blockSize = 10;
		// fix bad artifact
		blkMicro(82, 4);
#endif /* ASSET_MPL == 1 */
	} break;
	case FILE_CAVES_SOL:
	{	// patch dSolidTable - L3.SOL
		if (*dwLen <= 249) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		nSolidTable(249, false); // sync tile 68 and 69 by making subtile 249 of tile 68 walkable.
	} break;
	case FILE_HELL_CEL:
	{	// patch dMicroCels - L4.CEL
		size_t minLen;
		BYTE* minBuf = LoadFileInMem(filesToPatch[FILE_HELL_MIN], &minLen);
		if (minBuf == NULL) {
			mem_free_dbg(buf);
			app_warn("Unable to open file %s in the mpq.", filesToPatch[FILE_HELL_MIN]);
			return NULL;
		}
		size_t tilLen;
		BYTE* tilBuf = LoadFileInMem(filesToPatch[FILE_HELL_TIL], &tilLen);
		if (tilBuf == NULL) {
			mem_free_dbg(minBuf);
			mem_free_dbg(buf);
			app_warn("Unable to open file %s in the mpq.", filesToPatch[FILE_HELL_TIL]);
			return NULL;
		}
		buf = patchHellCel(tilBuf, tilLen, minBuf, minLen, buf, dwLen, 45);
		mem_free_dbg(minBuf);
		mem_free_dbg(tilBuf);
	} break;
	case FILE_HELL_MIN:
	{	// patch dMiniTiles - L4.MIN
		constexpr int blockSize = 16;
		uint16_t *pSubtiles = (uint16_t*)buf;
		// patch exit tile II.
		// - move the frames to the bottom right subtile
		pSubtiles[MICRO_IDX(140 - 1, blockSize, 3)] = (pSubtiles[MICRO_IDX(137 - 1, blockSize, 1)] & 0xFFF) | (MET_TRANSPARENT << 12); // 369
		// pSubtiles[MICRO_IDX(137 - 1, blockSize, 1)] = 0;

		pSubtiles[MICRO_IDX(140 - 1, blockSize, 2)] = (pSubtiles[MICRO_IDX(137 - 1, blockSize, 0)] & 0xFFF) | (MET_SQUARE << 12);      // 368
		pSubtiles[MICRO_IDX(140 - 1, blockSize, 4)] = (pSubtiles[MICRO_IDX(137 - 1, blockSize, 2)] & 0xFFF) | (MET_TRANSPARENT << 12); // 367
		// pSubtiles[MICRO_IDX(137 - 1, blockSize, 0)] = 0;
		// pSubtiles[MICRO_IDX(137 - 1, blockSize, 2)] = 0;

		// - eliminate right frame of the bottom left subtile
		pSubtiles[MICRO_IDX(139 - 1, blockSize, 1)] = 0;

		// - adjust the frame types
		pSubtiles[MICRO_IDX(140 - 1, blockSize, 0)] = (pSubtiles[MICRO_IDX(140 - 1, blockSize, 0)] & 0xFFF) | (MET_LTRAPEZOID << 12);  // 376
		pSubtiles[MICRO_IDX(140 - 1, blockSize, 1)] = (pSubtiles[MICRO_IDX(140 - 1, blockSize, 1)] & 0xFFF) | (MET_TRANSPARENT << 12); // 377
	} break;
	case FILE_HELL_TIL:
	{	// patch dMegaTiles - L4.TIL
		uint16_t *pTiles = (uint16_t*)buf;
		// patch exit tile III.
		pTiles[(45 - 1) * 4 + 0] = SwapLE16(17 - 1);
		pTiles[(45 - 1) * 4 + 1] = SwapLE16(18 - 1);
	} break;
	case FILE_HELL_SOL:
	{	// patch dSolidTable - L4.SOL
		if (*dwLen <= 211) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		nMissileTable(141, false); // fix missile-blocking tile of down-stairs.
		// nMissileTable(137, false); // fix missile-blocking tile of down-stairs. - skip to keep in sync with the nSolidTable
		// nSolidTable(137, false);   // fix non-walkable tile of down-stairs. - skip, because it causes a graphic glitch
		nSolidTable(130, true);    // make the inner tiles of the down-stairs non-walkable I.
		nSolidTable(132, true);    // make the inner tiles of the down-stairs non-walkable II.
		nSolidTable(131, true);    // make the inner tiles of the down-stairs non-walkable III.
		// fix all-blocking tile on the diablo-level
		nSolidTable(211, false);
		nMissileTable(211, false);
		nBlockTable(211, false);
	} break;
	case FILE_HELL_AMP:
	{	// patch dAutomapData - L4.AMP
		if (*dwLen < 56 * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		uint16_t *automaptype = (uint16_t*)buf;
		automaptype[52 - 1] |= SwapLE16(MAPFLAG_VERTGRATE);
		automaptype[56 - 1] |= SwapLE16(MAPFLAG_HORZGRATE);
	} break;
	case FILE_BHSM_TRN:
	{	// patch TRN for 'Blighthorn Steelmace' - BHSM.TRN
		if (*dwLen != 256) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		// assert(buf[188] == 255 || buf[188] == 0);
		buf[188] = 0;
	} break;
	case FILE_BSM_TRN:
	{	// patch TRN for 'Baron Sludge' - BSM.TRN
		if (*dwLen != 256) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		// assert(buf[241] == 255 || buf[241] == 0);
		buf[241] = 0;
	} break;
	case FILE_ACIDB_TRN:
	case FILE_ACIDBLK_TRN:
	case FILE_BLKKNTBE_TRN:
	case FILE_DARK_TRN:
	case FILE_BLUE_TRN:
	case FILE_FATB_TRN:
	case FILE_GARGB_TRN:
	case FILE_GARGBR_TRN:
	case FILE_GRAY_TRN:
	case FILE_CNSELBK_TRN:
	case FILE_CNSELG_TRN:
	case FILE_GUARD_TRN:
	case FILE_VTEXL_TRN:
	case FILE_RHINOB_TRN:
	case FILE_BLACK_TRN:
	case FILE_WHITE_TRN:
	case FILE_THINV1_TRN:
	case FILE_GREY_TRN:
	{	// patch TRN for normal monsters - ...TRN
		if (*dwLen != 256) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		for (int i = 0; i < NUM_COLORS - 1; i++) {
			if (buf[i] == 255)
				buf[i] = 0;
		}
	} break;
	case FILE_PLR_WHBAT:
	case FILE_PLR_WLBAT:
	case FILE_PLR_WMBAT:
	{	// reencode player gfx files - W*BAT.CL2
		buf = ReEncodeCL2(buf, dwLen, NUM_DIRS, 16, 128, 96);
	} break;
#ifdef HELLFIRE
	case FILE_NTOWN_MIN:
	{	// patch dMiniTiles - Town.MIN
#if ASSET_MPL == 1
		if (*dwLen < MICRO_IDX(1303 - 1, 16, 7) * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq. File len: %d vs %d", filesToPatch[index], *dwLen, MICRO_IDX(1303 - 1, 16, 7) * 2);
			return NULL;
		}
		patchTownFile(buf);
		uint16_t *pSubtiles = (uint16_t*)buf;
		constexpr int blockSize = 16;
		// fix bad artifacts
		blkMicro(1273, 7);
		blkMicro(1303, 7);
#endif // ASSET_MPL
	} break;
	case FILE_NEST_MIN:
	{	// patch dMiniTiles - L6.MIN
#if ASSET_MPL == 1
		if (*dwLen < MICRO_IDX(366 - 1, 10, 1) * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		uint16_t *pSubtiles = (uint16_t*)buf;
		constexpr int blockSize = 10;
		// useless black micros
		blkMicro(21, 0);
		blkMicro(21, 1);
		// fix bad artifacts
		blkMicro(132, 7);
		blkMicro(366, 1);
#endif /* ASSET_MPL == 1 */
	} break;
	case FILE_NEST_SOL:
	{	// patch dSolidTable - L6.SOL
		if (*dwLen <= 416) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		nSolidTable(390, false); // make a pool tile walkable I.
		nSolidTable(413, false); // make a pool tile walkable II.
		nSolidTable(416, false); // make a pool tile walkable III.
	} break;
	case FILE_CRYPT_TIL:
	{	// patch dMegaTiles - L5.TIL
		if (*dwLen < (4 * (72 - 1) + 2) * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		// use common subtiles of doors
		uint16_t *pMegaTiles = (uint16_t*)buf;
		assert(pMegaTiles[4 * (71 - 1) + 2] == SwapLE16(213 - 1) || pMegaTiles[4 * (71 - 1) + 2] ==  SwapLE16(206 - 1));
		pMegaTiles[4 * (71 - 1) + 2] = SwapLE16(206 - 1);
		assert(pMegaTiles[4 * (72 - 1) + 2] == SwapLE16(216 - 1) || pMegaTiles[4 * (72 - 1) + 2] ==  SwapLE16(206 - 1));
		pMegaTiles[4 * (72 - 1) + 2] = SwapLE16(206 - 1);
	} break;
	case FILE_CRYPT_MIN:
	{	// patch dMiniTiles - L5.MIN
#if ASSET_MPL == 1
		if (*dwLen < MICRO_IDX(197 - 1, 10, 1) * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		// pointless door micros (re-drawn by dSpecial)
		uint16_t *pSubtiles = (uint16_t*)buf;
		constexpr int blockSize = 10;
		blkMicro(77, 6);
		blkMicro(77, 8);
		blkMicro(80, 7);
		blkMicro(80, 9);
		blkMicro(206, 6);
		blkMicro(206, 8);
		blkMicro(209, 7);
		blkMicro(209, 9);
		blkMicro(213, 6);
		blkMicro(213, 8);
		blkMicro(216, 6);
		blkMicro(216, 8);
		// useless black micros
		blkMicro(130, 0);
		blkMicro(130, 1);
		blkMicro(132, 1);
		blkMicro(134, 0);
		blkMicro(134, 1);
		blkMicro(149, 0);
		blkMicro(149, 1);
		blkMicro(149, 2);
		blkMicro(150, 0);
		blkMicro(150, 1);
		blkMicro(150, 2);
		blkMicro(150, 4);
		blkMicro(151, 0);
		blkMicro(151, 1);
		blkMicro(151, 3);
		blkMicro(152, 0);
		blkMicro(152, 1);
		blkMicro(152, 3);
		blkMicro(152, 5);
		blkMicro(153, 0);
		blkMicro(153, 1);
		// fix bad artifact
		blkMicro(156, 2);
		// useless black micros
		blkMicro(172, 0);
		blkMicro(172, 1);
		blkMicro(172, 2);
		blkMicro(173, 0);
		blkMicro(173, 1);
		blkMicro(174, 0);
		blkMicro(174, 1);
		blkMicro(174, 2);
		blkMicro(174, 4);
		blkMicro(175, 0);
		blkMicro(175, 1);
		blkMicro(176, 0);
		blkMicro(176, 1);
		blkMicro(176, 3);
		blkMicro(177, 0);
		blkMicro(177, 1);
		blkMicro(177, 3);
		blkMicro(177, 5);
		blkMicro(178, 0);
		blkMicro(178, 1);
		blkMicro(179, 0);
		blkMicro(179, 1);
#endif // ASSET_MPL
	} break;
	case FILE_CRYPT_SOL:
	{ // patch dSolidTable - L5.SOL
		if (*dwLen <= 600) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		nSolidTable(143, false); // make right side of down-stairs consistent (walkable)
		nSolidTable(148, false); // make the back of down-stairs consistent (walkable)
		// make collision-checks more reasonable
		//  - prevent non-crossable floor-tile configurations I.
		nSolidTable(461, false);
		//  - set top right tile of an arch non-walkable (full of lava)
		//nSolidTable(471, true);
		//  - set top right tile of a pillar walkable (just a small obstacle)
		nSolidTable(481, false);
		//  - tile 491 is the same as tile 594 which is not solid
		//  - prevents non-crossable floor-tile configurations
		nSolidTable(491, false);
		//  - set bottom left tile of a rock non-walkable (rather large obstacle, feet of the hero does not fit)
		//  - prevents non-crossable floor-tile configurations
		nSolidTable(523, true);
		//  - set the top right tile of a floor mega walkable (similar to 594 which is not solid)
		nSolidTable(570, false);
		//  - prevent non-crossable floor-tile configurations II.
		nSolidTable(598, false);
		nSolidTable(600, false);
	} break;
	case FILE_OBJCURS_CEL:
	{
		size_t sizeB, sizeAB;
		BYTE *aCursCels, *bCursCels;
		DWORD numA, numAB;

		numA = SwapLE32(((DWORD*)buf)[0]);
		if (numA != 179) {
			if (numA != 179 + 61 - 2) {
				mem_free_dbg(buf);
				app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
				buf = NULL;
			}
			return buf;
		}
		bCursCels = LoadFileInMem("Data\\Inv\\Objcurs2.CEL", &sizeB);
		// merge the two cel files
		aCursCels = buf;
		buf = CelMerge(aCursCels, *dwLen, bCursCels, sizeB);

		*dwLen += sizeB - 4 * 2;

		mem_free_dbg(aCursCels);
		mem_free_dbg(bCursCels);

		// remove the last two entries
		numAB = SwapLE32(((DWORD*)buf)[0]) - 2;
		sizeAB = SwapLE32(((DWORD*)buf)[numAB + 1]) - 4 * 2;
		aCursCels = DiabloAllocPtr(sizeAB);
		*(DWORD*)aCursCels = SwapLE32(numAB);
		for (unsigned i = 0; i < numAB + 1; i++) {
			((DWORD*)aCursCels)[i + 1] = SwapLE32(((DWORD*)buf)[i + 1]) - 4 * 2;
		}
		memcpy(aCursCels + (numAB + 2) * 4, buf + (numAB + 2 + 2) * 4 , sizeAB - (numAB + 2) * 4);

		mem_free_dbg(buf);
		buf = aCursCels;
		*dwLen = sizeAB;
	} break;
#endif // HELLFIRE
	default:
		ASSUME_UNREACHABLE
		break;
	}
	return buf;
}

static int patcher_callback()
{
	switch (workPhase) {
	case 0:
	{	// first round - read the content and prepare the metadata
		std::string listpath = std::string(GetBasePath()) + "mpqfiles.txt";
		std::ifstream input(listpath);
		if (input.fail()) {
			app_warn("Can not find/access '%s' in the game folder.", "mpqfiles.txt");
			return RETURN_ERROR;
		}
		std::string line;
		int entryCount = lengthof(filesToPatch);
		while (std::getline(input, line)) {
			for (int i = 0; i < NUM_MPQS; i++) {
				//if (diabdat_mpqs[i] != NULL && SFileHasFile(diabdat_mpqs[i], line.c_str())) {
				if (diabdat_mpqs[i] != NULL && SFileOpenFileEx(diabdat_mpqs[i], line.c_str(), SFILE_OPEN_CHECK_EXISTS, NULL)) {
					entryCount++;
					break;
				}
			}
		}

		if (entryCount == 0) {
			// app_warn("Can not find/access '%s' in the game folder.", "mpqfiles.txt");
			return RETURN_ERROR;
		}

		// calculate the required number of hashes
		// TODO: use GetNearestPowerOfTwo of StormCommon.h?
		hashCount = 1;
		while (hashCount <= entryCount) {
			hashCount <<= 1;
		}
		workPhase++;
	} break;
	case 1:
	{	// create the mpq file
		std::string path = std::string(GetBasePath()) + "devilx.mpq.foo";
		if (!OpenMPQ(path.c_str(), hashCount, hashCount)) {
			app_warn("Unable to open MPQ file %s.", path.c_str());
			return RETURN_ERROR;
		}
		hashCount = 0;
		workPhase++;
	} break;
	case 2:
	{	// add the current content of devilx.mpq
		std::string listpath = std::string(GetBasePath()) + "mpqfiles.txt";
		std::ifstream input(listpath);
		if (input.fail()) {
			app_warn("Can not find/access '%s' in the game folder.", "mpqfiles.txt");
			return RETURN_ERROR;
		}		
		int skip = hashCount;
		std::string line;
		while (std::getline(input, line)) {
			if (--skip >= 0) {
				continue;
			}
			if (skip <= -10) {
				break;
			}
			for (int i = 0; i < NUM_MPQS; i++) {
				HANDLE hFile;
				if (diabdat_mpqs[i] != NULL && SFileOpenFileEx(diabdat_mpqs[i], line.c_str(), SFILE_OPEN_FROM_MPQ, &hFile)) {
					DWORD dwLen = SFileGetFileSize(hFile);
					BYTE* buf = DiabloAllocPtr(dwLen);
					if (!SFileReadFile(hFile, buf, dwLen)) {
						app_warn("Unable to open file %s in the mpq.", line.c_str());
						return RETURN_ERROR;
					}
					if (!mpqapi_write_entry(line.c_str(), buf, dwLen)) {
						app_warn("Unable to write %s to the MPQ.", line.c_str());
						return RETURN_ERROR;
					}
					mem_free_dbg(buf);
					SFileCloseFile(hFile);
					break;
				}
			}
			hashCount++;
		}
		input.close();
		if (skip <= -10)
			break;
		hashCount = 0;
		workPhase++;
	} break;
	case 3:
	{	// add patches
		int i = hashCount;
		for ( ; i < lengthof(filesToPatch); i++) {
			if (i >= hashCount + 10) {
				break;
			}
			size_t dwLen;
			BYTE* buf = patchFile(i, &dwLen);
			if (buf == NULL) {
				return RETURN_ERROR;
			}
			if (!mpqapi_write_entry(filesToPatch[i], buf, dwLen)) {
				app_warn("Unable to write %s to the MPQ.", filesToPatch[i]);
				return RETURN_ERROR;
			}
			mem_free_dbg(buf);
		}
		hashCount += 10;
		if (i >= hashCount)
			break;
		mpqapi_flush_and_close(true);
		workPhase++;
	} break;
	case 4:
	{	// replace the devilx.mpq with the new file
		std::string pathTo = std::string(GetBasePath()) + "devilx.mpq";
		if (diabdat_mpqs[MPQ_DEVILX] != NULL) {
			SFileCloseArchive(diabdat_mpqs[MPQ_DEVILX]);
			diabdat_mpqs[MPQ_DEVILX] = NULL;
			RemoveFile(pathTo.c_str());
		}
		std::string pathFrom = std::string(GetBasePath()) + "devilx.mpq.foo";
		RenameFile(pathFrom.c_str(), pathTo.c_str());
		if (FileExists(pathFrom.c_str())) {
			app_warn("Failed to rename %s to %s.", pathFrom.c_str(), pathTo.c_str());
			return RETURN_ERROR;
		}
		workPhase++;
	} break;
	case 5:
	{	// test the result
		std::string path = std::string(GetBasePath()) + "devilx.mpq";
		diabdat_mpqs[MPQ_DEVILX] = SFileOpenArchive(path.c_str(), MPQ_OPEN_READ_ONLY);
		if (diabdat_mpqs[MPQ_DEVILX] == NULL) {
			app_warn("Failed to create %s.", path.c_str());
			return RETURN_ERROR;
		}
	} return RETURN_DONE;
	}

	while (++workProgress >= 100)
		workProgress -= 100;
	return workProgress;
}

void UiPatcherDialog()
{
	workProgress = 0;
	workPhase = 0;
	bool result = UiProgressDialog("...Patch in progress...", patcher_callback);

	if (!result) {
		return;
	}

	if (diabdat_mpqs[NUM_MPQS] != NULL) {
		char dialogTitle[32];
		char dialogText[256];
		snprintf(dialogTitle, sizeof(dialogTitle), "");
		snprintf(dialogText, sizeof(dialogText), "The base assets are patched. The file of the merged assets needs to be recreated.");

		UiSelOkDialog(dialogTitle, dialogText);
	}
}

DEVILUTION_END_NAMESPACE
