#include <string>
#include <fstream>
#include <set>

#include "diabloui.h"
#include "selok.h"
#include "utils/paths.h"
#include "utils/file_util.h"
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
	FILE_CATHEDRAL_MIN,
#endif
	FILE_CATHEDRAL_SOL,
#if ASSET_MPL == 1
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
/*FILE_CATHEDRAL_MIN*/ "Levels\\L1Data\\L1.MIN",
#endif
/*FILE_CATHEDRAL_SOL*/ "Levels\\L1Data\\L1.SOL",
#if ASSET_MPL == 1
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
		{ 553 - 1, 5, MET_TRANSPARENT }, // 1470
		{ 553 - 1, 3, MET_TRANSPARENT }, // 1471
		{ 553 - 1, 1, MET_RTRIANGLE },   // 1473
		{ 554 - 1, 0, MET_TRANSPARENT }, // 1475
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

	// move the image up 553[5] (1470) and 553[3] (1471)
	for (int x = MICRO_WIDTH / 2; x < MICRO_WIDTH; x++) {
		for (int y = MICRO_HEIGHT / 2; y < 2 * MICRO_HEIGHT; y++) {
			gpBuffer[x + (y - MICRO_HEIGHT / 2) * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
			gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
		}
	}
	// copy image to the other micros 553[1] (1473) -> 553[3] 1471, 554[0] 1475
	for (int x = MICRO_WIDTH / 2 + 2; x < MICRO_WIDTH - 4; x++) {
		for (int y = 2 * MICRO_HEIGHT; y < 2 * MICRO_HEIGHT + MICRO_HEIGHT / 2 + 8; y++) {
			BYTE color = gpBuffer[x + y * BUFFER_WIDTH];
			if (color == TRANS_COLOR)
				continue;
			if (y < 2 * MICRO_HEIGHT + MICRO_HEIGHT / 2) {
				gpBuffer[x + (y - MICRO_HEIGHT / 2) * BUFFER_WIDTH] = color; // 1471
			} else {
				gpBuffer[x + MICRO_WIDTH + (y - 2 * MICRO_HEIGHT - MICRO_HEIGHT / 2) * BUFFER_WIDTH] = color; // 1475
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
		{ 807 - 1, 12 }, // 2145
		{ 805 - 1, 12 }, // 2123
		{ 805 - 1, 13 }, // 2124
		{ 806 - 1, 13 }, // 2137
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
		gpBuffer[x + y * BUFFER_WIDTH] = gpBuffer[x + (y + 6) * BUFFER_WIDTH]; // 2145
	}
	for (int x = 0; x < MICRO_WIDTH - 4; x++) {
		int y = MICRO_HEIGHT / 2 - x / 2 + 1 * MICRO_HEIGHT;
		gpBuffer[x + y * BUFFER_WIDTH] = gpBuffer[x + 4 + (y + 4) * BUFFER_WIDTH]; // 2123 I.
	}
	for (int x = MICRO_WIDTH - 4; x < MICRO_WIDTH; x++) {
		int y = MICRO_HEIGHT / 2 - x / 2 + 1 * MICRO_HEIGHT;
		gpBuffer[x + y * BUFFER_WIDTH] = gpBuffer[x + (y + 2) * BUFFER_WIDTH]; // 2123 II.
	}
	for (int x = 0; x < 20; x++) {
		int y = 1 + x / 2 + 2 * MICRO_HEIGHT;
		gpBuffer[x + y * BUFFER_WIDTH] = gpBuffer[x + (y + 1) * BUFFER_WIDTH]; // 2124 I.
	}
	for (int x = 20; x < MICRO_WIDTH; x++) {
		int y = 1 + x / 2 + 2 * MICRO_HEIGHT;
		gpBuffer[x + y * BUFFER_WIDTH] = gpBuffer[x - 12 + (y - 6) * BUFFER_WIDTH]; // 2124 II.
	}
	for (int x = 0; x < MICRO_WIDTH; x++) {
		int y = 1 + x / 2 + 3 * MICRO_HEIGHT;
		gpBuffer[x + y * BUFFER_WIDTH] = gpBuffer[x + (y - MICRO_HEIGHT) * BUFFER_WIDTH]; // 2137
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
		{ 731 - 1, 9, MET_TRANSPARENT }, // 1923 move micro
		{ 755 - 1, 0, MET_LTRIANGLE },   // 1975 change type
		{ 974 - 1, 0, MET_LTRIANGLE },   // 2805 change type
		{ 1030 - 1, 0, MET_LTRIANGLE },  // 2943 change type
		{ 220 - 1, 1, MET_TRANSPARENT }, // 514  move micro
		{ 221 - 1, 0, MET_TRANSPARENT }, // 516
		{ 962 - 1, 1, MET_TRANSPARENT }, // 2775
		{ 218 - 1, 0, MET_TRANSPARENT }, // 511 move micro
		{ 219 - 1, 1, MET_TRANSPARENT }, // 513
		{ 1166 - 1, 0, MET_TRANSPARENT }, // 3289 move micro
		{ 1167 - 1, 1, MET_TRANSPARENT }, // 3292
		{ 1171 - 1, 1, MET_TRANSPARENT }, // 3302
		{ 1172 - 1, 0, MET_TRANSPARENT }, // 3303
		{ 1175 - 1, 1, MET_TRANSPARENT }, // 3311
		{ 1176 - 1, 0, MET_TRANSPARENT }, // 3317
		{ 845 - 1, 4, MET_TRANSPARENT }, // 2358
		//{ 493 - 1, 0, MET_LTRIANGLE },   // 866 TODO: fix light?
		//{ 290 - 1, 0, MET_LTRIANGLE },   // 662 TODO: fix grass?
		//{ 290 - 1, 1, MET_RTRIANGLE },   // 663
		//{ 334 - 1, 0, MET_LTRIANGLE },   // 750 TODO: fix grass? + (349 & nest)
		//{ 334 - 1, 1, MET_RTRIANGLE },   // 751
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

	// move the image up - 1923
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

	// mask and move down the image - 514, 516
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
			// - move to 516
			for (int y = MICRO_HEIGHT / 2; y < 21; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR) {
					unsigned addr2 = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr2] = color; // 516
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
	// mask and move down the image - 2775
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
	// mask and move down the image - 511, 513
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
			// - move to 513
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR) {
					unsigned addr2 = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr2] = color; // 513
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
	// mask and move down the image 1166[0] (3289), 1167[1] (3292)
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
			// - move to 3292
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR) {
					unsigned addr2 = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr2] = color; // 3292
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
	// mask the image 1171[1] (3302), 1172[0] (3303), 1175[1] (3311) and 1176[0] (3317)
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
	// mask the image 845[4] (2358)
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
/*  0 */{ 724 - 1, 0, -1 }, // 1903 move micro
/*  1 */{ 724 - 1, 1, -1 }, // 1904
/*  2 */{ 724 - 1, 3, -1 }, // 1902
/*  3 */{ 723 - 1, 1, -1 }, // 1901
/*  4 */{ 715 - 1, 11, -1 }, // 1848
/*  5 */{ 715 - 1, 9, -1 }, // 1849
/*  6 */{ 715 - 1, 7, -1 }, // 1850 - unused
/*  7 */{ 715 - 1, 5, -1 }, // 1851 - unused
/*  8 */{ 715 - 1, 3, -1 }, // 1852
/*  9 */{ 715 - 1, 1, MET_RTRIANGLE }, // 1854
/* 10 */{ 721 - 1, 4, MET_SQUARE }, // 1893
/* 11 */{ 721 - 1, 2, MET_SQUARE }, // 1894
/* 12 */{ 719 - 1, 4, MET_SQUARE }, // 1875
/* 13 */{ 719 - 1, 2, MET_SQUARE }, // 1877
/* 14 */{ 727 - 1, 7, MET_SQUARE }, // 1911
/* 15 */{ 727 - 1, 5, MET_SQUARE }, // 1912
/* 16 */{ 725 - 1, 4, MET_TRANSPARENT }, // 1905
/* 17 */{ 725 - 1, 2, MET_TRANSPARENT }, // 1906
/* 18 */{ 725 - 1, 0, MET_TRANSPARENT }, // 1907

/* 19 */{ 428 - 1, 4, -1 }, // 1049
/* 20 */{ 428 - 1, 2, -1 }, // 1050
/* 21 */{ 428 - 1, 0, -1 }, // 1051
/* 22 */{ 418 - 1, 5, MET_SQUARE }, // 1005
/* 23 */{ 418 - 1, 3, MET_SQUARE }, // 1006
/* 24 */{ 418 - 1, 1, MET_RTRAPEZOID }, // 1008
/* 25 */{ 426 - 1, 2, -1 }, // 1045
/* 26 */{ 426 - 1, 0, -1 }, // 1046
/* 27 */{ 428 - 1, 1, -1 }, // 1052
/* 28 */{ 429 - 1, 0, -1 }, // 1053
/* 29 */{ 419 - 1, 5, MET_SQUARE }, // 1013
/* 30 */{ 419 - 1, 3, MET_SQUARE }, // 1014
/* 31 */{ 419 - 1, 1, MET_RTRAPEZOID }, // 1016

/* 32 */{ 911 - 1, 9, -1 }, // 2560
/* 33 */{ 911 - 1, 7, -1 }, // 2561
/* 34 */{ 911 - 1, 5, -1 }, // 2562
/* 35 */{ 931 - 1, 5, MET_SQUARE }, // 2643
/* 36 */{ 931 - 1, 3, MET_SQUARE }, // 2644
/* 37 */{ 931 - 1, 1, MET_RTRAPEZOID }, // 2646
/* 38 */{ 402 - 1, 0, -1 }, // 939
/* 39 */{ 954 - 1, 2, -1 }, // 2746
/* 40 */{ 919 - 1, 9, -1 }, // 2587
/* 41 */{ 919 - 1, 5, -1 }, // 2589
/* 42 */{ 927 - 1, 5, MET_SQUARE }, // 2625
/* 43 */{ 927 - 1, 1, MET_RTRAPEZOID }, // 2627
/* 44 */{ 956 - 1, 0, MET_LTRIANGLE }, // 2760 - unused
		// { 956 - 1, 2, -1 }, // 2759
/* 45 */{ 954 - 1, 0, MET_LTRIANGLE }, // 2748 - unused
/* 46 */{ 919 - 1, 7, MET_SQUARE }, // 2588
/* 47 */{ 918 - 1, 9, -1 }, // 2578
/* 48 */{ 926 - 1, 5, MET_SQUARE }, // 2619
/* 49 */{ 927 - 1, 0, -1 }, // 2626
/* 50 */{ 918 - 1, 3, -1 }, // 2584
/* 51 */{ 918 - 1, 2, -1 }, // 2583
/* 52 */{ 918 - 1, 5, MET_SQUARE }, // 2582
/* 53 */{ 929 - 1, 0, MET_LTRAPEZOID }, // 2632
/* 54 */{ 929 - 1, 1, MET_RTRAPEZOID }, // 2633
/* 55 */{ 918 - 1, 8, -1 }, // 2577
/* 56 */{ 926 - 1, 4, MET_SQUARE }, // 2618
/* 57 */{ 928 - 1, 4, -1 }, // 2631
/* 58 */{ 920 - 1, 8, MET_SQUARE }, // 2592
/* 59 */{ 551 - 1, 0, -1 }, // 1467
/* 60 */{ 552 - 1, 1, -1 }, // 1469
/* 61 */{ 519 - 1, 0, -1 }, // 1342
/* 62 */{ 509 - 1, 5, MET_SQUARE }, // 1315
/* 63 */{ 509 - 1, 3, MET_SQUARE }, // 1317
/* 64 */{ 509 - 1, 1, MET_RTRAPEZOID }, // 1319

/* 65 */{ 510 - 1, 7, -1 }, // 1321
/* 66 */{ 510 - 1, 5, -1 }, // 1322
/* 67 */{ 551 - 1, 3, MET_SQUARE }, // 1466
/* 68 */{ 551 - 1, 1, MET_RTRAPEZOID }, // 1468

/* 69 */{ 728 - 1, 9, -1 }, // 1916
/* 70 */{ 728 - 1, 7, -1 }, // 1917
/* 71 */{ 716 - 1, 13, MET_TRANSPARENT }, // 1855
/* 72 */{ 716 - 1, 11, MET_SQUARE }, // 1856

/* 73 */{ 910 - 1, 9, -1 }, // 2556
/* 74 */{ 910 - 1, 7, -1 }, // 2557
/* 75 */{ 930 - 1, 5, MET_TRANSPARENT }, // 2636
/* 76 */{ 930 - 1, 3, MET_TRANSPARENT }, // 2638

/* 77 */{ 537 - 1, 0, -1 }, // 1429
/* 78 */{ 539 - 1, 0, -1 }, // 1435
/* 79 */{ 529 - 1, 4, MET_SQUARE }, // 1394
/* 80 */{ 531 - 1, 4, MET_SQUARE }, // 1400

/* 81 */{ 478 - 1, 0, -1 }, // 1230
/* 82 */{ 477 - 1, 1, MET_SQUARE }, // 1226
/* 83 */{ 480 - 1, 1, MET_RTRAPEZOID }, // 1240
/* 84 */{ 479 - 1, 1, -1 }, // 1231
/* 85 */{ 477 - 1, 0, MET_SQUARE }, // 1225
/* 86 */{ 480 - 1, 0, MET_LTRAPEZOID }, // 1239
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
	// copy 724[0] (1903) to 721[2] (1894)
	for (int i = 0; i < 1; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					gpBuffer[x + MICRO_WIDTH * ((i + 11) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 11) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color;
				}
			}
		}
	}
	// copy 724[1] (1903) to 719[2] (1875)
	for (int i = 1; i < 2; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					gpBuffer[x + MICRO_WIDTH * ((i + 12) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 12) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color; // 1875
				}
			}
		}
	}
	// copy 724[3] (1903) to 719[4] (1877) and 719[2] (1875)
	for (int i = 2; i < 3; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					if (y < MICRO_HEIGHT / 2) {
						gpBuffer[x + MICRO_WIDTH * ((i + 10) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 10) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color; // 1877
					} else {
						gpBuffer[x + MICRO_WIDTH * ((i + 11) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 11) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color; // 1875
					}
				}
			}
		}
	}
	// copy 723[1] (1901) to 721[2] (1894) and 721[4] (1893)
	for (int i = 3; i < 4; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					if (y < MICRO_HEIGHT / 2) {
						gpBuffer[x + MICRO_WIDTH * ((i + 7) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 7) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color; // 1877
					} else {
						gpBuffer[x + MICRO_WIDTH * ((i + 8) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 8) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color; // 1875
					}
				}
			}
		}
	}
	// copy 715[11] (1848) to 727[7] (1911)
	for (int i = 4; i < 5; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					gpBuffer[x + MICRO_WIDTH * ((i + 10) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 10) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color; // 1911
				}
			}
		}
	}
	// copy 715[9] (1849) to 727[5] (1912)
	for (int i = 5; i < 6; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					gpBuffer[x + MICRO_WIDTH * ((i + 10) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 10) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color; // 1912
				}
			}
		}
	}
	// copy 715[3] (1849) to 725[2] (1912) and 725[0] (1912)
	for (int i = 8; i < 9; i++) {
		for (int x = 9; x < 24; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					if (y < MICRO_HEIGHT / 2) {
						gpBuffer[x + MICRO_WIDTH * ((i + 9) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 9) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color; // 1877
					} else {
						gpBuffer[x + MICRO_WIDTH * ((i + 10) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 10) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color; // 1875
					}
				}
			}
		}
	}
	// copy 715[1] (1849) to 725[0] (1912)
	for (int i = 9; i < 10; i++) {
		for (int x = 9; x < 24; x++) {
			for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR && y <= x / 2) {
					gpBuffer[x + MICRO_WIDTH * ((i + 9) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 9) % DRAW_HEIGHT)) * BUFFER_WIDTH] = color; // 1877
				}
			}
		}
	}
	// copy 428[4] (1849) to 418[5] (1912)
	for (int i = 19; i < 20; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 3) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 3) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					if (gpBuffer[addr] == TRANS_COLOR) {
						gpBuffer[addr] = color; // 1877
					}
				}
			}
		}
	}
	// copy 428[2] (1849) to 418[5] (1912) and 418[3] (1912)
	for (int i = 20; i < 21; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr;
					if (y < MICRO_HEIGHT / 2) {
						addr = x + MICRO_WIDTH * ((i + 2) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 2) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 1877
					} else {
						addr = x + MICRO_WIDTH * ((i + 3) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 3) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 1875
					}
					if (gpBuffer[addr] == TRANS_COLOR) {
						gpBuffer[addr] = color; // 1877
					}
				}
			}
		}
	}
	// copy 428[0] (1849) to 418[3] (1912) and 418[1] (1912)
	for (int i = 21; i < 22; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr;
					if (y < MICRO_HEIGHT / 2) {
						addr = x + MICRO_WIDTH * ((i + 2) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 2) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 1877
					} else {
						addr = x + MICRO_WIDTH * ((i + 3) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 3) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 1875
					}
					if (gpBuffer[addr] == TRANS_COLOR) {
						gpBuffer[addr] = color; // 1877
					}
				}
			}
		}
	}
	// copy 426[2] (1849) to 419[5] (1912)
	for (int i = 25; i < 26; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 4) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 4) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					if (gpBuffer[addr] == TRANS_COLOR) {
						gpBuffer[addr] = color; // 1877
					}
				}
			}
		}
	}
	// copy 426[0] (1849) to 419[5] (1912) and 419[3] (1912)
	for (int i = 26; i < 27; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr;
					if (y < MICRO_HEIGHT / 2) {
						addr = x + MICRO_WIDTH * ((i + 3) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 3) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 1877
					} else {
						addr = x + MICRO_WIDTH * ((i + 4) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 4) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 1875
					}
					if (gpBuffer[addr] == TRANS_COLOR) {
						gpBuffer[addr] = color; // 1877
					}
				}
			}
		}
	}
	// copy 428[1] (1849) to 419[3] (1912)
	for (int i = 27; i < 28; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 3) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 3) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					if (gpBuffer[addr] == TRANS_COLOR) {
						gpBuffer[addr] = color; // 1877
					}
				}
			}
		}
	}
	// copy 429[0] (1849) to 419[3] (1912) and 419[1] (1912)
	for (int i = 28; i < 29; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr;
					if (y < MICRO_HEIGHT / 2) {
						addr = x + MICRO_WIDTH * ((i + 2) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 2) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 1877
					} else {
						addr = x + MICRO_WIDTH * ((i + 3) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 3) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 1875
					}
					if (gpBuffer[addr] == TRANS_COLOR) {
						gpBuffer[addr] = color; // 1877
					}
				}
			}
		}
	}
	// copy 911[9] (1849) to 931[5] (1912)
	// copy 911[7] (1849) to 931[3] (1912)
	// copy 911[5] (1849) to 931[1] (1912)
	for (int i = 32; i < 35; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 3) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 3) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr] = color; // 1877
				}
			}
		}
	}
	// copy 919[9] (1849) to 927[5] (1912)
	// copy 919[5] (1849) to 927[1] (1912)
	for (int i = 40; i < 42; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 2) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 2) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr] = color; // 1877
				}
			}
		}
	}
	// copy 402[0] (1849) to 927[1] (1912)
	for (int i = 38; i < 39; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 5) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 5) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					if (gpBuffer[addr] == TRANS_COLOR) {
						gpBuffer[addr] = color; // 1877
					}
				}
			}
		}
	}
	// copy 954[2] (1849) to 919[7] (1912 -> 927[3]) and 927[1] (1912)
	for (int i = 39; i < 40; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr;
					if (y < MICRO_HEIGHT / 2) {
						addr = x + MICRO_WIDTH * ((i + 7) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 7) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 1875
					} else {
						addr = x + MICRO_WIDTH * ((i + 4) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 4) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 1877
					}
					gpBuffer[addr] = color; // 1877
				}
			}
		}
	}
	// copy 918[9] (1849) to 926[5] (1912)
	for (int i = 47; i < 48; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr] = color; // 1877
				}
			}
		}
	}
	// copy 927[0] (1849) to 918[5] (1912) and 929[1] (1912)
	for (int i = 49; i < 50; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr;
					if (y < MICRO_HEIGHT / 2) {
						addr = x + MICRO_WIDTH * ((i + 3) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 3) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 1875
					} else {
						addr = x + MICRO_WIDTH * ((i + 5) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 5) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 1877
					}
					if (gpBuffer[addr] == TRANS_COLOR) {
						gpBuffer[addr] = color; // 1877
					}
				}
			}
		}
	}
	// copy 918[3] (1849) to 929[1] (1912)
	for (int i = 50; i < 51; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 4) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 4) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr] = color; // 1877
				}
			}
		}
	}
	// copy 918[2] (1849) to 929[0] (1912)
	for (int i = 51; i < 52; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 2) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 2) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr] = color; // 1877
				}
			}
		}
	}
	// copy 918[8] (1849) to 926[4] (1912)
	for (int i = 55; i < 56; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr] = color; // 1877
				}
			}
		}
	}
	// copy 928[4] (1849) to 920[8] (1912)
	for (int i = 57; i < 58; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr] = color; // 1877
				}
			}
		}
	}
	// copy 551[0] (1849) to 509[5] (1912) and 509[3] (1912)
	for (int i = 59; i < 60; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr;
					if (y < MICRO_HEIGHT / 2) {
						addr = x + MICRO_WIDTH * ((i + 3) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 3) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 1875
					} else {
						addr = x + MICRO_WIDTH * ((i + 4) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 4) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 1877
					}
					if (gpBuffer[addr] == TRANS_COLOR) {
						gpBuffer[addr] = color; // 1877
					}
				}
			}
		}
	}
	// copy 552[1] (1849) to 509[3] (1912)
	for (int i = 60; i < 61; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 3) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 3) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr] = color; // 1877
				}
			}
		}
	}
	// copy 519[0] (1849) to 509[3] (1912) and 509[1] (1912)
	for (int i = 61; i < 62; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr;
					if (y < MICRO_HEIGHT / 2) {
						addr = x + MICRO_WIDTH * ((i + 2) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 2) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 1875
					} else {
						addr = x + MICRO_WIDTH * ((i + 3) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 3) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 1877
					}
					if (gpBuffer[addr] == TRANS_COLOR) {
						gpBuffer[addr] = color; // 1877
					}
				}
			}
		}
	}
	// copy 510[7] (1849) to 551[3] (1912)
	// copy 510[5] (1849) to 551[1] (1912)
	for (int i = 65; i < 67; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 2) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 2) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr] = color; // 1877
				}
			}
		}
	}
	// copy 728[9] (1849) to 716[13] (1912)
	// copy 728[7] (1849) to 716[11] (1912)
	for (int i = 69; i < 71; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 2) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 2) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr] = color; // 1877
				}
			}
		}
	}
	// copy 910[9] (1849) to 930[5] (1912)
	// copy 910[7] (1849) to 930[3] (1912)
	for (int i = 73; i < 75; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 2) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 2) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr] = color; // 1877
				}
			}
		}
	}
	// copy 537[0] (1849) to 529[4] (1912)
	// copy 539[0] (1849) to 531[4] (1912)
	for (int i = 77; i < 79; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				BYTE color = gpBuffer[x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH];
				if (color != TRANS_COLOR) {
					unsigned addr = x + MICRO_WIDTH * ((i + 2) / DRAW_HEIGHT) + (y + MICRO_HEIGHT * ((i + 2) % DRAW_HEIGHT)) * BUFFER_WIDTH;
					gpBuffer[addr] = color; // 1877
				}
			}
		}
	}
	// copy 478[0] (1849) to 477[1] (1912) and 480[1] (1912)
	for (int i = 81; i < 82; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR) {
					if (y < MICRO_HEIGHT / 2) {
						addr = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 1875
					} else {
						addr = x + MICRO_WIDTH * ((i + 2) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 2) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 1875
					}
					gpBuffer[addr] = color; // 1877
				}
			}
		}
	}
	// copy 479[1] (1849) to 477[0] (1912) and 480[0] (1912)
	for (int i = 84; i < 85; i++) {
		for (int x = 0; x < MICRO_WIDTH; x++) {
			for (int y = 0; y < MICRO_HEIGHT; y++) {
				unsigned addr = x + MICRO_WIDTH * (i / DRAW_HEIGHT) + (y + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
				BYTE color = gpBuffer[addr];
				if (color != TRANS_COLOR) {
					if (y < MICRO_HEIGHT / 2) {
						addr = x + MICRO_WIDTH * ((i + 1) / DRAW_HEIGHT) + (y + MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 1) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 1875
					} else {
						addr = x + MICRO_WIDTH * ((i + 2) / DRAW_HEIGHT) + (y - MICRO_HEIGHT / 2 + MICRO_HEIGHT * ((i + 2) % DRAW_HEIGHT)) * BUFFER_WIDTH; // 1875
					}
					gpBuffer[addr] = color; // 1877
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
/*  0 */{ 180 - 1, 1, MET_RTRIANGLE }, // 1854
/*  1 */{ 180 - 1, 3, MET_TRANSPARENT }, // 1854

/*  2 */{ 224 - 1, 0, MET_LTRAPEZOID }, // 1854
/*  3 */{ 224 - 1, 2, MET_TRANSPARENT }, // 1854
/*  4 */{ 225 - 1, 2, MET_TRANSPARENT }, // 1854

/*  5 */{ 362 - 1, 9, MET_TRANSPARENT }, // 1854
/*  6 */{ 383 - 1, 3, MET_SQUARE }, // 1854

/*  7 */{ 632 - 1, 11, MET_SQUARE }, // 1854
/*  8 */{ 632 - 1, 13, MET_TRANSPARENT }, // 1854
/*  9 */{ 631 - 1, 11, MET_TRANSPARENT }, // 1854

/* 10 */{ 832 - 1, 10, MET_TRANSPARENT }, // 1854

/* 11 */{ 834 - 1, 10, MET_SQUARE }, // 1854
/* 12 */{ 834 - 1, 12, MET_TRANSPARENT }, // 1854
/* 13 */{ 828 - 1, 12, MET_TRANSPARENT }, // 1854

/* 14 */{ 864 - 1, 12, MET_SQUARE }, // 1854
/* 15 */{ 864 - 1, 14, MET_TRANSPARENT }, // 1854

/* 16 */{ 926 - 1, 12, MET_TRANSPARENT }, // 1854
/* 17 */{ 926 - 1, 13, MET_TRANSPARENT }, // 1854

/* 18 */{ 944 - 1, 6, MET_SQUARE }, // 1854
/* 19 */{ 944 - 1, 8, MET_TRANSPARENT }, // 1854
/* 20 */{ 942 - 1, 6, MET_TRANSPARENT }, // 1854

/* 21 */{ 955 - 1, 13, MET_TRANSPARENT }, // 1854
/* 22 */{ 950 - 1, 13, MET_TRANSPARENT }, // 1854
/* 23 */{ 951 - 1, 13, MET_TRANSPARENT }, // 1854
/* 24 */{ 946 - 1, 13, MET_TRANSPARENT }, // 1854
/* 25 */{ 947 - 1, 13, MET_TRANSPARENT }, // 1854
/* 26 */{ 940 - 1, 12, MET_TRANSPARENT }, // 1854

/* 27 */{ 383 - 1, 5, MET_SQUARE }, // 1854
/* 28 */{ 383 - 1, 7, MET_SQUARE }, // 1854
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
	{ // 828[12] (1854)
		int i = 13;
		unsigned addr = 30 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = TRANS_COLOR;
		gpBuffer[addr + 1] = TRANS_COLOR;
	}
	{ // 180[3] (1854)
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
	{ // 224[0] (1854)
		int i = 2;
		unsigned addr =  0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = gpBuffer[addr + 2];
	}
	{ // + 225[2] (1854)
		int i = 4;
		unsigned addr = 31 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (17 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		unsigned addr2 = 29 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (20 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr + 0 * BUFFER_WIDTH] = gpBuffer[addr2 + 0 * BUFFER_WIDTH];
		gpBuffer[addr + 1 * BUFFER_WIDTH] = gpBuffer[addr2 + 1 * BUFFER_WIDTH];
		gpBuffer[addr - 1 + 1 * BUFFER_WIDTH] = gpBuffer[addr2 - 1 + 1 * BUFFER_WIDTH];
	}
	{ // 362[9] (1854)
		int i = 5;
		unsigned addr = 12 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr +  0 + 0 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  3 + 0 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr -  1 + 1 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr +  1 + 1 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // 383[3] (1854)
		int i = 6;
		unsigned addr  =  0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 3 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		unsigned addr2 =  5 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (18 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = gpBuffer[addr2];
	}
	{ // 632[11] (1854)
		int i = 7;
		unsigned addr  =  0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		unsigned addr2 =  7 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 2 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = gpBuffer[addr2];
		gpBuffer[addr + 1] = gpBuffer[addr2 + 1];
	}
	{ // + 632[13] (1854)
		int i = 8;
		unsigned addr  =  8 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (30 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		unsigned addr2 =  4 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (31 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = gpBuffer[addr + 2];
		gpBuffer[addr2] = gpBuffer[addr2 + 2];
	}
	{ // 832[10] (1854)
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
	{ // 834[10] (1854)
		int i = 11;
		unsigned addr  =  0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		unsigned addr2 =  1 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 1 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = gpBuffer[addr2];
		gpBuffer[addr + 1] = gpBuffer[addr2 + 1];
	}
	{ // + 834[12] (1854)
		int i = 12;
		unsigned addr  =  3 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (31 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		unsigned addr2 =  6 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (30 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = gpBuffer[addr + 2];
		gpBuffer[addr + 1] = gpBuffer[addr + 3];
		gpBuffer[addr2] = gpBuffer[addr2 + 2];
	}
	{ // + 828[12] (1854)
		int i = 13;
		unsigned addr  = 29 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (17 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		unsigned addr2 = 27 + MICRO_WIDTH * (i / DRAW_HEIGHT) + (18 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = gpBuffer[addr2 + 1];
		gpBuffer[addr + 1] = gpBuffer[addr2 + 2];
		gpBuffer[addr2] = gpBuffer[addr2 + 3];
		gpBuffer[addr2 - 3 + 1 * BUFFER_WIDTH] = gpBuffer[addr2 + 3];
	}
	{ // 864[12] (1854)
		int i = 14;
		unsigned addr  =  0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = gpBuffer[addr + 2];
	}
	{ // + 864[14] (1854)
		int i = 15;
		unsigned addr  =  4 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 30 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = gpBuffer[addr + 1];
		gpBuffer[addr - 2 + 1 * BUFFER_WIDTH] = gpBuffer[addr + 1];
	}
	{ // . 926[12] (1854)
		int i = 16;
		unsigned addr  =  26 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = TRANS_COLOR;
		gpBuffer[addr + 1] = TRANS_COLOR;
		gpBuffer[addr + 2] = TRANS_COLOR;
		gpBuffer[addr + 0 + 1 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr - 1 + 2 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // . 926[13] (1854)
		int i = 17;
		unsigned addr  =  7 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = TRANS_COLOR;
		gpBuffer[addr - 1] = TRANS_COLOR;
		gpBuffer[addr + 7 + 3 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // 944[6] (1854)
		int i = 18;
		unsigned addr  = 31 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = gpBuffer[addr - 2];
	}
	{ // + 944[8] (1854)
		int i = 19;
		unsigned addr  = 30 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 31 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = gpBuffer[addr - 2];
		gpBuffer[addr - 1 - 1 * BUFFER_WIDTH] = gpBuffer[addr - 2];
	}
	{ // + 942[6] (1854)
		int i = 20;
		unsigned addr  = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 17 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = gpBuffer[addr + BUFFER_WIDTH];
	}
	{ // . 955[13] (1854)
		int i = 21;
		unsigned addr  = 31 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 14 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = TRANS_COLOR;
		gpBuffer[addr - 1] = TRANS_COLOR;
		gpBuffer[addr - 1 - 1 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[addr - 2 - 1 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // . 950[13] (1854)
		int i = 22;
		unsigned addr  = 3 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = TRANS_COLOR;
		unsigned addr2  = 31 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 14 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr2] = TRANS_COLOR;
		gpBuffer[addr2 - 2 - 1 * BUFFER_WIDTH] = TRANS_COLOR;
	}
	{ // . 951[13] (1854)
		int i = 23;
		unsigned addr  = 3 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = TRANS_COLOR;
	}
	{ // . 946[13] (1854)
		int i = 24;
		unsigned addr  = 2 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = TRANS_COLOR;
	}
	{ // . 947[13] (1854)
		int i = 25;
		unsigned addr  = 3 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 0 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = TRANS_COLOR;
	}
	{ // . 940[12] (1854)
		int i = 26;
		unsigned addr  = 0 + MICRO_WIDTH * (i / DRAW_HEIGHT) + ( 14 + MICRO_HEIGHT * (i % DRAW_HEIGHT)) * BUFFER_WIDTH;
		gpBuffer[addr] = TRANS_COLOR;
	}
	{ // 383[5] (1854) <- 180[1]
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
	{ // 383[7] (1854) <- 180[1]
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

#ifdef HELLFIRE
static void patchCryptMin(BYTE *buf)
{
	uint16_t *pSubtiles = (uint16_t*)buf;
	constexpr int blockSize = BLOCK_SIZE_L5;
	// adjust the frame types
	// - after fillCryptShapes
	SetFrameType(159, 3, MET_SQUARE);  // 473
	SetFrameType(185, 3, MET_SQUARE);  // 473
	// SetFrameType(159, 1, MET_RTRAPEZOID); // 475
	SetFrameType(336, 0, MET_LTRIANGLE); // 907
	SetFrameType(409, 0, MET_LTRIANGLE); // 1168
	SetFrameType(481, 1, MET_RTRIANGLE); // 1406
	SetFrameType(492, 0, MET_LTRIANGLE); // 1436
	SetFrameType(519, 0, MET_LTRIANGLE); // 1493
	SetFrameType(595, 1, MET_RTRIANGLE); // 1710
	// - after maskCryptBlacks
	SetFrameType(126, 1, MET_TRANSPARENT); // 347
	SetFrameType(129, 0, MET_TRANSPARENT); // 356
	SetFrameType(129, 1, MET_TRANSPARENT); // 357
	SetFrameType(131, 0, MET_TRANSPARENT); // 362
	SetFrameType(131, 1, MET_TRANSPARENT); // 363
	SetFrameType(132, 0, MET_TRANSPARENT); // 364
	SetFrameType(133, 0, MET_TRANSPARENT); // 371
	SetFrameType(133, 1, MET_TRANSPARENT); // 372
	SetFrameType(134, 3, MET_TRANSPARENT); // 375
	SetFrameType(135, 0, MET_TRANSPARENT); // 379
	SetFrameType(135, 1, MET_TRANSPARENT); // 380
	SetFrameType(142, 0, MET_TRANSPARENT); // 403
//	SetFrameType(146, 0, MET_TRANSPARENT); // 356
//	SetFrameType(149, 4, MET_TRANSPARENT); // 356
//	SetFrameType(150, 6, MET_TRANSPARENT); // 356
	SetFrameType(151, 2, MET_TRANSPARENT); // 438
	SetFrameType(151, 4, MET_TRANSPARENT); // 436
	SetFrameType(151, 5, MET_TRANSPARENT); // 437
	SetFrameType(152, 7, MET_TRANSPARENT); // 439
	SetFrameType(153, 2, MET_TRANSPARENT); // 442
	SetFrameType(153, 4, MET_TRANSPARENT); // 441
	SetFrameType(159, 1, MET_TRANSPARENT); // 475
//	SetFrameType(178, 2, MET_TRANSPARENT); // 442
	// - after fixCryptShadows
	// SetFrameType(630, 0, MET_TRANSPARENT); // 1804
	SetFrameType(620, 0, MET_RTRIANGLE);   // 1798
	SetFrameType(621, 1, MET_SQUARE);      // 1800
	SetFrameType(625, 0, MET_RTRIANGLE);   // 1805
	SetFrameType(624, 0, MET_TRANSPARENT); // 1813
	SetFrameType(619, 1, MET_LTRAPEZOID);  // 1797
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
	ReplaceMcr(564, 0, 303, 0);
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
	ReplaceMcr(564, 9, 6, 9); // lost details
	ReplaceMcr(277, 7, 18, 7); // lost details
	ReplaceMcr(562, 7, 18, 7); // lost details
	ReplaceMcr(277, 5, 459, 5); // lost details
	ReplaceMcr(562, 5, 459, 5); // lost details
	ReplaceMcr(277, 3, 459, 3); // lost details
	ReplaceMcr(562, 1, 277, 1); // lost details
	ReplaceMcr(564, 1, 277, 1); // lost details
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
		8, 10, 11, 16, 19, 20, 23, 24, 26, 28, 30, 35, 38, 40, 43, 44, 50, 52, 56, 76, 78, 81, 82, 87, 90, 92, 94, 96, 98, 100, 102, 103, 105, 106, 108, 110, 112, 114, 116, 124, 127, 128, 137, 138, 139, 141, 143, 147, 148, 167, 172, 174, 176, 177, 193, 202, 205, 207, 210, 211, 214, 217, 219, 221, 223, 225, 227, 233, 235, 239, 249, 251, 253, 257, 259, 262, 263, 270, 273, 278, 279, 295, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 354, 373, 381, 390, 472, 489, 490, 540, 560, 640, 643, 648
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
		{ 159 - 1, 3, MET_SQUARE },      // 473
//		{ 159 - 1, 3, MET_RTRAPEZOID },  // 475
		{ 336 - 1, 0, MET_LTRIANGLE },   // 907
		{ 409 - 1, 0, MET_LTRIANGLE },   // 1168
		{ 481 - 1, 1, MET_RTRIANGLE },   // 1406
		{ 492 - 1, 0, MET_LTRIANGLE },   // 1436
		{ 519 - 1, 0, MET_LTRIANGLE },   // 1493
		{ 595 - 1, 1, MET_RTRIANGLE },   // 1710
		{ 368 - 1, 1, MET_RTRIANGLE },   // 1034
		{ 162 - 1, 2, MET_TRANSPARENT }, // 483
		{  63 - 1, 4, MET_SQUARE },      // 239
		{ 450 - 1, 0, MET_TRANSPARENT }, // 1315
		{ 206 - 1, 0, MET_TRANSPARENT }, // 571
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
	memset(&gpBuffer[0], TRANS_COLOR, 3 * BUFFER_WIDTH * MICRO_HEIGHT);

	unsigned xx = 0, yy = MICRO_HEIGHT - 1;
	for (int i = 0; i < lengthof(micros); i++) {
		const CelMicro &micro = micros[i];
		unsigned index = MICRO_IDX(micro.subtileIndex, blockSize, micro.microIndex);
		RenderMicro(&gpBuffer[xx + yy * BUFFER_WIDTH], SwapLE16(pSubtiles[index]), DMT_NONE);
		yy += MICRO_HEIGHT;
		if (yy == 4 * MICRO_HEIGHT - 1) {
			yy = MICRO_HEIGHT - 1;
			xx += MICRO_WIDTH;
		}
	}

	gpBuffer[0 * MICRO_WIDTH + 30 + (MICRO_HEIGHT * 1 +  1) * BUFFER_WIDTH] = 46; // 907
	gpBuffer[0 * MICRO_WIDTH + 31 + (MICRO_HEIGHT * 1 +  1) * BUFFER_WIDTH] = 76;

	gpBuffer[1 * MICRO_WIDTH +  0 + (MICRO_HEIGHT * 2 + 16) * BUFFER_WIDTH] = 43; // 1493

	gpBuffer[2 * MICRO_WIDTH +  0 + (MICRO_HEIGHT * 1 +  7) * BUFFER_WIDTH] = 43; // 1043
	gpBuffer[2 * MICRO_WIDTH +  0 + (MICRO_HEIGHT * 1 +  9) * BUFFER_WIDTH] = 41;

	gpBuffer[2 * MICRO_WIDTH + 31 + (MICRO_HEIGHT * 2 + 13) * BUFFER_WIDTH] = 41; // 483
	gpBuffer[2 * MICRO_WIDTH + 31 + (MICRO_HEIGHT * 2 + 14) * BUFFER_WIDTH] = 36;
	gpBuffer[2 * MICRO_WIDTH + 31 + (MICRO_HEIGHT * 2 + 18) * BUFFER_WIDTH] = 36;
	gpBuffer[2 * MICRO_WIDTH + 30 + (MICRO_HEIGHT * 2 + 19) * BUFFER_WIDTH] = 35;
	gpBuffer[2 * MICRO_WIDTH + 31 + (MICRO_HEIGHT * 2 + 20) * BUFFER_WIDTH] = 45;
	gpBuffer[2 * MICRO_WIDTH + 31 + (MICRO_HEIGHT * 2 + 21) * BUFFER_WIDTH] = 63;
	gpBuffer[2 * MICRO_WIDTH + 31 + (MICRO_HEIGHT * 2 + 22) * BUFFER_WIDTH] = 40;
	gpBuffer[2 * MICRO_WIDTH + 31 + (MICRO_HEIGHT * 2 + 29) * BUFFER_WIDTH] = 36;

	gpBuffer[3 * MICRO_WIDTH +  0 + (MICRO_HEIGHT * 0 + 19) * BUFFER_WIDTH] = 91; // 239
	gpBuffer[3 * MICRO_WIDTH +  0 + (MICRO_HEIGHT * 0 + 20) * BUFFER_WIDTH] = 93;

	gpBuffer[3 * MICRO_WIDTH +  26 + (MICRO_HEIGHT * 2 + 4) * BUFFER_WIDTH] = 45; // 571

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
				gpBuffer[3 * MICRO_WIDTH + xx + (MICRO_HEIGHT * 1 + yy) * BUFFER_WIDTH] = color; // 1315
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
		if (yy == 4 * MICRO_HEIGHT - 1) {
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
		{ 126 - 1, 1 }, // 347
		{ 129 - 1, 0 }, // 356
		{ 129 - 1, 1 }, // 357
		{ 131 - 1, 0 }, // 362
		{ 131 - 1, 1 }, // 363
		{ 132 - 1, 0 }, // 364
		{ 133 - 1, 0 }, // 371
		{ 133 - 1, 1 }, // 372
		{ 134 - 1, 3 }, // 375
		{ 135 - 1, 0 }, // 379
		{ 135 - 1, 1 }, // 380
		{ 142 - 1, 0 }, // 403
//		{ 146 - 1, 0 }, // 356
//		{ 149 - 1, 4 }, // 356
//		{ 150 - 1, 6 }, // 356
//		{ 151 - 1, 2 }, // 438
		{ 151 - 1, 4 }, // 436
		{ 151 - 1, 5 }, // 437
		{ 152 - 1, 7 }, // 439
		{ 153 - 1, 2 }, // 442
		{ 153 - 1, 4 }, // 441
		{ 159 - 1, 1 }, // 475
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
					if (i == 0 && x < 9) { // 126, 1
						continue;
					}
					if (i == 1 && y < 10) { // 129, 0
						continue;
					}
					if (i == 6 && y < 10) { // 133, 0
						continue;
					}
					if (i == 7 && y == 0) { // 133, 1
						continue;
					}
					if (i == 12 && (y < MICRO_HEIGHT - (x - 4) / 2)) { // 151, 4
						continue;
					}
					if (i == 13 && ((x < 6 && y < 18) || (x >= 6 && y < 18 - (x - 6) / 2))) { // 151, 5
						continue;
					}
					if (i == 14 && (y < 16 - (x - 3) / 2)) { // 152, 7
						continue;
					}
					if (i == 16 && y < 21) { // 153, 4
						continue;
					}
					if (i == 17 && x < 10 && y < 3) { // 159, 1
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
	unsigned back3_FrameRef0 = pSubtiles[back3_FrameIndex0] & 0xFFF; // 719
	unsigned back2_FrameIndex1 = MICRO_IDX((backSubtileRef2 - 1), blockSize, 1);
	unsigned back2_FrameRef1 = pSubtiles[back2_FrameIndex1] & 0xFFF; // 718
	unsigned back0_FrameIndex0 = MICRO_IDX((backSubtileRef0 - 1), blockSize, 0);
	unsigned back0_FrameRef0 = pSubtiles[back0_FrameIndex0] & 0xFFF; // 716

	unsigned stairs_FrameIndex0 = MICRO_IDX((stairsSubtileRef1 - 1), blockSize, 0);
	unsigned stairs_FrameRef0 = pSubtiles[stairs_FrameIndex0] & 0xFFF; // 770
	unsigned stairs_FrameIndex2 = MICRO_IDX((stairsSubtileRef1 - 1), blockSize, 2);
	unsigned stairs_FrameRef2 = pSubtiles[stairs_FrameIndex2] & 0xFFF; // 769
	unsigned stairs_FrameIndex4 = MICRO_IDX((stairsSubtileRef1 - 1), blockSize, 4);
	unsigned stairs_FrameRef4 = pSubtiles[stairs_FrameIndex4] & 0xFFF; // 768
	unsigned stairs_FrameIndex6 = MICRO_IDX((stairsSubtileRef1 - 1), blockSize, 6);
	unsigned stairs_FrameRef6 = pSubtiles[stairs_FrameIndex6] & 0xFFF; // 767

	unsigned stairsExt_FrameIndex1 = MICRO_IDX((stairsExtSubtileRef1 - 1), blockSize, 1);
	unsigned stairsExt_FrameRef1 = pSubtiles[stairsExt_FrameIndex1] & 0xFFF; // 762
	unsigned stairsExt_FrameIndex3 = MICRO_IDX((stairsExtSubtileRef1 - 1), blockSize, 3);
	unsigned stairsExt_FrameRef3 = pSubtiles[stairsExt_FrameIndex3] & 0xFFF; // 761
	unsigned stairsExt_FrameIndex5 = MICRO_IDX((stairsExtSubtileRef1 - 1), blockSize, 5);
	unsigned stairsExt_FrameRef5 = pSubtiles[stairsExt_FrameIndex5] & 0xFFF; // 760

	if (back3_FrameRef0 == 0 || back2_FrameRef1 == 0 || back0_FrameRef0 == 0) {
		mem_free_dbg(celBuf);
		app_warn("The back-stairs tile (%d) has invalid (missing) frames.", backTileIndex1 + 1);
		return NULL;
	}

	if (stairs_FrameRef0 == 0 || stairs_FrameRef2 == 0 || stairs_FrameRef4 == 0 || stairs_FrameRef6 == 0
		|| stairsExt_FrameRef1 == 0 || stairsExt_FrameRef3 == 0 || stairsExt_FrameRef5 == 0) {
		return celBuf;
	}

	unsigned stairs2_FrameIndex0 = MICRO_IDX((stairsSubtileRef2 - 1), blockSize, 0);
	unsigned stairs2_FrameRef0 = pSubtiles[stairs2_FrameIndex0] & 0xFFF; // 770
	unsigned stairs2_FrameIndex2 = MICRO_IDX((stairsSubtileRef2 - 1), blockSize, 2);
	unsigned stairs2_FrameRef2 = pSubtiles[stairs2_FrameIndex2] & 0xFFF; // 769
	unsigned stairs2_FrameIndex4 = MICRO_IDX((stairsSubtileRef2 - 1), blockSize, 4);
	unsigned stairs2_FrameRef4 = pSubtiles[stairs2_FrameIndex4] & 0xFFF; // 768
	unsigned stairs2_FrameIndex6 = MICRO_IDX((stairsSubtileRef2 - 1), blockSize, 6);
	unsigned stairs2_FrameRef6 = pSubtiles[stairs2_FrameIndex6] & 0xFFF; // 767
	if (stairs2_FrameRef0 != stairs_FrameRef0) {
		mem_free_dbg(celBuf);
		app_warn("The stairs subtiles (%d, %d) have invalid (mismatching) floor frames.", stairsSubtileRef1, stairsSubtileRef2);
		return NULL;
	}
	unsigned stairsExt2_FrameIndex1 = MICRO_IDX((stairsExtSubtileRef2 - 1), blockSize, 1);
	unsigned stairsExt2_FrameRef1 = pSubtiles[stairsExt2_FrameIndex1] & 0xFFF; // 762
	unsigned stairsExt2_FrameIndex3 = MICRO_IDX((stairsExtSubtileRef2 - 1), blockSize, 3);
	unsigned stairsExt2_FrameRef3 = pSubtiles[stairsExt2_FrameIndex3] & 0xFFF; // 761
	unsigned stairsExt2_FrameIndex5 = MICRO_IDX((stairsExtSubtileRef2 - 1), blockSize, 5);
	unsigned stairsExt2_FrameRef5 = pSubtiles[stairsExt2_FrameIndex5] & 0xFFF; // 760
	if (stairsExt2_FrameRef1 != stairsExt_FrameRef1
		|| stairsExt2_FrameRef3 != stairsExt_FrameRef3
		|| stairsExt2_FrameRef5 != stairsExt_FrameRef5) {
		mem_free_dbg(celBuf);
		app_warn("The stairs external subtiles (%d, %d) have invalid (mismatching) frames.", stairsExtSubtileRef1, stairsExtSubtileRef2);
		return NULL;
	}

	// draw the micros to the back-buffer
	pMicrosCel = celBuf;
	constexpr BYTE TRANS_COLOR = 128;
	memset(&gpBuffer[0], TRANS_COLOR, 5 * BUFFER_WIDTH * MICRO_HEIGHT);

	// RenderMicro(&gpBuffer[0 + (MICRO_HEIGHT * 3 - 1) * BUFFER_WIDTH], pSubtiles[back0_FrameIndex0], DMT_NONE); // 716
	// RenderMicro(&gpBuffer[0 + (MICRO_HEIGHT * 4 - MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[back2_FrameIndex1], DMT_NONE); // 718
	RenderMicro(&gpBuffer[0 + (MICRO_HEIGHT * 4 - 1) * BUFFER_WIDTH], pSubtiles[back3_FrameIndex0], DMT_NONE); // 719

	RenderMicro(&gpBuffer[0 + (MICRO_HEIGHT * 2 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[stairsExt_FrameIndex5], DMT_NONE); // 760
	RenderMicro(&gpBuffer[0 + (MICRO_HEIGHT * 3 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[stairsExt_FrameIndex3], DMT_NONE); // 761
	// RenderMicro(&gpBuffer[0 + (MICRO_HEIGHT * 4 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[stairsExt_FrameIndex1], DMT_NONE); // 762

	RenderMicro(&gpBuffer[MICRO_WIDTH + (MICRO_HEIGHT * 1 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[stairs_FrameIndex6], DMT_NONE); // 767
	RenderMicro(&gpBuffer[MICRO_WIDTH + (MICRO_HEIGHT * 2 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[stairs_FrameIndex4], DMT_NONE); // 768
	RenderMicro(&gpBuffer[MICRO_WIDTH + (MICRO_HEIGHT * 3 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[stairs_FrameIndex2], DMT_NONE); // 769
	RenderMicro(&gpBuffer[MICRO_WIDTH + (MICRO_HEIGHT * 4 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[stairs_FrameIndex0], DMT_NONE); // 770

	RenderMicro(&gpBuffer[2 * MICRO_WIDTH + (MICRO_HEIGHT * 1 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[back0_FrameIndex0], DMT_NONE);     // 716
	RenderMicro(&gpBuffer[2 * MICRO_WIDTH + (MICRO_HEIGHT * 2 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[back2_FrameIndex1], DMT_NONE);     // 718
	RenderMicro(&gpBuffer[2 * MICRO_WIDTH + (MICRO_HEIGHT * 3 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[stairsExt_FrameIndex1], DMT_NONE); // 762
	RenderMicro(&gpBuffer[2 * MICRO_WIDTH + (MICRO_HEIGHT * 4 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[stairs_FrameIndex0], DMT_NONE);    // 770

	RenderMicro(&gpBuffer[3 * MICRO_WIDTH + (MICRO_HEIGHT * 3 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[stairsExt_FrameIndex1], DMT_NONE); // 762

	// add micros to be used as masks
	RenderMicro(&gpBuffer[3 * MICRO_WIDTH + (MICRO_HEIGHT * 1 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[back3_FrameIndex0], DMT_NONE); // 719
	RenderMicro(&gpBuffer[3 * MICRO_WIDTH + (MICRO_HEIGHT * 2 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH], pSubtiles[back2_FrameIndex1], DMT_NONE); // 718

	// mask the drawing
	for (int x = 0; x < MICRO_WIDTH; x++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			BYTE pixel = gpBuffer[3 * MICRO_WIDTH + x + (MICRO_HEIGHT * 1 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH];
			if (pixel == TRANS_COLOR) {
				gpBuffer[2 * MICRO_WIDTH + x + (MICRO_HEIGHT * 4 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH] = TRANS_COLOR; // mask the stair-floor with 719
			} else {
				gpBuffer[MICRO_WIDTH + x + (MICRO_HEIGHT * 4 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH] = TRANS_COLOR;// mask the stairs with ~719
			}
		}
	}
	for (int x = 0; x < MICRO_WIDTH; x++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			BYTE pixel = gpBuffer[3 * MICRO_WIDTH + x + (MICRO_HEIGHT * 2 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH];
			if (pixel == TRANS_COLOR) {
				gpBuffer[2 * MICRO_WIDTH + x + (MICRO_HEIGHT * 3 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH] = TRANS_COLOR; // mask the external back-subtile floor with 718
			} else {
				gpBuffer[3 * MICRO_WIDTH + x + (MICRO_HEIGHT * 3 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH] = TRANS_COLOR; // mask the external back-subtile with ~718
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
				gpBuffer[2 * MICRO_WIDTH + x + (MICRO_HEIGHT * 1 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH] = TRANS_COLOR; // 716
			}
		}
	}
	for (int x = 0; x < MICRO_WIDTH; x++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			BYTE pixel = gpBuffer[2 * MICRO_WIDTH + x + (MICRO_HEIGHT * 2 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH];
			if (pixel == TRANS_COLOR)
				continue;
			if (x < 22 || y >= MICRO_HEIGHT - 12 || y < MICRO_HEIGHT - 20 || (pixel % 16) < 11) {
				gpBuffer[2 * MICRO_WIDTH + x + (MICRO_HEIGHT * 2 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH] = TRANS_COLOR; // 718
			}
		}
	}
	// copy the stair of back-subtile to its position
	for (int x = 0; x < MICRO_WIDTH; x++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			BYTE pixel = gpBuffer[3 * MICRO_WIDTH + x + (MICRO_HEIGHT * 3 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH]; // 762
			if (pixel == TRANS_COLOR)
				continue;
			// if (gpBuffer[0 + x + (MICRO_HEIGHT * 3 - y - 1) * BUFFER_WIDTH] == TRANS_COLOR)
				gpBuffer[0 + x + (MICRO_HEIGHT * 4 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH] = pixel;
		}
	}
	// copy the shadows to their position
	for (int x = 0; x < MICRO_WIDTH; x++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			BYTE pixel = gpBuffer[2 * MICRO_WIDTH + x + (MICRO_HEIGHT * 1 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH]; // 716
			if (pixel == TRANS_COLOR)
				continue;
			if (gpBuffer[0 + x + (MICRO_HEIGHT * 3 - y - 1) * BUFFER_WIDTH] == TRANS_COLOR)
				gpBuffer[0 + x + (MICRO_HEIGHT * 3 - y - 1) * BUFFER_WIDTH] = pixel;
		}
	}
	for (int x = 0; x < MICRO_WIDTH; x++) {
		for (int y = 0; y < MICRO_HEIGHT; y++) {
			BYTE pixel = gpBuffer[2 * MICRO_WIDTH + x + (MICRO_HEIGHT * 2 + MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH]; // 718
			if (pixel == TRANS_COLOR)
				continue;
			if (gpBuffer[0 + x + (MICRO_HEIGHT * 4 - MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH] == TRANS_COLOR)
				gpBuffer[0 + x + (MICRO_HEIGHT * 4 - MICRO_HEIGHT / 2 - y - 1) * BUFFER_WIDTH] = pixel;
		}
	}

	// fix bad artifacts
	gpBuffer[0 + 23 + (MICRO_HEIGHT * 3 - (MICRO_HEIGHT - 20 - 1) - 1) * BUFFER_WIDTH] = TRANS_COLOR; // 761
	gpBuffer[0 + 24 + (MICRO_HEIGHT * 3 - (MICRO_HEIGHT - 20 - 1) - 1) * BUFFER_WIDTH] = TRANS_COLOR; // 761
	gpBuffer[0 + 22 + (MICRO_HEIGHT * 3 - (MICRO_HEIGHT - 21 - 1) - 1) * BUFFER_WIDTH] = TRANS_COLOR; // 761
	gpBuffer[0 + 23 + (MICRO_HEIGHT * 3 - (MICRO_HEIGHT - 21 - 1) - 1) * BUFFER_WIDTH] = TRANS_COLOR; // 761

	// create the new CEL file
	BYTE* resCelBuf = DiabloAllocPtr(*celLen + 8 * MICRO_WIDTH * MICRO_HEIGHT);

	typedef struct {
		int type;
		unsigned frameRef;
	} CelFrameEntry;
	CelFrameEntry entries[8];
	entries[0].type = 0; // 762
	// assert(stairsExt_FrameRef1 == 762);
	entries[0].frameRef = stairsExt_FrameRef1;
	entries[1].type = 1; // 770
	// assert(stairs_FrameRef0 == 770);
	entries[1].frameRef = stairs_FrameRef0;
	entries[2].type = 2; // 767
	// assert(stairs_FrameRef6 == 767);
	entries[2].frameRef = stairs_FrameRef6;
	entries[3].type = 3; // 768
	// assert(stairs_FrameRef4 == 768);
	entries[3].frameRef = stairs_FrameRef4;
	entries[4].type = 4; // 769
	// assert(stairs_FrameRef2 == 769);
	entries[4].frameRef = stairs_FrameRef2;
	entries[5].type = 5; // 760
	// assert(stairsExt_FrameRef5 == 760);
	entries[5].frameRef = stairsExt_FrameRef5;
	entries[6].type = 6; // 719
	// assert(back3_FrameRef0 == 719);
	entries[6].frameRef = back3_FrameRef0;
	entries[7].type = 7; // 761
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
		case 0: // 762
			frameSrc = &gpBuffer[2 * MICRO_WIDTH + (MICRO_HEIGHT * 3 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH];
			encoding = MET_RTRIANGLE;
			break;
		case 1: // 770
			frameSrc = &gpBuffer[2 * MICRO_WIDTH + (MICRO_HEIGHT * 4 + MICRO_HEIGHT / 2 - 1) * BUFFER_WIDTH];
			encoding = MET_LTRIANGLE;
			break;
		case 2: // 767
			frameSrc = &gpBuffer[MICRO_WIDTH + (MICRO_HEIGHT * 1 - 1) * BUFFER_WIDTH];
			break;
		case 3: // 768
			frameSrc = &gpBuffer[MICRO_WIDTH + (MICRO_HEIGHT * 2 - 1) * BUFFER_WIDTH];
			break;
		case 4: // 769
			frameSrc = &gpBuffer[MICRO_WIDTH + (MICRO_HEIGHT * 3 - 1) * BUFFER_WIDTH];
			break;
		case 5: // 760
			frameSrc = &gpBuffer[MICRO_WIDTH + (MICRO_HEIGHT * 4 - 1) * BUFFER_WIDTH];
			encoding = MET_RTRAPEZOID;
			break;
		case 6: // 719
			frameSrc = &gpBuffer[0 + (MICRO_HEIGHT * 4 - 1) * BUFFER_WIDTH];
			break;
		case 7: // 761
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
	case FILE_CATHEDRAL_MIN:
	{	// patch dMiniTiles - L1.MIN
		if (*dwLen < MICRO_IDX(140 - 1, BLOCK_SIZE_L1, 1) * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		uint16_t *pSubtiles = (uint16_t*)buf;
		constexpr int blockSize = BLOCK_SIZE_L1;
		// useless black micros
		Blk2Mcr(107, 0);
		Blk2Mcr(107, 1);
		Blk2Mcr(109, 1);
		Blk2Mcr(137, 1);
		Blk2Mcr(138, 0);
		Blk2Mcr(138, 1);
		Blk2Mcr(140, 1);
	} break;
#endif /* ASSET_MPL == 1 */
	case FILE_CATHEDRAL_SOL:
	{	// patch dSolidTable - L1.SOL
		if (*dwLen <= 8) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		nMissileTable(8, false); // the only column which was blocking missiles
	} break;
#if ASSET_MPL == 1
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
		mem_free_dbg(minBuf);
		mem_free_dbg(tilBuf);
	} break;
	case FILE_CATACOMBS_MIN:
	{	// patch dMiniTiles - L2.MIN
		// add separate tiles and subtiles for the arches II.
		constexpr int blockSize = BLOCK_SIZE_L2;
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
		// fix the upstairs III.
		if (pSubtiles[MICRO_IDX(265 - 1, blockSize, 3)] != 0) {
			// move the frames to the back subtile
			// - left side
			pSubtiles[MICRO_IDX(252 - 1, blockSize, 2)] = pSubtiles[MICRO_IDX(265 - 1, blockSize, 3)]; // 761
			pSubtiles[MICRO_IDX(265 - 1, blockSize, 3)] = 0;
			pSubtiles[MICRO_IDX(556 - 1, blockSize, 3)] = 0;

			// - right side
			pSubtiles[MICRO_IDX(252 - 1, blockSize, 1)] = pSubtiles[MICRO_IDX(265 - 1, blockSize, 5)]; // 760
			pSubtiles[MICRO_IDX(265 - 1, blockSize, 5)] = 0;
			pSubtiles[MICRO_IDX(556 - 1, blockSize, 5)] = 0;

			pSubtiles[MICRO_IDX(252 - 1, blockSize, 3)] = pSubtiles[MICRO_IDX(267 - 1, blockSize, 2)]; // 769
			pSubtiles[MICRO_IDX(267 - 1, blockSize, 2)] = 0;
			pSubtiles[MICRO_IDX(559 - 1, blockSize, 2)] = 0;

			pSubtiles[MICRO_IDX(252 - 1, blockSize, 5)] = pSubtiles[MICRO_IDX(267 - 1, blockSize, 4)]; // 768
			pSubtiles[MICRO_IDX(267 - 1, blockSize, 4)] = 0;
			pSubtiles[MICRO_IDX(559 - 1, blockSize, 4)] = 0;

			pSubtiles[MICRO_IDX(252 - 1, blockSize, 7)] = pSubtiles[MICRO_IDX(267 - 1, blockSize, 6)]; // 767
			pSubtiles[MICRO_IDX(267 - 1, blockSize, 6)] = 0;
			pSubtiles[MICRO_IDX(559 - 1, blockSize, 6)] = 0;

			// - adjust the frame types
			SetFrameType(267, 0, MET_LTRIANGLE);
			SetFrameType(559, 0, MET_LTRIANGLE);
			SetFrameType(252, 3, MET_TRANSPARENT);
			SetFrameType(252, 1, MET_RTRAPEZOID);
			SetFrameType(265, 1, MET_RTRIANGLE);
			SetFrameType(556, 1, MET_RTRIANGLE);
			SetFrameType(252, 0, MET_TRANSPARENT);
		}
		// fix bad artifact
		Blk2Mcr(288, 7);
	} break;
#endif
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
		// fix the upstairs II.
		pTiles[(72 - 1) * 4 + 0] = SwapLE16(9 - 1);   // use common subtile
		pTiles[(72 - 1) * 4 + 1] = SwapLE16(56 - 1);  // make the back of the stairs non-walkable
		pTiles[(72 - 1) * 4 + 2] = SwapLE16(11 - 1);  // use common subtile
		pTiles[(76 - 1) * 4 + 1] = SwapLE16(10 - 1);  // use common subtile
		pTiles[(158 - 1) * 4 + 0] = SwapLE16(9 - 1);  // use common subtile
		pTiles[(158 - 1) * 4 + 1] = SwapLE16(56 - 1); // make the back of the stairs non-walkable
		pTiles[(158 - 1) * 4 + 2] = SwapLE16(11 - 1); // use common subtile
		pTiles[(159 - 1) * 4 + 0] = SwapLE16(9 - 1);  // use common subtile
		pTiles[(159 - 1) * 4 + 1] = SwapLE16(10 - 1); // use common subtile
		pTiles[(159 - 1) * 4 + 2] = SwapLE16(11 - 1); // use common subtile
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
		// fix the upstairs IV.
		// - make the back of the stairs non-walkable
		nSolidTable(252, true);
		nBlockTable(252, true);
		nMissileTable(252, true);
		// - make the stair-floor non light-blocker
		nBlockTable(267, false);
		nBlockTable(559, false);
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
		uint16_t *pMegaTiles = (uint16_t*)buf;
		// fix automap of the entrance I.
		pMegaTiles[4 * (52 - 1) + 0] = SwapLE16(73 - 1); // 45 - copy from 23
		pMegaTiles[4 * (52 - 1) + 1] = SwapLE16(64 - 1); // 46
		pMegaTiles[4 * (52 - 1) + 2] = SwapLE16(65 - 1); // 148
		pMegaTiles[4 * (52 - 1) + 3] = SwapLE16(66 - 1); // 48
		pMegaTiles[4 * (58 - 1) + 0] = SwapLE16(63 - 1); // 166 - copy from 18
		pMegaTiles[4 * (58 - 1) + 1] = SwapLE16(64 - 1); // 167
		pMegaTiles[4 * (58 - 1) + 2] = SwapLE16(65 - 1); // 47
		pMegaTiles[4 * (58 - 1) + 3] = SwapLE16(66 - 1); // 48
		pMegaTiles[4 * (53 - 1) + 1] = SwapLE16(148 - 1); // 130
		pMegaTiles[4 * (53 - 1) + 3] = SwapLE16(148 - 1); // 130
		// use common subtiles of doors
		pMegaTiles[4 * (71 - 1) + 2] = SwapLE16(206 - 1);
		pMegaTiles[4 * (72 - 1) + 2] = SwapLE16(206 - 1);
		// use common subtiles
		pMegaTiles[4 * (4 - 1) + 1] = SwapLE16(6 - 1); // 14
		pMegaTiles[4 * (14 - 1) + 1] = SwapLE16(6 - 1);
		pMegaTiles[4 * (115 - 1) + 1] = SwapLE16(6 - 1);
		pMegaTiles[4 * (132 - 1) + 1] = SwapLE16(6 - 1);
		pMegaTiles[4 * (1 - 1) + 2] = SwapLE16(15 - 1); // 3
		pMegaTiles[4 * (27 - 1) + 2] = SwapLE16(15 - 1);
		pMegaTiles[4 * (43 - 1) + 2] = SwapLE16(15 - 1);
		pMegaTiles[4 * (79 - 1) + 2] = SwapLE16(15 - 1);
		pMegaTiles[4 * (6 - 1) + 2] = SwapLE16(15 - 1);  // 23
		pMegaTiles[4 * (127 - 1) + 2] = SwapLE16(4 - 1); // 372
		pMegaTiles[4 * (132 - 1) + 2] = SwapLE16(15 - 1); // 388
		pMegaTiles[4 * (156 - 1) + 2] = SwapLE16(31 - 1); // 468
		// use better subtiles
		// - increase glow
		pMegaTiles[4 * (96 - 1) + 3] = SwapLE16(293 - 1); // 279
		pMegaTiles[4 * (187 - 1) + 3] = SwapLE16(293 - 1);
		pMegaTiles[4 * (188 - 1) + 3] = SwapLE16(293 - 1);
		pMegaTiles[4 * (90 - 1) + 1] = SwapLE16(297 - 1); // 253
		pMegaTiles[4 * (175 - 1) + 1] = SwapLE16(297 - 1);
		// - reduce glow
		pMegaTiles[4 * (162 - 1) + 1] = SwapLE16(297 - 1); // 489
		pMegaTiles[4 * (162 - 1) + 2] = SwapLE16(266 - 1); // 490
		// create the new shadows
		// - use the shadows created by fixCryptShadows
		pMegaTiles[4 * (203 - 1) + 0] = SwapLE16(638 - 1); // 619
		pMegaTiles[4 * (203 - 1) + 1] = SwapLE16(639 - 1); // 620
		pMegaTiles[4 * (203 - 1) + 2] = SwapLE16(623 - 1); // 47
		pMegaTiles[4 * (203 - 1) + 3] = SwapLE16(627 - 1); // 621
		pMegaTiles[4 * (204 - 1) + 0] = SwapLE16(638 - 1); // 622
		pMegaTiles[4 * (204 - 1) + 1] = SwapLE16(639 - 1); // 46
		pMegaTiles[4 * (204 - 1) + 2] = SwapLE16(636 - 1); // 623
		pMegaTiles[4 * (204 - 1) + 3] = SwapLE16(627 - 1); // 624
		pMegaTiles[4 * (108 - 1) + 2] = SwapLE16(631 - 1); // 810
		pMegaTiles[4 * (108 - 1) + 3] = SwapLE16(626 - 1); // 811
		pMegaTiles[4 * (210 - 1) + 3] = SwapLE16(371 - 1); // 637

		pMegaTiles[4 * (109 - 1) + 0] = SwapLE16(1 - 1);   // 312
		pMegaTiles[4 * (109 - 1) + 1] = SwapLE16(2 - 1);   // 313
		pMegaTiles[4 * (109 - 1) + 2] = SwapLE16(3 - 1);   // 314
		pMegaTiles[4 * (109 - 1) + 3] = SwapLE16(627 - 1); // 315
		pMegaTiles[4 * (110 - 1) + 0] = SwapLE16(21 - 1);  // 316
		pMegaTiles[4 * (110 - 1) + 1] = SwapLE16(22 - 1);  // 313
		pMegaTiles[4 * (110 - 1) + 2] = SwapLE16(3 - 1);   // 314
		pMegaTiles[4 * (110 - 1) + 3] = SwapLE16(627 - 1); // 315
		pMegaTiles[4 * (111 - 1) + 0] = SwapLE16(39 - 1);  // 317
		pMegaTiles[4 * (111 - 1) + 1] = SwapLE16(4 - 1);   // 318
		pMegaTiles[4 * (111 - 1) + 2] = SwapLE16(242 - 1); // 319
		pMegaTiles[4 * (111 - 1) + 3] = SwapLE16(627 - 1); // 320
		pMegaTiles[4 * (215 - 1) + 0] = SwapLE16(101 - 1); // 645
		pMegaTiles[4 * (215 - 1) + 1] = SwapLE16(4 - 1);   // 646
		pMegaTiles[4 * (215 - 1) + 2] = SwapLE16(178 - 1); // 45
		pMegaTiles[4 * (215 - 1) + 3] = SwapLE16(627 - 1); // 647
		// - 'add' new shadow-types with glow TODO: add wall/grate+glow
		pMegaTiles[4 * (216 - 1) + 0] = SwapLE16(39 - 1);  // 622
		pMegaTiles[4 * (216 - 1) + 1] = SwapLE16(4 - 1);   // 46
		pMegaTiles[4 * (216 - 1) + 2] = SwapLE16(238 - 1); // 648
		pMegaTiles[4 * (216 - 1) + 3] = SwapLE16(635 - 1); // 624
		pMegaTiles[4 * (217 - 1) + 0] = SwapLE16(638 - 1); // 625
		pMegaTiles[4 * (217 - 1) + 1] = SwapLE16(639 - 1); // 46
		pMegaTiles[4 * (217 - 1) + 2] = SwapLE16(634 - 1); // 649 TODO: could be better
		pMegaTiles[4 * (217 - 1) + 3] = SwapLE16(635 - 1); // 650
		// - 'add' new shadow-types with horizontal arches
		pMegaTiles[4 * (71 - 1) + 0] = SwapLE16(5 - 1); // copy from tile 2
		pMegaTiles[4 * (71 - 1) + 1] = SwapLE16(6 - 1);
		pMegaTiles[4 * (71 - 1) + 2] = SwapLE16(631 - 1);
		pMegaTiles[4 * (71 - 1) + 3] = SwapLE16(627 - 1);
		pMegaTiles[4 * (80 - 1) + 0] = SwapLE16(5 - 1); // copy from tile 2
		pMegaTiles[4 * (80 - 1) + 1] = SwapLE16(6 - 1);
		pMegaTiles[4 * (80 - 1) + 2] = SwapLE16(623 - 1);
		pMegaTiles[4 * (80 - 1) + 3] = SwapLE16(627 - 1);

		pMegaTiles[4 * (81 - 1) + 0] = SwapLE16(42 - 1); // copy from tile 12
		pMegaTiles[4 * (81 - 1) + 1] = SwapLE16(34 - 1);
		pMegaTiles[4 * (81 - 1) + 2] = SwapLE16(631 - 1);
		pMegaTiles[4 * (81 - 1) + 3] = SwapLE16(627 - 1);
		pMegaTiles[4 * (82 - 1) + 0] = SwapLE16(42 - 1); // copy from tile 12
		pMegaTiles[4 * (82 - 1) + 1] = SwapLE16(34 - 1);
		pMegaTiles[4 * (82 - 1) + 2] = SwapLE16(623 - 1);
		pMegaTiles[4 * (82 - 1) + 3] = SwapLE16(627 - 1);

		pMegaTiles[4 * (83 - 1) + 0] = SwapLE16(104 - 1); // copy from tile 36
		pMegaTiles[4 * (83 - 1) + 1] = SwapLE16(84 - 1);
		pMegaTiles[4 * (83 - 1) + 2] = SwapLE16(631 - 1);
		pMegaTiles[4 * (83 - 1) + 3] = SwapLE16(627 - 1);
		pMegaTiles[4 * (84 - 1) + 0] = SwapLE16(104 - 1); // copy from tile 36
		pMegaTiles[4 * (84 - 1) + 1] = SwapLE16(84 - 1);
		pMegaTiles[4 * (84 - 1) + 2] = SwapLE16(623 - 1);
		pMegaTiles[4 * (84 - 1) + 3] = SwapLE16(627 - 1);

		pMegaTiles[4 * (85 - 1) + 0] = SwapLE16(25 - 1); // copy from tile 7
		pMegaTiles[4 * (85 - 1) + 1] = SwapLE16(6 - 1);
		pMegaTiles[4 * (85 - 1) + 2] = SwapLE16(631 - 1);
		pMegaTiles[4 * (85 - 1) + 3] = SwapLE16(627 - 1);
		pMegaTiles[4 * (86 - 1) + 0] = SwapLE16(25 - 1); // copy from tile 7
		pMegaTiles[4 * (86 - 1) + 1] = SwapLE16(6 - 1);
		pMegaTiles[4 * (86 - 1) + 2] = SwapLE16(623 - 1);
		pMegaTiles[4 * (86 - 1) + 3] = SwapLE16(627 - 1);

		pMegaTiles[4 * (87 - 1) + 0] = SwapLE16(79 - 1); // copy from tile 26
		pMegaTiles[4 * (87 - 1) + 1] = SwapLE16(80 - 1);
		pMegaTiles[4 * (87 - 1) + 2] = SwapLE16(623 - 1);
		pMegaTiles[4 * (87 - 1) + 3] = SwapLE16(627 - 1);
		pMegaTiles[4 * (88 - 1) + 0] = SwapLE16(79 - 1); // copy from tile 26
		pMegaTiles[4 * (88 - 1) + 1] = SwapLE16(80 - 1);
		pMegaTiles[4 * (88 - 1) + 2] = SwapLE16(631 - 1);
		pMegaTiles[4 * (88 - 1) + 3] = SwapLE16(627 - 1);

		// use common subtiles instead of minor alterations
		pMegaTiles[4 * (7 - 1) + 1] = SwapLE16(6 - 1); // 26
		pMegaTiles[4 * (159 - 1) + 1] = SwapLE16(6 - 1); // 479
		pMegaTiles[4 * (133 - 1) + 2] = SwapLE16(31 - 1); // 390
		pMegaTiles[4 * (10 - 1) + 1] = SwapLE16(18 - 1); // 37
		pMegaTiles[4 * (138 - 1) + 1] = SwapLE16(18 - 1);
		pMegaTiles[4 * (188 - 1) + 1] = SwapLE16(277 - 1); // 564
		pMegaTiles[4 * (178 - 1) + 2] = SwapLE16(258 - 1); // 564
		pMegaTiles[4 * (5 - 1) + 2] = SwapLE16(31 - 1); // 19
		pMegaTiles[4 * (14 - 1) + 2] = SwapLE16(31 - 1);
		pMegaTiles[4 * (159 - 1) + 2] = SwapLE16(31 - 1);
		pMegaTiles[4 * (185 - 1) + 2] = SwapLE16(274 - 1); // 558
		pMegaTiles[4 * (186 - 1) + 2] = SwapLE16(274 - 1); // 560
		pMegaTiles[4 * (139 - 1) + 0] = SwapLE16(39 - 1); // 402

		pMegaTiles[4 * (2 - 1) + 3] = SwapLE16(4 - 1); // 8
		pMegaTiles[4 * (3 - 1) + 1] = SwapLE16(60 - 1); // 10
		pMegaTiles[4 * (114 - 1) + 1] = SwapLE16(32 - 1);
		pMegaTiles[4 * (3 - 1) + 2] = SwapLE16(4 - 1); // 11
		pMegaTiles[4 * (114 - 1) + 2] = SwapLE16(4 - 1);
		pMegaTiles[4 * (5 - 1) + 3] = SwapLE16(7 - 1); // 20
		pMegaTiles[4 * (14 - 1) + 3] = SwapLE16(4 - 1);
		pMegaTiles[4 * (133 - 1) + 3] = SwapLE16(4 - 1);
		pMegaTiles[4 * (125 - 1) + 3] = SwapLE16(7 - 1); // 50
		pMegaTiles[4 * (159 - 1) + 3] = SwapLE16(7 - 1);
		pMegaTiles[4 * (4 - 1) + 3] = SwapLE16(7 - 1); // 16
		pMegaTiles[4 * (132 - 1) + 3] = SwapLE16(4 - 1);
		pMegaTiles[4 * (10 - 1) + 3] = SwapLE16(7 - 1); // 38
		pMegaTiles[4 * (138 - 1) + 3] = SwapLE16(4 - 1);
		pMegaTiles[4 * (121 - 1) + 3] = SwapLE16(4 - 1); // 354
		pMegaTiles[4 * (8 - 1) + 3] = SwapLE16(4 - 1); // 32
		pMegaTiles[4 * (136 - 1) + 3] = SwapLE16(7 - 1);
		pMegaTiles[4 * (91 - 1) + 1] = SwapLE16(47 - 1); // 257
		pMegaTiles[4 * (178 - 1) + 1] = SwapLE16(47 - 1);
		pMegaTiles[4 * (91 - 1) + 3] = SwapLE16(48 - 1); // 259
		pMegaTiles[4 * (177 - 1) + 3] = SwapLE16(7 - 1);
		pMegaTiles[4 * (178 - 1) + 3] = SwapLE16(48 - 1);
		pMegaTiles[4 * (130 - 1) + 2] = SwapLE16(395 - 1); // 381
		pMegaTiles[4 * (157 - 1) + 2] = SwapLE16(4 - 1);   // 472
		pMegaTiles[4 * (177 - 1) + 1] = SwapLE16(4 - 1);   // 540
		pMegaTiles[4 * (211 - 1) + 3] = SwapLE16(48 - 1);  // 621
		pMegaTiles[4 * (205 - 1) + 0] = SwapLE16(45 - 1);  // 625
		pMegaTiles[4 * (207 - 1) + 0] = SwapLE16(45 - 1);  // 630
		pMegaTiles[4 * (207 - 1) + 3] = SwapLE16(627 - 1); // 632
		pMegaTiles[4 * (208 - 1) + 0] = SwapLE16(45 - 1);  // 633

		pMegaTiles[4 * (27 - 1) + 3] = SwapLE16(4 - 1); // 85
		// pMegaTiles[4 * (28 - 1) + 3] = SwapLE16(4 - 1); // 87
		pMegaTiles[4 * (29 - 1) + 3] = SwapLE16(4 - 1); // 90
		pMegaTiles[4 * (30 - 1) + 3] = SwapLE16(4 - 1); // 92
		pMegaTiles[4 * (31 - 1) + 3] = SwapLE16(4 - 1); // 94
		pMegaTiles[4 * (32 - 1) + 3] = SwapLE16(4 - 1); // 96
		pMegaTiles[4 * (33 - 1) + 3] = SwapLE16(4 - 1); // 98
		pMegaTiles[4 * (34 - 1) + 3] = SwapLE16(4 - 1); // 100
		pMegaTiles[4 * (37 - 1) + 3] = SwapLE16(4 - 1); // 108
		pMegaTiles[4 * (38 - 1) + 3] = SwapLE16(4 - 1); // 110
		pMegaTiles[4 * (39 - 1) + 3] = SwapLE16(4 - 1); // 112
		pMegaTiles[4 * (40 - 1) + 3] = SwapLE16(4 - 1); // 114
		pMegaTiles[4 * (41 - 1) + 3] = SwapLE16(4 - 1); // 116
		pMegaTiles[4 * (42 - 1) + 3] = SwapLE16(4 - 1); // 118
		pMegaTiles[4 * (43 - 1) + 3] = SwapLE16(4 - 1); // 120
		pMegaTiles[4 * (44 - 1) + 3] = SwapLE16(4 - 1); // 122
		pMegaTiles[4 * (45 - 1) + 3] = SwapLE16(4 - 1); // 124
		// pMegaTiles[4 * (71 - 1) + 3] = SwapLE16(4 - 1); // 214
		pMegaTiles[4 * (72 - 1) + 3] = SwapLE16(4 - 1); // 217
		pMegaTiles[4 * (73 - 1) + 3] = SwapLE16(4 - 1); // 219
		pMegaTiles[4 * (74 - 1) + 3] = SwapLE16(4 - 1); // 221
		pMegaTiles[4 * (75 - 1) + 3] = SwapLE16(4 - 1); // 223
		pMegaTiles[4 * (76 - 1) + 3] = SwapLE16(4 - 1); // 225
		pMegaTiles[4 * (77 - 1) + 3] = SwapLE16(4 - 1); // 227
		pMegaTiles[4 * (78 - 1) + 3] = SwapLE16(4 - 1); // 229
		pMegaTiles[4 * (79 - 1) + 3] = SwapLE16(4 - 1); // 231
		// pMegaTiles[4 * (80 - 1) + 3] = SwapLE16(4 - 1); // 233
		// pMegaTiles[4 * (81 - 1) + 3] = SwapLE16(4 - 1); // 235
		pMegaTiles[4 * (15 - 1) + 1] = SwapLE16(4 - 1); // 52
		pMegaTiles[4 * (15 - 1) + 2] = SwapLE16(4 - 1); // 53
		pMegaTiles[4 * (15 - 1) + 3] = SwapLE16(4 - 1); // 54
		pMegaTiles[4 * (16 - 1) + 1] = SwapLE16(4 - 1); // 56
		pMegaTiles[4 * (144 - 1) + 1] = SwapLE16(4 - 1);
		pMegaTiles[4 * (16 - 1) + 2] = SwapLE16(4 - 1); // 57
		pMegaTiles[4 * (16 - 1) + 3] = SwapLE16(4 - 1); // 58
		pMegaTiles[4 * (144 - 1) + 3] = SwapLE16(7 - 1);
		pMegaTiles[4 * (94 - 1) + 2] = SwapLE16(60 - 1); // 270
		pMegaTiles[4 * (183 - 1) + 2] = SwapLE16(60 - 1);
		pMegaTiles[4 * (184 - 1) + 2] = SwapLE16(60 - 1);
		pMegaTiles[4 * (17 - 1) + 2] = SwapLE16(4 - 1); // 61
		pMegaTiles[4 * (128 - 1) + 2] = SwapLE16(4 - 1);
		pMegaTiles[4 * (92 - 1) + 2] = SwapLE16(62 - 1); // 262
		pMegaTiles[4 * (179 - 1) + 2] = SwapLE16(62 - 1);
		pMegaTiles[4 * (25 - 1) + 1] = SwapLE16(4 - 1); // 76
		pMegaTiles[4 * (25 - 1) + 3] = SwapLE16(4 - 1); // 78
		pMegaTiles[4 * (35 - 1) + 1] = SwapLE16(4 - 1); // 102
		pMegaTiles[4 * (35 - 1) + 3] = SwapLE16(4 - 1); // 103
		pMegaTiles[4 * (69 - 1) + 1] = SwapLE16(4 - 1); // 205
		pMegaTiles[4 * (69 - 1) + 3] = SwapLE16(4 - 1); // 207
		pMegaTiles[4 * (26 - 1) + 2] = SwapLE16(4 - 1); // 81
		pMegaTiles[4 * (26 - 1) + 3] = SwapLE16(4 - 1); // 82
		pMegaTiles[4 * (36 - 1) + 2] = SwapLE16(4 - 1); // 105
		pMegaTiles[4 * (36 - 1) + 3] = SwapLE16(4 - 1); // 106
		pMegaTiles[4 * (46 - 1) + 2] = SwapLE16(4 - 1); // 127
		pMegaTiles[4 * (46 - 1) + 3] = SwapLE16(4 - 1); // 128
		pMegaTiles[4 * (70 - 1) + 2] = SwapLE16(4 - 1); // 210
		pMegaTiles[4 * (70 - 1) + 3] = SwapLE16(4 - 1); // 211
		pMegaTiles[4 * (49 - 1) + 1] = SwapLE16(4 - 1); // 137
		pMegaTiles[4 * (167 - 1) + 1] = SwapLE16(4 - 1);
		pMegaTiles[4 * (49 - 1) + 2] = SwapLE16(4 - 1); // 138
		pMegaTiles[4 * (167 - 1) + 2] = SwapLE16(4 - 1);
		pMegaTiles[4 * (49 - 1) + 3] = SwapLE16(4 - 1); // 139
		pMegaTiles[4 * (167 - 1) + 3] = SwapLE16(4 - 1);
		pMegaTiles[4 * (50 - 1) + 1] = SwapLE16(4 - 1); // 141
		pMegaTiles[4 * (50 - 1) + 3] = SwapLE16(4 - 1); // 143
		pMegaTiles[4 * (51 - 1) + 3] = SwapLE16(4 - 1); // 147
		pMegaTiles[4 * (103 - 1) + 1] = SwapLE16(4 - 1); // 295
		pMegaTiles[4 * (105 - 1) + 1] = SwapLE16(4 - 1);
		pMegaTiles[4 * (127 - 1) + 3] = SwapLE16(4 - 1); // 373
		pMegaTiles[4 * (89 - 1) + 3] = SwapLE16(4 - 1); // 251
		pMegaTiles[4 * (173 - 1) + 3] = SwapLE16(7 - 1);
		pMegaTiles[4 * (174 - 1) + 3] = SwapLE16(7 - 1);
		pMegaTiles[4 * (6 - 1) + 3] = SwapLE16(4 - 1); // 24
		pMegaTiles[4 * (134 - 1) + 3] = SwapLE16(7 - 1);
		pMegaTiles[4 * (7 - 1) + 3] = SwapLE16(7 - 1); // 28
		pMegaTiles[4 * (8 - 1) + 1] = SwapLE16(2 - 1); // 30
		pMegaTiles[4 * (30 - 1) + 1] = SwapLE16(2 - 1);
		pMegaTiles[4 * (32 - 1) + 1] = SwapLE16(2 - 1);
		pMegaTiles[4 * (72 - 1) + 1] = SwapLE16(2 - 1);
		pMegaTiles[4 * (9 - 1) + 3] = SwapLE16(4 - 1); // 35
		pMegaTiles[4 * (137 - 1) + 3] = SwapLE16(4 - 1);
		pMegaTiles[4 * (11 - 1) + 1] = SwapLE16(4 - 1); // 40
		pMegaTiles[4 * (122 - 1) + 1] = SwapLE16(4 - 1);
		pMegaTiles[4 * (12 - 1) + 2] = SwapLE16(4 - 1); // 43
		pMegaTiles[4 * (123 - 1) + 2] = SwapLE16(4 - 1);
		pMegaTiles[4 * (12 - 1) + 3] = SwapLE16(4 - 1); // 44
		pMegaTiles[4 * (123 - 1) + 3] = SwapLE16(7 - 1);
		pMegaTiles[4 * (95 - 1) + 1] = SwapLE16(4 - 1); // 273
		pMegaTiles[4 * (185 - 1) + 1] = SwapLE16(7 - 1);
		pMegaTiles[4 * (186 - 1) + 1] = SwapLE16(4 - 1);
		pMegaTiles[4 * (89 - 1) + 1] = SwapLE16(293 - 1); // 249
		pMegaTiles[4 * (173 - 1) + 1] = SwapLE16(293 - 1);
		pMegaTiles[4 * (174 - 1) + 1] = SwapLE16(293 - 1);
		pMegaTiles[4 * (92 - 1) + 3] = SwapLE16(271 - 1); // 263
		pMegaTiles[4 * (179 - 1) + 3] = SwapLE16(271 - 1);
		pMegaTiles[4 * (96 - 1) + 2] = SwapLE16(12 - 1); // 278
		pMegaTiles[4 * (187 - 1) + 2] = SwapLE16(12 - 1);
		pMegaTiles[4 * (188 - 1) + 2] = SwapLE16(12 - 1);
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
