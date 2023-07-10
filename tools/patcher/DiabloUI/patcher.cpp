#include <string>
#include <fstream>
#include <set>

#include "diabloui.h"
#include "selok.h"
#include "utils/paths.h"
#include "utils/file_util.h"
#include "engine/render/cel_render.h"
#include "engine/render/cl2_render.h"
#include "engine/render/dun_render.h"

DEVILUTION_BEGIN_NAMESPACE

static unsigned workProgress;
static unsigned workPhase;
static HANDLE mpqone;
static int hashCount;
static constexpr int RETURN_ERROR = 101;
static constexpr int RETURN_DONE = 100;
static std::set<unsigned> removeMicros;

static constexpr int BLOCK_SIZE_TOWN = 16;
static constexpr int BLOCK_SIZE_L1 = 10;
static constexpr int BLOCK_SIZE_L2 = 10;
static constexpr int BLOCK_SIZE_L3 = 10;
static constexpr int BLOCK_SIZE_L4 = 16;
static constexpr int BLOCK_SIZE_L5 = 10;
static constexpr int BLOCK_SIZE_L6 = 10;

typedef enum filenames {
#if ASSET_MPL == 1
	FILE_TOWN_CEL,
	FILE_TOWN_MIN,
	FILE_CATHEDRAL_CEL,
	FILE_CATHEDRAL_MIN,
#endif
	FILE_CATHEDRAL_TIL,
	FILE_CATHEDRAL_SOL,
	FILE_CATHEDRAL_AMP,
	FILE_BONESTR1_DUN,
	FILE_BONESTR2_DUN,
	FILE_BONECHA1_DUN,
	FILE_BONECHA2_DUN,
	FILE_BLIND1_DUN,
	FILE_BLIND2_DUN,
	FILE_BLOOD1_DUN,
	FILE_BLOOD2_DUN,
#if ASSET_MPL == 1
	FILE_L2DOORS_SCEL,
	FILE_CATACOMBS_SCEL,
	FILE_CATACOMBS_CEL,
	FILE_CATACOMBS_MIN,
#endif
	FILE_CATACOMBS_TIL,
	FILE_CATACOMBS_SOL,
	FILE_CATACOMBS_AMP,
#if ASSET_MPL == 1
	FILE_CAVES_MIN,
#endif
	FILE_CAVES_SOL,
#if ASSET_MPL == 1
	FILE_HELL_CEL,
	FILE_HELL_MIN,
#endif
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
#if ASSET_MPL == 1
	FILE_NTOWN_CEL,
	FILE_NTOWN_MIN,
#endif
#if ASSET_MPL == 1
	FILE_CRYPT_CEL,
	FILE_CRYPT_MIN,
#endif
	FILE_CRYPT_TIL,
	FILE_CRYPT_SOL,
	FILE_CRYPT_AMP,
#if ASSET_MPL == 1
	FILE_NEST_MIN,
#endif
	FILE_NEST_SOL,
	FILE_OBJCURS_CEL,
#endif
	NUM_FILENAMES
} filenames;

static const char* const filesToPatch[NUM_FILENAMES] = {
#if ASSET_MPL == 1
/*FILE_TOWN_CEL*/      "Levels\\TownData\\Town.CEL",
/*FILE_TOWN_MIN*/      "Levels\\TownData\\Town.MIN",
/*FILE_CATHEDRAL_CEL*/ "Levels\\L1Data\\L1.CEL",
/*FILE_CATHEDRAL_MIN*/ "Levels\\L1Data\\L1.MIN",
#endif
/*FILE_CATHEDRAL_TIL*/ "Levels\\L1Data\\L1.TIL",
/*FILE_CATHEDRAL_SOL*/ "Levels\\L1Data\\L1.SOL",
/*FILE_CATHEDRAL_AMP*/ "Levels\\L1Data\\L1.AMP",
/*FILE_BONESTR1_DUN*/  "Levels\\L2Data\\Bonestr1.DUN",
/*FILE_BONESTR2_DUN*/  "Levels\\L2Data\\Bonestr2.DUN",
/*FILE_BONECHA1_DUN*/  "Levels\\L2Data\\Bonecha1.DUN",
/*FILE_BONECHA2_DUN*/  "Levels\\L2Data\\Bonecha2.DUN",
/*FILE_BLIND1_DUN*/    "Levels\\L2Data\\Blind1.DUN",
/*FILE_BLIND2_DUN*/    "Levels\\L2Data\\Blind2.DUN",
/*FILE_BLOOD1_DUN*/    "Levels\\L2Data\\Blood1.DUN",
/*FILE_BLOOD2_DUN*/    "Levels\\L2Data\\Blood2.DUN",
#if ASSET_MPL == 1
/*FILE_L2DOORS_SCEL*/  "Objects\\L2Doors.CEL",
/*FILE_CATACOMBS_SCEL*/"Levels\\L2Data\\L2S.CEL",
/*FILE_CATACOMBS_CEL*/ "Levels\\L2Data\\L2.CEL",
/*FILE_CATACOMBS_MIN*/ "Levels\\L2Data\\L2.MIN",
#endif
/*FILE_CATACOMBS_TIL*/ "Levels\\L2Data\\L2.TIL",
/*FILE_CATACOMBS_SOL*/ "Levels\\L2Data\\L2.SOL",
/*FILE_CATACOMBS_AMP*/ "Levels\\L2Data\\L2.AMP",
/*FILE_CAVES_MIN*/     "Levels\\L3Data\\L3.MIN",
/*FILE_CAVES_SOL*/     "Levels\\L3Data\\L3.SOL",
#if ASSET_MPL == 1
/*FILE_HELL_CEL*/      "Levels\\L4Data\\L4.CEL",
/*FILE_HELL_MIN*/      "Levels\\L4Data\\L4.MIN",
#endif
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
#if ASSET_MPL == 1
/*FILE_NTOWN_CEL*/     "NLevels\\TownData\\Town.CEL",
/*FILE_NTOWN_MIN*/     "NLevels\\TownData\\Town.MIN",
#endif
#if ASSET_MPL == 1
/*FILE_CRYPT_CEL*/     "NLevels\\L5Data\\L5.CEL",
/*FILE_CRYPT_MIN*/     "NLevels\\L5Data\\L5.MIN",
#endif
/*FILE_CRYPT_TIL*/     "NLevels\\L5Data\\L5.TIL",
/*FILE_CRYPT_SOL*/     "NLevels\\L5Data\\L5.SOL",
/*FILE_CRYPT_AMP*/     "NLevels\\L5Data\\L5.AMP",
#if ASSET_MPL == 1
/*FILE_NEST_MIN*/      "NLevels\\L6Data\\L6.MIN",
#endif
/*FILE_NEST_SOL*/      "NLevels\\L6Data\\L6.SOL",
/*FILE_OBJCURS_CEL*/   "Data\\Inv\\Objcurs.CEL",
#endif
};

#define DESCRIPTION_WIDTH (SELGAME_LPANEL_WIDTH - 2 * 10)

#define MICRO_IDX(subtile, blockSize, microIndex) ((subtile) * (blockSize) + (blockSize) - (2 + ((microIndex) & ~1)) + ((microIndex) & 1))
// if ((currIndex == FILE_CRYPT_CEL || currIndex == FILE_CRYPT_MIN) && (pSubtiles[MICRO_IDX(subtileRef - 1, blockSize, microIndex)] & 0xFFF) == 101) { app_fatal("Ref%d, bs:%d, idx:%d", subtileRef, blockSize, microIndex, currIndex); } ; \

#define HideMcr(subtileRef, microIndex) \
{ \
	pSubtiles[MICRO_IDX(subtileRef - 1, blockSize, microIndex)] = 0; \
}
#define Blk2Mcr(subtileRef, microIndex) \
{ \
	removeMicros.insert(pSubtiles[MICRO_IDX(subtileRef - 1, blockSize, microIndex)] & 0xFFF); \
	pSubtiles[MICRO_IDX(subtileRef - 1, blockSize, microIndex)] = 0; \
}

// if ((currIndex == FILE_CRYPT_CEL || currIndex == FILE_CRYPT_MIN) && (dstValue & 0xFFF) == 270) { app_fatal("Ref%d, bs:%d, idx:%d from %d@%d", dstSubtileRef, blockSize, dstMicroIndex, srcSubtileRef, dstMicroIndex, currIndex); } ; \

#define ReplaceMcr(dstSubtileRef, dstMicroIndex, srcSubtileRef, srcMicroIndex) \
{ \
	unsigned dstMicro = MICRO_IDX(dstSubtileRef - 1, blockSize, dstMicroIndex); \
	uint16_t dstValue = pSubtiles[dstMicro]; \
	uint16_t srcValue = pSubtiles[MICRO_IDX(srcSubtileRef - 1, blockSize, srcMicroIndex)]; \
	if (dstValue != srcValue) { \
		removeMicros.insert(dstValue & 0xFFF); \
		pSubtiles[dstMicro] = srcValue; \
	} \
}

#define SetMcr(dstSubtileRef, dstMicroIndex, srcSubtileRef, srcMicroIndex) \
{ \
	unsigned dstMicro = MICRO_IDX(dstSubtileRef - 1, blockSize, dstMicroIndex); \
	uint16_t srcValue = pSubtiles[MICRO_IDX(srcSubtileRef - 1, blockSize, srcMicroIndex)]; \
	pSubtiles[dstMicro] = srcValue; \
}

#define MoveMcr(dstSubtileRef, dstMicroIndex, srcSubtileRef, srcMicroIndex) \
{ \
	unsigned dstMicro = MICRO_IDX(dstSubtileRef - 1, blockSize, dstMicroIndex); \
	unsigned srcMicro = MICRO_IDX(srcSubtileRef - 1, blockSize, srcMicroIndex); \
	pSubtiles[dstMicro] = pSubtiles[srcMicro]; \
	pSubtiles[srcMicro] = 0; \
}

#define SetFrameType(srcSubtileRef, microIndex, frameType) \
pSubtiles[MICRO_IDX(srcSubtileRef - 1, blockSize, microIndex)] = SwapLE16((SwapLE16(pSubtiles[MICRO_IDX(srcSubtileRef - 1, blockSize, microIndex)]) & 0xFFF) | (frameType << 12));

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

