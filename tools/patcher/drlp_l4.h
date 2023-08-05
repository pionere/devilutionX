/**
 * @file drlp_l4.h
 *
 * Interface of the hell level patching functionality.
 */
#ifndef __DRLP_L4_H__
#define __DRLP_L4_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

#define BLOCK_SIZE_L4 16

BYTE* DRLP_L4_PatchCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen);
void DRLP_L4_PatchMin(BYTE* minBuf);
void DRLP_L4_PatchTil(BYTE* tilBuf);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DRLP_L4_H__ */
