/**
 * @file render.h
 *
 *  of basic engine helper functions:
 * - Sprite blitting
 * - Drawing
 */
#ifndef __RENDER_H__
#define __RENDER_H__

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

#define CEL_BLOCK_HEIGHT      32

inline const BYTE* CelGetFrameStart(const BYTE* pCelBuff, int nCel)
{
	const uint32_t* pFrameTable;

	pFrameTable = (const uint32_t*)pCelBuff;

	return &pCelBuff[SwapLE32(pFrameTable[nCel])];
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

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __RENDER_H__ */
