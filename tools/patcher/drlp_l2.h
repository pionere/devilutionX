/**
 * @file drlp_l2.h
 *
 * Interface of the catacombs level patching functionality.
 */
#ifndef __DRLP_L2_H__
#define __DRLP_L2_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

#define BLOCK_SIZE_L2 10

BYTE* DRLP_L2_PatchCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen);
void DRLP_L2_PatchMin(BYTE* minBuf);
void DRLP_L2_PatchTil(BYTE* tilBuf);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DRLP_L2_H__ */