static BYTE* buildBlkCel(BYTE* celBuf, size_t *celLen)
{
	removeMicros.erase(0);

	if (removeMicros.empty()) {
		return celBuf;
	}
	// create the new CEL file
	BYTE* resCelBuf = DiabloAllocPtr(*celLen);

	DWORD* srcHeaderCursor = (DWORD*)celBuf;
	DWORD srcCelEntries = SwapLE32(srcHeaderCursor[0]);
	srcHeaderCursor++;
	DWORD* dstHeaderCursor = (DWORD*)resCelBuf;
	DWORD dstCelEntries = srcCelEntries - removeMicros.size();
	dstHeaderCursor[0] = SwapLE32(dstCelEntries);
	dstHeaderCursor++;
	BYTE* dstDataCursor = resCelBuf + 4 * (dstCelEntries + 2);
	while (!removeMicros.empty()) {
		// select the next frame
		unsigned nextRef = *removeMicros.begin();
		removeMicros.erase(nextRef);

		// copy entries till the next frame
		int numEntries = nextRef - ((size_t)srcHeaderCursor - (size_t)celBuf) / 4;
		for (int i = 0; i < numEntries; i++) {
			dstHeaderCursor[0] = SwapLE32((size_t)dstDataCursor - (size_t)resCelBuf);
			dstHeaderCursor++;
			DWORD len = srcHeaderCursor[1] - srcHeaderCursor[0];
			memcpy(dstDataCursor, celBuf + srcHeaderCursor[0], len);
			dstDataCursor += len;
			srcHeaderCursor++;
		}

		// skip the original frame
		srcHeaderCursor++;
	}
	// add remaining entries
	int numEntries = srcCelEntries + 1 - ((size_t)srcHeaderCursor - (size_t)celBuf) / 4;
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

static BYTE* buildBlkMin(BYTE* minBuf, size_t *minLen, unsigned blockSize)
{
	removeMicros.erase(0);

	if (removeMicros.empty()) {
		return minBuf;
	}

	uint16_t* pSubtiles = (uint16_t*)minBuf;

	for (unsigned i = 0; i < *minLen / 2; i++) {
		if (pSubtiles[i] == 0) {
			continue;
		}
		unsigned frameRef = SwapLE16(pSubtiles[i]) & 0xFFF;
		unsigned newFrameRef = frameRef;
		for (unsigned removedRef : removeMicros) {
			if (removedRef > frameRef)
				continue;
			if (removedRef == frameRef) {
				removeMicros.clear();
				mem_free_dbg(minBuf);
				app_warn("Frame %d is removed, but it is still used by subtile (%d).", frameRef, (i / blockSize) + 1);
				return NULL;
			}
			newFrameRef--;
		}
		pSubtiles[i] = SwapLE16((SwapLE16(pSubtiles[i]) & ~0xFFF) | newFrameRef);
	}
	removeMicros.clear();
	return minBuf;
}

typedef struct {
	int type;
	unsigned frameRef;
	int encoding;
	BYTE *frameSrc;
} CelFrameEntry;
static int encodeCelMicros(CelFrameEntry* entries, int numEntries, BYTE* resCelBuf, const BYTE* celBuf, BYTE TRANS_COLOR)
{
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
		for (int i = 0; i < numEntries; i++) {
			if (entries[i].frameRef != 0 && (next == -1 || entries[i].frameRef < entries[next].frameRef)) {
				next = i;
			}
		}
		if (next == -1)
			break;
		// copy entries till the next frame
		int midEntries = entries[next].frameRef - ((size_t)srcHeaderCursor - (size_t)celBuf) / 4;
		for (int i = 0; i < midEntries; i++) {
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

		BYTE* frameSrc = entries[next].frameSrc;
		int encoding = entries[next].encoding;
		dstDataCursor = EncodeMicro(encoding, dstDataCursor, frameSrc, TRANS_COLOR);

		// skip the original frame
		srcHeaderCursor++;

		// remove entry
		entries[next].frameRef = 0;
	}
	// add remaining entries
	int remEntries = celEntries + 1 - ((size_t)srcHeaderCursor - (size_t)celBuf) / 4;
	for (int i = 0; i < remEntries; i++) {
		dstHeaderCursor[0] = SwapLE32((size_t)dstDataCursor - (size_t)resCelBuf);
		dstHeaderCursor++;
		DWORD len = srcHeaderCursor[1] - srcHeaderCursor[0];
		memcpy(dstDataCursor, celBuf + srcHeaderCursor[0], len);
		dstDataCursor += len;
		srcHeaderCursor++;
	}
	// add file-size
	dstHeaderCursor[0] = SwapLE32((size_t)dstDataCursor - (size_t)resCelBuf);

	return SwapLE32(dstHeaderCursor[0]);
}

static BYTE* EncodeFrame(BYTE* pBuf, int width, int height, int subHeaderSize, BYTE transparentPixel)
{
	// add optional {CEL FRAME HEADER}
	BYTE *pHeader = pBuf;
	if (subHeaderSize != 0) {
		*(WORD*)&pBuf[0] = SwapLE16(subHeaderSize);
		memset(pBuf + 2, 0, subHeaderSize - 2);
		pBuf += subHeaderSize;
	}
	// write the pixels
	BYTE *pHead;
	for (int i = 1; i <= height; i++) {
		pHead = pBuf;
		pBuf++;
		bool alpha = false;
		BYTE* data = &gpBuffer[(height - i) * BUFFER_WIDTH];
		if (/*subHeaderSize != 0 &&*/ (i % CEL_BLOCK_HEIGHT) == 1 && (i / CEL_BLOCK_HEIGHT) * 2 < subHeaderSize) {
			*(WORD*)(&pHeader[(i / CEL_BLOCK_HEIGHT) * 2]) = SwapLE16(pHead - pHeader);//pHead - buf - SUB_HEADER_SIZE;
		}
		for (int j = 0; j < width; j++) {
			if (data[j] != transparentPixel) {
				// add opaque pixel
				if (alpha || *pHead > 126) {
					pHead = pBuf;
					pBuf++;
				}
				++*pHead;
				*pBuf = data[j];
				pBuf++;
				alpha = false;
			} else {
				// add transparent pixel
				if (j != 0 && (!alpha || (char)*pHead == -128)) {
					pHead = pBuf;
					pBuf++;
				}
				--*pHead;
				alpha = true;
			}
		}
	}

	return pBuf;
}

static void patchDungeon(int fileIndex, BYTE* fileBuf, size_t* fileSize)
{
	uint16_t* lm = (uint16_t*)fileBuf;

	// TODO: validate file-size
	switch (fileIndex) {
	case FILE_BONESTR1_DUN:
	{	// patch premap - Bonestr1.DUN
		// useless tiles
		lm[2 + 0 + 0 * 7] = 0;
		lm[2 + 0 + 4 * 7] = 0;
		lm[2 + 0 + 5 * 7] = 0;
		lm[2 + 0 + 6 * 7] = 0;
		lm[2 + 6 + 6 * 7] = 0;
		lm[2 + 6 + 0 * 7] = 0;
		lm[2 + 2 + 3 * 7] = 0;
		lm[2 + 3 + 3 * 7] = 0;
		// + eliminate obsolete stair-tile
		lm[2 + 2 + 4 * 7] = 0;
		// shadow of the external-left column
		lm[2 + 0 + 4 * 7] = SwapLE16(48);
		lm[2 + 0 + 5 * 7] = SwapLE16(50);
		// protect inner tiles from spawning additional monsters/objects
		for (int y = 1; y < 6; y++) {
			for (int x = 1; x < 6; x++) {
				lm[2 + 7 * 7 + x + y * 7] = SwapLE16((3 << 8) | (3 << 10) | (3 << 12) | (3 << 14));
			}
		}
	} break;
	case FILE_BONESTR2_DUN:
	{	// patch the map - Bonestr2.DUN
		// useless tiles
		lm[2 + 0 + 0 * 7] = 0;
		lm[2 + 0 + 6 * 7] = 0;
		lm[2 + 6 + 6 * 7] = 0;
		lm[2 + 6 + 0 * 7] = 0;
		// add tiles with subtiles for arches
		lm[2 + 2 + 1 * 7] = SwapLE16(45);
		lm[2 + 4 + 1 * 7] = SwapLE16(45);
		lm[2 + 2 + 5 * 7] = SwapLE16(45);
		lm[2 + 4 + 5 * 7] = SwapLE16(45);
		lm[2 + 1 + 2 * 7] = SwapLE16(44);
		lm[2 + 1 + 4 * 7] = SwapLE16(44);
		lm[2 + 5 + 2 * 7] = SwapLE16(44);
		lm[2 + 5 + 4 * 7] = SwapLE16(44);
		// - remove tile to leave space for shadow
		lm[2 + 2 + 4 * 7] = 0;
		// protect the main structure
		for (int y = 1; y < 6; y++) {
			for (int x = 1; x < 6; x++) {
				lm[2 + 7 * 7 + x + y * 7] = SwapLE16(3);
			}
		}
		// remove monsters, objects, items
		*fileSize = (2 + 7 * 7 + 7 * 7 * 2 * 2) * 2;
	} break;
	case FILE_BONECHA1_DUN:
	{	// patch premap - Bonecha1.DUN
		// external tiles
		lm[2 + 20 +  4 * 32] = 12;
		lm[2 + 21 +  4 * 32] = 12;
		// useless tiles
		for (int y = 0; y < 18; y++) {
			for (int x = 0; x < 32; x++) {
				if (x >= 13 && x <= 21 && y >= 1 && y <= 4) {
					continue;
				}
				if (x == 18 && y == 5) {
					continue;
				}
				if (x == 14 && y == 5) {
					continue;
				}
				lm[2 + x + y * 32] = 0;
			}
		}
		// remove rooms
		*fileSize = (2 + 32 * 18 + 32 * 18 * 2 * 2 + 32 * 18 * 2 * 2 + 32 * 18 * 2 * 2) * 2;
	} break;
	case FILE_BONECHA2_DUN:
	{	// patch the map - Bonecha2.DUN
		// reduce pointless bone-chamber complexity
		lm[2 + 16 + 9 * 32] = SwapLE16(57);
		lm[2 + 16 + 10 * 32] = SwapLE16(62);
		lm[2 + 16 + 11 * 32] = SwapLE16(62);
		lm[2 + 16 + 12 * 32] = SwapLE16(62);
		lm[2 + 13 + 12 * 32] = SwapLE16(53);
		lm[2 + 14 + 12 * 32] = SwapLE16(62);
		lm[2 + 15 + 12 * 32] = SwapLE16(62);
		// external tiles
		lm[2 + 2 + 15 * 32] = SwapLE16(11);
		lm[2 + 3 + 15 * 32] = SwapLE16(11);
		lm[2 + 4 + 15 * 32] = SwapLE16(11);
		lm[2 + 5 + 15 * 32] = SwapLE16(11);
		lm[2 + 6 + 15 * 32] = SwapLE16(11);
		lm[2 + 7 + 15 * 32] = SwapLE16(11);
		lm[2 + 8 + 15 * 32] = SwapLE16(11);

		lm[2 + 10 + 17 * 32] = SwapLE16(11);
		lm[2 + 11 + 17 * 32] = SwapLE16(11);
		lm[2 + 12 + 17 * 32] = SwapLE16(11);
		lm[2 + 13 + 17 * 32] = SwapLE16(15);
		lm[2 + 14 + 17 * 32] = SwapLE16(11);
		lm[2 + 15 + 17 * 32] = SwapLE16(11);
		lm[2 + 16 + 17 * 32] = SwapLE16(11);
		lm[2 + 17 + 17 * 32] = SwapLE16(15);
		lm[2 + 18 + 17 * 32] = SwapLE16(11);
		lm[2 + 19 + 17 * 32] = SwapLE16(11);
		lm[2 + 20 + 17 * 32] = SwapLE16(11);
		lm[2 + 21 + 17 * 32] = SwapLE16(16);
		lm[2 + 21 + 16 * 32] = SwapLE16(10);
		lm[2 + 21 + 15 * 32] = SwapLE16(10);
		lm[2 + 21 + 14 * 32] = SwapLE16(10);

		lm[2 + 20 + 0 * 32] = SwapLE16(12);
		lm[2 + 21 + 0 * 32] = SwapLE16(12);
		lm[2 + 21 + 1 * 32] = SwapLE16(14);
		lm[2 + 21 + 2 * 32] = SwapLE16(10);
		lm[2 + 21 + 3 * 32] = SwapLE16(10);
		lm[2 + 21 + 4 * 32] = SwapLE16(10);
		lm[2 + 21 + 5 * 32] = SwapLE16(14);
		lm[2 + 21 + 6 * 32] = SwapLE16(10);
		lm[2 + 21 + 7 * 32] = SwapLE16(10);
		lm[2 + 21 + 8 * 32] = SwapLE16(10);

		lm[2 + 31 + 8 * 32] = SwapLE16(10);
		lm[2 + 31 + 9 * 32] = SwapLE16(10);
		lm[2 + 31 + 10 * 32] = SwapLE16(10);
		lm[2 + 31 + 11 * 32] = SwapLE16(10);
		lm[2 + 31 + 12 * 32] = SwapLE16(10);
		lm[2 + 31 + 13 * 32] = SwapLE16(10);
		lm[2 + 31 + 14 * 32] = SwapLE16(10);
		lm[2 + 31 + 15 * 32] = SwapLE16(16);
		lm[2 + 24 + 15 * 32] = SwapLE16(11);
		lm[2 + 25 + 15 * 32] = SwapLE16(11);
		lm[2 + 26 + 15 * 32] = SwapLE16(11);
		lm[2 + 27 + 15 * 32] = SwapLE16(11);
		lm[2 + 28 + 15 * 32] = SwapLE16(11);
		lm[2 + 29 + 15 * 32] = SwapLE16(11);
		lm[2 + 30 + 15 * 32] = SwapLE16(11);

		lm[2 + 21 + 13 * 32] = SwapLE16(13);
		lm[2 + 22 + 13 * 32] = SwapLE16(11);

		lm[2 + 8 + 15 * 32] = SwapLE16(11);
		lm[2 + 8 + 16 * 32] = SwapLE16(12);
		lm[2 + 8 + 17 * 32] = SwapLE16(12);
		lm[2 + 9 + 17 * 32] = SwapLE16(15);

		// add tiles with subtiles for arches
		lm[2 + 13 + 6 * 32] = SwapLE16(44);
		lm[2 + 13 + 8 * 32] = SwapLE16(44);
		lm[2 + 17 + 6 * 32] = SwapLE16(44);
		lm[2 + 17 + 8 * 32] = SwapLE16(96);

		lm[2 + 13 + 14 * 32] = SwapLE16(44);
		lm[2 + 13 + 16 * 32] = SwapLE16(44);
		lm[2 + 17 + 14 * 32] = SwapLE16(44);
		lm[2 + 17 + 16 * 32] = SwapLE16(44);

		lm[2 + 18 + 9 * 32] = SwapLE16(45);
		lm[2 + 20 + 9 * 32] = SwapLE16(45);
		lm[2 + 18 + 13 * 32] = SwapLE16(45);
		lm[2 + 20 + 13 * 32] = SwapLE16(45);

		// place pieces with closed doors
		lm[2 + 17 + 11 * 32] = SwapLE16(150);
		// place shadows
		// - right corridor
		lm[2 + 12 + 6 * 32] = SwapLE16(47);
		lm[2 + 12 + 7 * 32] = SwapLE16(51);
		lm[2 + 16 + 6 * 32] = SwapLE16(47);
		lm[2 + 16 + 7 * 32] = SwapLE16(51);
		lm[2 + 16 + 8 * 32] = SwapLE16(47);
		// - central room (top)
		// lm[2 + 17 + 8 * 32] = SwapLE16(96);
		lm[2 + 18 + 8 * 32] = SwapLE16(49);
		lm[2 + 19 + 8 * 32] = SwapLE16(49);
		lm[2 + 20 + 8 * 32] = SwapLE16(49);
		// - central room (bottom)
		lm[2 + 18 + 12 * 32] = SwapLE16(46);
		// lm[2 + 19 + 12 * 32] = SwapLE16(49); -- ugly with the candle
		// - left corridor
		lm[2 + 12 + 14 * 32] = SwapLE16(47);
		lm[2 + 12 + 15 * 32] = SwapLE16(51);
		lm[2 + 16 + 14 * 32] = SwapLE16(47);
		lm[2 + 16 + 15 * 32] = SwapLE16(51);
		// remove monsters, objects, items
		*fileSize = (2 + 32 * 18 + 32 * 18 * 2 * 2) * 2;
	} break;
	case FILE_BLIND1_DUN:
	{	// patch the map - Blind1.DUN
		// place pieces with closed doors
		lm[2 + 4 + 3 * 11] = SwapLE16(150);
		lm[2 + 6 + 7 * 11] = SwapLE16(150);
		// remove obsolete 'protection' (item)
		// lm[2 + 11 * 11 + 5 + 10 * 11] = 0;
		// protect the main structure
		for (int y = 0; y < 7; y++) {
			for (int x = 0; x < 7; x++) {
				lm[2 + 11 * 11 + x + y * 11] = SwapLE16(3);
			}
		}
		for (int y = 4; y < 11; y++) {
			for (int x = 4; x < 11; x++) {
				lm[2 + 11 * 11 + x + y * 11] = SwapLE16(3);
			}
		}
		// remove monsters, objects, items
		*fileSize = (2 + 11 * 11 + 11 * 11 * 2 * 2) * 2;
	} break;
	case FILE_BLIND2_DUN:
	{	// patch premap - Blind2.DUN
		// external tiles
		lm[2 + 2 + 2 * 11] = SwapLE16(13);
		lm[2 + 2 + 3 * 11] = SwapLE16(10);
		lm[2 + 3 + 2 * 11] = SwapLE16(11);
		lm[2 + 3 + 3 * 11] = SwapLE16(12);

		lm[2 + 6 + 6 * 11] = SwapLE16(13);
		lm[2 + 6 + 7 * 11] = SwapLE16(10);
		lm[2 + 7 + 6 * 11] = SwapLE16(11);
		lm[2 + 7 + 7 * 11] = SwapLE16(12);
		// useless tiles
		for (int y = 0; y < 11; y++) {
			for (int x = 0; x < 11; x++) {
				// keep the boxes
				if (x >= 2 && y >= 2 && x < 4 && y < 4) {
					continue;
				}
				if (x >= 6 && y >= 6 && x < 8 && y < 8) {
					continue;
				}
				// keep the doors
				if (x == 0 && y == 1/* || x == 4 && y == 3*/ || x == 10 && y == 8) {
					continue;
				}
				lm[2 + x + y * 11] = 0;
			}
		}
		// replace the door with wall
		lm[2 + 4 + 3 * 11] = SwapLE16(25);
		// remove obsolete 'protection' (item)
		// lm[2 + 11 * 11 + 5 + 10 * 11] = 0;
		// protect inner tiles from spawning additional monsters/objects
		for (int y = 0; y < 6; y++) {
			for (int x = 0; x < 6; x++) {
				lm[2 + 11 * 11 + x + y * 11] = SwapLE16((3 << 8) | (3 << 10) | (3 << 12) | (3 << 14));
			}
		}
		for (int y = 4; y < 11; y++) {
			for (int x = 4; x < 11; x++) {
				lm[2 + 11 * 11 + x + y * 11] = SwapLE16((3 << 8) | (3 << 10) | (3 << 12) | (3 << 14));
			}
		}
		// remove rooms
		*fileSize = (2 + 11 * 11 + 11 * 11 * 2 * 2 + 11 * 11 * 2 * 2 + 11 * 11 * 2 * 2) * 2;
	} break;
	case FILE_BLOOD1_DUN:
	{	// patch the map - Blood1.DUN
		// eliminate invisible 'fancy' tile to leave space for shadow
		lm[2 + 3 + 9 * 10] = 0;
		// - place pieces with closed doors
		lm[2 + 4 + 10 * 10] = SwapLE16(151);
		lm[2 + 4 + 15 * 10] = SwapLE16(151);
		lm[2 + 5 + 15 * 10] = SwapLE16(151);
		// protect the main structure
		for (int y = 0; y <= 15; y++) {
			for (int x = 2; x <= 7; x++) {
				lm[2 + 10 * 16 + x + y * 10] = SwapLE16(3);
			}
		}
		for (int y = 3; y <= 8; y++) {
			for (int x = 0; x <= 9; x++) {
				lm[2 + 10 * 16 + x + y * 10] = SwapLE16(3);
			}
		}
		// remove monsters, objects, items
		*fileSize = (2 + 10 * 16 + 10 * 16 * 2 * 2) * 2;
	} break;
	case FILE_BLOOD2_DUN:
	{	// patch premap - Blood2.DUN
		// external tiles
		for (int y = 0; y < 8; y++) {
			for (int x = 0; x < 10; x++) {
				uint16_t wv = SwapLE16(lm[2 + x + y * 10]);
				if (wv >= 143 && wv <= 149) {
					lm[2 + x + y * 10] = SwapLE16(wv - 133);
				}
			}
		}
		// useless tiles
		for (int y = 8; y < 16; y++) {
			for (int x = 0; x < 10; x++) {
				lm[2 + x + y * 10] = 0;
			}
		}
		// - place pieces with closed doors
		// lm[2 + 4 + 10 * 10] = SwapLE16(151);
		// lm[2 + 4 + 15 * 10] = SwapLE16(151);
		// lm[2 + 5 + 15 * 10] = SwapLE16(151);
		// shadow of the external-left column -- do not place to prevent overwriting large decorations
		//dungeon[pSetPieces[0]._spx - 1][pSetPieces[0]._spy + 7] = 48;
		//dungeon[pSetPieces[0]._spx - 1][pSetPieces[0]._spy + 8] = 50;
		// - shadow of the bottom-left column(s) -- one is missing
		// lm[2 + 1 + 13 * 10] = SwapLE16(48);
		// lm[2 + 1 + 14 * 10] = SwapLE16(50);
		// - shadow of the internal column next to the pedistal
		lm[2 + 5 + 7 * 10] = SwapLE16(142);
		lm[2 + 5 + 8 * 10] = SwapLE16(50);
		// remove 'items'
		lm[2 + 10 * 16 + 9 + 2 * 10 * 2] = 0;
		// adjust objects
		// - add book and pedistal
		lm[2 + 10 * 16 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2 + 9 + 24 * 10 * 2] = SwapLE16(15);
		lm[2 + 10 * 16 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2 + 9 + 16 * 10 * 2] = SwapLE16(91);
		// - remove torches
		lm[2 + 10 * 16 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2 + 11 + 8 * 10 * 2] = 0;
		lm[2 + 10 * 16 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2 + 11 + 10 * 10 * 2] = 0;
		lm[2 + 10 * 16 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2 + 11 + 12 * 10 * 2] = 0;
		lm[2 + 10 * 16 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2 + 6 + 8 * 10 * 2] = 0;
		lm[2 + 10 * 16 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2 + 6 + 10 * 10 * 2] = 0;
		lm[2 + 10 * 16 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2 + 6 + 12 * 10 * 2] = 0;
		// protect inner tiles from spawning additional monsters/objects
		for (int y = 7; y < 15; y++) {
			for (int x = 2; x <= 6; x++) {
				lm[2 + 10 * 16 + x + y * 10] = SwapLE16((3 << 8) | (3 << 10) | (3 << 12) | (3 << 14));
			}
		}
		// remove rooms
		*fileSize = (2 + 10 * 16 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2) * 2;
	} break;
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
			entries[idx].type = idx;
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
			entries[idx].type = idx;
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
		{ 731 - 1, 9, MET_TRANSPARENT }, // move micro
		{ 755 - 1, 0, MET_LTRIANGLE },   // change type
		{ 974 - 1, 0, MET_LTRIANGLE },   // change type
		{ 1030 - 1, 0, MET_LTRIANGLE },  // change type
		{ 220 - 1, 1, MET_TRANSPARENT }, // move micro
		{ 221 - 1, 0, MET_TRANSPARENT },
		{ 962 - 1, 1, MET_TRANSPARENT },
		{ 218 - 1, 0, MET_TRANSPARENT }, // move micro
		{ 219 - 1, 1, MET_TRANSPARENT },
		{ 1166 - 1, 0, MET_TRANSPARENT }, // move micro
		{ 1167 - 1, 1, MET_TRANSPARENT },
		{ 1171 - 1, 1, MET_TRANSPARENT },
		{ 1172 - 1, 0, MET_TRANSPARENT },
		{ 1175 - 1, 1, MET_TRANSPARENT },
		{ 1176 - 1, 0, MET_TRANSPARENT },
		{ 845 - 1, 4, MET_TRANSPARENT },
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
		unsigned addr = 12 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (28 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = TRANS_COLOR;
		unsigned addr2 = 19 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (27 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr2] = TRANS_COLOR;
	}
	{ //  1167[1]
		int i = 10;
		unsigned addr = 17 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 5 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = gpBuffer[addr - 1 + BUFFER_WIDTH];
		gpBuffer[addr + 1] = gpBuffer[addr - 1];
		unsigned addr2 = 8 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 4 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr2] = gpBuffer[addr2 - 3 - 2 * BUFFER_WIDTH];
		gpBuffer[addr2 - 1 + 0 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr2 - 1 + 1 * BUFFER_WIDTH] = gpBuffer[addr2 + 1 * BUFFER_WIDTH];
		gpBuffer[addr2 - 3 - 3 * BUFFER_WIDTH] = TRANS_COLOR;
	}

	// create the new CEL file
	BYTE* resCelBuf = DiabloAllocPtr(*celLen + lengthof(micros) * MICRO_WIDTH * MICRO_HEIGHT);

	CelFrameEntry entries[lengthof(micros)];
	xx = 0, yy = MICRO_HEIGHT - 1;
	int idx = 0;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		// if (micro.res_encoding >= 0) {
			entries[idx].type = idx;
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
/*  0 */{ 724 - 1, 0, -1 }, // move micro
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
/* 68 */{ 551 - 1, 1, MET_RTRAPEZOID },

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
			// TODO: report error + additional checks
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

	// create the new CEL file
	constexpr int newEntries = lengthof(micros);
	BYTE* resCelBuf = DiabloAllocPtr(*celLen + newEntries * MICRO_WIDTH * MICRO_HEIGHT);

	CelFrameEntry entries[newEntries];
	xx = 0, yy = MICRO_HEIGHT - 1;
	int idx = 0;
	for (int i = 0; i < newEntries; i++) {
		const CelMicro &micro = micros[i];
		if (micro.res_encoding >= 0) {
			entries[idx].type = idx;
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
			entries[idx].type = idx;
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

static void patchTownMin(BYTE* buf, bool isHellfireTown)
{
	uint16_t *pSubtiles = (uint16_t*)buf;
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
	}
	// use micros created by patchTownDoorCel
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
		SetFrameType(551, 1, MET_RTRAPEZOID);
		MoveMcr(551, 5, 510, 9);

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
	Blk2Mcr(1258, 0);
	Blk2Mcr(1258, 1);
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
	Blk2Mcr(1218, 3);
	Blk2Mcr(1218, 5);
	Blk2Mcr(1239, 1);
	Blk2Mcr(1254, 0);
	int unusedSubtiles[] = {
		40, 43, 49, 50, 51, 52, 66, 67, 69, 70, 71, 72, 73, 74, 75, 76, 77, 79, 80, 81, 83, 85, 86, 89, 90, 91, 93, 94, 95, 97, 99, 100, 101, 102, 103, 122, 123, 124, 136, 137, 140, 141, 142, 145, 147, 150, 151, 155, 161, 163, 164, 166, 167, 171, 176, 179, 183, 190, 191, 193, 194, 195, 196, 197, 199, 204, 205, 206, 208, 209, 228, 230, 236, 238, 241, 242, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 256, 278, 280, 291, 298, 299, 304, 305, 314, 316, 318, 320, 321, 328, 329, 335, 336, 337, 342, 350, 351, 352, 353, 354, 355, 356, 357, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 380, 392, 411, 413, 415, 417, 442, 444, 446, 447, 448, 449, 450, 451, 452, 453, 455, 456, 457, 460, 461, 462, 464, 467, 490, 491, 492, 497, 499, 500, 505, 506, 508, 534, 536, 544, 546, 548, 549, 558, 560, 565, 566, 567, 568, 570, 572, 573, 574, 575, 576, 577, 578, 579, 580, 581, 582, 583, 584, 585, 589, 591, 594, 595, 597, 598, 599, 600, 602, 609, 615, 622, 625, 648, 650, 654, 662, 664, 666, 667, 679, 680, 681, 682, 688, 690, 691, 693, 695, 696, 698, 699, 700, 701, 702, 703, 705, 730, 735, 737, 741, 742, 747, 748, 749, 750, 751, 752, 753, 756, 758, 760, 765, 766, 769, 790, 792, 796, 798, 800, 801, 802, 804, 851, 857, 859, 860, 861, 863, 865, 876, 877, 878, 879, 880, 881, 882, 883, 884, 885, 887, 888, 889, 890, 891, 893, 894, 895, 896, 897, 901, 903, 937, 960, 961, 964, 965, 967, 968, 969, 972, 973, 976, 977, 979, 980, 981, 984, 985, 988, 989, 991, 992, 993, 996, 997, 1000, 1001, 1003, 1004, 1005, 1008, 1009, 1012, 1013, 1016, 1017, 1019, 1020, 1021, 1022, 1024, 1029, 1032, 1033, 1035, 1036, 1037, 1039, 1040, 1041, 1044, 1045, 1047, 1048, 1049, 1050, 1051, 1064, 1066, 1067, 1068, 1069, 1070, 1071, 1072, 1073, 1075, 1076, 1077, 1078, 1079, 1080, 1081, 1084, 1085, 1088, 1092, 1093, 1100, 1101, 1102, 1103, 1104, 1105, 1106, 1107, 1108, 1109, 1110, 1111, 1112, 1113, 1114, 1115, 1116, 1117, 1118, 1121, 1123, 1135, 1136, 1137, 1138, 1140, 1141, 1142, 1143, 1144, 1145, 1146, 1147, 1148, 1149, 1150, 1151, 1153, 1154, 1155, 1156, 1157, 1158, 1159, 1161, 1163, 1165, 1169, 1170, 1184, 1186, 1189, 1190, 1193, 1194, 1198, 1199, 1200, 1201, 1202, 1221, 1222, 1223, 1224, 1225, 1226, 1227, 1228, 1229, 1230, 1231, 1232, 1233, 1234, 1235, 1236, 1237, 1256
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
		int unusedSubtilesHellfire[] = {
			1260, 1268, 1274, 1283, 1284, 1291, 1292, 1293, 1322, 1341, 1342, 1343, 1344, 1345, 1346, 1347, 1348, 1349, 1350, 1351, 1352, 1353, 1354, 1355, 1356, 1357, 1358, 1359, 1361, 1362, 1363, 1364, 1365, 1366, 1367, 1368, 1369, 1371, 1372, 1373, 1374, 1375, 1377, 1378, 1379
		};
		for (int n = 0; n < lengthof(unusedSubtilesHellfire); n++) {
			for (int i = 0; i < blockSize; i++) {
				Blk2Mcr(unusedSubtilesHellfire[n], i);
			}
		}
	}
}

static BYTE* patchCathedralFloorCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	typedef struct {
		int subtileIndex;
		unsigned microIndex;
		int res_encoding;
	} CelMicro;
	const CelMicro micros[] = {
/*  0 */{ 137 - 1, 5, MET_SQUARE },     // change type
		// { 250 - 1, 0, MET_LTRAPEZOID }, // change type
/*  1 */{ 286 - 1, 1, MET_RTRIANGLE },  // change type
/*  2 */{ 408 - 1, 0, MET_LTRIANGLE },  // change type
/*  3 */{ 248 - 1, 0, MET_LTRIANGLE },  // change type

/*  4 */{ 392 - 1, 0, MET_TRANSPARENT }, // mask door
/*  5 */{ 392 - 1, 2, MET_TRANSPARENT },
/*  6 */{ 394 - 1, 1, MET_TRANSPARENT },
/*  7 */{ 394 - 1, 3, MET_TRANSPARENT },

/*  8 */{ 108 - 1, 1, -1 },
/*  9 */{ 106 - 1, 0, MET_TRANSPARENT },
/* 10 */{ 109 - 1, 0, MET_TRANSPARENT },
/* 11 */{ 106 - 1, 1, MET_TRANSPARENT },

/* 12 */{ 178 - 1, 2, MET_TRANSPARENT },
/* 13 */{ 450 - 1, 1, MET_RTRAPEZOID }, // unused

/* 14 */{ 2 - 1, 1, -1 },
/* 15 */{ 276 - 1, 1, MET_RTRIANGLE },

/* 16 */{ 407 - 1, 0, MET_TRANSPARENT }, // mask door

/* 17 */{ 156 - 1, 0, MET_LTRIANGLE },
/* 18 */{ 160 - 1, 1, MET_RTRIANGLE },

/* 19 */{ 152 - 1, 1, MET_TRANSPARENT },
/* 20 */{ 159 - 1, 0, MET_LTRIANGLE },

/* 21 */{ 163 - 1, 1, MET_RTRIANGLE },

/* 22 */{ 137 - 1, 0, MET_TRANSPARENT },

/* 23 */{ 171 - 1, 3, MET_TRANSPARENT }, // unused
/* 24 */{ 171 - 1, 1, MET_RTRIANGLE },

/* 25 */{ 153 - 1, 0, MET_LTRIANGLE },
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
	// copy 2[1] to 276[1]
	for (int i = 14; i < 15; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR) {
					addr = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 276[1]
					BYTE destColor = gpBuffer[addr];
					if (destColor < 124 && destColor > 21 && (destColor < 44 || destColor > 110 || destColor == 101 || destColor == 107)) {
						gpBuffer[addr] = color;
					}
				}
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
	BYTE* resCelBuf = DiabloAllocPtr(*celLen + lengthof(micros) * MICRO_WIDTH * MICRO_HEIGHT);

	CelFrameEntry entries[lengthof(micros)];
	xx = 0, yy = MICRO_HEIGHT - 1;
	int idx = 0;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		if (micro.res_encoding >= 0) {
			entries[idx].type = idx;
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
	typedef struct {
		int subtileIndex;
		unsigned microIndex;
		int res_encoding;
	} CelMicro;
	const CelMicro micros[] = {
		// add shadow of the grate
/*  0 */{ 306 - 1, 1, -1 },
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
	BYTE* resCelBuf = DiabloAllocPtr(*celLen + lengthof(micros) * MICRO_WIDTH * MICRO_HEIGHT);

	CelFrameEntry entries[lengthof(micros)];
	xx = 0, yy = MICRO_HEIGHT - 1;
	int idx = 0;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		if (micro.res_encoding >= 0) {
			entries[idx].type = idx;
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

static void patchCathedralMin(BYTE *buf)
{
	uint16_t *pSubtiles = (uint16_t*)buf;
	constexpr int blockSize = BLOCK_SIZE_L1;
	// adjust the frame types
	// - after patchCathedralFloorCel
	SetFrameType(137, 5, MET_SQUARE);
	SetFrameType(286, 1, MET_RTRIANGLE);
	SetFrameType(408, 0, MET_LTRIANGLE);
	SetFrameType(248, 0, MET_LTRIANGLE);

	SetFrameType(392, 0, MET_TRANSPARENT);
	SetFrameType(43, 0, MET_TRANSPARENT);
	SetFrameType(396, 0, MET_TRANSPARENT);
	SetFrameType(397, 0, MET_TRANSPARENT);
	SetFrameType(399, 0, MET_TRANSPARENT);
	SetFrameType(401, 0, MET_TRANSPARENT);
	SetFrameType(403, 0, MET_TRANSPARENT);
	SetFrameType(409, 0, MET_TRANSPARENT);
	SetFrameType(411, 0, MET_TRANSPARENT);
	SetFrameType(392, 2, MET_TRANSPARENT);
	SetFrameType(43, 2, MET_TRANSPARENT);
	SetFrameType(212, 2, MET_TRANSPARENT);
	SetFrameType(396, 2, MET_TRANSPARENT);
	SetFrameType(397, 2, MET_TRANSPARENT);
	SetFrameType(399, 2, MET_TRANSPARENT);
	SetFrameType(401, 2, MET_TRANSPARENT);
	SetFrameType(403, 2, MET_TRANSPARENT);
	SetFrameType(409, 2, MET_TRANSPARENT);
	SetFrameType(407, 2, MET_TRANSPARENT);
	SetFrameType(411, 2, MET_TRANSPARENT);
	SetFrameType(394, 1, MET_TRANSPARENT);
	SetFrameType(45, 1, MET_TRANSPARENT);
	SetFrameType(396, 1, MET_TRANSPARENT);
	SetFrameType(398, 1, MET_TRANSPARENT);
	SetFrameType(400, 1, MET_TRANSPARENT);
	SetFrameType(404, 1, MET_TRANSPARENT);
	SetFrameType(406, 1, MET_TRANSPARENT);
	SetFrameType(410, 1, MET_TRANSPARENT);
	SetFrameType(412, 1, MET_TRANSPARENT);
	SetFrameType(394, 3, MET_TRANSPARENT);
	SetFrameType(45, 3, MET_TRANSPARENT);
	SetFrameType(396, 3, MET_TRANSPARENT);
	SetFrameType(398, 3, MET_TRANSPARENT);
	SetFrameType(400, 3, MET_TRANSPARENT);
	SetFrameType(404, 3, MET_TRANSPARENT);
	SetFrameType(406, 3, MET_TRANSPARENT);
	SetFrameType(410, 3, MET_TRANSPARENT);
	SetFrameType(412, 3, MET_TRANSPARENT);
	SetFrameType(407, 0, MET_TRANSPARENT);
	SetFrameType(212, 0, MET_TRANSPARENT);
	SetFrameType(171, 1, MET_RTRIANGLE);

	Blk2Mcr(108, 1);
	SetFrameType(106, 0, MET_TRANSPARENT);
	SetFrameType(109, 0, MET_TRANSPARENT); // TODO: ==  110[0] ?
	SetFrameType(106, 1, MET_TRANSPARENT);
	// use micros created by patchCathedralFloorCel
	ReplaceMcr(160, 0, 23, 0);
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
	SetMcr(334, 4, 63, 4);
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
	// pointless door micros (re-drawn by dSpecial or the object)
	// - vertical doors	
	ReplaceMcr(392, 4, 231, 4);
	ReplaceMcr(407, 4, 231, 4);
	Blk2Mcr(214, 6);
	Blk2Mcr(214, 4);
	Blk2Mcr(214, 2);
	ReplaceMcr(214, 0, 408, 0);
	ReplaceMcr(214, 1, 408, 1);
	ReplaceMcr(212, 0, 407, 0);
	ReplaceMcr(212, 2, 392, 2);
	ReplaceMcr(212, 4, 231, 4);
	Blk2Mcr(408, 4);
	Blk2Mcr(408, 2);
	HideMcr(44, 6);
	Blk2Mcr(44, 4);
	Blk2Mcr(44, 2);
	ReplaceMcr(44, 0, 7, 0);
	ReplaceMcr(44, 1, 7, 1);
	Blk2Mcr(393, 4);
	Blk2Mcr(393, 2);
	ReplaceMcr(393, 0, 7, 0);
	ReplaceMcr(393, 1, 7, 1);
	ReplaceMcr(43, 0, 392, 0);
	ReplaceMcr(43, 2, 392, 2);
	ReplaceMcr(43, 4, 231, 4);
	HideMcr(51, 6);
	Blk2Mcr(51, 4);
	Blk2Mcr(51, 2);
	ReplaceMcr(51, 0, 7, 0);
	ReplaceMcr(51, 1, 7, 1);
	// - horizontal doors
	ReplaceMcr(394, 5, 5, 5);
	Blk2Mcr(395, 3);
	Blk2Mcr(395, 5);
	ReplaceMcr(395, 1, 11, 1);
	ReplaceMcr(395, 0, 11, 0);
	HideMcr(46, 7);
	Blk2Mcr(46, 5);
	Blk2Mcr(46, 3);
	ReplaceMcr(46, 1, 11, 1);
	ReplaceMcr(46, 0, 11, 0);
	ReplaceMcr(45, 1, 394, 1); // lost details
	ReplaceMcr(45, 3, 394, 3); // lost details
	ReplaceMcr(45, 5, 5, 5);
	// useless black micros
	Blk2Mcr(107, 0);
	Blk2Mcr(107, 1);
	Blk2Mcr(109, 1);
	Blk2Mcr(137, 1);
	Blk2Mcr(138, 0);
	Blk2Mcr(138, 1);
	Blk2Mcr(140, 1);
	// pointless pixels
	// Blk2Mcr(152, 5); - TODO: Chop?
	// Blk2Mcr(241, 0);
	// // Blk2Mcr(250, 3);
	Blk2Mcr(148, 4);
	Blk2Mcr(190, 3);
	Blk2Mcr(190, 5);
	Blk2Mcr(247, 2);
	Blk2Mcr(247, 6);
	Blk2Mcr(426, 0);
	HideMcr(427, 1);
	Blk2Mcr(428, 0);
	Blk2Mcr(428, 1);
	// - pwater column
	ReplaceMcr(171, 6, 37, 6);
	ReplaceMcr(171, 7, 37, 7);
	Blk2Mcr(171, 4);
	Blk2Mcr(171, 5);
	ReplaceMcr(171, 3, 176, 3); // lost details
	// fix graphical glitch
	ReplaceMcr(15, 1, 6, 1);
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
	ReplaceMcr(63, 0, 53, 0);
	ReplaceMcr(49, 0, 3, 0);
	ReplaceMcr(206, 0, 3, 0);
	ReplaceMcr(208, 0, 3, 0); // lost details
	ReplaceMcr(240, 0, 3, 0);
	ReplaceMcr(53, 1, 58, 1);
	ReplaceMcr(206, 1, 3, 1);
	ReplaceMcr(15, 7, 6, 7); // lost details
	// ReplaceMcr(56, 7, 6, 7); // lost details
	ReplaceMcr(60, 7, 6, 7); // lost details
	ReplaceMcr(127, 7, 6, 7);
	ReplaceMcr(134, 7, 6, 7); // lost details
	ReplaceMcr(138, 7, 6, 7);
	ReplaceMcr(198, 7, 6, 7);
	ReplaceMcr(202, 7, 6, 7);
	ReplaceMcr(204, 7, 6, 7);
	ReplaceMcr(230, 7, 6, 7);
	ReplaceMcr(234, 7, 6, 7);
	ReplaceMcr(238, 7, 6, 7);
	ReplaceMcr(242, 7, 6, 7);
	ReplaceMcr(244, 7, 6, 7);
	ReplaceMcr(246, 7, 6, 7);
	// ReplaceMcr(251, 7, 6, 7);
	ReplaceMcr(323, 7, 6, 7);
	ReplaceMcr(333, 7, 6, 7);
	ReplaceMcr(365, 7, 6, 7);
	ReplaceMcr(369, 7, 6, 7);
	ReplaceMcr(373, 7, 6, 7);

	ReplaceMcr(15, 5, 6, 5);
	// ReplaceMcr(46, 5, 6, 5);
	// ReplaceMcr(56, 5, 6, 5);
	ReplaceMcr(127, 5, 6, 5);
	ReplaceMcr(134, 5, 6, 5);
	ReplaceMcr(198, 5, 6, 5);
	ReplaceMcr(202, 5, 6, 5);
	ReplaceMcr(204, 5, 6, 5);
	ReplaceMcr(230, 5, 6, 5); // lost details
	ReplaceMcr(234, 5, 6, 5); // lost details
	ReplaceMcr(242, 5, 6, 5);
	ReplaceMcr(244, 5, 6, 5);
	ReplaceMcr(246, 5, 6, 5);
	// ReplaceMcr(251, 5, 6, 5);
	ReplaceMcr(323, 5, 6, 5);
	ReplaceMcr(333, 5, 6, 5);
	// ReplaceMcr(416, 5, 6, 5);
	ReplaceMcr(6, 3, 15, 3);
	ReplaceMcr(204, 3, 15, 3);
	ReplaceMcr(242, 3, 15, 3);
	ReplaceMcr(244, 3, 15, 3); // lost details
	ReplaceMcr(246, 3, 15, 3); // lost details
	// ReplaceMcr(251, 3, 15, 3);
	// ReplaceMcr(416, 3, 15, 3);
	ReplaceMcr(15, 1, 6, 1);
	ReplaceMcr(134, 1, 6, 1);
	ReplaceMcr(198, 1, 6, 1);
	ReplaceMcr(202, 1, 6, 1);
	ReplaceMcr(323, 1, 6, 1);
	// ReplaceMcr(416, 1, 6, 1);
	ReplaceMcr(15, 0, 6, 0);

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
	ReplaceMcr(212, 6, 1, 6);
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
	ReplaceMcr(45, 0, 8, 0);
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
	ReplaceMcr(191, 0, 167, 0); // lost details
	ReplaceMcr(191, 1, 167, 1); // lost details
	ReplaceMcr(175, 1, 167, 1); // lost details
	ReplaceMcr(177, 0, 167, 0); // lost details
	ReplaceMcr(177, 1, 167, 1); // lost details
	ReplaceMcr(177, 2, 167, 2); // lost details
	// ReplaceMcr(177, 4, 168, 4); // lost details
	ReplaceMcr(175, 4, 177, 4);
	ReplaceMcr(191, 4, 177, 4);

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
	ReplaceMcr(45, 5, 5, 5);
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
	ReplaceMcr(43, 1, 8, 1);  // lost details
	ReplaceMcr(62, 1, 8, 1);  // lost details
	ReplaceMcr(205, 1, 8, 1); // lost details
	ReplaceMcr(207, 1, 8, 1); // lost details
	ReplaceMcr(212, 1, 8, 1); // lost details 
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
	ReplaceMcr(276, 0, 2, 0);
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

	ReplaceMcr(72, 1, 45, 1);
	ReplaceMcr(72, 3, 45, 3);
	ReplaceMcr(72, 5, 45, 5);

	// eliminate micros of unused subtiles
	// Blk2Mcr(311 ...),
	Blk2Mcr(50, 0);
	Blk2Mcr(50, 1);
	Blk2Mcr(50, 2);
	Blk2Mcr(50, 3);
	Blk2Mcr(50, 4);
	Blk2Mcr(50, 5);
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
	Blk2Mcr(61, 0);
	Blk2Mcr(61, 2);
	Blk2Mcr(61, 4);
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
	Blk2Mcr(213, 0);
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

	int unusedSubtiles[] = {
		18, 19, 71, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 113, 117, 119, 120, 121, 122, 125, 200, 220, 250, 253, 267, 268, 273, 275, 278, 280, 281, 282, 303, 305, 316, 318, 329, 331, 341, 405, 425, 430, 432, 435, 436, 440
	};
	for (int n = 0; n < lengthof(unusedSubtiles); n++) {
		for (int i = 0; i < blockSize; i++) {
			Blk2Mcr(unusedSubtiles[n], i);
		}
	}
}

static void patchCathedralTil(BYTE* buf)
{
	uint16_t *pTiles = (uint16_t*)buf;
	// reuse subtiles
	pTiles[(43 - 1) * 4 + 2] = SwapLE16(3 - 1);
	pTiles[(61 - 1) * 4 + 0] = SwapLE16(23 - 1);
	pTiles[(62 - 1) * 4 + 0] = SwapLE16(23 - 1);
	pTiles[(73 - 1) * 4 + 0] = SwapLE16(23 - 1);
	pTiles[(74 - 1) * 4 + 0] = SwapLE16(23 - 1);
	pTiles[(75 - 1) * 4 + 0] = SwapLE16(23 - 1);
	pTiles[(77 - 1) * 4 + 0] = SwapLE16(23 - 1);
	pTiles[(129 - 1) * 4 + 0] = SwapLE16(23 - 1);
	pTiles[(136 - 1) * 4 + 0] = SwapLE16(23 - 1);
	pTiles[(105 - 1) * 4 + 0] = SwapLE16(23 - 1);
	pTiles[(137 - 1) * 4 + 0] = SwapLE16(23 - 1);
	pTiles[(130 - 1) * 4 + 0] = SwapLE16(23 - 1);
	pTiles[(133 - 1) * 4 + 0] = SwapLE16(23 - 1);
	pTiles[(58 - 1) * 4 + 1] = SwapLE16(2 - 1);
	pTiles[(60 - 1) * 4 + 1] = SwapLE16(2 - 1);
	pTiles[(103 - 1) * 4 + 1] = SwapLE16(2 - 1);
	pTiles[(186 - 1) * 4 + 1] = SwapLE16(2 - 1);
	pTiles[(128 - 1) * 4 + 1] = SwapLE16(2 - 1);
	pTiles[(134 - 1) * 4 + 1] = SwapLE16(2 - 1);
	pTiles[(136 - 1) * 4 + 1] = SwapLE16(2 - 1);
	// pTiles[(42 - 1) * 4 + 2] = SwapLE16(12 - 1);
	pTiles[(44 - 1) * 4 + 2] = SwapLE16(12 - 1);
	// pTiles[(159 - 1) * 4 + 2] = SwapLE16(12 - 1);
	pTiles[(9 - 1) * 4 + 2] = SwapLE16(7 - 1);
	pTiles[(59 - 1) * 4 + 2] = SwapLE16(7 - 1);
	pTiles[(60 - 1) * 4 + 2] = SwapLE16(7 - 1);
	pTiles[(62 - 1) * 4 + 2] = SwapLE16(7 - 1);
	pTiles[(128 - 1) * 4 + 2] = SwapLE16(7 - 1);
	pTiles[(129 - 1) * 4 + 2] = SwapLE16(7 - 1);
	pTiles[(136 - 1) * 4 + 2] = SwapLE16(7 - 1);
	pTiles[(9 - 1) * 4 + 3] = SwapLE16(4 - 1);
	pTiles[(58 - 1) * 4 + 3] = SwapLE16(4 - 1);
	pTiles[(59 - 1) * 4 + 3] = SwapLE16(4 - 1);
	pTiles[(60 - 1) * 4 + 3] = SwapLE16(4 - 1);
	pTiles[(74 - 1) * 4 + 3] = SwapLE16(4 - 1);
	pTiles[(76 - 1) * 4 + 3] = SwapLE16(4 - 1);
	pTiles[(97 - 1) * 4 + 3] = SwapLE16(4 - 1);
	pTiles[(130 - 1) * 4 + 3] = SwapLE16(4 - 1);
	pTiles[(137 - 1) * 4 + 3] = SwapLE16(4 - 1);
	pTiles[(193 - 1) * 4 + 3] = SwapLE16(4 - 1);
	// create the new shadows
	// - use the shadows created by fixCathedralShadows
	pTiles[(131 - 1) * 4 + 0] = SwapLE16(23 - 1);
	pTiles[(131 - 1) * 4 + 1] = SwapLE16(2 - 1);
	pTiles[(131 - 1) * 4 + 2] = SwapLE16(301 - 1);
	pTiles[(131 - 1) * 4 + 3] = SwapLE16(302 - 1);
	pTiles[(132 - 1) * 4 + 0] = SwapLE16(296 - 1);
	pTiles[(132 - 1) * 4 + 1] = SwapLE16(297 - 1);
	pTiles[(132 - 1) * 4 + 2] = SwapLE16(310 - 1);
	pTiles[(132 - 1) * 4 + 3] = SwapLE16(302 - 1);
	// pTiles[(139 - 1) * 4 + 0] = SwapLE16(296 - 1);
	// pTiles[(139 - 1) * 4 + 1] = SwapLE16(297 - 1);
	pTiles[(139 - 1) * 4 + 2] = SwapLE16(328 - 1);
	// pTiles[(139 - 1) * 4 + 3] = SwapLE16(299 - 1);
	pTiles[(140 - 1) * 4 + 0] = SwapLE16(23 - 1);
	// pTiles[(140 - 1) * 4 + 1] = SwapLE16(2 - 1);
	// pTiles[(140 - 1) * 4 + 2] = SwapLE16(301 - 1);
	pTiles[(140 - 1) * 4 + 3] = SwapLE16(330 - 1);
	pTiles[(141 - 1) * 4 + 0] = SwapLE16(296 - 1);
	pTiles[(141 - 1) * 4 + 1] = SwapLE16(297 - 1);
	pTiles[(141 - 1) * 4 + 2] = SwapLE16(310 - 1);
	pTiles[(141 - 1) * 4 + 3] = SwapLE16(299 - 1);
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
	pTiles[(154 - 1) * 4 + 1] = SwapLE16(15 - 1);
	pTiles[(154 - 1) * 4 + 2] = SwapLE16(328 - 1);
	pTiles[(154 - 1) * 4 + 3] = SwapLE16(299 - 1);
	pTiles[(155 - 1) * 4 + 0] = SwapLE16(340 - 1);
	pTiles[(155 - 1) * 4 + 1] = SwapLE16(323 - 1);
	pTiles[(155 - 1) * 4 + 2] = SwapLE16(328 - 1);
	pTiles[(155 - 1) * 4 + 3] = SwapLE16(299 - 1);
	pTiles[(156 - 1) * 4 + 0] = SwapLE16(45 - 1);
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
	pTiles[(161 - 1) * 4 + 2] = SwapLE16(58 - 1);
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
	pTiles[(46 - 1) * 4 + 1] = SwapLE16(15 - 1);
	pTiles[(46 - 1) * 4 + 2] = SwapLE16(301 - 1);
	pTiles[(46 - 1) * 4 + 3] = SwapLE16(302 - 1);
	// eliminate subtiles of unused tiles
	const int unusedTiles[] = {
		28, 30, 31, 34,/* 38,*/ 39, 40, 41, 42,/* 44,*/ 45, 79, 82, 86, 87, 88, 89, 90, 91, 92, 95, 96, 119, 120, 126, 127, 177, 178, 179, 180, 181, 182, 183, 184, 185, 187, 188, 189, 190, 191, 192, 195, 197, 198, 199, 200, 201, 202, 203, 204, 205
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

static BYTE *patchCatacombsDoors(BYTE* celBuf, size_t* celLen)
{
	typedef struct {
		int frameIndex;
		int frameWidth;
		int frameHeight;
	} CelFrame;
	const CelFrame frames[] = {
		{ 0, 64, 128 },
		{ 1, 64, 128 },
	};

	constexpr BYTE TRANS_COLOR = 128;
	constexpr BYTE SUB_HEADER_SIZE = 10;
	int idx = 0;

	DWORD* srcHeaderCursor = (DWORD*)celBuf;
	int srcCelEntries = SwapLE32(srcHeaderCursor[0]);
	srcHeaderCursor++;

	// create the new CEL file
	BYTE* resCelBuf = DiabloAllocPtr(*celLen + 2 * *celLen);
	memset(resCelBuf, 0, *celLen + 2 * *celLen);

	DWORD* dstHeaderCursor = (DWORD*)resCelBuf;
	*dstHeaderCursor = SwapLE32(srcCelEntries);
	dstHeaderCursor++;

	BYTE* dstDataCursor = resCelBuf + 4 * (srcCelEntries + 2);
	for (int i = 0; i < srcCelEntries; i++) {
		const CelFrame &frame = frames[idx];
		if (i == frame.frameIndex) {
			// draw the frame to the back-buffer
			memset(&gpBuffer[0], TRANS_COLOR, frame.frameHeight * BUFFER_WIDTH);
			CelClippedDrawLightTbl(0, frame.frameHeight - 1, celBuf, frame.frameIndex + 1, frame.frameWidth, 0);

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

			dstHeaderCursor[0] = SwapLE32((size_t)dstDataCursor - (size_t)resCelBuf);
			dstHeaderCursor++;

			dstDataCursor = EncodeFrame(dstDataCursor, frame.frameWidth, frame.frameHeight, SUB_HEADER_SIZE, TRANS_COLOR);

			// skip the original frame
			srcHeaderCursor++;

			idx++;
		} else {
			dstHeaderCursor[0] = SwapLE32((size_t)dstDataCursor - (size_t)resCelBuf);
			dstHeaderCursor++;
			DWORD len = srcHeaderCursor[1] - srcHeaderCursor[0];
			memcpy(dstDataCursor, celBuf + srcHeaderCursor[0], len);
			dstDataCursor += len;
			srcHeaderCursor++;
		}
	}
	// add file-size
	*celLen = (size_t)dstDataCursor - (size_t)resCelBuf;
	dstHeaderCursor[0] = SwapLE32(*celLen);

	return resCelBuf;
}

static BYTE* patchCatacombsSpec(BYTE* celBuf, size_t* celLen)
{
	typedef struct {
		int frameIndex;
		int frameWidth;
		int frameHeight;
	} CelFrame;
	const CelFrame frames[] = {
		{ 0, 64, 160 },
		{ 1, 64, 160 },
		{ 4, 64, 160 },
	};

	constexpr BYTE TRANS_COLOR = 128;
	constexpr BYTE SUB_HEADER_SIZE = 10;
	int idx = 0;

	DWORD* srcHeaderCursor = (DWORD*)celBuf;
	int srcCelEntries = SwapLE32(srcHeaderCursor[0]);
	srcHeaderCursor++;

	// create the new CEL file
	BYTE* resCelBuf = DiabloAllocPtr(*celLen + 2 * *celLen);
	memset(resCelBuf, 0, *celLen + 2 * *celLen);

	DWORD* dstHeaderCursor = (DWORD*)resCelBuf;
	*dstHeaderCursor = SwapLE32(srcCelEntries);
	dstHeaderCursor++;

	BYTE* dstDataCursor = resCelBuf + 4 * (srcCelEntries + 2);
	for (int i = 0; i < srcCelEntries; i++) {
		const CelFrame &frame = frames[idx];
		if (i == frame.frameIndex) {
			// draw the frame to the back-buffer
			memset(&gpBuffer[0], TRANS_COLOR, frame.frameHeight * BUFFER_WIDTH);
			CelClippedDrawLightTbl(0, frame.frameHeight - 1, celBuf, frame.frameIndex + 1, frame.frameWidth, 0);

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

			dstHeaderCursor[0] = SwapLE32((size_t)dstDataCursor - (size_t)resCelBuf);
			dstHeaderCursor++;

			dstDataCursor = EncodeFrame(dstDataCursor, frame.frameWidth, frame.frameHeight, SUB_HEADER_SIZE, TRANS_COLOR);

			// skip the original frame
			srcHeaderCursor++;

			idx++;
		} else {
			dstHeaderCursor[0] = SwapLE32((size_t)dstDataCursor - (size_t)resCelBuf);
			dstHeaderCursor++;
			DWORD len = srcHeaderCursor[1] - srcHeaderCursor[0];
			memcpy(dstDataCursor, celBuf + srcHeaderCursor[0], len);
			dstDataCursor += len;
			srcHeaderCursor++;
		}
	}
	// add file-size
	*celLen = (size_t)dstDataCursor - (size_t)resCelBuf;
	dstHeaderCursor[0] = SwapLE32(*celLen);

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
static BYTE* fixCatacombsShadows(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	typedef struct {
		int subtileIndex;
		unsigned microIndex;
		int res_encoding;
	} CelMicro;
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
	BYTE* resCelBuf = DiabloAllocPtr(*celLen + lengthof(micros) * MICRO_WIDTH * MICRO_HEIGHT);

	CelFrameEntry entries[lengthof(micros)];
	xx = 0, yy = MICRO_HEIGHT - 1;
	int idx = 0;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		if (micro.res_encoding >= 0) {
			entries[idx].type = idx;
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
	typedef struct {
		int subtileIndex;
		unsigned microIndex;
		int res_encoding;
	} CelMicro;
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
	BYTE* resCelBuf = DiabloAllocPtr(*celLen + lengthof(micros) * MICRO_WIDTH * MICRO_HEIGHT);

	CelFrameEntry entries[lengthof(micros)];
	xx = 0, yy = MICRO_HEIGHT - 1;
	int idx = 0;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		if (micro.res_encoding >= 0) {
			entries[idx].type = idx;
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

static void patchCatacombsMin(BYTE *buf)
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
	// - horizontal doors
	// Blk2Mcr(17, 3);
	ReplaceMcr(540, 0, 17, 0);
	ReplaceMcr(540, 1, 17, 1);
	ReplaceMcr(540, 2, 17, 2);
	ReplaceMcr(540, 3, 17, 3);
	ReplaceMcr(540, 4, 17, 4);
	// Blk2Mcr(540, 3);
	Blk2Mcr(540, 5);
	Blk2Mcr(540, 6);
	Blk2Mcr(540, 7);
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
	ReplaceMcr(85, 6, 3, 6);
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
	ReplaceMcr(236, 6, 3, 6);
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
	ReplaceMcr(85, 4, 3, 4);
	ReplaceMcr(88, 4, 3, 4);
	// ReplaceMcr(92, 4, 3, 4);
	ReplaceMcr(96, 4, 3, 4);
	// ReplaceMcr(117, 4, 3, 4);
	// ReplaceMcr(120, 4, 3, 4);
	ReplaceMcr(129, 4, 3, 4);
	// ReplaceMcr(172, 4, 3, 4);
	ReplaceMcr(176, 4, 3, 4);
	ReplaceMcr(236, 4, 66, 4);
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
	ReplaceMcr(85, 2, 3, 2);
	ReplaceMcr(88, 2, 3, 2);
	// ReplaceMcr(92, 2, 3, 2);
	ReplaceMcr(96, 2, 3, 2);
	// ReplaceMcr(117, 2, 3, 2);
	ReplaceMcr(129, 2, 3, 2);
	// ReplaceMcr(172, 2, 3, 2);
	ReplaceMcr(176, 2, 3, 2);
	ReplaceMcr(180, 2, 3, 2);
	ReplaceMcr(236, 2, 66, 2);
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
	ReplaceMcr(236, 0, 62, 0);
	ReplaceMcr(240, 0, 62, 0);
	ReplaceMcr(244, 0, 62, 0);
	ReplaceMcr(277, 0, 66, 0);
	ReplaceMcr(281, 0, 62, 0);
	ReplaceMcr(285, 0, 62, 0);
	ReplaceMcr(305, 0, 62, 0);
	ReplaceMcr(448, 0, 3, 0);
	ReplaceMcr(480, 0, 62, 0);
	ReplaceMcr(552, 0, 62, 0);

	ReplaceMcr(85, 1, 82, 1);
	// ReplaceMcr(117, 1, 244, 1);
	// ReplaceMcr(120, 1, 244, 1);
	ReplaceMcr(236, 1, 244, 1);
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
	// ReplaceMcr(548, 5, 6, 5); // Frame 159 is used by subtiles 46, 529.

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
	Blk2Mcr(269, 0);
	Blk2Mcr(269, 1);
	Blk2Mcr(269, 2);
	Blk2Mcr(269, 3);
	Blk2Mcr(269, 4);
	Blk2Mcr(269, 5);
	Blk2Mcr(269, 6);
	Blk2Mcr(269, 7);
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
	Blk2Mcr(537, 0);
	Blk2Mcr(537, 1);
	Blk2Mcr(537, 6);
	Blk2Mcr(539, 1);
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
	int unusedSubtiles[] = {
		2, 7, 14, 19, 20, 48, 50, 53, 55, 56, 57, 58, 59, 70, 71, 106, 109, 110, 116, 117, 118, 120, 122, 123, 124, 126, 137, 140, 145, 149, 157, 159, 160, 168, 170, 171, 172, 173, 192, 193, 194, 195, 196, 197, 198, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 235, 243, 246, 247, 255, 256, 264, 327, 328, 329, 330, 335, 336, 337, 338, 343, 344, 345, 346, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 366, 367, 368, 369, 370, 376, 391, 397, 400, 434, 487, 489, 491, 493, 504, 505, 507, 509, 511, 516, 518, 531, 533, 536, 541, 
	};

	for (int n = 0; n < lengthof(unusedSubtiles); n++) {
		for (int i = 0; i < blockSize; i++) {
			Blk2Mcr(unusedSubtiles[n], i);
		}
	}
}

static void patchCatacombsTil(BYTE* buf)
{
	uint16_t *pTiles = (uint16_t*)buf;
	// reuse subtiles
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
	pTiles[(14 - 1) * 4 + 0] = SwapLE16(45 - 1); // 48
	pTiles[(14 - 1) * 4 + 2] = SwapLE16(39 - 1); // 49
	pTiles[(15 - 1) * 4 + 0] = SwapLE16(45 - 1); // 50
	pTiles[(15 - 1) * 4 + 1] = SwapLE16(42 - 1); // 51
	pTiles[(15 - 1) * 4 + 2] = SwapLE16(43 - 1); // 52
	pTiles[(16 - 1) * 4 + 0] = SwapLE16(45 - 1); // 53
	pTiles[(16 - 1) * 4 + 1] = SwapLE16(38 - 1); // 54
	pTiles[(16 - 1) * 4 + 2] = SwapLE16(43 - 1); // 52
	pTiles[(24 - 1) * 4 + 1] = SwapLE16(77 - 1); // 81
	pTiles[(25 - 1) * 4 + 1] = SwapLE16(77 - 1);
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
	pTiles[(73 - 1) * 4 + 2] = SwapLE16(11 - 1);  // 255
	pTiles[(73 - 1) * 4 + 3] = SwapLE16(12 - 1);  // 256
	pTiles[(75 - 1) * 4 + 3] = SwapLE16(12 - 1);  // 264
	pTiles[(78 - 1) * 4 + 0] = SwapLE16(21 - 1); // 269
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
	// - pillar tile for a pillar
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
	// fix graphical glitch
	pTiles[(157 - 1) * 4 + 1] = SwapLE16(99 - 1); // 548
	// fix the upstairs II.
	pTiles[(72 - 1) * 4 + 1] = SwapLE16(56 - 1); // 10 make the back of the stairs non-walkable
	pTiles[(72 - 1) * 4 + 0] = SwapLE16(9 - 1);  // 250
	pTiles[(72 - 1) * 4 + 2] = SwapLE16(11 - 1); // 251
	pTiles[(76 - 1) * 4 + 1] = SwapLE16(10 - 1); // 265
	// pTiles[(158 - 1) * 4 + 0] = SwapLE16(9 - 1);  // 250
	// pTiles[(158 - 1) * 4 + 1] = SwapLE16(56 - 1); // 10 make the back of the stairs non-walkable
	// pTiles[(158 - 1) * 4 + 2] = SwapLE16(11 - 1); // 554
	// pTiles[(159 - 1) * 4 + 0] = SwapLE16(9 - 1);  // 555
	// pTiles[(159 - 1) * 4 + 1] = SwapLE16(10 - 1); // 556
	// pTiles[(159 - 1) * 4 + 2] = SwapLE16(11 - 1); // 557
	// eliminate subtiles of unused tiles
	const int unusedTiles[] = {
		52, 58, 61, 64, 65, 66, 67, 76, 93, 98, 101, 102, 103, 104, 143, 144, 145, 146, 147, 148, 149, 152, 153, 154, 155, 158, 159, 160
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

#ifdef HELLFIRE
static void patchCryptMin(BYTE *buf)
{
	uint16_t *pSubtiles = (uint16_t*)buf;
	constexpr int blockSize = BLOCK_SIZE_L5;
	// adjust the frame types
	// - after fillCryptShapes
	SetFrameType(159, 3, MET_SQUARE);
	SetFrameType(185, 3, MET_SQUARE);
	// SetFrameType(159, 1, MET_RTRAPEZOID);
	SetFrameType(336, 0, MET_LTRIANGLE);
	SetFrameType(409, 0, MET_LTRIANGLE);
	SetFrameType(481, 1, MET_RTRIANGLE);
	SetFrameType(492, 0, MET_LTRIANGLE);
	SetFrameType(519, 0, MET_LTRIANGLE);
	SetFrameType(595, 1, MET_RTRIANGLE);
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
	ReplaceMcr(242, 6, 31, 6);
	SetMcr(242, 8, 31, 8);
	ReplaceMcr(178, 0, 619, 1);
	ReplaceMcr(178, 1, 625, 0);
	SetMcr(178, 2, 624, 0);
	Blk2Mcr(178, 4);
	ReplaceMcr(178, 6, 31, 6);
	ReplaceMcr(178, 8, 31, 8);
	ReplaceMcr(238, 0, 634, 0);
	ReplaceMcr(238, 1, 634, 1);
	Blk2Mcr(238, 4);
	SetMcr(238, 6, 31, 6);
	SetMcr(238, 8, 31, 8);
	// pointless door micros (re-drawn by dSpecial or the object)
	Blk2Mcr(77, 2);
	Blk2Mcr(77, 4);
	Blk2Mcr(77, 6);
	Blk2Mcr(77, 8);
	ReplaceMcr(77, 0, 206, 0);
	ReplaceMcr(77, 1, 206, 1);
	ReplaceMcr(75, 0, 204, 0);
	ReplaceMcr(75, 1, 204, 1);
	ReplaceMcr(75, 2, 204, 2);
	ReplaceMcr(75, 4, 204, 4);
	ReplaceMcr(91, 0, 204, 0);
	ReplaceMcr(91, 2, 204, 2);
	ReplaceMcr(91, 4, 204, 4);
	ReplaceMcr(99, 0, 204, 0);
	ReplaceMcr(99, 2, 204, 2);
	ReplaceMcr(99, 4, 204, 4);
	ReplaceMcr(113, 0, 204, 0);
	ReplaceMcr(113, 2, 204, 2);
	ReplaceMcr(113, 4, 204, 4);
	ReplaceMcr(115, 0, 204, 0);
	ReplaceMcr(115, 2, 204, 2);
	ReplaceMcr(115, 4, 204, 4);
	Blk2Mcr(80, 7);
	Blk2Mcr(80, 9);
	ReplaceMcr(80, 0, 209, 0);
	ReplaceMcr(80, 1, 209, 1);
	ReplaceMcr(80, 3, 209, 3);
	ReplaceMcr(80, 5, 209, 5);
	ReplaceMcr(79, 0, 208, 0);
	ReplaceMcr(79, 1, 208, 1);
	ReplaceMcr(79, 3, 208, 3);
	ReplaceMcr(79, 5, 208, 5);
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
	Blk2Mcr(398, 5);
	// fix graphical glitch
	ReplaceMcr(21, 1, 55, 1);
	ReplaceMcr(25, 0, 33, 0);
	ReplaceMcr(22, 0, 2, 0);
	ReplaceMcr(22, 1, 2, 1);
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
	ReplaceMcr(627, 0, 626, 0);
	SetMcr(627, 1, 626, 1);
	// prepare new subtiles for the shadows
	ReplaceMcr(623, 0, 631, 0);
	ReplaceMcr(623, 1, 638, 1);
	ReplaceMcr(636, 0, 626, 0);
	ReplaceMcr(636, 1, 638, 1);
	// reuse subtiles
	ReplaceMcr(631, 1, 626, 1);
	ReplaceMcr(149, 4, 1, 4);
	ReplaceMcr(150, 6, 15, 6);
	ReplaceMcr(324, 7, 6, 7);
	ReplaceMcr(432, 7, 6, 7);
	ReplaceMcr(440, 7, 6, 7);
	// ReplaceMcr(26, 9, 6, 9);
	ReplaceMcr(340, 9, 6, 9);
	ReplaceMcr(394, 9, 6, 9);
	ReplaceMcr(451, 9, 6, 9);
	// ReplaceMcr(14, 7, 6, 7); // lost shine
	// ReplaceMcr(26, 7, 6, 7); // lost shine
	// ReplaceMcr(80, 7, 6, 7);
	ReplaceMcr(451, 7, 6, 7); // lost shine
	ReplaceMcr(340, 7, 6, 7); // lost shine
	ReplaceMcr(364, 7, 6, 7); // lost crack
	ReplaceMcr(394, 7, 6, 7); // lost shine
	ReplaceMcr(554, 7, 269, 7);
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
	ReplaceMcr(340, 5, 432, 5); // lost details
	ReplaceMcr(364, 5, 432, 5); // lost details
	ReplaceMcr(380, 5, 432, 5); // lost details
	ReplaceMcr(394, 5, 432, 5); // lost details
	ReplaceMcr(6, 3, 14, 3);    // lost details
	// ReplaceMcr(26, 3, 14, 3); // lost details
	// ReplaceMcr(80, 3, 14, 3); // lost details
	ReplaceMcr(269, 3, 14, 3); // lost details
	ReplaceMcr(414, 3, 14, 3); // lost details
	ReplaceMcr(451, 3, 14, 3); // lost details
	ReplaceMcr(554, 3, 14, 3); // lost details
	ReplaceMcr(556, 3, 14, 3); // lost details
	// ? ReplaceMcr(608, 3, 103, 3); // lost details
	ReplaceMcr(324, 3, 380, 3); // lost details
	ReplaceMcr(340, 3, 380, 3); // lost details
	ReplaceMcr(364, 3, 380, 3); // lost details
	ReplaceMcr(432, 3, 380, 3); // lost details
	ReplaceMcr(440, 3, 380, 3); // lost details
	ReplaceMcr(6, 0, 14, 0);
	// ReplaceMcr(26, 0, 14, 0);
	ReplaceMcr(269, 0, 14, 0);
	ReplaceMcr(554, 0, 14, 0); // lost details
	ReplaceMcr(340, 0, 324, 0); // lost details
	ReplaceMcr(364, 0, 324, 0); // lost details
	ReplaceMcr(451, 0, 324, 0); // lost details
	// ReplaceMcr(14, 1, 6, 1);
	// ReplaceMcr(26, 1, 6, 1);
	// ReplaceMcr(80, 1, 6, 1);
	ReplaceMcr(269, 1, 6, 1);
	ReplaceMcr(380, 1, 6, 1);
	ReplaceMcr(451, 1, 6, 1);
	ReplaceMcr(554, 1, 6, 1);
	ReplaceMcr(556, 1, 6, 1);
	ReplaceMcr(324, 1, 340, 1);
	ReplaceMcr(364, 1, 340, 1);
	ReplaceMcr(394, 1, 340, 1); // lost details
	ReplaceMcr(432, 1, 340, 1); // lost details
	ReplaceMcr(551, 5, 265, 5);
	ReplaceMcr(551, 0, 265, 0);
	ReplaceMcr(551, 1, 265, 1);
	ReplaceMcr(261, 0, 14, 0); // lost details
	ReplaceMcr(545, 0, 14, 0); // lost details
	ReplaceMcr(18, 9, 6, 9); // lost details
	ReplaceMcr(34, 9, 6, 9); // lost details
	// ReplaceMcr(37, 9, 6, 9);
	ReplaceMcr(277, 9, 6, 9); // lost details
	ReplaceMcr(332, 9, 6, 9); // lost details
	ReplaceMcr(348, 9, 6, 9); // lost details
	ReplaceMcr(352, 9, 6, 9); // lost details
	ReplaceMcr(358, 9, 6, 9); // lost details
	ReplaceMcr(406, 9, 6, 9); // lost details
	ReplaceMcr(444, 9, 6, 9); // lost details
	ReplaceMcr(459, 9, 6, 9); // lost details
	ReplaceMcr(463, 9, 6, 9); // lost details
	ReplaceMcr(562, 9, 6, 9); // lost details
	// ReplaceMcr(564, 9, 6, 9); // lost details
	ReplaceMcr(277, 7, 18, 7); // lost details
	ReplaceMcr(562, 7, 18, 7); // lost details
	ReplaceMcr(277, 5, 459, 5); // lost details
	ReplaceMcr(562, 5, 459, 5); // lost details
	ReplaceMcr(277, 3, 459, 3); // lost details
	ReplaceMcr(562, 1, 277, 1); // lost details
	// ReplaceMcr(564, 1, 277, 1); // lost details
	ReplaceMcr(585, 1, 284, 1);
	ReplaceMcr(590, 1, 285, 1); // lost details
	ReplaceMcr(598, 1, 289, 1); // lost details
	// ReplaceMcr(564, 7, 18, 7); // lost details
	// ReplaceMcr(564, 5, 459, 5); // lost details
	// ReplaceMcr(564, 3, 459, 3); // lost details
	ReplaceMcr(34, 7, 18, 7); // lost details
	// ReplaceMcr(37, 7, 18, 7);
	ReplaceMcr(84, 7, 18, 7); // lost details
	ReplaceMcr(406, 7, 18, 7); // lost details
	ReplaceMcr(444, 7, 18, 7); // lost details
	ReplaceMcr(463, 7, 18, 7); // lost details
	ReplaceMcr(332, 7, 18, 7); // lost details
	ReplaceMcr(348, 7, 18, 7); // lost details
	ReplaceMcr(352, 7, 18, 7); // lost details
	ReplaceMcr(358, 7, 18, 7); // lost details
	ReplaceMcr(459, 7, 18, 7); // lost details
	ReplaceMcr(34, 5, 18, 5); // lost details
	ReplaceMcr(348, 5, 332, 5); // lost details
	ReplaceMcr(352, 5, 332, 5); // lost details
	ReplaceMcr(358, 5, 332, 5); // lost details
	ReplaceMcr(34, 3, 18, 3); // lost details
	ReplaceMcr(358, 3, 18, 3); // lost details
	ReplaceMcr(348, 3, 332, 3); // lost details
	ReplaceMcr(352, 3, 332, 3); // lost details
	ReplaceMcr(34, 0, 18, 0);
	ReplaceMcr(352, 0, 18, 0);
	ReplaceMcr(358, 0, 18, 0);
	ReplaceMcr(406, 0, 18, 0); // lost details
	ReplaceMcr(34, 1, 18, 1);
	ReplaceMcr(332, 1, 18, 1);
	ReplaceMcr(348, 1, 352, 1);
	ReplaceMcr(358, 1, 352, 1);
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
	ReplaceMcr(554, 9, 6, 9); // lost details
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
	ReplaceMcr(292, 8, 3, 8);
	ReplaceMcr(299, 8, 3, 8);
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
	ReplaceMcr(398, 0, 2, 0);
	ReplaceMcr(398, 1, 2, 1); // lost details
	// ReplaceMcr(540, 0, 257, 0);
	// ReplaceMcr(30, 0, 2, 0);
	// ReplaceMcr(76, 0, 2, 0);
	// ReplaceMcr(205, 0, 2, 0);
	ReplaceMcr(407, 0, 2, 0); // lost details
	ReplaceMcr(379, 0, 5, 0);
	ReplaceMcr(27, 0, 7, 0);
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
	ReplaceMcr(275, 0, 32, 0);
	ReplaceMcr(309, 0, 45, 0);
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
	ReplaceMcr(41, 1, 4, 1);
	// ReplaceMcr(24, 1, 4, 1); // lost details
	ReplaceMcr(32, 1, 4, 1); // lost details
	// ReplaceMcr(92, 1, 4, 1); // lost details
	// ReplaceMcr(96, 1, 4, 1); // lost details
	// ReplaceMcr(217, 1, 4, 1); // lost details
	ReplaceMcr(275, 1, 4, 1); // lost details
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
	ReplaceMcr(304, 1, 48, 1);
	ReplaceMcr(27, 1, 7, 1);
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
	ReplaceMcr(471, 7, 265, 7);
	ReplaceMcr(547, 7, 261, 7);
	ReplaceMcr(471, 9, 6, 9);
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
	// ReplaceMcr(19, 8, 31, 8);  // lost details
	// ReplaceMcr(390, 8, 31, 8);  // lost details
	ReplaceMcr(89, 8, 31, 8);
	ReplaceMcr(254, 8, 31, 8); // lost details
	ReplaceMcr(534, 8, 31, 8); // lost details
	ReplaceMcr(537, 8, 31, 8); // lost details
	ReplaceMcr(333, 8, 31, 8); // lost details
	ReplaceMcr(345, 8, 31, 8);
	ReplaceMcr(365, 8, 31, 8); // lost details
	ReplaceMcr(456, 8, 31, 8);
	ReplaceMcr(274, 8, 31, 8); // lost details
	// ReplaceMcr(558, 8, 31, 8); // lost details
	// ReplaceMcr(560, 8, 31, 8); // lost details
	ReplaceMcr(258, 8, 296, 8); // lost details
	ReplaceMcr(541, 8, 296, 8); // lost details
	// ReplaceMcr(543, 8, 296, 8); // lost details
	ReplaceMcr(89, 6, 31, 6);  // lost details
	ReplaceMcr(274, 6, 31, 6); // lost details
	// ReplaceMcr(558, 6, 31, 6); // lost details
	// ReplaceMcr(560, 6, 31, 6); // lost details
	ReplaceMcr(356, 6, 31, 6);  // lost details
	ReplaceMcr(333, 6, 445, 6); // lost details
	ReplaceMcr(345, 6, 445, 6); // lost details
	ReplaceMcr(365, 6, 445, 6); // lost details
	ReplaceMcr(274, 4, 31, 4);  // lost details
	// ReplaceMcr(560, 4, 31, 4); // lost details
	ReplaceMcr(333, 4, 345, 4); // lost details
	ReplaceMcr(365, 4, 345, 4); // lost details
	ReplaceMcr(445, 4, 345, 4); // lost details
	ReplaceMcr(299, 2, 274, 2); // lost details
	// ReplaceMcr(560, 2, 274, 2); // lost details
	ReplaceMcr(333, 2, 345, 2); // lost details
	ReplaceMcr(365, 2, 345, 2); // lost details
	ReplaceMcr(415, 2, 345, 2); // lost details
	ReplaceMcr(445, 2, 345, 2); // lost details
	ReplaceMcr(333, 0, 31, 0);  // lost details
	ReplaceMcr(345, 0, 31, 0);  // lost details
	ReplaceMcr(365, 0, 31, 0);  // lost details
	ReplaceMcr(445, 0, 31, 0);  // lost details
	ReplaceMcr(333, 1, 31, 1);  // lost details
	ReplaceMcr(365, 1, 31, 1);

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
	ReplaceMcr(75, 8, 95, 8);
	ReplaceMcr(83, 8, 95, 8); // lost details
	ReplaceMcr(91, 8, 95, 8);
	ReplaceMcr(99, 8, 95, 8);  // lost details
	ReplaceMcr(113, 8, 95, 8); // lost details
	ReplaceMcr(115, 8, 95, 8); // lost details
	ReplaceMcr(119, 8, 95, 8); // lost details
	ReplaceMcr(149, 8, 95, 8); // lost details
	ReplaceMcr(151, 8, 95, 8); // lost details
	// ReplaceMcr(172, 8, 95, 8);
	ReplaceMcr(204, 8, 95, 8);
	ReplaceMcr(215, 8, 95, 8);
	ReplaceMcr(220, 8, 95, 8); // lost details
	ReplaceMcr(224, 8, 95, 8); // lost details
	ReplaceMcr(226, 8, 95, 8); // lost details
	ReplaceMcr(230, 8, 95, 8); // lost details
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
	ReplaceMcr(75, 6, 99, 6);   // lost details
	ReplaceMcr(91, 6, 99, 6);   // lost details
	ReplaceMcr(115, 6, 99, 6);  // lost details
	ReplaceMcr(204, 6, 99, 6);  // lost details
	ReplaceMcr(215, 6, 99, 6);  // lost details

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

	ReplaceMcr(300, 6, 294, 6); // lost details
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
	ReplaceMcr(230, 2, 256, 2); // lost details

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
	ReplaceMcr(218, 8, 95, 8); // lost details
	ReplaceMcr(222, 8, 95, 8); // lost details
	ReplaceMcr(228, 8, 95, 8); // lost details
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
	ReplaceMcr(218, 6, 25, 6); // lost details
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
	ReplaceMcr(222, 6, 95, 6); // lost details
	ReplaceMcr(228, 6, 95, 6); // lost details
	ReplaceMcr(272, 6, 95, 6); // lost details
	ReplaceMcr(389, 6, 95, 6); // lost details
	ReplaceMcr(397, 6, 95, 6); // lost details
	// ReplaceMcr(402, 6, 95, 6); // lost details
	ReplaceMcr(443, 6, 95, 6);  // lost details
	ReplaceMcr(466, 6, 95, 6);  // lost details
	ReplaceMcr(478, 6, 95, 6);  // lost details
	ReplaceMcr(347, 6, 393, 6); // lost details
	ReplaceMcr(399, 6, 393, 6); // lost details
	ReplaceMcr(417, 6, 393, 6); // lost details
	ReplaceMcr(331, 6, 343, 6); // lost details
	ReplaceMcr(355, 6, 343, 6); // lost details
	ReplaceMcr(363, 6, 343, 6); // lost details
	ReplaceMcr(557, 6, 343, 6); // lost details
	ReplaceMcr(559, 6, 343, 6); // lost details

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
	ReplaceMcr(331, 4, 343, 4); // lost details
	ReplaceMcr(355, 4, 343, 4); // lost details
	ReplaceMcr(363, 4, 343, 4); // lost details
	ReplaceMcr(443, 4, 343, 4); // lost details
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
	ReplaceMcr(218, 2, 29, 2);  // lost details
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
	ReplaceMcr(218, 0, 33, 0);
	ReplaceMcr(228, 0, 33, 0);  // lost details
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
	ReplaceMcr(79, 9, 13, 9);
	ReplaceMcr(93, 9, 13, 9);
	ReplaceMcr(151, 9, 13, 9);
	ReplaceMcr(208, 9, 13, 9);
	ReplaceMcr(218, 9, 13, 9);
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
	ReplaceMcr(79, 7, 13, 7);
	ReplaceMcr(93, 7, 13, 7);
	ReplaceMcr(107, 7, 13, 7);
	ReplaceMcr(111, 7, 13, 7);
	ReplaceMcr(115, 7, 13, 7);
	ReplaceMcr(117, 7, 13, 7);
	ReplaceMcr(119, 7, 13, 7);
	ReplaceMcr(208, 7, 13, 7);
	ReplaceMcr(218, 7, 13, 7);
	ReplaceMcr(222, 7, 13, 7);
	ReplaceMcr(226, 7, 13, 7);
	ReplaceMcr(228, 7, 13, 7);
	ReplaceMcr(230, 7, 13, 7);
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
	ReplaceMcr(226, 5, 25, 5);
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
	ReplaceMcr(226, 3, 25, 3);
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
	ReplaceMcr(91, 9, 13, 9); // lost details
	ReplaceMcr(95, 9, 13, 9);
	ReplaceMcr(97, 9, 13, 9);  // lost details
	ReplaceMcr(99, 9, 13, 9);  // lost details
	ReplaceMcr(104, 9, 13, 9); // lost details
	ReplaceMcr(109, 9, 13, 9); // lost details
	ReplaceMcr(113, 9, 13, 9); // lost details
	ReplaceMcr(121, 9, 13, 9); // lost details
	ReplaceMcr(215, 9, 13, 9); // lost details
	ReplaceMcr(220, 9, 13, 9); // lost details
	ReplaceMcr(224, 9, 13, 9); // lost details
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
	ReplaceMcr(220, 7, 17, 7);  // lost details
	ReplaceMcr(224, 7, 17, 7);  // lost details
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
	ReplaceMcr(91, 1, 55, 1);   // lost details
	ReplaceMcr(95, 1, 55, 1);   // lost details
	ReplaceMcr(215, 1, 55, 1);  // lost details
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
	Blk2Mcr(212, 6);
	Blk2Mcr(212, 7);
	Blk2Mcr(212, 8);
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
	Blk2Mcr(213, 0);
	Blk2Mcr(213, 2);
	Blk2Mcr(213, 4);
	Blk2Mcr(213, 6);
	Blk2Mcr(213, 8);
	Blk2Mcr(216, 0);
	Blk2Mcr(216, 2);
	Blk2Mcr(216, 4);
	Blk2Mcr(216, 6);
	Blk2Mcr(216, 8);
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
	Blk2Mcr(558, 0);
	Blk2Mcr(558, 2);
	Blk2Mcr(558, 4);
	Blk2Mcr(558, 6);
	Blk2Mcr(558, 8);
	Blk2Mcr(468, 1);
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
	int unusedSubtiles[] = {
		8, 10, 11, 16, 19, 20, 23, 24, 26, 28, 30, 35, 38, 40, 43, 44, 50, 52, 56, 76, 78, 81, 82, 87, 90, 92, 94, 96, 98, 100, 102, 103, 105, 106, 108, 110, 112, 114, 116, 124, 127, 128, 137, 138, 139, 141, 143, 147, 148, 167, 172, 174, 176, 177, 193, 202, 205, 207, 210, 211, 214, 217, 219, 221, 223, 225, 227, 233, 235, 239, 249, 251, 253, 257, 259, 262, 263, 270, 273, 278, 279, 295, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 354, 373, 381, 390, 472, 489, 490, 540, 560, 564, 640, 643, 648
	};
	for (int n = 0; n < lengthof(unusedSubtiles); n++) {
		for (int i = 0; i < blockSize; i++) {
			Blk2Mcr(unusedSubtiles[n], i);
		}
	}
}

static BYTE* fillCryptShapes(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen)
{
	typedef struct {
		unsigned subtileIndex;
		unsigned microIndex;
		unsigned res_encoding;
	} CelMicro;
	const CelMicro micros[] = {
		{ 159 - 1, 3, MET_SQUARE },
//		{ 159 - 1, 3, MET_RTRAPEZOID },
		{ 336 - 1, 0, MET_LTRIANGLE },
		{ 409 - 1, 0, MET_LTRIANGLE },
		{ 481 - 1, 1, MET_RTRIANGLE },
		{ 492 - 1, 0, MET_LTRIANGLE },
		{ 519 - 1, 0, MET_LTRIANGLE },
		{ 595 - 1, 1, MET_RTRIANGLE },
		{ 368 - 1, 1, MET_RTRIANGLE },
		{ 162 - 1, 2, MET_TRANSPARENT },
		{  63 - 1, 4, MET_SQUARE },
		{ 450 - 1, 0, MET_TRANSPARENT },
		{ 206 - 1, 0, MET_TRANSPARENT },
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

	gpBuffer[0 * MICRO_WIDTH + 30 + (MICRO_HEIGHT * 1 +  1) * BUFFER_WIDTH] = 46; // 336[0]
	gpBuffer[0 * MICRO_WIDTH + 31 + (MICRO_HEIGHT * 1 +  1) * BUFFER_WIDTH] = 76;

	gpBuffer[1 * MICRO_WIDTH +  0 + (MICRO_HEIGHT * 2 + 16) * BUFFER_WIDTH] = 43; // 519[0]

	gpBuffer[2 * MICRO_WIDTH +  0 + (MICRO_HEIGHT * 1 +  7) * BUFFER_WIDTH] = 43; // 368[1]
	gpBuffer[2 * MICRO_WIDTH +  0 + (MICRO_HEIGHT * 1 +  9) * BUFFER_WIDTH] = 41;

	gpBuffer[2 * MICRO_WIDTH + 31 + (MICRO_HEIGHT * 2 + 13) * BUFFER_WIDTH] = 41; // 162[2]
	gpBuffer[2 * MICRO_WIDTH + 31 + (MICRO_HEIGHT * 2 + 14) * BUFFER_WIDTH] = 36;
	gpBuffer[2 * MICRO_WIDTH + 31 + (MICRO_HEIGHT * 2 + 18) * BUFFER_WIDTH] = 36;
	gpBuffer[2 * MICRO_WIDTH + 30 + (MICRO_HEIGHT * 2 + 19) * BUFFER_WIDTH] = 35;
	gpBuffer[2 * MICRO_WIDTH + 31 + (MICRO_HEIGHT * 2 + 20) * BUFFER_WIDTH] = 45;
	gpBuffer[2 * MICRO_WIDTH + 31 + (MICRO_HEIGHT * 2 + 21) * BUFFER_WIDTH] = 63;
	gpBuffer[2 * MICRO_WIDTH + 31 + (MICRO_HEIGHT * 2 + 22) * BUFFER_WIDTH] = 40;
	gpBuffer[2 * MICRO_WIDTH + 31 + (MICRO_HEIGHT * 2 + 29) * BUFFER_WIDTH] = 36;

	gpBuffer[3 * MICRO_WIDTH +  0 + (MICRO_HEIGHT * 0 + 19) * BUFFER_WIDTH] = 91; // 63[4]
	gpBuffer[3 * MICRO_WIDTH +  0 + (MICRO_HEIGHT * 0 + 20) * BUFFER_WIDTH] = 93;

	gpBuffer[3 * MICRO_WIDTH +  26 + (MICRO_HEIGHT * 2 + 4) * BUFFER_WIDTH] = 45; // 206[0]

	for (yy = 13; yy < 16; yy++) {
		for (xx = 2; xx < 8; xx++) {
			if (yy > 14 - (xx - 2) / 2) {
				BYTE color = 43;
				if (yy == 14 - (xx - 2) / 2) {
					if ((xx & 1) == 0) {
						color = 44;
					} else if (xx == 7) {
						color = 77;
					}
				}
				gpBuffer[3 * MICRO_WIDTH + xx + (MICRO_HEIGHT * 1 + yy) * BUFFER_WIDTH] = color; // 450[0]
			}
		}
	}

	// create the new CEL file
	BYTE* resCelBuf = DiabloAllocPtr(*celLen + lengthof(micros) * MICRO_WIDTH * MICRO_HEIGHT);

	CelFrameEntry entries[lengthof(micros)];
	xx = 0, yy = MICRO_HEIGHT - 1;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		entries[i].type = i;
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

typedef struct {
	unsigned subtileIndex;
	unsigned microIndex;
} CelMicro;
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
	memset(&gpBuffer[0], TRANS_COLOR, 4 * BUFFER_WIDTH * MICRO_HEIGHT);

	unsigned xx = 0, yy = MICRO_HEIGHT - 1;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
		RenderMicro(&gpBuffer[xx + yy * BUFFER_WIDTH], SwapLE16(pSubtiles[index]), DMT_NONE);
		yy += MICRO_HEIGHT;
		if (yy == 5 * MICRO_HEIGHT - 1) {
			yy = MICRO_HEIGHT - 1;
			xx += MICRO_WIDTH;
		}
	}

	// mask the black pixels
	xx = 0, yy = 0;
	for (int i = 0; i < lengthof(micros); i++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 0; x < MICRO_WIDTH; x++) {
				BYTE *pixel = &gpBuffer[xx + x + (yy + y) * BUFFER_WIDTH];
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
		if (yy == 4 * MICRO_HEIGHT) {
			yy = 0;
			xx += MICRO_WIDTH;
		}
	}

	// create the new CEL file
	BYTE* resCelBuf = DiabloAllocPtr(*celLen + lengthof(micros) * MICRO_WIDTH * MICRO_HEIGHT);

	CelFrameEntry entries[lengthof(micros)];
	xx = 0, yy = MICRO_HEIGHT - 1;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		entries[i].type = i;
		entries[i].encoding = MET_TRANSPARENT;
		unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
		entries[i].frameRef = SwapLE16(pSubtiles[index]) & 0xFFF;
		entries[i].frameSrc = &gpBuffer[xx + yy * BUFFER_WIDTH];
		yy += MICRO_HEIGHT;
		if (yy == 5 * MICRO_HEIGHT - 1) {
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
	typedef struct {
		int subtileIndex;
		unsigned microIndex;
		unsigned res_encoding;
	} CelMicro;
	const CelMicro micros[] = {
		{ 626 - 1, 0, MET_LTRIANGLE },   // 1806 - 205
		{ 626 - 1, 1, MET_RTRIANGLE },   // 1807
		{ -1, 0, 0 }, // { 627 - 1, 0, MET_LTRIANGLE },   // 1808
		{ 638 - 1, 1, MET_RTRIANGLE },   // 1824 - 211
		{ 639 - 1, 0, MET_LTRIANGLE },   // 1825
		{ 639 - 1, 1, MET_RTRIANGLE },   // 1799
		{ -1, 0, 0 }, // { 631 - 1, 1, MET_RTRIANGLE },   // 1815 - 207
		{ 634 - 1, 0, MET_LTRIANGLE },   // 1818 - 208
		{ 634 - 1, 1, MET_RTRIANGLE },   // 1819

		{ 277 - 1, 1, MET_TRANSPARENT }, // 722 - 96
		{ 303 - 1, 1, MET_RTRIANGLE },   // 797
		{ -1, 0, 0 },

		{ -1, 0, 0 }, // { 630 - 1, 0, MET_TRANSPARENT }, // 1804 - '111'
		{ 620 - 1, 0, MET_RTRIANGLE },   // 1798 - '109'
		{ 621 - 1, 1, MET_SQUARE },      // 1800
		{ 625 - 1, 0, MET_RTRIANGLE },   // 1805 - '215'
		{ 624 - 1, 0, MET_TRANSPARENT }, // 1804
		{ 619 - 1, 1, MET_LTRAPEZOID },  // 1797 - '109' + '215'

		{ 31 - 1, 0, 0 },                // 152
		{ 15 - 1, 1, 0 },                // 14
		{ 15 - 1, 2, 0 },                // 12
		{ 89 - 1, 1, 0 },                // 311
		{ 89 - 1, 2, 0 },                // 309
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
	memset(&gpBuffer[0], TRANS_COLOR, 3 * BUFFER_WIDTH * MICRO_HEIGHT);

	unsigned xx = 0, yy = MICRO_HEIGHT - 1;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		if (micro.subtileIndex >= 0) {
			unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
			RenderMicro(&gpBuffer[xx + yy * BUFFER_WIDTH], SwapLE16(pSubtiles[index]), DMT_NONE);
		}
		yy += MICRO_HEIGHT;
		if (yy == 4 * MICRO_HEIGHT - 1) {
			yy = MICRO_HEIGHT - 1;
			xx += MICRO_WIDTH;
		}
	}

	// extend the shadows to NE: 205[2][0, 1], 205[3][0]
	for (int i = 0; i < 2; i++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 0; x < MICRO_WIDTH; x++) {
				BYTE *pixel = &gpBuffer[0 * MICRO_WIDTH + x + (i * MICRO_HEIGHT + y) * BUFFER_WIDTH];
				if (*pixel == TRANS_COLOR) {
					continue;
				}
				if (i == 0 && *pixel != 79 && y <= (x / 2) + 13 - MICRO_HEIGHT / 2) { // 1806
					continue;
				}
				if (i == 1 && *pixel != 79 && y <= (x / 2) + 13) { // 1807
					continue;
				}
				//if (i == 2 && *pixel != 79 && y <= (x / 2) + 13 - MICRO_HEIGHT / 2) { // 1808
				//	continue;
				//}
				*pixel = SHADOW_COLOR;
			}
		}
	}
	// extend the shadows to NW: 211[0][1], 211[1][0]
	for (int i = 0; i < 3; i++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 0; x < MICRO_WIDTH; x++) {
				BYTE *pixel = &gpBuffer[1 * MICRO_WIDTH + x + (i * MICRO_HEIGHT + y) * BUFFER_WIDTH];
				if (*pixel == TRANS_COLOR) {
					continue;
				}
				if (i == 0 && *pixel != 79 && y <= 13 - (x / 2)) { // 1824
					continue;
				}
				if (i == 1 && *pixel != 79 && (x > 19 || y > 23) && (x != 16 || y != 24)) { // 1825
					continue;
				}
				if (i == 2 && *pixel != 79 && x <= 7) { // 1799
					continue;
				}
				*pixel = SHADOW_COLOR;
			}
		}
	}
	// extend the shadows to NW: 207[2][1]
	/*for (int i = 0; i < 1; i++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 0; x < MICRO_WIDTH; x++) {
				BYTE *pixel = &gpBuffer[2 * MICRO_WIDTH + x + (i * MICRO_HEIGHT + y) * BUFFER_WIDTH];
				if (*pixel == TRANS_COLOR) {
					continue;
				}
				if (i == 0 && *pixel != 79 && y <= (x / 2) + 15) { // 1815
					continue;
				}
				*pixel = SHADOW_COLOR;
			}
		}
	}*/
	// extend the shadows to NE: 208[2][0, 1]
	for (int i = 1; i < 3; i++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 0; x < MICRO_WIDTH; x++) {
				BYTE *pixel = &gpBuffer[2 * MICRO_WIDTH + x + (i * MICRO_HEIGHT + y) * BUFFER_WIDTH];
				if (*pixel == TRANS_COLOR) {
					continue;
				}
				if (i == 1 && *pixel != 79
				&& (y <= (x / 2) - 3 || (x >= 20 && y >= 14 && *pixel >= 59 && *pixel <= 95 && (*pixel >= 77 || *pixel <= 63)))) { // 1818
					continue;
				}
				if (i == 2 && *pixel != 79
				&& (y <= (x / 2) + 13 || (x <= 8 && y >= 12 && *pixel >= 62 && *pixel <= 95 && (*pixel >= 80 || *pixel <= 63)))) { // 1819
					continue;
				}
				*pixel = SHADOW_COLOR;
			}
		}
	}
	// use consistent lava + shadow micro II.
	for (int i = 0; i < 1; i++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 0; x < MICRO_WIDTH; x++) {
				BYTE *pixel = &gpBuffer[3 * MICRO_WIDTH + x + (i * MICRO_HEIGHT + y) * BUFFER_WIDTH];
				if (*pixel == TRANS_COLOR) {
					continue;
				}
				BYTE pixelSrc = gpBuffer[3 * MICRO_WIDTH + x + ((i + 1) * MICRO_HEIGHT + y) * BUFFER_WIDTH];
				if (pixelSrc == TRANS_COLOR) {
					continue;
				}
				if (i == 0) { // 722
					if (x > 11) {
						continue;
					}
					if (x == 11 && (y < 9 || (y >= 17 && y <= 20))) {
						continue;
					}
					if (x == 10 && (y == 18 || y == 19)) {
						continue;
					}
				}
				*pixel = pixelSrc;
			}
		}
	}
	// draw the new micros
	for (int i = 0; i < 6; i++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			for (int x = 0; x < MICRO_WIDTH; x++) {
				BYTE *pixel = &gpBuffer[(4 + i / 3) * MICRO_WIDTH + x + ((i % 3) * MICRO_HEIGHT + y) * BUFFER_WIDTH];
				BYTE srcPixel = gpBuffer[(4 + i / 3 + 2) * MICRO_WIDTH + x + ((i % 3) * MICRO_HEIGHT + y) * BUFFER_WIDTH];
				if (i == 5) { // 1797
					srcPixel = SHADOW_COLOR;
				}
				if (i == 0) { // 1804
					/*// mask with 626[0]
					BYTE maskPixel = gpBuffer[0 * MICRO_WIDTH + x + (0 * MICRO_HEIGHT + y) * BUFFER_WIDTH];
					if (maskPixel == SHADOW_COLOR) {
						srcPixel = maskPixel;
					}*/
					continue;
				}
				if (i == 1) { // 1798
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
				if (i == 3) { // 1805
					// grate/floor in shadow
					if (x <= 1 && y >= 7 * x) {
						srcPixel = SHADOW_COLOR;
					}
					if (x > 1 && y > 14 + (x - 1) / 2) {
						srcPixel = SHADOW_COLOR;
					}
				}
				if (i == 2 || i == 4) { // 1800, 1804
					// wall/grate in shadow
					if (y >= 7 * (x - 27) && srcPixel != TRANS_COLOR) {
						srcPixel = SHADOW_COLOR;
					}
				}
				*pixel = srcPixel;
			}
		}
	}

	// fix bad artifacts
	gpBuffer[2 * MICRO_WIDTH + 22 + (MICRO_HEIGHT * 1 + 20) * BUFFER_WIDTH] = TRANS_COLOR;     // 1818

	// create the new CEL file
	BYTE* resCelBuf = DiabloAllocPtr(*celLen + lengthof(micros) * MICRO_WIDTH * MICRO_HEIGHT);

	CelFrameEntry entries[lengthof(micros) - (5 + 4)];
	xx = 0, yy = MICRO_HEIGHT - 1;
	int idx = 0;
	for (int i = 0; i < lengthof(micros) - 5; i++) {
		const CelMicro &micro = micros[i];
		if (micro.subtileIndex >= 0) {
			entries[idx].type = idx;
			entries[idx].encoding = micro.res_encoding;
			unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
			entries[idx].frameRef = SwapLE16(pSubtiles[index]) & 0xFFF;
			entries[idx].frameSrc = &gpBuffer[xx + yy * BUFFER_WIDTH];
			idx++;
		}
		yy += MICRO_HEIGHT;
		if (yy == 4 * MICRO_HEIGHT - 1) {
			yy = MICRO_HEIGHT - 1;
			xx += MICRO_WIDTH;
		}
	}

	*celLen = encodeCelMicros(entries, idx, resCelBuf, celBuf, TRANS_COLOR);

	mem_free_dbg(celBuf);

	return resCelBuf;
}

static void patchCryptTil(BYTE* buf)
{
	uint16_t *pTiles = (uint16_t*)buf;
	// fix automap of the entrance I.
	pTiles[4 * (52 - 1) + 0] = SwapLE16(73 - 1); // 45 - copy from 23
	pTiles[4 * (52 - 1) + 1] = SwapLE16(64 - 1); // 46
	pTiles[4 * (52 - 1) + 2] = SwapLE16(65 - 1); // 148
	pTiles[4 * (52 - 1) + 3] = SwapLE16(66 - 1); // 48
	pTiles[4 * (58 - 1) + 0] = SwapLE16(63 - 1); // 166 - copy from 18
	pTiles[4 * (58 - 1) + 1] = SwapLE16(64 - 1); // 167
	pTiles[4 * (58 - 1) + 2] = SwapLE16(65 - 1); // 47
	pTiles[4 * (58 - 1) + 3] = SwapLE16(66 - 1); // 48
	pTiles[4 * (53 - 1) + 1] = SwapLE16(148 - 1); // 130
	pTiles[4 * (53 - 1) + 3] = SwapLE16(148 - 1); // 130
	// use common subtiles of doors
	pTiles[4 * (71 - 1) + 2] = SwapLE16(206 - 1);
	pTiles[4 * (72 - 1) + 2] = SwapLE16(206 - 1);
	// use common subtiles
	pTiles[4 * (4 - 1) + 1] = SwapLE16(6 - 1); // 14
	pTiles[4 * (14 - 1) + 1] = SwapLE16(6 - 1);
	pTiles[4 * (115 - 1) + 1] = SwapLE16(6 - 1);
	pTiles[4 * (132 - 1) + 1] = SwapLE16(6 - 1);
	pTiles[4 * (1 - 1) + 2] = SwapLE16(15 - 1); // 3
	pTiles[4 * (27 - 1) + 2] = SwapLE16(15 - 1);
	pTiles[4 * (43 - 1) + 2] = SwapLE16(15 - 1);
	pTiles[4 * (79 - 1) + 2] = SwapLE16(15 - 1);
	pTiles[4 * (6 - 1) + 2] = SwapLE16(15 - 1);  // 23
	pTiles[4 * (127 - 1) + 2] = SwapLE16(4 - 1); // 372
	pTiles[4 * (132 - 1) + 2] = SwapLE16(15 - 1); // 388
	pTiles[4 * (156 - 1) + 2] = SwapLE16(31 - 1); // 468
	// use better subtiles
	// - increase glow
	pTiles[4 * (96 - 1) + 3] = SwapLE16(293 - 1); // 279
	pTiles[4 * (187 - 1) + 3] = SwapLE16(293 - 1);
	pTiles[4 * (188 - 1) + 3] = SwapLE16(293 - 1);
	pTiles[4 * (90 - 1) + 1] = SwapLE16(297 - 1); // 253
	pTiles[4 * (175 - 1) + 1] = SwapLE16(297 - 1);
	// - reduce glow
	pTiles[4 * (162 - 1) + 1] = SwapLE16(297 - 1); // 489
	pTiles[4 * (162 - 1) + 2] = SwapLE16(266 - 1); // 490
	// create the new shadows
	// - use the shadows created by fixCryptShadows
	pTiles[4 * (203 - 1) + 0] = SwapLE16(638 - 1); // 619
	pTiles[4 * (203 - 1) + 1] = SwapLE16(639 - 1); // 620
	pTiles[4 * (203 - 1) + 2] = SwapLE16(623 - 1); // 47
	pTiles[4 * (203 - 1) + 3] = SwapLE16(627 - 1); // 621
	pTiles[4 * (204 - 1) + 0] = SwapLE16(638 - 1); // 622
	pTiles[4 * (204 - 1) + 1] = SwapLE16(639 - 1); // 46
	pTiles[4 * (204 - 1) + 2] = SwapLE16(636 - 1); // 623
	pTiles[4 * (204 - 1) + 3] = SwapLE16(627 - 1); // 624
	pTiles[4 * (108 - 1) + 2] = SwapLE16(631 - 1); // 810
	pTiles[4 * (108 - 1) + 3] = SwapLE16(626 - 1); // 811
	pTiles[4 * (210 - 1) + 3] = SwapLE16(371 - 1); // 637

	pTiles[4 * (109 - 1) + 0] = SwapLE16(1 - 1);   // 312
	pTiles[4 * (109 - 1) + 1] = SwapLE16(2 - 1);   // 313
	pTiles[4 * (109 - 1) + 2] = SwapLE16(3 - 1);   // 314
	pTiles[4 * (109 - 1) + 3] = SwapLE16(627 - 1); // 315
	pTiles[4 * (110 - 1) + 0] = SwapLE16(21 - 1);  // 316
	pTiles[4 * (110 - 1) + 1] = SwapLE16(22 - 1);  // 313
	pTiles[4 * (110 - 1) + 2] = SwapLE16(3 - 1);   // 314
	pTiles[4 * (110 - 1) + 3] = SwapLE16(627 - 1); // 315
	pTiles[4 * (111 - 1) + 0] = SwapLE16(39 - 1);  // 317
	pTiles[4 * (111 - 1) + 1] = SwapLE16(4 - 1);   // 318
	pTiles[4 * (111 - 1) + 2] = SwapLE16(242 - 1); // 319
	pTiles[4 * (111 - 1) + 3] = SwapLE16(627 - 1); // 320
	pTiles[4 * (215 - 1) + 0] = SwapLE16(101 - 1); // 645
	pTiles[4 * (215 - 1) + 1] = SwapLE16(4 - 1);   // 646
	pTiles[4 * (215 - 1) + 2] = SwapLE16(178 - 1); // 45
	pTiles[4 * (215 - 1) + 3] = SwapLE16(627 - 1); // 647
	// - 'add' new shadow-types with glow TODO: add wall/grate+glow
	pTiles[4 * (216 - 1) + 0] = SwapLE16(39 - 1);  // 622
	pTiles[4 * (216 - 1) + 1] = SwapLE16(4 - 1);   // 46
	pTiles[4 * (216 - 1) + 2] = SwapLE16(238 - 1); // 648
	pTiles[4 * (216 - 1) + 3] = SwapLE16(635 - 1); // 624
	pTiles[4 * (217 - 1) + 0] = SwapLE16(638 - 1); // 625
	pTiles[4 * (217 - 1) + 1] = SwapLE16(639 - 1); // 46
	pTiles[4 * (217 - 1) + 2] = SwapLE16(634 - 1); // 649 TODO: could be better
	pTiles[4 * (217 - 1) + 3] = SwapLE16(635 - 1); // 650
	// - 'add' new shadow-types with horizontal arches
	pTiles[4 * (71 - 1) + 0] = SwapLE16(5 - 1); // copy from tile 2
	pTiles[4 * (71 - 1) + 1] = SwapLE16(6 - 1);
	pTiles[4 * (71 - 1) + 2] = SwapLE16(631 - 1);
	pTiles[4 * (71 - 1) + 3] = SwapLE16(627 - 1);
	pTiles[4 * (80 - 1) + 0] = SwapLE16(5 - 1); // copy from tile 2
	pTiles[4 * (80 - 1) + 1] = SwapLE16(6 - 1);
	pTiles[4 * (80 - 1) + 2] = SwapLE16(623 - 1);
	pTiles[4 * (80 - 1) + 3] = SwapLE16(627 - 1);

	pTiles[4 * (81 - 1) + 0] = SwapLE16(42 - 1); // copy from tile 12
	pTiles[4 * (81 - 1) + 1] = SwapLE16(34 - 1);
	pTiles[4 * (81 - 1) + 2] = SwapLE16(631 - 1);
	pTiles[4 * (81 - 1) + 3] = SwapLE16(627 - 1);
	pTiles[4 * (82 - 1) + 0] = SwapLE16(42 - 1); // copy from tile 12
	pTiles[4 * (82 - 1) + 1] = SwapLE16(34 - 1);
	pTiles[4 * (82 - 1) + 2] = SwapLE16(623 - 1);
	pTiles[4 * (82 - 1) + 3] = SwapLE16(627 - 1);

	pTiles[4 * (83 - 1) + 0] = SwapLE16(104 - 1); // copy from tile 36
	pTiles[4 * (83 - 1) + 1] = SwapLE16(84 - 1);
	pTiles[4 * (83 - 1) + 2] = SwapLE16(631 - 1);
	pTiles[4 * (83 - 1) + 3] = SwapLE16(627 - 1);
	pTiles[4 * (84 - 1) + 0] = SwapLE16(104 - 1); // copy from tile 36
	pTiles[4 * (84 - 1) + 1] = SwapLE16(84 - 1);
	pTiles[4 * (84 - 1) + 2] = SwapLE16(623 - 1);
	pTiles[4 * (84 - 1) + 3] = SwapLE16(627 - 1);

	pTiles[4 * (85 - 1) + 0] = SwapLE16(25 - 1); // copy from tile 7
	pTiles[4 * (85 - 1) + 1] = SwapLE16(6 - 1);
	pTiles[4 * (85 - 1) + 2] = SwapLE16(631 - 1);
	pTiles[4 * (85 - 1) + 3] = SwapLE16(627 - 1);
	pTiles[4 * (86 - 1) + 0] = SwapLE16(25 - 1); // copy from tile 7
	pTiles[4 * (86 - 1) + 1] = SwapLE16(6 - 1);
	pTiles[4 * (86 - 1) + 2] = SwapLE16(623 - 1);
	pTiles[4 * (86 - 1) + 3] = SwapLE16(627 - 1);

	pTiles[4 * (87 - 1) + 0] = SwapLE16(79 - 1); // copy from tile 26
	pTiles[4 * (87 - 1) + 1] = SwapLE16(80 - 1);
	pTiles[4 * (87 - 1) + 2] = SwapLE16(623 - 1);
	pTiles[4 * (87 - 1) + 3] = SwapLE16(627 - 1);
	pTiles[4 * (88 - 1) + 0] = SwapLE16(79 - 1); // copy from tile 26
	pTiles[4 * (88 - 1) + 1] = SwapLE16(80 - 1);
	pTiles[4 * (88 - 1) + 2] = SwapLE16(631 - 1);
	pTiles[4 * (88 - 1) + 3] = SwapLE16(627 - 1);

	// use common subtiles instead of minor alterations
	pTiles[4 * (7 - 1) + 1] = SwapLE16(6 - 1); // 26
	pTiles[4 * (159 - 1) + 1] = SwapLE16(6 - 1); // 479
	pTiles[4 * (133 - 1) + 2] = SwapLE16(31 - 1); // 390
	pTiles[4 * (10 - 1) + 1] = SwapLE16(18 - 1); // 37
	pTiles[4 * (138 - 1) + 1] = SwapLE16(18 - 1);
	pTiles[4 * (188 - 1) + 1] = SwapLE16(277 - 1); // 564
	pTiles[4 * (178 - 1) + 2] = SwapLE16(258 - 1); // 543
	pTiles[4 * (5 - 1) + 2] = SwapLE16(31 - 1); // 19
	pTiles[4 * (14 - 1) + 2] = SwapLE16(31 - 1);
	pTiles[4 * (159 - 1) + 2] = SwapLE16(31 - 1);
	pTiles[4 * (185 - 1) + 2] = SwapLE16(274 - 1); // 558
	pTiles[4 * (186 - 1) + 2] = SwapLE16(274 - 1); // 560
	pTiles[4 * (139 - 1) + 0] = SwapLE16(39 - 1); // 402

	pTiles[4 * (2 - 1) + 3] = SwapLE16(4 - 1); // 8
	pTiles[4 * (3 - 1) + 1] = SwapLE16(60 - 1); // 10
	pTiles[4 * (114 - 1) + 1] = SwapLE16(32 - 1);
	pTiles[4 * (3 - 1) + 2] = SwapLE16(4 - 1); // 11
	pTiles[4 * (114 - 1) + 2] = SwapLE16(4 - 1);
	pTiles[4 * (5 - 1) + 3] = SwapLE16(7 - 1); // 20
	pTiles[4 * (14 - 1) + 3] = SwapLE16(4 - 1);
	pTiles[4 * (133 - 1) + 3] = SwapLE16(4 - 1);
	pTiles[4 * (125 - 1) + 3] = SwapLE16(7 - 1); // 50
	pTiles[4 * (159 - 1) + 3] = SwapLE16(7 - 1);
	pTiles[4 * (4 - 1) + 3] = SwapLE16(7 - 1); // 16
	pTiles[4 * (132 - 1) + 3] = SwapLE16(4 - 1);
	pTiles[4 * (10 - 1) + 3] = SwapLE16(7 - 1); // 38
	pTiles[4 * (138 - 1) + 3] = SwapLE16(4 - 1);
	pTiles[4 * (121 - 1) + 3] = SwapLE16(4 - 1); // 354
	pTiles[4 * (8 - 1) + 3] = SwapLE16(4 - 1); // 32
	pTiles[4 * (136 - 1) + 3] = SwapLE16(7 - 1);
	pTiles[4 * (91 - 1) + 1] = SwapLE16(47 - 1); // 257
	pTiles[4 * (178 - 1) + 1] = SwapLE16(47 - 1);
	pTiles[4 * (91 - 1) + 3] = SwapLE16(48 - 1); // 259
	pTiles[4 * (177 - 1) + 3] = SwapLE16(7 - 1);
	pTiles[4 * (178 - 1) + 3] = SwapLE16(48 - 1);
	pTiles[4 * (130 - 1) + 2] = SwapLE16(395 - 1); // 381
	pTiles[4 * (157 - 1) + 2] = SwapLE16(4 - 1);   // 472
	pTiles[4 * (177 - 1) + 1] = SwapLE16(4 - 1);   // 540
	pTiles[4 * (211 - 1) + 3] = SwapLE16(48 - 1);  // 621
	pTiles[4 * (205 - 1) + 0] = SwapLE16(45 - 1);  // 625
	pTiles[4 * (207 - 1) + 0] = SwapLE16(45 - 1);  // 630
	pTiles[4 * (207 - 1) + 3] = SwapLE16(627 - 1); // 632
	pTiles[4 * (208 - 1) + 0] = SwapLE16(45 - 1);  // 633

	pTiles[4 * (27 - 1) + 3] = SwapLE16(4 - 1); // 85
	// pTiles[4 * (28 - 1) + 3] = SwapLE16(4 - 1); // 87
	pTiles[4 * (29 - 1) + 3] = SwapLE16(4 - 1); // 90
	pTiles[4 * (30 - 1) + 3] = SwapLE16(4 - 1); // 92
	pTiles[4 * (31 - 1) + 3] = SwapLE16(4 - 1); // 94
	pTiles[4 * (32 - 1) + 3] = SwapLE16(4 - 1); // 96
	pTiles[4 * (33 - 1) + 3] = SwapLE16(4 - 1); // 98
	pTiles[4 * (34 - 1) + 3] = SwapLE16(4 - 1); // 100
	pTiles[4 * (37 - 1) + 3] = SwapLE16(4 - 1); // 108
	pTiles[4 * (38 - 1) + 3] = SwapLE16(4 - 1); // 110
	pTiles[4 * (39 - 1) + 3] = SwapLE16(4 - 1); // 112
	pTiles[4 * (40 - 1) + 3] = SwapLE16(4 - 1); // 114
	pTiles[4 * (41 - 1) + 3] = SwapLE16(4 - 1); // 116
	pTiles[4 * (42 - 1) + 3] = SwapLE16(4 - 1); // 118
	pTiles[4 * (43 - 1) + 3] = SwapLE16(4 - 1); // 120
	pTiles[4 * (44 - 1) + 3] = SwapLE16(4 - 1); // 122
	pTiles[4 * (45 - 1) + 3] = SwapLE16(4 - 1); // 124
	// pTiles[4 * (71 - 1) + 3] = SwapLE16(4 - 1); // 214
	pTiles[4 * (72 - 1) + 3] = SwapLE16(4 - 1); // 217
	pTiles[4 * (73 - 1) + 3] = SwapLE16(4 - 1); // 219
	pTiles[4 * (74 - 1) + 3] = SwapLE16(4 - 1); // 221
	pTiles[4 * (75 - 1) + 3] = SwapLE16(4 - 1); // 223
	pTiles[4 * (76 - 1) + 3] = SwapLE16(4 - 1); // 225
	pTiles[4 * (77 - 1) + 3] = SwapLE16(4 - 1); // 227
	pTiles[4 * (78 - 1) + 3] = SwapLE16(4 - 1); // 229
	pTiles[4 * (79 - 1) + 3] = SwapLE16(4 - 1); // 231
	// pTiles[4 * (80 - 1) + 3] = SwapLE16(4 - 1); // 233
	// pTiles[4 * (81 - 1) + 3] = SwapLE16(4 - 1); // 235
	pTiles[4 * (15 - 1) + 1] = SwapLE16(4 - 1); // 52
	pTiles[4 * (15 - 1) + 2] = SwapLE16(4 - 1); // 53
	pTiles[4 * (15 - 1) + 3] = SwapLE16(4 - 1); // 54
	pTiles[4 * (16 - 1) + 1] = SwapLE16(4 - 1); // 56
	pTiles[4 * (144 - 1) + 1] = SwapLE16(4 - 1);
	pTiles[4 * (16 - 1) + 2] = SwapLE16(4 - 1); // 57
	pTiles[4 * (16 - 1) + 3] = SwapLE16(4 - 1); // 58
	pTiles[4 * (144 - 1) + 3] = SwapLE16(7 - 1);
	pTiles[4 * (94 - 1) + 2] = SwapLE16(60 - 1); // 270
	pTiles[4 * (183 - 1) + 2] = SwapLE16(60 - 1);
	pTiles[4 * (184 - 1) + 2] = SwapLE16(60 - 1);
	pTiles[4 * (17 - 1) + 2] = SwapLE16(4 - 1); // 61
	pTiles[4 * (128 - 1) + 2] = SwapLE16(4 - 1);
	pTiles[4 * (92 - 1) + 2] = SwapLE16(62 - 1); // 262
	pTiles[4 * (179 - 1) + 2] = SwapLE16(62 - 1);
	pTiles[4 * (25 - 1) + 1] = SwapLE16(4 - 1); // 76
	pTiles[4 * (25 - 1) + 3] = SwapLE16(4 - 1); // 78
	pTiles[4 * (35 - 1) + 1] = SwapLE16(4 - 1); // 102
	pTiles[4 * (35 - 1) + 3] = SwapLE16(4 - 1); // 103
	pTiles[4 * (69 - 1) + 1] = SwapLE16(4 - 1); // 205
	pTiles[4 * (69 - 1) + 3] = SwapLE16(4 - 1); // 207
	pTiles[4 * (26 - 1) + 2] = SwapLE16(4 - 1); // 81
	pTiles[4 * (26 - 1) + 3] = SwapLE16(4 - 1); // 82
	pTiles[4 * (36 - 1) + 2] = SwapLE16(4 - 1); // 105
	pTiles[4 * (36 - 1) + 3] = SwapLE16(4 - 1); // 106
	pTiles[4 * (46 - 1) + 2] = SwapLE16(4 - 1); // 127
	pTiles[4 * (46 - 1) + 3] = SwapLE16(4 - 1); // 128
	pTiles[4 * (70 - 1) + 2] = SwapLE16(4 - 1); // 210
	pTiles[4 * (70 - 1) + 3] = SwapLE16(4 - 1); // 211
	pTiles[4 * (49 - 1) + 1] = SwapLE16(4 - 1); // 137
	pTiles[4 * (167 - 1) + 1] = SwapLE16(4 - 1);
	pTiles[4 * (49 - 1) + 2] = SwapLE16(4 - 1); // 138
	pTiles[4 * (167 - 1) + 2] = SwapLE16(4 - 1);
	pTiles[4 * (49 - 1) + 3] = SwapLE16(4 - 1); // 139
	pTiles[4 * (167 - 1) + 3] = SwapLE16(4 - 1);
	pTiles[4 * (50 - 1) + 1] = SwapLE16(4 - 1); // 141
	pTiles[4 * (50 - 1) + 3] = SwapLE16(4 - 1); // 143
	pTiles[4 * (51 - 1) + 3] = SwapLE16(4 - 1); // 147
	pTiles[4 * (103 - 1) + 1] = SwapLE16(4 - 1); // 295
	pTiles[4 * (105 - 1) + 1] = SwapLE16(4 - 1);
	pTiles[4 * (127 - 1) + 3] = SwapLE16(4 - 1); // 373
	pTiles[4 * (89 - 1) + 3] = SwapLE16(4 - 1); // 251
	pTiles[4 * (173 - 1) + 3] = SwapLE16(7 - 1);
	pTiles[4 * (174 - 1) + 3] = SwapLE16(7 - 1);
	pTiles[4 * (6 - 1) + 3] = SwapLE16(4 - 1); // 24
	pTiles[4 * (134 - 1) + 3] = SwapLE16(7 - 1);
	pTiles[4 * (7 - 1) + 3] = SwapLE16(7 - 1); // 28
	pTiles[4 * (8 - 1) + 1] = SwapLE16(2 - 1); // 30
	pTiles[4 * (30 - 1) + 1] = SwapLE16(2 - 1);
	pTiles[4 * (32 - 1) + 1] = SwapLE16(2 - 1);
	pTiles[4 * (72 - 1) + 1] = SwapLE16(2 - 1);
	pTiles[4 * (9 - 1) + 3] = SwapLE16(4 - 1); // 35
	pTiles[4 * (137 - 1) + 3] = SwapLE16(4 - 1);
	pTiles[4 * (11 - 1) + 1] = SwapLE16(4 - 1); // 40
	pTiles[4 * (122 - 1) + 1] = SwapLE16(4 - 1);
	pTiles[4 * (12 - 1) + 2] = SwapLE16(4 - 1); // 43
	pTiles[4 * (123 - 1) + 2] = SwapLE16(4 - 1);
	pTiles[4 * (12 - 1) + 3] = SwapLE16(4 - 1); // 44
	pTiles[4 * (123 - 1) + 3] = SwapLE16(7 - 1);
	pTiles[4 * (95 - 1) + 1] = SwapLE16(4 - 1); // 273
	pTiles[4 * (185 - 1) + 1] = SwapLE16(7 - 1);
	pTiles[4 * (186 - 1) + 1] = SwapLE16(4 - 1);
	pTiles[4 * (89 - 1) + 1] = SwapLE16(293 - 1); // 249
	pTiles[4 * (173 - 1) + 1] = SwapLE16(293 - 1);
	pTiles[4 * (174 - 1) + 1] = SwapLE16(293 - 1);
	pTiles[4 * (92 - 1) + 3] = SwapLE16(271 - 1); // 263
	pTiles[4 * (179 - 1) + 3] = SwapLE16(271 - 1);
	pTiles[4 * (96 - 1) + 2] = SwapLE16(12 - 1); // 278
	pTiles[4 * (187 - 1) + 2] = SwapLE16(12 - 1);
	pTiles[4 * (188 - 1) + 2] = SwapLE16(12 - 1);
	// eliminate subtiles of unused tiles
	const int unusedTiles[] = {
		28,/* 29,*/ 30, 31, 34,/* 38,*/ 39, 40, 41, 42,/* 43, 44, */ 61, 62, 63, 64, 65, 66, 67, 68, 72, 73, 74, 75, 76, 77, 78, 79, 212, 213, 214
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
#endif // HELLFIRE

static BYTE* patchHellCel(const BYTE* tilBuf, size_t tilLen, const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen, int exitTileIndex)
{
	const uint16_t* pTiles = (const uint16_t*)tilBuf;
	// TODO: check tilLen
	int topLeftSubtileIndex = SwapLE16(pTiles[exitTileIndex * 4 + 0]);
	int topRightSubtileIndex = SwapLE16(pTiles[exitTileIndex * 4 + 1]);
	int bottomLeftSubtileIndex = SwapLE16(pTiles[exitTileIndex * 4 + 2]);
	int bottomRightSubtileIndex = SwapLE16(pTiles[exitTileIndex * 4 + 3]);

	if (topLeftSubtileIndex != (137 - 1) || topRightSubtileIndex != (138 - 1) || bottomLeftSubtileIndex != (139 - 1) || bottomRightSubtileIndex != (140 - 1)) {
		return celBuf; // non-original subtiles -> assume it is already done
	}

	const uint16_t* pSubtiles = (const uint16_t*)minBuf;

	// TODO: check minLen
	const unsigned blockSize = BLOCK_SIZE_L4;
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
		app_warn("Invalid (empty) floor on tile (%d).", exitTileIndex + 1);
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

static BYTE* patchCatacombsStairs(const BYTE* tilBuf, size_t tilLen, const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t *celLen, int backTileIndex1, int backTileIndex2, int stairsSubtileRef1, int stairsSubtileRef2, int stairsExtSubtileRef1, int stairsExtSubtileRef2)
{
	const uint16_t* pTiles = (const uint16_t*)tilBuf;
	// TODO: check tilLen
	int topLeftSubtileIndex = SwapLE16(pTiles[backTileIndex1 * 4 + 0]);
	int bottomLeftSubtileIndex = SwapLE16(pTiles[backTileIndex1 * 4 + 2]);
	int bottomRightSubtileIndex = SwapLE16(pTiles[backTileIndex1 * 4 + 3]);

	constexpr int backSubtileRef0 = 250;
	constexpr int backSubtileRef2 = 251;
	constexpr int backSubtileRef3 = 252;
	constexpr int backSubtileRef0Replacement = 9;
	constexpr int backSubtileRef2Replacement = 11;
	if (topLeftSubtileIndex != (backSubtileRef0 - 1) || bottomLeftSubtileIndex != (backSubtileRef2 - 1) || bottomRightSubtileIndex != (backSubtileRef3 - 1)) {
		return celBuf; // non-original subtiles -> assume it is already done
	}

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

	if (back3_FrameRef0 == 0 || back2_FrameRef1 == 0 || back0_FrameRef0 == 0) {
		mem_free_dbg(celBuf);
		app_warn("The back-stairs tile (%d) has invalid (missing) frames.", backTileIndex1 + 1);
		return NULL;
	}

	if (stairs_FrameRef0 == 0 || stairs_FrameRef2 == 0 || stairs_FrameRef4 == 0 || stairs_FrameRef6 == 0
		|| stairsExt_FrameRef1 == 0 || stairsExt_FrameRef3 == 0 || stairsExt_FrameRef5 == 0) {
		return celBuf;
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
	BYTE* resCelBuf = DiabloAllocPtr(*celLen + 8 * MICRO_WIDTH * MICRO_HEIGHT);

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

static BYTE* patchFile(int index, size_t *dwLen)
{
	BYTE* buf = LoadFileInMem(filesToPatch[index], dwLen);
	if (buf == NULL) {
		app_warn("Unable to open file %s in the mpq.", filesToPatch[index]);
		return NULL;
	}

	switch (index) {
#if ASSET_MPL == 1
	case FILE_TOWN_CEL:
#ifdef HELLFIRE
	case FILE_NTOWN_CEL:
#endif
	{
		// patch dMicroCels - TOWN.CEL
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
		// patch subtiles around the pot of Adria to prevent graphical glitch when a player passes it II.
		buf = patchTownPotCel(minBuf, minLen, buf, dwLen);
		if (buf != NULL) {
			// patch subtiles of the cathedral to fix graphical glitch
			buf = patchTownCathedralCel(minBuf, minLen, buf, dwLen);
			if (buf != NULL) {
				buf = patchTownFloorCel(minBuf, minLen, buf, dwLen);
				if (buf != NULL) {
					buf = patchTownDoorCel(minBuf, minLen, buf, dwLen);
					if (buf != NULL) {
						buf = patchTownChopCel(minBuf, minLen, buf, dwLen);
						if (buf != NULL) {
							patchTownMin(minBuf, index != FILE_TOWN_MIN);
							buf = buildBlkCel(buf, dwLen);
						}
					}
				}
			}
		}
		mem_free_dbg(minBuf);
	} break;
	case FILE_TOWN_MIN:
	{	// patch dMiniTiles - Town.MIN
		if (*dwLen < MICRO_IDX(1219 - 1, BLOCK_SIZE_TOWN, 0) * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		patchTownMin(buf, false);
		buf = buildBlkMin(buf, dwLen, BLOCK_SIZE_TOWN);
	} break;
	case FILE_CATHEDRAL_CEL:
	{	// patch dMicroCels - L1.CEL
		size_t minLen;
		BYTE* minBuf = LoadFileInMem(filesToPatch[FILE_CATHEDRAL_MIN], &minLen);
		if (minBuf == NULL) {
			mem_free_dbg(buf);
			app_warn("Unable to open file %s in the mpq.", filesToPatch[FILE_CATHEDRAL_MIN]);
			return NULL;
		}
		buf = patchCathedralFloorCel(minBuf, minLen, buf, dwLen);
		if (buf != NULL) {
			buf = fixCathedralShadows(minBuf, minLen, buf, dwLen);
			if (buf != NULL) {
				patchCathedralMin(minBuf);
				buf = buildBlkCel(buf, dwLen);
			}
		}
		mem_free_dbg(minBuf);
	} break;
	case FILE_CATHEDRAL_MIN:
	{	// patch dMiniTiles - L1.MIN
		if (*dwLen < MICRO_IDX(140 - 1, BLOCK_SIZE_L1, 1) * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		patchCathedralMin(buf);
		buf = buildBlkMin(buf, dwLen, BLOCK_SIZE_L1);
	} break;
#endif /* ASSET_MPL == 1 */
	case FILE_CATHEDRAL_TIL:
	{	// patch dMegaTiles - L1.TIL
		if (*dwLen < 206 * 4 * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		patchCathedralTil(buf);
	} break;
	case FILE_CATHEDRAL_SOL:
	{	// patch dSolidTable - L1.SOL
		if (*dwLen < 453) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		nMissileTable(8, false); // the only column which was blocking missiles
		// adjust SOL after fixCathedralShadows
		nSolidTable(298, true);
		nSolidTable(304, true);
		// nBlockTable(330, false);
		nBlockTable(334, false);
		nMissileTable(334, false);
		// nBlockTable(339, true);
		// nBlockTable(340, true);
		// nBlockTable(342, true);
		// nBlockTable(343, true);
		// nBlockTable(344, false);
		// - special subtiles for the banner setpiece
		nBlockTable(336, false);
		nMissileTable(336, false);
		nBlockTable(337, false);
		nMissileTable(337, false);
		nBlockTable(338, false);
		nMissileTable(338, false);
		// - special subtile for the vile setmap
		nMissileTable(335, false);
	} break;
	case FILE_CATHEDRAL_AMP:
	{	// patch dAutomapData - L1.AMP
		if (*dwLen < 206 * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}

		uint16_t *automaptype = (uint16_t*)buf;
		// adjust AMP after fixCathedralShadows
		// automaptype[131 - 1] = automaptype[13 - 1];
		// automaptype[132 - 1] = automaptype[13 - 1];
		// automaptype[139 - 1] = automaptype[13 - 1];
		// automaptype[140 - 1] = automaptype[13 - 1];
		// automaptype[141 - 1] = automaptype[13 - 1];
		// automaptype[142 - 1] = automaptype[13 - 1];
		// automaptype[143 - 1] = automaptype[13 - 1];
		// automaptype[144 - 1] = automaptype[13 - 1];
		automaptype[145 - 1] = automaptype[11 - 1];
		// automaptype[146 - 1] = automaptype[1 - 1];
		automaptype[147 - 1] = automaptype[6 - 1];
		// automaptype[148 - 1] = automaptype[2 - 1];
		automaptype[149 - 1] = automaptype[12 - 1];
		automaptype[150 - 1] = automaptype[2 - 1];
		automaptype[151 - 1] = automaptype[12 - 1];
		automaptype[152 - 1] = automaptype[36 - 1];
		// automaptype[153 - 1] = automaptype[36 - 1];
		automaptype[154 - 1] = automaptype[7 - 1];
		automaptype[155 - 1] = automaptype[2 - 1];
		// automaptype[156 - 1] = automaptype[26 - 1];
		automaptype[157 - 1] = automaptype[35 - 1];
		// automaptype[158 - 1] = automaptype[4 - 1];
		automaptype[159 - 1] = automaptype[13 - 1];
		// automaptype[160 - 1] = automaptype[14 - 1];
		// automaptype[161 - 1] = automaptype[37 - 1];
		automaptype[164 - 1] = automaptype[13 - 1];
		automaptype[165 - 1] = automaptype[13 - 1];
		// - shadows for the banner setpiece
		automaptype[56 - 1] = automaptype[1 - 1];
		automaptype[55 - 1] = automaptype[1 - 1];
		automaptype[54 - 1] = automaptype[60 - 1];
		automaptype[53 - 1] = automaptype[58 - 1];
		// - shadows for the vile setmap
		automaptype[52 - 1] = automaptype[2 - 1];
		automaptype[51 - 1] = automaptype[2 - 1];
		automaptype[50 - 1] = automaptype[1 - 1];
		automaptype[49 - 1] = automaptype[17 - 1];
		automaptype[48 - 1] = automaptype[11 - 1];
		automaptype[47 - 1] = automaptype[2 - 1];
		automaptype[46 - 1] = automaptype[7 - 1];
	} break;
	case FILE_BONESTR1_DUN:
	case FILE_BONESTR2_DUN:
	case FILE_BONECHA1_DUN:
	case FILE_BONECHA2_DUN:
	case FILE_BLIND1_DUN:
	case FILE_BLIND2_DUN:
	case FILE_BLOOD1_DUN:
	case FILE_BLOOD2_DUN:
	{	// patch .DUN
		patchDungeon(index, buf, dwLen);
	} break;
#if ASSET_MPL == 1
	case FILE_L2DOORS_SCEL:
	{	// patch L2Doors.CEL
		buf = patchCatacombsDoors(buf, dwLen);
	} break;
	case FILE_CATACOMBS_SCEL:
	{	// patch pSpecialsCel - L2S.CEL
		buf = patchCatacombsSpec(buf, dwLen);
	} break;
	case FILE_CATACOMBS_CEL:
	{	// patch dMicroCels - L2.CEL
		// fix the upstairs I.
		size_t minLen;
		BYTE* minBuf = LoadFileInMem(filesToPatch[FILE_CATACOMBS_MIN], &minLen);
		if (minBuf == NULL) {
			mem_free_dbg(buf);
			app_warn("Unable to open file %s in the mpq.", filesToPatch[FILE_CATACOMBS_MIN]);
			return NULL;
		}
		size_t tilLen;
		BYTE* tilBuf = LoadFileInMem(filesToPatch[FILE_CATACOMBS_TIL], &tilLen);
		if (tilBuf == NULL) {
			mem_free_dbg(minBuf);
			mem_free_dbg(buf);
			app_warn("Unable to open file %s in the mpq.", filesToPatch[FILE_CATACOMBS_TIL]);
			return NULL;
		}
		buf = patchCatacombsStairs(tilBuf, tilLen, minBuf, minLen, buf, dwLen, 72 - 1, 158 - 1, 267, 559, 265, 556);
		if (buf != NULL) {
			buf = patchCatacombsFloorCel(minBuf, minLen, buf, dwLen);
			if (buf != NULL) {
				buf = fixCatacombsShadows(minBuf, minLen, buf, dwLen);
				if (buf != NULL) {
					patchCatacombsMin(minBuf);
					buf = buildBlkCel(buf, dwLen);
				}
			}
		}
		mem_free_dbg(minBuf);
		mem_free_dbg(tilBuf);
	} break;
	case FILE_CATACOMBS_MIN:
	{	// patch dMiniTiles - L2.MIN
		// add separate tiles and subtiles for the arches II.
		constexpr int blockSize = BLOCK_SIZE_L2;
		uint16_t *pSubtiles = (uint16_t*)buf;
		if (*dwLen < 559 * blockSize * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}

		patchCatacombsMin(buf);
		buf = buildBlkMin(buf, dwLen, blockSize);
	} break;
#endif
	case FILE_CATACOMBS_TIL:
	{	// patch dMegaTiles - L2.TIL
		if (*dwLen < 160 * 4 * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		patchCatacombsTil(buf);
	} break;
	case FILE_CATACOMBS_SOL:
	{	// patch dSolidTable - L2.SOL
		// add separate tiles and subtiles for the arches III.
		if (*dwLen < 559) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		// fix the upstairs IV.
		// - make the back of the stairs non-walkable
		nSolidTable(252, true);
		nBlockTable(252, true);
		nMissileTable(252, true);
		// - make the stair-floor non light-blocker
		nBlockTable(267, false);
		// nBlockTable(559, false);
		// - adjust SOL after patchCatacombsMin
		nSolidTable(268, true);
		nBlockTable(268, true);
		nMissileTable(268, true);
	} break;
	case FILE_CATACOMBS_AMP:
	{	// patch dAutomapData - L2.AMP
		if (*dwLen < 157 * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		uint16_t *automaptype = (uint16_t*)buf;
		automaptype[17 - 1] = automaptype[5 - 1];
		// automaptype[18 - 1] = automaptype[5 - 1];
		automaptype[34 - 1] = automaptype[6 - 1];
		automaptype[35 - 1] = automaptype[7 - 1];
		automaptype[36 - 1] = automaptype[9 - 1];
		automaptype[37 - 1] = automaptype[9 - 1];
		// automaptype[44 - 1] = automaptype[3 - 1];
		// automaptype[46 - 1] = automaptype[3 - 1];
		// automaptype[47 - 1] = automaptype[3 - 1];
		// automaptype[48 - 1] = automaptype[3 - 1];
		// automaptype[49 - 1] = automaptype[3 - 1];
		// automaptype[95 - 1] = automaptype[3 - 1];
		// automaptype[96 - 1] = automaptype[3 - 1];
		// automaptype[100 - 1] = automaptype[3 - 1];
		automaptype[42 - 1] &= SwapLE16(~MAPFLAG_HORZARCH);
		automaptype[156 - 1] &= SwapLE16(~(MAPFLAG_VERTDOOR | MAPFLAG_TYPE));
		automaptype[157 - 1] &= SwapLE16(~(MAPFLAG_HORZDOOR | MAPFLAG_TYPE));
	} break;
#if ASSET_MPL == 1
	case FILE_CAVES_MIN:
	{	// patch dMiniTiles - L3.MIN
		if (*dwLen < MICRO_IDX(82 - 1, BLOCK_SIZE_L3, 4) * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		uint16_t *pSubtiles = (uint16_t*)buf;
		constexpr int blockSize = BLOCK_SIZE_L3;
		// fix bad artifact
		Blk2Mcr(82, 4);
	} break;
#endif /* ASSET_MPL == 1 */
	case FILE_CAVES_SOL:
	{	// patch dSolidTable - L3.SOL
		if (*dwLen <= 249) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		nSolidTable(249, false); // sync tile 68 and 69 by making subtile 249 of tile 68 walkable.
	} break;
#if ASSET_MPL == 1
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
		buf = patchHellCel(tilBuf, tilLen, minBuf, minLen, buf, dwLen, 45 - 1);
		mem_free_dbg(minBuf);
		mem_free_dbg(tilBuf);
	} break;
	case FILE_HELL_MIN:
	{	// patch dMiniTiles - L4.MIN
		constexpr int blockSize = BLOCK_SIZE_L4;
		uint16_t *pSubtiles = (uint16_t*)buf;
		// patch exit tile II.
		// - move the frames to the bottom right subtile
		pSubtiles[MICRO_IDX(140 - 1, blockSize, 3)] = SwapLE16((SwapLE16(pSubtiles[MICRO_IDX(137 - 1, blockSize, 1)]) & 0xFFF) | (MET_TRANSPARENT << 12)); // 369
		// pSubtiles[MICRO_IDX(137 - 1, blockSize, 1)] = 0;

		pSubtiles[MICRO_IDX(140 - 1, blockSize, 2)] = SwapLE16((SwapLE16(pSubtiles[MICRO_IDX(137 - 1, blockSize, 0)]) & 0xFFF) | (MET_SQUARE << 12));      // 368
		pSubtiles[MICRO_IDX(140 - 1, blockSize, 4)] = SwapLE16((SwapLE16(pSubtiles[MICRO_IDX(137 - 1, blockSize, 2)]) & 0xFFF) | (MET_TRANSPARENT << 12)); // 367
		// pSubtiles[MICRO_IDX(137 - 1, blockSize, 0)] = 0;
		// pSubtiles[MICRO_IDX(137 - 1, blockSize, 2)] = 0;

		// - eliminate right frame of the bottom left subtile
		pSubtiles[MICRO_IDX(139 - 1, blockSize, 1)] = 0;

		// - adjust the frame types after patchHellCel
		SetFrameType(140, 0, MET_LTRAPEZOID);
		SetFrameType(140, 1, MET_TRANSPARENT);
	} break;
#endif
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
		// nMissileTable(137, false); // fix missile-blocking tile of down-stairs.
		// nSolidTable(137, false);   // fix non-walkable tile of down-stairs. - the subtile is not used any more
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
#if ASSET_MPL == 1
	case FILE_NTOWN_MIN:
	{	// patch dMiniTiles - Town.MIN
		if (*dwLen < MICRO_IDX(1303 - 1, BLOCK_SIZE_TOWN, 7) * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq. File len: %d vs %d", filesToPatch[index], *dwLen, MICRO_IDX(1303 - 1, BLOCK_SIZE_TOWN, 7) * 2);
			return NULL;
		}
		patchTownMin(buf, true);
		buf = buildBlkMin(buf, dwLen, BLOCK_SIZE_TOWN);
	} break;
	case FILE_NEST_MIN:
	{	// patch dMiniTiles - L6.MIN
		if (*dwLen < MICRO_IDX(366 - 1, BLOCK_SIZE_L6, 1) * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		uint16_t *pSubtiles = (uint16_t*)buf;
		constexpr int blockSize = BLOCK_SIZE_L6;
		// useless black micros
		Blk2Mcr(21, 0);
		Blk2Mcr(21, 1);
		// fix bad artifacts
		Blk2Mcr(132, 7);
		Blk2Mcr(366, 1);
	} break;
#endif /* ASSET_MPL == 1 */
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
#if ASSET_MPL == 1
	case FILE_CRYPT_CEL:
	{	// patch dMicroCels - L5.CEL
		size_t minLen;
		BYTE* minBuf = LoadFileInMem(filesToPatch[FILE_CRYPT_MIN], &minLen);
		if (minBuf == NULL) {
			mem_free_dbg(buf);
			app_warn("Unable to open file %s in the mpq.", filesToPatch[FILE_CRYPT_MIN]);
			return NULL;
		}
		buf = fillCryptShapes(minBuf, minLen, buf, dwLen);
		if (buf != NULL) {
			buf = maskCryptBlacks(minBuf, minLen, buf, dwLen);
			if (buf != NULL) {
				buf = fixCryptShadows(minBuf, minLen, buf, dwLen);
				if (buf != NULL) {
					patchCryptMin(minBuf);
					buf = buildBlkCel(buf, dwLen);
				}
			}
		}
		mem_free_dbg(minBuf);
	} break;
	case FILE_CRYPT_MIN:
	{	// patch dMiniTiles - L5.MIN
		if (*dwLen < MICRO_IDX(197 - 1, BLOCK_SIZE_L5, 1) * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		patchCryptMin(buf);
		buf = buildBlkMin(buf, dwLen, BLOCK_SIZE_L5);
	} break;
#endif // ASSET_MPL
	case FILE_CRYPT_TIL:
	{	// patch dMegaTiles - L5.TIL
		if (*dwLen < 4 * 217 * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		patchCryptTil(buf);
	} break;
	case FILE_CRYPT_SOL:
	{ // patch dSolidTable - L5.SOL
		if (*dwLen <= 600) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		nSolidTable(143, false); // make right side of down-stairs consistent (walkable)
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
		// - adjust SOL after patchCryptMin
		nSolidTable(238, false);
		nMissileTable(238, false);
		nBlockTable(238, false);
		nMissileTable(178, false);
		nBlockTable(178, false);
		nSolidTable(242, false);
		nMissileTable(242, false);
		nBlockTable(242, false);
		// fix automap of the entrance II.
		nMissileTable(158, false);
		nBlockTable(158, false);
		nSolidTable(159, false);
		nMissileTable(159, false);
		nBlockTable(159, false);
		nMissileTable(148, true);
		nBlockTable(148, true);
		nSolidTable(148, true);
	} break;
	case FILE_CRYPT_AMP:
	{	// patch dAutomapData - L5.AMP
		if (*dwLen < 216 * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		uint16_t *automaptype = (uint16_t*)buf;
		// fix automap of the entrance III.
		automaptype[47 - 1] = SwapLE16(MAPFLAG_STAIRS | 2);
		automaptype[50 - 1] = SwapLE16(2);
		automaptype[48 - 1] = SwapLE16(MAPFLAG_STAIRS | 4);
		automaptype[51 - 1] = SwapLE16(5);
		automaptype[52 - 1] = SwapLE16(MAPFLAG_DIRT);
		automaptype[53 - 1] = SwapLE16(MAPFLAG_STAIRS | 4);
		automaptype[54 - 1] = SwapLE16(MAPFLAG_DIRT);
		automaptype[56 - 1] = SwapLE16(0);
		automaptype[58 - 1] = SwapLE16(MAPFLAG_DIRT | 5);
		// adjust AMP after patchCryptMin
		// - use the shadows created by fixCryptShadows
		automaptype[109 - 1] = SwapLE16(2);
		automaptype[110 - 1] = SwapLE16(2);
		automaptype[111 - 1] = SwapLE16(MAPFLAG_VERTARCH | 2);
		automaptype[215 - 1] = SwapLE16(MAPFLAG_VERTGRATE | 2);
		// - 'add' new shadow-types with glow
		automaptype[216 - 1] = SwapLE16(MAPFLAG_VERTARCH | 2);
		// - 'add' new shadow-types with horizontal arches
		automaptype[71 - 1] = SwapLE16(3);
		automaptype[80 - 1] = SwapLE16(3);
		automaptype[81 - 1] = SwapLE16(MAPFLAG_HORZARCH | 3);
		automaptype[82 - 1] = SwapLE16(MAPFLAG_HORZARCH | 3);
		automaptype[83 - 1] = SwapLE16(MAPFLAG_HORZGRATE | 3);
		automaptype[84 - 1] = SwapLE16(MAPFLAG_HORZGRATE | 3);
		automaptype[85 - 1] = SwapLE16(3);
		automaptype[86 - 1] = SwapLE16(3);
		automaptype[87 - 1] = SwapLE16(MAPFLAG_HORZDOOR | 3);
		automaptype[88 - 1] = SwapLE16(MAPFLAG_HORZDOOR | 3);
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

	// ignore the merged mpq during the patch
	mpqone = diabdat_mpqs[NUM_MPQS];
	diabdat_mpqs[NUM_MPQS] = NULL;
	bool result = UiProgressDialog("...Patch in progress...", patcher_callback);
	// restore the merged mpq
	diabdat_mpqs[NUM_MPQS] = mpqone;
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
