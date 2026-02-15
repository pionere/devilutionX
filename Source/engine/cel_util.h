/**
 * @file cel_util.h
 *
 *  helper functions to handle cel/cl2 files.
 */
#ifndef __CEL_UTIL_H__
#define __CEL_UTIL_H__

DEVILUTION_BEGIN_NAMESPACE

#define CEL_BLOCK_HEIGHT      32

/*
 * @brief Get the address of a frame in a .CEL asset
 * @param pCelBuff pointer to CEL-frame offsets and data
 * @param nCel CEL-frame number
 * @param nDataSize Will be set to the length of the frame
 * @return the address of the frame
 */
const BYTE* CelGetFrame(const BYTE* pCelBuff, int nCel, int* nDataSize);

/*
 * @brief Get the address of a clipped frame-block in a .CEL asset
 * @param pCelBuff pointer to CEL-frame offsets and data
 * @param nCel CEL-frame number
 * @param nDataSize Will be set to the length of the remaining frame
 * @param sy the starting y-coordinate. It will be adjusted to skip out-of-screen pixels.
 * @return the address of the frame-block
 */
const BYTE* CelGetFrameClipped(const BYTE* pCelBuff, int nCel, int* nDataSize, int* sy);

/*
 * @brief Get the address of a clipped frame-block in a .CEL asset
 * @param pCelBuff pointer to CEL-frame offsets and data
 * @param nCel CEL-frame number
 * @param block frame-block index
 * @param nDataSize Will be set to the length of the remaining frame
 * @return the address of the frame-block
 */
const BYTE* CelGetFrameClippedAt(const BYTE* pCelBuff, int nCel, int block, int* nDataSize);

/*
 * @brief Load a .CEL asset and overwrite the first (unused) uint32_t with nWidth
 * @param name name of asset
 * @param nWidth width of the asset
 * @return pointer to CEL-frame offsets and data with width information
 */
CelImageBuf* CelLoadImage(const char* name, uint32_t nWidth);

/*
 * @brief Retrive the offsets of the metainfo in a .CEL asset
 * @param pCelBuff pointer to CEL-frame offsets and data
 * @param mi Will be set to the offsets of the metainfo
 */
void LoadCelMetaInfo(const BYTE* pCelBuff, CelMetaInfo &mi);

/*
 * @brief Retrive the offsets of the metainfo in a .CEL asset
 * @param pCelBuff pointer to groupped CEL-frames offsets and data
 * @param mi Will be set to the offsets of the metainfo
 */
void LoadCelGroupMetaInfo(const BYTE* pCelBuff, CelMetaInfo &mi);

/*
 * @brief Get the addresses of frame-groups in a .CEL asset
 * @param pCelBuff pointer to groupped CEL-frames offsets and data
 * @param pGroups Will be set to the addresses of the groups
 */
void LoadFrameGroups(const BYTE* pCelBuff, const BYTE* (&pGroups)[8]);

/*
 * @brief Merge two non-groupped .CEL assets into a new one
 * @param celA the first asset to merge
 * @param nDataSizeA the size of the first asset
 * @param celA the second asset to merge
 * @param nDataSizeA the size of the second asset
 * @return the merged asset
 */
BYTE* CelMerge(BYTE* celA, size_t nDataSizeA, BYTE* celB, size_t nDataSizeB);

/**
 * @brief calculate the width of the CEL-frame using the clipping information
 * @param pCelBuff pointer to CEL-frame offsets and data
 * @return the width of the CEL-frame
 */
unsigned CelClippedWidth(const BYTE* pCelBuff);

/**
 * @brief Apply the color swaps to a CL2-frame
 * @param pCelBuff pointer to CL2-frame offsets and data
 * @param ttbl Palette translation table
 */
void Cl2ApplyTrans(BYTE* pCelBuff, const BYTE* ttbl);

/**
 * @brief calculate the width of the CL2-frame using the clipping information
 * @param pCelBuff pointer to CL2-frame offsets and data
 * @return the width of the CL2-frame
 */
unsigned Cl2Width(const BYTE* pCelBuff);

DEVILUTION_END_NAMESPACE

#endif /* __CEL_UTIL_H__ */
