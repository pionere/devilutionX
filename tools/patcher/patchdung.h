/**
 * @file patchdung.h
 *
 * Interface of the general level patching functionality.
 */
#ifndef __PATCHDUNG_H__
#define __PATCHDUNG_H__

#include <set>

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

#define BAD_CELFILE_IDX 100
#define BAD_MINFILE_IDX 100
#define BAD_FRAME_IDX 0

#define MICRO_IDX(subtile, blockSize, microIndex) ((subtile) * (blockSize) + (blockSize) - (2 + ((microIndex) & ~1)) + ((microIndex) & 1))

// if ((currIndex == BAD_CELFILE_IDX || currIndex == BAD_MINFILE_IDX) && (pSubtiles[MICRO_IDX(subtileRef - 1, blockSize, microIndex)] & 0xFFF) == BAD_FRAME_IDX) { app_fatal("Ref%d, bs:%d, idx:%d", subtileRef, blockSize, microIndex); } ; \

#define HideMcr(subtileRef, microIndex) \
{ \
	pSubtiles[MICRO_IDX(subtileRef - 1, blockSize, microIndex)] = 0; \
}
#define Blk2Mcr(subtileRef, microIndex) \
{ \
	removeMicros.insert(SwapLE16(pSubtiles[MICRO_IDX(subtileRef - 1, blockSize, microIndex)]) & 0xFFF); \
	pSubtiles[MICRO_IDX(subtileRef - 1, blockSize, microIndex)] = 0; \
}

// if ((currIndex == BAD_CELFILE_IDX || currIndex == BAD_MINFILE_IDX) && (dstValue & 0xFFF) == BAD_FRAME_IDX) { app_fatal("Ref%d, bs:%d, idx:%d from %d@%d", dstSubtileRef, blockSize, dstMicroIndex, srcSubtileRef, srcMicroIndex); } ; \

#define ReplaceMcr(dstSubtileRef, dstMicroIndex, srcSubtileRef, srcMicroIndex) \
{ \
	unsigned dstMicro = MICRO_IDX(dstSubtileRef - 1, blockSize, dstMicroIndex); \
	uint16_t dstValue = pSubtiles[dstMicro]; \
	uint16_t srcValue = pSubtiles[MICRO_IDX(srcSubtileRef - 1, blockSize, srcMicroIndex)]; \
	if (dstValue != srcValue) { \
		removeMicros.insert(SwapLE16(dstValue) & 0xFFF); \
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

typedef struct {
	unsigned frameRef;
	int encoding;
	BYTE *frameSrc;
} CelFrameEntry;

extern std::set<unsigned> removeMicros;

int encodeCelMicros(CelFrameEntry* entries, int numEntries, BYTE* resCelBuf, const BYTE* celBuf, BYTE TRANS_COLOR);
BYTE* EncodeMicro(int encoding, BYTE* pDst, BYTE* pSrc, BYTE transparentPixel);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __PATCHDUNG_H__ */
