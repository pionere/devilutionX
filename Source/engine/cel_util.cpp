/**
 * @file cel_util.cpp
 *
 *  helper functions to handle cel/cl2 files.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/*
 * @brief Get the address of a group in a .CEL asset
 * @param pCelBuff pointer to groupped CEL-frames offsets and data
 * @param nGroup CEL group number
 * @return the address of the group
 */
static const BYTE* CelGetFrameGroup(const BYTE* pCelBuff, int nGroup)
{
	const uint32_t* pFrameTable;

	pFrameTable = (const uint32_t*)pCelBuff;

	return &pCelBuff[SwapLE32(pFrameTable[nGroup])];
}

static void CelLoadMetaInfoAt(const BYTE* anim, uint32_t nMetaStart, uint32_t nMetaEnd, uint32_t frameCount, CelMetaInfo &mi)
{
	memset(&mi, 0, sizeof(mi));

	while (nMetaStart < nMetaEnd) {
		uint8_t type = anim[nMetaStart];
		nMetaStart++;

		switch (type) {
		case CELMETA_DIMENSIONS:
			mi.cmiDimensions = nMetaStart;
			nMetaStart += 2 * sizeof(uint32_t);
			continue;
		case CELMETA_DIMENSIONS_PER_FRAME:
			mi.cmiDimensionsPerFrame = nMetaStart;
			nMetaStart += frameCount * 2 * sizeof(uint32_t);
			continue;
		case CELMETA_ANIMDELAY:
			mi.cmiAnimDelay = anim[nMetaStart];
			nMetaStart++;
			continue;
		case CELMETA_ANIMORDER:
			mi.cmiAnimOrder = nMetaStart;
			break;
		case CELMETA_ACTIONFRAMES:
			mi.cmiActionFrames = nMetaStart;
			break;
		default:
			ASSUME_UNREACHABLE;
		}
		while (true) {
			BYTE idx = anim[nMetaStart];
			nMetaStart++;
			if (idx == 0) {
				break;
			}
		}
	}
}

/*
 * @brief Get the address of a frame in a .CEL asset
 * @param pCelBuff pointer to CEL-frame offsets and data
 * @param nCel CEL-frame number
 * @param nDataSize Will be set to the length of the frame
 * @return the address of the frame
 */
const BYTE* CelGetFrame(const BYTE* pCelBuff, int nCel, int* nDataSize)
{
	const uint32_t* pFrameTable;
	uint32_t nCellStart;

	pFrameTable = (const uint32_t*)&pCelBuff[nCel * 4];
	nCellStart = SwapLE32(pFrameTable[0]);
	*nDataSize = SwapLE32(pFrameTable[1]) - nCellStart;
	return &pCelBuff[nCellStart];
}

/*
 * @brief Get the address of a clipped frame-block in a .CEL asset
 * @param pCelBuff pointer to CEL-frame offsets and data
 * @param nCel CEL-frame number
 * @param nDataSize Will be set to the length of the remaining frame
 * @param sy the starting y-coordinate. It will be adjusted to skip out-of-screen pixels.
 * @return the address of the frame-block
 */
const BYTE* CelGetFrameClipped(const BYTE* pCelBuff, int nCel, int* nDataSize, int* sy)
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
	headerSize = LOAD_LE16(&pRLEBytes[0]);
	if (startblock >= headerSize) {
		*nDataSize = 0;
		return pRLEBytes;
	}

	nDataStart = LOAD_LE16(&pRLEBytes[startblock]);
	if (endblock >= headerSize) {
		nDataEnd = 0;
	} else {
		nDataEnd = LOAD_LE16(&pRLEBytes[endblock]);
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

/*
 * @brief Get the address of a clipped frame-block in a .CEL asset
 * @param pCelBuff pointer to CEL-frame offsets and data
 * @param nCel CEL-frame number
 * @param block frame-block index
 * @param nDataSize Will be set to the length of the remaining frame
 * @return the address of the frame-block
 */
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

void LoadCelMetaInfo(const BYTE* pCelBuff, CelMetaInfo &mi)
{
	const uint32_t* pFrameTable;
	uint32_t frameCount;
	uint32_t nMetaStart, nMetaEnd;

	pFrameTable = (const uint32_t*)&pCelBuff[0];
	frameCount = SwapLE32(pFrameTable[0]);
	nMetaStart = (1 + frameCount + 1) * sizeof(uint32_t);
	nMetaEnd = SwapLE32(pFrameTable[1]);

	CelLoadMetaInfoAt(pCelBuff, nMetaStart, nMetaEnd, frameCount, mi);
}

void LoadCelGroupMetaInfo(const BYTE* pCelBuff, CelMetaInfo &mi)
{
	constexpr int groupCount = 8;
	const uint32_t* pFrameTable;
	uint32_t nMetaStart, nMetaEnd;

	pFrameTable = (const uint32_t*)&pCelBuff[0];
	nMetaStart = groupCount * sizeof(uint32_t);
	nMetaEnd = SwapLE32(pFrameTable[0]);

	CelLoadMetaInfoAt(pCelBuff, nMetaStart, nMetaEnd, 0, mi);
}

/*
 * @brief Get the addresses of frame-groups in a .CEL asset
 * @param pCelBuff pointer to groupped CEL-frames offsets and data
 * @param pGroups Will be set to the addresses of the groups
 */
void LoadFrameGroups(const BYTE* pCelBuff, const BYTE* (&pGroups)[8])
{
	int i;

	for (i = 0; i < lengthof(pGroups); i++) {
		pGroups[i] = CelGetFrameGroup(pCelBuff, i);
	}
}

/*
 * @brief Load a .CEL asset and overwrite the first (unused) uint32_t with nWidth
 * @param name name of asset
 * @param nWidth width of the asset
 * @return pointer to CEL-frame offsets and data with width information
 */
CelImageBuf* CelLoadImage(const char* name, uint32_t nWidth)
{
	CelImageBuf* res;

	res = (CelImageBuf*)LoadFileInMem(name);
#if DEBUG_MODE
	res->ciFrameCnt = LOAD_LE32(res);
#endif
	res->ciWidth = nWidth;
	return res;
}

/*
 * @brief Merge two non-groupped .CEL assets into a new one
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
 * @brief calculate the width of the CEL-frame using the clipping information
 * @param pCelBuff pointer to CEL-frame offsets and data
 * @return the width of the CEL-frame
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
 * @brief Apply the color swaps to a CL2-frame
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
 * @brief calculate the width of the CL2-frame using the clipping information
 * @param pCelBuff pointer to CL2-frame offsets and data
 * @return the width of the CL2-frame
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
