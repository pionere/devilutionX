/**
 * @file cel_util.h
 *
 *  helper functions to handle cel/cl2 files.
 */
#ifndef __CEL_UTIL_H__
#define __CEL_UTIL_H__

#include "display.h"
#include "gameui.h"

DEVILUTION_BEGIN_NAMESPACE

#define CEL_BLOCK_HEIGHT      32

inline const BYTE* CelGetFrameGroup(const BYTE* pCelBuff, int nGroup)
{
	const uint32_t* pFrameTable;

	pFrameTable = (const uint32_t*)pCelBuff;

	return &pCelBuff[SwapLE32(pFrameTable[nGroup])];
}

inline const BYTE* CelGetFrame(const BYTE* pCelBuff, int nCel, int* nDataSize)
{
	const uint32_t* pFrameTable;
	uint32_t nCellStart;

	pFrameTable = (const uint32_t*)&pCelBuff[nCel * 4];
	nCellStart = SwapLE32(pFrameTable[0]);
	*nDataSize = SwapLE32(pFrameTable[1]) - nCellStart;
	return &pCelBuff[nCellStart];
}

inline const BYTE* CelGetFrameClipped(const BYTE* pCelBuff, int nCel, int* nDataSize, int* sy)
{
	int dy, startblock, endblock, headerSize;
	uint16_t nDataStart, nDataEnd;
	const BYTE* pRLEBytes = CelGetFrame(pCelBuff, nCel, nDataSize);
	// check if it is too high on the screen
	dy = *sy - SCREEN_Y;
	if (dy < 0) {
		*nDataSize = 0;
		return pRLEBytes;
	}
	// limit blocks to the top of the screen
	endblock = ((unsigned)dy / CEL_BLOCK_HEIGHT + 1) * 2;
	// limit blocks to the bottom of the screen
	startblock = 0;
	dy = dy - (SCREEN_HEIGHT + CEL_BLOCK_HEIGHT);
	if (dy >= 0) {
		startblock = ((unsigned)dy / CEL_BLOCK_HEIGHT + 1) * 2;
		*sy -= startblock * (CEL_BLOCK_HEIGHT / 2);
	}
	
	// check if it is too down on the screen
	headerSize = SwapLE16(*(const uint16_t*)(&pRLEBytes[0]));
	if (startblock >= headerSize) {
		*nDataSize = 0;
		return pRLEBytes;
	}

	nDataStart = SwapLE16(*(const uint16_t*)(&pRLEBytes[startblock]));
	if (endblock >= headerSize) {
		nDataEnd = 0;
	} else {
		nDataEnd = SwapLE16(*(const uint16_t*)(&pRLEBytes[endblock]));
	}

	if (nDataEnd != 0) {
		*nDataSize = nDataEnd - nDataStart;
	} else if (nDataStart != 0) {
		*nDataSize -= nDataStart;
	} else {
		*nDataSize = 0;
	}

	return &pRLEBytes[nDataStart];
}

inline const BYTE* CelGetFrameClippedAt(const BYTE* pCelBuff, int nCel, int block, int* nDataSize)
{
	const uint16_t* pFrameTable;
	uint16_t nDataStart;
	const BYTE* pRLEBytes = CelGetFrame(pCelBuff, nCel, nDataSize);

	pFrameTable = (const uint16_t*)&pRLEBytes[0];
	nDataStart = SwapLE16(pFrameTable[block]);
	// assert(nDataStart != 0);
	*nDataSize -= nDataStart;

	return &pRLEBytes[nDataStart];
}

inline void LoadFrameGroups(const BYTE* pCelBuff, const BYTE* (&pGroups)[8])
{
	int i;

	for (i = 0; i < lengthof(pGroups); i++) {
		pGroups[i] = CelGetFrameGroup(pCelBuff, i);
	}
}

/* Load a .CEL asset and overwrite the first (unused) uint32_t with nWidth */
inline CelImageBuf* CelLoadImage(const char* name, uint32_t nWidth)
{
	CelImageBuf* res;

	res = (CelImageBuf*)LoadFileInMem(name);
#if DEBUG_MODE
	res->ciFrameCnt = SwapLE32(*((uint32_t*)res));
#endif
	res->ciWidth = nWidth;
	return res;
}

/*
 * @brief Merge two .CEL assets into a new one
 * @param celA the first asset to merge
 * @param nDataSizeA the size of the first asset
 * @param celA the second asset to merge
 * @param nDataSizeA the size of the second asset
 * @return the merged asset
 */
BYTE* CelMerge(BYTE* celA, size_t nDataSizeA, BYTE* celB, size_t nDataSizeB);

/**
 * @brief calculate the width of the CEL sprite using the clipping information
 * @param pCelBuff pointer to CEL-frame offsets and data
 * @return the width of the CEL sprite
 */
unsigned CelClippedWidth(const BYTE* pCelBuff);

/**
 * @brief Apply the color swaps to a CL2 sprite
 * @param pCelBuff pointer to CL2-frame offsets and data
 * @param ttbl Palette translation table
 * @param nFrames number of frames in the CL2 file
 */
void Cl2ApplyTrans(BYTE* pCelBuff, const BYTE* ttbl, int nFrames);

/**
 * @brief calculate the width of the CL2 sprite using the clipping information
 * @param pCelBuff pointer to CL2-frame offsets and data
 * @return the width of the CL2 sprite
 */
unsigned Cl2Width(const BYTE* pCelBuff);

DEVILUTION_END_NAMESPACE

#endif /* __CEL_UTIL_H__ */
