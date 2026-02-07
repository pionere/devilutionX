/**
 * @file cel_util.h
 *
 *  helper functions to handle cel/cl2 files.
 */
#ifndef __CEL_UTIL_H__
#define __CEL_UTIL_H__

DEVILUTION_BEGIN_NAMESPACE

#define CEL_BLOCK_HEIGHT      32

const BYTE* CelGetFrame(const BYTE* pCelBuff, int nCel, int* nDataSize);

const BYTE* CelGetFrameClipped(const BYTE* pCelBuff, int nCel, int* nDataSize, int* sy);

const BYTE* CelGetFrameClippedAt(const BYTE* pCelBuff, int nCel, int block, int* nDataSize);

void LoadFrameGroups(const BYTE* pCelBuff, const BYTE* (&pGroups)[8]);

/* Load a .CEL asset and overwrite the first (unused) uint32_t with nWidth */
CelImageBuf* CelLoadImage(const char* name, uint32_t nWidth);

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
