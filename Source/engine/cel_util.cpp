/**
 * @file cel_util.cpp
 *
 *  helper functions to handle cel/cl2 files.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

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

/**
 * @brief calculate the width of the CEL sprite using the clipping information
 * @param pCelBuff pointer to CEL-frame offsets and data
 * @return the width of the CEL sprite
 */
unsigned CelClippedWidth(const BYTE* pCelBuff)
{
	int nDataSize;
	const BYTE *pRLEBytes;

	pRLEBytes = CelGetFrameClippedAt(pCelBuff, 1, 0, &nDataSize);

	const BYTE *src, *end;
	int width;

	src = pRLEBytes;

	end = CelGetFrameClippedAt(pCelBuff, 1, 1, &nDataSize);

	unsigned n = 0;
	while (src < end) {
		width = (int8_t)*src++;
		if (width >= 0) {
			n += width;
			src += width;
		} else {
			n -= width;
		}
	}

	return n / CEL_BLOCK_HEIGHT;
}

/**
 * @brief Apply the color swaps to a CL2 sprite
 * @param pCelBuff pointer to CL2-frame offsets and data
 * @param ttbl Palette translation table
 * @param nFrames number of frames in the CL2 file
 */
void Cl2ApplyTrans(BYTE* pCelBuff, const BYTE* ttbl, int nFrames)
{
	int i, nDataSize;
	int width;
	BYTE* dst;
	const BYTE* end;

	assert(pCelBuff != NULL);
	assert(ttbl != NULL);

	for (i = 1; i <= nFrames; i++) {
		dst = const_cast<BYTE*>(CelGetFrameClippedAt(pCelBuff, i, 0, &nDataSize));
		end = &dst[nDataSize];
		while (dst != end) {
			width = (int8_t)*dst++;
			assert(dst <= end);
			if (width < 0) {
				width = -width;
				if (width > 65) {
					*dst = ttbl[*dst];
					dst++;
					assert(dst <= end);
				} else {
					assert(dst + width <= end);
					while (width--) {
						*dst = ttbl[*dst];
						dst++;
					}
				}
			}
		}
	}
}

/**
 * @brief calculate the width of the CL2 sprite using the clipping information
 * @param pCelBuff pointer to CL2-frame offsets and data
 * @return the width of the CL2 sprite
 */
unsigned Cl2Width(const BYTE* pCelBuff)
{
	int nDataSize;
	const BYTE *pRLEBytes;

	pRLEBytes = CelGetFrameClippedAt(pCelBuff, 1, 0, &nDataSize);

	const BYTE *src, *end;
	int width;

	src = pRLEBytes;

	end = CelGetFrameClippedAt(pCelBuff, 1, 1, &nDataSize);

	unsigned n = 0;
	while (src < end) {
		width = (int8_t)*src++;
		if (width < 0) {
			width = -width;
			if (width > 65) {
				width -= 65;
				src++;
			} else {
				src += width;
			}
		}
		n += width;
	}

	return n / CEL_BLOCK_HEIGHT;
}

DEVILUTION_END_NAMESPACE
