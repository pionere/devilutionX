/**
 * @file codec.cpp
 *
 * Implementation of save game encryption algorithm.
 */
#include "all.h"

//#include <cstddef>
//#include <cstdint>

DEVILUTION_BEGIN_NAMESPACE

typedef struct CodecSignature {
	uint32_t checksum;
	BYTE error;
	BYTE last_chunk_size;
	uint16_t unused;
} CodecSignature;

static void CodecInitKey(const char* pszPassword)
{
	BYTE key[72 + SHA1BlockSize]; // last 64 bytes are the SHA1
	uint32_t rand_state = 0x7058;
	for (unsigned i = 0; i < sizeof(key); ++i) {
		rand_state = rand_state * 214013 + 2531011;
		key[i] = rand_state >> 16; // Downcasting to BYTE keeps the least-significant byte
	}

	BYTE pw[SHA1BlockSize];
	unsigned password_i = 0;
	for (unsigned i = 0; i < sizeof(pw); ++i, ++password_i) {
		if (pszPassword[password_i] == '\0')
			password_i = 0;
		pw[i] = static_cast<BYTE>(pszPassword[password_i]);
	}

	BYTE digest[SHA1HashSize];
	SHA1Reset(/*0*/);
	SHA1Calculate(/*0,*/ pw, digest);
	SHA1Clear();
	for (unsigned i = 0; i < sizeof(key); ++i)
		key[i] ^= digest[i % SHA1HashSize];
	memset(pw, 0, sizeof(pw));
	memset(digest, 0, sizeof(digest));
	static_assert(SHA1ContextNum == 1, "CodecInitKey must initialize the sha1 contexts.");
	// for (int n = 0; n < SHA1ContextNum; ++n) {
		SHA1Reset(/*n*/);
		SHA1Calculate(/*n,*/ &key[sizeof(key) - SHA1BlockSize], NULL);
	// }
	memset(key, 0, sizeof(key));
}

int codec_decode(BYTE* pbSrcDst, DWORD size, const char* pszPassword)
{
	BYTE buf[SHA1BlockSize];
	BYTE dst[SHA1HashSize];
	int i;
	CodecSignature* sig;

	CodecInitKey(pszPassword);
	if (size <= sizeof(CodecSignature))
		return 0;
	size -= sizeof(CodecSignature);
	if (size % SHA1BlockSize != 0)
		return 0;
	for (i = size; i != 0; pbSrcDst += SHA1BlockSize, i -= SHA1BlockSize) {
		memcpy(buf, pbSrcDst, SHA1BlockSize);
		SHA1Result(/*0, */dst);
		for (int j = 0; j < SHA1BlockSize; j++) {
			buf[j] ^= dst[j % SHA1HashSize];
		}
		SHA1Calculate(/*0,*/ buf, NULL);
		memcpy(pbSrcDst, buf, SHA1BlockSize);
	}

	memset(buf, 0, sizeof(buf));

	SHA1Result(/*0,*/ dst);
	sig = (CodecSignature*)pbSrcDst;
	if (sig->error == 0 && sig->checksum == *(uint32_t*)dst) {
		size += sig->last_chunk_size - SHA1BlockSize;
	} else {
		size = 0;
	}

	memset(dst, 0, sizeof(dst));
	SHA1Clear();
	return size;
}

DWORD codec_get_encoded_len(DWORD dwSrcBytes)
{
	if (dwSrcBytes % SHA1BlockSize != 0)
		dwSrcBytes += SHA1BlockSize - (dwSrcBytes % SHA1BlockSize);
	return dwSrcBytes + sizeof(CodecSignature);
}

void codec_encode(BYTE* pbSrcDst, DWORD size, DWORD encodedSize, const char* pszPassword)
{
	BYTE buf[SHA1BlockSize];
	BYTE dst[SHA1HashSize];
	DWORD chunk;
	CodecSignature* sig;

	assert(encodedSize == codec_get_encoded_len(size));
	CodecInitKey(pszPassword);

	chunk = 0;
	while (size != 0) {
		chunk = size < SHA1BlockSize ? size : SHA1BlockSize;
		memcpy(buf, pbSrcDst, chunk);
		if (chunk < SHA1BlockSize)
			memset(buf + chunk, 0, SHA1BlockSize - chunk);
		SHA1Result(/*0,*/ dst);
		SHA1Calculate(/*0,*/ buf, NULL);
		for (int i = 0; i < SHA1BlockSize; i++) {
			buf[i] ^= dst[i % SHA1HashSize];
		}
		memcpy(pbSrcDst, buf, SHA1BlockSize);
		pbSrcDst += SHA1BlockSize;
		size -= chunk;
	}
	memset(buf, 0, sizeof(buf));
	SHA1Result(/*0,*/ dst);
	sig = (CodecSignature*)pbSrcDst;
	sig->error = 0;
	sig->unused = 0;
	sig->checksum = *(uint32_t*)&dst[0];
	sig->last_chunk_size = chunk;
	memset(dst, 0, sizeof(dst));
	SHA1Clear();
}

DEVILUTION_END_NAMESPACE
