/**
 * @file sha.cpp
 *
 * Implementation of functionality for calculating X-SHA-1 (a flawed implementation of SHA-1).
 */
//#include <cstdint>

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

static SHA1Context sgSHA1[SHA1ContextNum];

/**
 * Standard circular left shift, portable version.
 * Necessary because the MSVC-compiler messed up the inlining of the original code.
 */
static uint32_t SHA1CircularShiftA(uint32_t bits, uint32_t word)
{
	assert(bits < 32);
	assert(bits > 0);
	return (word << bits) | (word >> (32 - bits));
}
/**
 * Diablo-"SHA1" circular left shift (arithmetic shift), portable version.
 */
static uint32_t SHA1CircularShiftB(uint32_t bits, uint32_t word)
{
	assert(bits < 32);
	assert(bits > 0);
	return (word << bits) | ((int32_t)word >> (32 - bits));
}

static void SHA1Init(SHA1Context* context)
{
#if DEBUG_MODE
	context->count[0] = 0;
	context->count[1] = 0;
#endif
	context->state[0] = 0x67452301;
	context->state[1] = 0xEFCDAB89;
	context->state[2] = 0x98BADCFE;
	context->state[3] = 0x10325476;
	context->state[4] = 0xC3D2E1F0;
}

static void SHA1ProcessMessageBlock(SHA1Context* context)
{
	DWORD i, temp;
	DWORD W[80];
	DWORD A, B, C, D, E;

	DWORD* buf = (DWORD*)context->buffer;
	for (i = 0; i < 16; i++)
		W[i] = SwapLE32(buf[i]);

	for (i = 16; i < 80; i++) {
		W[i] = W[i - 16] ^ W[i - 14] ^ W[i - 8] ^ W[i - 3];
	}

	A = context->state[0];
	B = context->state[1];
	C = context->state[2];
	D = context->state[3];
	E = context->state[4];

	for (i = 0; i < 20; i++) {
		temp = SHA1CircularShiftA(5, A) + ((B & C) | ((~B) & D)) + E + W[i] + 0x5A827999;
		E = D;
		D = C;
		C = SHA1CircularShiftB(30, B);
		B = A;
		A = temp;
	}

	for (i = 20; i < 40; i++) {
		temp = SHA1CircularShiftA(5, A) + (B ^ C ^ D) + E + W[i] + 0x6ED9EBA1;
		E = D;
		D = C;
		C = SHA1CircularShiftA(30, B);
		B = A;
		A = temp;
	}

	for (i = 40; i < 60; i++) {
		temp = SHA1CircularShiftA(5, A) + ((B & C) | (B & D) | (C & D)) + E + W[i] + 0x8F1BBCDC;
		E = D;
		D = C;
		C = SHA1CircularShiftA(30, B);
		B = A;
		A = temp;
	}

	for (i = 60; i < 80; i++) {
		temp = SHA1CircularShiftA(5, A) + (B ^ C ^ D) + E + W[i] + 0xCA62C1D6;
		E = D;
		D = C;
		C = SHA1CircularShiftA(30, B);
		B = A;
		A = temp;
	}

	context->state[0] += A;
	context->state[1] += B;
	context->state[2] += C;
	context->state[3] += D;
	context->state[4] += E;
}

static void SHA1Input(SHA1Context* context, const BYTE* message_array, DWORD len)
{
	DWORD i;
#if DEBUG_MODE
	DWORD count;

	count = context->count[0] + 8 * len;
	if (count < context->count[0])
		context->count[1]++;

	context->count[0] = count;
#endif
	static_assert(sizeof(context->buffer) == SHA1BlockSize, "SHA1 buffer size is too small.");
	for (i = len; i >= SHA1BlockSize; i -= SHA1BlockSize) {
		memcpy(context->buffer, message_array, SHA1BlockSize);
		SHA1ProcessMessageBlock(context);
		message_array += SHA1BlockSize;
	}
}

void SHA1Clear()
{
	memset(sgSHA1, 0, sizeof(sgSHA1));
}

void SHA1Result(/*int n,*/ BYTE Message_Digest[SHA1HashSize])
{
	DWORD* Message_Digest_Block;
	int i, n = 0;

	assert(Message_Digest != NULL);
	Message_Digest_Block = (DWORD*)Message_Digest;
	for (i = 0; i < lengthof(sgSHA1[n].state); i++) {
		*Message_Digest_Block = SwapLE32(sgSHA1[n].state[i]);
		Message_Digest_Block++;
	}
}

void SHA1Calculate(/*int n,*/ const BYTE* data, BYTE Message_Digest[SHA1HashSize])
{
	int n = 0;
	SHA1Input(&sgSHA1[n], data, SHA1BlockSize);
	if (Message_Digest != NULL)
		SHA1Result(/*n,*/ Message_Digest);
}

void SHA1Reset(/*int n*/)
{
	int n = 0;
	SHA1Init(&sgSHA1[n]);
}

DEVILUTION_END_NAMESPACE
