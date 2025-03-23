/**
 * @file sha.cpp
 *
 * Interface of functionality for calculating X-SHA-1 (a flawed implementation of SHA-1).
 */
#ifndef __SHA_H__
#define __SHA_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

#define SHA1ContextNum 1
#define SHA1HashSize   20
#define SHA1BlockSize  64

void SHA1Clear();
void SHA1Result(/*int n, */BYTE Message_Digest[SHA1HashSize]);
void SHA1Calculate(/*int n, */const BYTE* data, BYTE Message_Digest[SHA1HashSize]);
void SHA1Reset(/*int n*/);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __SHA_H__ */
