/**
 * @file drlp_l1.h
 *
 * Interface of the cathedral (and crypt) level patching functionality.
 */
#ifndef __DRLP_L1_H__
#define __DRLP_L1_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

#define BLOCK_SIZE_L1 10

BYTE* DRLP_L1_PatchDoors(BYTE* celBuf, size_t* celLen);
BYTE* DRLP_L1_PatchSpec(BYTE* sCelBuf, size_t* sCelLen);
BYTE* DRLP_L1_PatchCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen);
void DRLP_L1_PatchMin(BYTE* minBuf);
void DRLP_L1_PatchTil(BYTE* tilBuf);

#ifdef HELLFIRE
#define BLOCK_SIZE_L5 10

BYTE* DRLP_L5_PatchSpec(const BYTE* minBuf, size_t minLen, const BYTE* celBuf, size_t celLen, BYTE* sCelBuf, size_t* sCelLen);
BYTE* DRLP_L5_PatchCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen);
void DRLP_L5_PatchMin(BYTE* minBuf);
void DRLP_L5_PatchTil(BYTE* tilBuf);
#endif

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DRLP_L1_H__ */
