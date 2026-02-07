/**
 * @file cel_util.cpp
 *
 *  helper functions to handle cel/cl2 files.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

const BYTE* CelGetFrameGroup(const BYTE* pCelBuff, int nGroup)
{
	const uint32_t* pFrameTable;

	pFrameTable = (const uint32_t*)pCelBuff;

	return &pCelBuff[SwapLE32(pFrameTable[nGroup])];
}

const BYTE* CelGetFrame(const BYTE* pCelBuff, int nCel, int* nDataSize)
{
	const uint32_t* pFrameTable;
	uint32_t nCellStart;

	pFrameTable = (const uint32_t*)&pCelBuff[nCel * 4];
	nCellStart = SwapLE32(pFrameTable[0]);
	*nDataSize = SwapLE32(pFrameTable[1]) - nCellStart;
	return &pCelBuff[nCellStart];
}

/*const BYTE* CelGetFrameClipped(const BYTE* pCelBuff, int nCel, int* nDataSize, int* sy)
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
*/
const BYTE* CelGetFrameClipped(const BYTE* pCelBuff, int nCel, int* nDataSize, int* sy)
{
	const uint16_t* pFrameTable;
	uint16_t nDataStart;
	const BYTE* pRLEBytes = CelGetFrame(pCelBuff, nCel, nDataSize);

	pFrameTable = (const uint16_t*)&pRLEBytes[0];
	nDataStart = SwapLE16(pFrameTable[0]);
	*nDataSize -= nDataStart;

	return &pRLEBytes[nDataStart];
}

const BYTE* CelGetFrameClippedAt(const BYTE* pCelBuff, int nCel, int block, int* nDataSize)
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

/* Load a .CEL asset and overwrite the first (unused) uint32_t with nWidth */
CelImageBuf* CelLoadImage(const char* name, uint32_t nWidth)
{
	CelImageBuf* res;

	res = (CelImageBuf*)LoadFileInMem(name);
#if DEBUG_MODE
	res->ciFrameCnt = SwapLE32(*((uint32_t*)res));
#endif
	res->ciWidth = nWidth;
	return res;
}

#define LOAD_LE32(b) (SwapLE32(*((DWORD*)(b))))
/*
 * @brief Merge two .CEL assets into a new one
 * @param celA the first asset to merge
 * @param nDataSizeA the size of the first asset
 * @param celA the second asset to merge
 * @param nDataSizeA the size of the second asset
 * @return the merged asset
 */
BYTE* CelMerge(BYTE* celA, size_t nDataSizeA, BYTE* celB, size_t nDataSizeB)
{
	size_t nDataSize;
	DWORD i, nCelA, nCelB, cData, nData;
	BYTE *cel, *pBuf;
	DWORD* pHead;

	nDataSize = nDataSizeA + nDataSizeB - 4 * 2;
	cel = DiabloAllocPtr(nDataSize);
	*(DWORD*)cel = 0;
	pBuf = cel;
	nCelA = LOAD_LE32(celA);
	nCelB = LOAD_LE32(celB);
	pHead = (DWORD*)pBuf;
	pHead++;
	pBuf += 4 * (nCelA + nCelB + 2);

	nData = LOAD_LE32(celA + 4);
	for (i = 1; i <= nCelA; i++) {
		cData = nData;
		nData = LOAD_LE32(celA + 4 * (i + 1));
		*pHead = SwapLE32((DWORD)((size_t)pBuf - (size_t)cel));
		memcpy(pBuf, &celA[cData], nData - cData);
		pBuf += nData - cData;
		++*cel;
		pHead++;
	}

	nData = LOAD_LE32(celB + 4);
	for (i = 1; i <= nCelB; i++) {
		cData = nData;
		nData = LOAD_LE32(celB + 4 * (i + 1));
		*pHead = SwapLE32((DWORD)((size_t)pBuf - (size_t)cel));
		memcpy(pBuf, &celB[cData], nData - cData);
		pBuf += nData - cData;
		++*cel;
		pHead++;
	}

	*pHead = SwapLE32((DWORD)((size_t)pBuf - (size_t)cel));
	// assert(*pHead == nDataSize);
	return cel;
}

DEVILUTION_END_NAMESPACE
