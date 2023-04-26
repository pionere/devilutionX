/**
 * @file encrypt.h
 *
 * Interface of functions for compression and decompressing MPQ data.
 */
#ifndef __ENCRYPT_H__
#define __ENCRYPT_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

DWORD PkwareCompress(BYTE* srcData, DWORD size);
void PkwareDecompress(BYTE* pbInBuff, unsigned recv_size, unsigned dwMaxBytes);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __ENCRYPT_H__ */
