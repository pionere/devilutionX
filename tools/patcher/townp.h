/**
 * @file townp.h
 *
 * Interface of the town level patching functionality.
 */
#ifndef __TOWNP_H__
#define __TOWNP_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

#define BLOCK_SIZE_TOWN 16

BYTE* Town_PatchCel(const BYTE* minBuf, size_t minLen, BYTE* celBuf, size_t* celLen);
//BYTE* Town_PatchSpec(const BYTE* minBuf, size_t minLen, const BYTE* celBuf, size_t celLen, BYTE* sCelBuf, size_t* sCelLen);
BYTE* Town_PatchMin(BYTE* minBuf, size_t* minLen, bool isHellfireTown);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __TOWNP_H__ */
