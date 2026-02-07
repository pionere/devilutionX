/**
 * @file cel_util.h
 *
 *  helper functions to handle cel/cl2 files.
 */
#ifndef __CEL_UTIL_H__
#define __CEL_UTIL_H__

DEVILUTION_BEGIN_NAMESPACE

#define CEL_BLOCK_HEIGHT      32

const BYTE* CelGetFrameGroup(const BYTE* pCelBuff, int nCel);

const BYTE* CelGetFrame(const BYTE* pCelBuff, int nCel, int* nDataSize);

const BYTE* CelGetFrameClipped(const BYTE* pCelBuff, int nCel, int* nDataSize, int* sy);

const BYTE* CelGetFrameClippedAt(const BYTE* pCelBuff, int nCel, int block, int* nDataSize);

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

DEVILUTION_END_NAMESPACE

#endif /* __CEL_UTIL_H__ */
