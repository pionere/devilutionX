/**
 * @file encrypt.cpp
 *
 * Implementation of functions for compression and decompressing MPQ data.
 */
#include "all.h"
#include "pkware.h"

DEVILUTION_BEGIN_NAMESPACE

/*DWORD hashtable[5][256];

void Decrypt(DWORD* castBlock, DWORD size, DWORD key)
{
	DWORD seed, i;

	seed = 0xEEEEEEEE;
	for (i = size >> 2; i != 0; i--) {
		DWORD t = SwapLE32(*castBlock);
		seed += hashtable[4][(key & 0xFF)];
		t ^= seed + key;
		*castBlock = t;
		seed += t + (seed << 5) + 3;
		castBlock++;
		key = (((key << 0x15) ^ 0xFFE00000) + 0x11111111) | (key >> 0x0B);
	}
}

void Encrypt(DWORD* castBlock, DWORD size, DWORD key)
{
	DWORD seed, i, ch;

	seed = 0xEEEEEEEE;
	for (i = size >> 2; i != 0; i--) {
		DWORD t = ch = *castBlock;
		seed += hashtable[4][(key & 0xFF)];
		t ^= seed + key;
		*castBlock = SwapLE32(t);
		castBlock++;
		seed += ch + (seed << 5) + 3;
		key = (((key << 0x15) ^ 0xFFE00000) + 0x11111111) | (key >> 0x0B);
	}
}

DWORD Hash(const char* s, int type)
{
	char ch;
	DWORD seed1, seed2;

	seed1 = 0x7FED7FED;
	seed2 = 0xEEEEEEEE;
	while (*s != '\0') {
		ch = *s++;
		ch = toupper(ch);
		seed1 = hashtable[type][ch] ^ (seed1 + seed2);
		seed2 += ch + seed1 + (seed2 << 5) + 3;
	}
	return seed1;
}

void InitHash()
{
	DWORD seed, ch;
	int i, j;

	seed = 0x00100001;

	for (i = 0; i < lengthof(hashtable[0]); i++) {
		for (j = 0; j < lengthof(hashtable); j++) {
			seed = (125 * seed + 3) % 0x2AAAAB;
			ch = (seed & 0xFFFF);
			seed = (125 * seed + 3) % 0x2AAAAB;
			hashtable[j][i] = ch << 16 | (seed & 0xFFFF);
		}
	}
}*/

static unsigned int PkwareBufferRead(char* buf, unsigned int* size, void* param)
{
	TDataInfo* pInfo;
	size_t sSize;

	pInfo = (TDataInfo*)param;

	sSize = pInfo->size - pInfo->srcOffset;
	if (*size < sSize)
		sSize = *size;

	memcpy(buf, pInfo->srcData + pInfo->srcOffset, sSize);
	pInfo->srcOffset += sSize;

	return sSize;
}

static void PkwareBufferWrite(char* buf, unsigned int* size, void* param)
{
	TDataInfo* pInfo;
	size_t sSize;

	pInfo = (TDataInfo*)param;

	sSize = *size;
	memcpy(pInfo->destData + pInfo->destOffset, buf, sSize);
	pInfo->destOffset += sSize;
}

DWORD PkwareCompress(BYTE* srcData, DWORD size)
{
	BYTE* destData;
	char* ptr;
	unsigned int destSize;

	ptr = (char*)DiabloAllocPtr(CMP_BUFFER_SIZE);

	destSize = 2 * size;
	if (destSize < 2 * CMP_IMPLODE_DICT_SIZE3)
		destSize = 2 * CMP_IMPLODE_DICT_SIZE3;

	destData = DiabloAllocPtr(destSize);

	TDataInfo info = TDataInfo(srcData, destData, size);

	// type = CMP_BINARY;
	// dsize = CMP_IMPLODE_DICT_SIZE3;
	implode(PkwareBufferRead, PkwareBufferWrite, ptr, &info);

	// size = info.size;
	if (info.destOffset < size) {
		size = info.destOffset;
		memcpy(info.srcData, info.destData, size);
	}
	mem_free_dbg(ptr);
	mem_free_dbg(info.destData);

	return size;
}

void PkwareDecompress(BYTE* srcData, unsigned recv_size, unsigned dwMaxBytes)
{
	BYTE* destData;
	char* ptr;

	ptr = (char*)DiabloAllocPtr(EXP_BUFFER_SIZE);
	destData = DiabloAllocPtr(dwMaxBytes);

	TDataInfo info = TDataInfo(srcData, destData, recv_size);

	explode(PkwareBufferRead, PkwareBufferWrite, ptr, &info);
	//if (info.destOffset > info.size) {
		memcpy(info.srcData, info.destData, info.destOffset);
	//}
	mem_free_dbg(ptr);
	mem_free_dbg(info.destData);
}

DEVILUTION_END_NAMESPACE
