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

BYTE* DRLP_L3_PatchDoors(BYTE* celBuf, size_t* celLen);
BYTE* DRLP_L3_PatchCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen);
void DRLP_L3_PatchMin(BYTE* minBuf);
void DRLP_L3_PatchTil(BYTE* tilBuf);

#ifdef HELLFIRE
#define BLOCK_SIZE_L6 10

BYTE* DRLP_L6_PatchCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen);
void DRLP_L6_PatchMin(BYTE* minBuf);
void DRLP_L6_PatchTil(BYTE* tilBuf);
#endif

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DRLP_L3_H__ */
