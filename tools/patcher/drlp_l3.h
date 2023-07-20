/**
 * @file drlp_l3.h
 *
 * Interface of the caves level patching functionality.
 */
#ifndef __DRLP_L3_H__
#define __DRLP_L3_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

#define BLOCK_SIZE_L3 10

BYTE* DRLP_L3_PatchCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen);
void DRLP_L3_PatchMin(BYTE* minBuf);
void DRLP_L3_PatchTil(BYTE* tilBuf);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DRLP_L3_H__ */
