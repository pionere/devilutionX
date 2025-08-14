/**
 * @file mpqapi.cpp
 *
 * Implementation of functions for creating and editing MPQ files.
 */
#include <cerrno>
#include <cstdio>

#include "all.h"
#include "utils/file_util.h"
#include "mpqapi.h"

DEVILUTION_BEGIN_NAMESPACE

// Amiga cannot seekp beyond EOF.
// See https://github.com/bebbo/libnix/issues/30
#ifndef __AMIGA__
#define CAN_SEEKP_BEYOND_EOF
#endif

namespace {

#if DEBUG_MODE
const char* DirToString(int dir)
{
	switch (dir) {
	case SEEK_SET:
		return "SEEK_SET";
	case SEEK_END:
		return "SEEK_END";
	case SEEK_CUR:
		return "SEEK_CUR";
	default:
		return "invalid";
	}
}

template <typename... PrintFArgs>
void PrintError(const char* fmt, PrintFArgs... args)
{
	std::string fmt_with_error = fmt;
	fmt_with_error.append(": failed with \"%s\"");
	const char* error_message = std::strerror(errno);
	if (error_message == NULL)
		error_message = "";
	DoLog(fmt_with_error.c_str(), args..., error_message);
}
#else
#define PrintError(fmt, ...) ((void)0)
/*template <typename... PrintFArgs>
void PrintError(const char* fmt, PrintFArgs... args)
{
	std::string fmt_with_error = fmt;
	fmt_with_error.append(" failed (%d)");
	//DoLogError(fmt_with_error.c_str(), args..., errno);
	DoLog(fmt_with_error.c_str(), args..., errno);
}*/
#endif /* DEBUG_MODE */
struct FStreamWrapper {
public:
#ifdef CAN_SEEKP_BEYOND_EOF
	void Open(FILE* file)
	{
		// assert(file != NULL);
		s_ = file;
	}
#else
	void Open(FILE* file, uint32_t size)
	{
		// assert(file != NULL);
		s_ = file;
		sFileSize = size;
		sFilePos = 0;
	}
#endif

	void Close()
	{
		// assert(s_ != NULL);
		std::fclose(s_);
		s_ = NULL;
	}

	bool IsOpen() const
	{
		return s_ != NULL;
	}

	bool writeTo(uint32_t offset, const char* data, uint32_t len)
	{
#ifndef CAN_SEEKP_BEYOND_EOF
		// Ensure we do not seekp beyond EOF by filling the missing space.
		uint32_t curSize = sFileSize;
		if (curSize < offset) {
			uint32_t fillerSize = offset - curSize;
			char* filler = (char*)DiabloAllocPtr(fillerSize);
			bool res = writeTo(curSize, filler, fillerSize);
			mem_free_dbg(filler);
			if (!res)
				return res;
		}
#endif
		return seekp(offset) && write(data, len);
	}

	bool readFrom(uint32_t offset, void* out, uint32_t len)
	{
		return seekp(offset) && read(out, len);
	}
#ifndef CAN_SEEKP_BEYOND_EOF
	uint32_t CurrentSize() const {
		return sFileSize;
	}
#endif
	void resize(uint32_t size) {
#if defined(_WIN32)
		seekp(size);
		SetEndOfFile(s_);
#else
		fflush(s_);
		ftruncate(fileno(s_), size);
#endif
#ifndef CAN_SEEKP_BEYOND_EOF
		// sFilePos = ?;
		// sFileSize = size;
#endif
	}

private:
	FILE *s_ = nullptr;
#ifndef CAN_SEEKP_BEYOND_EOF
	uint32_t sFilePos;
	uint32_t sFileSize;
#endif

	bool seekp(long pos)
	{
		if (std::fseek(s_, pos, SEEK_SET) == 0) {
#if DEBUG_MODE
			DoLog("seekp(%" PRIdMAX ", %s)", static_cast<std::intmax_t>(pos), DirToString(SEEK_SET));
#endif
#ifndef CAN_SEEKP_BEYOND_EOF
			sFilePos = pos;
#endif
			return true;
		}
		PrintError("seekp(%" PRIdMAX ", %d)", static_cast<std::intmax_t>(pos), SEEK_SET);
		return false;
	}

	bool read(void* out, size_t size)
	{
		if (ReadFile(out, size, s_)) {
#if DEBUG_MODE
			DoLog("read(out, %" PRIuMAX ")", static_cast<std::uintmax_t>(size));
#endif
#ifndef CAN_SEEKP_BEYOND_EOF
			sFilePos += size;
#endif
			return true;
		}
		PrintError("read(out, %" PRIuMAX ")", static_cast<std::uintmax_t>(size));
		return false;
	}

	bool write(const char* data, size_t size)
	{
		if (WriteFile(data, size, s_)) {
#if DEBUG_MODE
			DoLog("write(data, %" PRIuMAX ")", static_cast<std::uintmax_t>(size));
#endif
#ifndef CAN_SEEKP_BEYOND_EOF
			sFilePos += size;
			if (sFilePos > sFileSize)
				sFileSize = sFilePos;
#endif
			return true;
		}
		PrintError("write(data, %" PRIuMAX ")", static_cast<std::uintmax_t>(size));
		return false;
	}
};

//#define MPQ_BLOCK_SIZE			0x8000
//#define MPQ_HASH_SIZE			0x8000

//constexpr DWORD MPQ_BLOCK_COUNT = MPQ_BLOCK_SIZE / sizeof(FileMpqBlockEntry);
//constexpr DWORD MPQ_HASH_COUNT = MPQ_HASH_SIZE / sizeof(FileMpqHashEntry);
constexpr uint32_t MPQ_BLOCK_OFFSET = sizeof(FileMpqHeader);
constexpr uint32_t MPQ_SECTOR_SIZE = 512 << MPQ_SECTOR_SIZE_SHIFT_V1; // 4096
//constexpr long  MPQ_HASH_OFFSET = MPQ_BLOCK_OFFSET + MPQ_BLOCK_COUNT * sizeof(FileMpqBlockEntry);

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
static void ByteSwapHdr(FileMpqHeader* hdr)
{
	hdr->pqSignature = SwapLE32(hdr->pqSignature);
	hdr->pqHeaderSize = SwapLE32(hdr->pqHeaderSize);
	hdr->pqFileSize = SwapLE32(hdr->pqFileSize);
	hdr->pqVersion = SwapLE16(hdr->pqVersion);
	hdr->pqSectorSizeId = SwapLE16(hdr->pqSectorSizeId);
	hdr->pqHashOffset = SwapLE32(hdr->pqHashOffset);
	hdr->pqBlockOffset = SwapLE32(hdr->pqBlockOffset);
	hdr->pqHashCount = SwapLE32(hdr->pqHashCount);
	hdr->pqBlockCount = SwapLE32(hdr->pqBlockCount);
}

static void ByteSwapBlockTbl(FileMpqBlockEntry* blockTbl, int blockCount)
{
	int i;

	for (i = 0; i < blockCount; i++) {
		blockTbl[i].bqOffset = SwapLE32(blockTbl[i].bqOffset);
		blockTbl[i].bqSizeAlloc = SwapLE32(blockTbl[i].bqSizeAlloc);
		blockTbl[i].bqSizeFile = SwapLE32(blockTbl[i].bqSizeFile);
		blockTbl[i].bqFlags = SwapLE32(blockTbl[i].bqFlags);
	}
}

static void ByteSwapHashTbl(FileMpqHashEntry* hashTbl, int hashCount)
{
	int i;

	for (i = 0; i < hashCount; i++) {
		hashTbl[i].hqHashA = SwapLE32(hashTbl[i].hqHashA);
		hashTbl[i].hqHashB = SwapLE32(hashTbl[i].hqHashB);
		hashTbl[i].hqLocale = SwapLE16(hashTbl[i].hqLocale);
		hashTbl[i].hqPlatform = SwapLE16(hashTbl[i].hqPlatform);
		hashTbl[i].hqBlock = SwapLE32(hashTbl[i].hqBlock);
	}
}
#else
#define ByteSwapHdr(x)
#define ByteSwapBlockTbl(x, y)
#define ByteSwapHashTbl(x, y)
#endif

struct Archive {
	FStreamWrapper stream;
#if DEBUG_MODE
	std::string name;
#endif
	FileMpqHeader mpqHeader;
#ifndef CAN_SEEKP_BEYOND_EOF
	long stream_begin;
#endif

	FileMpqHashEntry* sgpHashTbl;
	FileMpqBlockEntry* sgpBlockTbl;

	bool OpenArchive(const char* name)
	{
		// assert(!stream.IsOpen());
#if DEBUG_MODE
		DoLog("Opening %s", name);
#endif
		FILE* file = FileOpen(name, "r+b");

		if (file == NULL) {
#if DEBUG_MODE
			DoLog("Failed to open file (\"%s\")", name);
#endif
			return false;
		}
#ifdef CAN_SEEKP_BEYOND_EOF
		this->stream.Open(file);
#else
		std::uintmax_t size;
		if (!GetFileSize(name, &size) || size > UINT32_MAX) {
#if DEBUG_MODE
			DoLog("Failed to get a valid file-size (\"%s\")", name, size);
#endif
			fclose(file);
			return false;
		}
		this->stream.Open(file, (uint32_t)size);
#endif
#if DEBUG_MODE
		this->name = name;
#endif
		return true;
	}

	bool CreateArchive(const char* name)
	{
		// assert(!stream.IsOpen());
#if DEBUG_MODE
		DoLog("Creating %s", name);
#endif
		FILE* file = FileOpen(name, "wb");
		if (file == NULL) {
#if DEBUG_MODE
			DoLog("Failed to create file (\"%s\")", name);
#endif
			return false;
		}
#ifdef CAN_SEEKP_BEYOND_EOF
		this->stream.Open(file);
#else
		this->stream.Open(file, 0);
#endif
#if DEBUG_MODE
		this->name = name;
#endif
		return true;
	}

	void FlushArchive()
	{
#if DEBUG_MODE
		DoLog("Flushing %s", name.c_str());
#endif
		// assert(stream.IsOpen());
		if (WriteHeaderAndTables()) {
			// assert(mpqHeader.pqFileSize != 0);
#if DEBUG_MODE
			DoLog("ResizeFile(\"%s\", %" PRIuMAX ")", name.c_str(), mpqHeader.pqFileSize);
#endif
			stream.resize(mpqHeader.pqFileSize);
			stream.Close();
		}
	}

	void CloseArchive(bool clear_tables)
	{
		if (stream.IsOpen()) {
#if DEBUG_MODE
			DoLog("Closing %s", name.c_str());
#endif
			stream.Close();
		}
		if (clear_tables) {
			MemFreeDbg(sgpHashTbl);
			MemFreeDbg(sgpBlockTbl);
		}
	}

	bool WriteHeaderAndTables()
	{
		return WriteHeader() && WriteBlockTable() && WriteHashTable();
	}

private:
	bool WriteHeader()
	{
		ByteSwapHdr(&mpqHeader);

		const bool success = stream.writeTo(0, reinterpret_cast<const char*>(&mpqHeader), sizeof(mpqHeader));
		ByteSwapHdr(&mpqHeader);
		return success;
	}

	bool WriteBlockTable()
	{
		DWORD blockSize, key = MPQ_KEY_BLOCK_TABLE; //HashStringSlash("(block table)", MPQ_HASH_FILE_KEY);

		ByteSwapBlockTbl(this->sgpBlockTbl, this->mpqHeader.pqBlockCount);

		blockSize = this->mpqHeader.pqBlockCount * sizeof(FileMpqBlockEntry);

		EncryptMpqBlock(this->sgpBlockTbl, blockSize, key);
		const bool success = stream.writeTo(this->mpqHeader.pqBlockOffset, reinterpret_cast<const char*>(this->sgpBlockTbl), blockSize);
		DecryptMpqBlock(this->sgpBlockTbl, blockSize, key);
		ByteSwapBlockTbl(this->sgpBlockTbl, this->mpqHeader.pqBlockCount);
		return success;
	}

	bool WriteHashTable()
	{
		DWORD hashSize, key = MPQ_KEY_HASH_TABLE; //HashStringSlash("(hash table)", MPQ_HASH_FILE_KEY);

		ByteSwapHashTbl(this->sgpHashTbl, this->mpqHeader.pqHashCount);

		hashSize = this->mpqHeader.pqHashCount * sizeof(FileMpqHashEntry);

		EncryptMpqBlock(this->sgpHashTbl, hashSize, key);
		const bool success = stream.writeTo(this->mpqHeader.pqHashOffset, reinterpret_cast<const char*>(this->sgpHashTbl), hashSize);
		DecryptMpqBlock(this->sgpHashTbl, hashSize, key);
		ByteSwapHashTbl(this->sgpHashTbl, this->mpqHeader.pqHashCount);
		return success;
	}
};

static Archive cur_archive;

static uint32_t CalcHashOffset(DWORD blockCount)
{
	return MPQ_BLOCK_OFFSET + blockCount * sizeof(FileMpqBlockEntry);
}

static bool IsValidMPQHeader(const FileMpqHeader* hdr)
{
#if DEBUG_MODE
	if (hdr->pqSignature != ID_MPQ
	 || hdr->pqHeaderSize != MPQ_HEADER_SIZE_V1
	 || hdr->pqVersion != MPQ_FORMAT_VERSION_1
	 || hdr->pqSectorSizeId != MPQ_SECTOR_SIZE_SHIFT_V1)
		DoLog("Invalid header format sig(%d vs. %d), hs(%d vs. %d) v(%d vs. %d) ss(%d vs. %d)", hdr->pqSignature, ID_MPQ, hdr->pqHeaderSize, MPQ_HEADER_SIZE_V1, hdr->pqVersion, MPQ_FORMAT_VERSION_1, hdr->pqSectorSizeId, MPQ_SECTOR_SIZE_SHIFT_V1);
		return false;
#endif
	return hdr->pqBlockCount != 0                         // required by OpenMPQ
	 && (int)hdr->pqHashCount > 0                         // required by mpqapi_has_entry / mpqapi_rename_entry / mpqapi_remove_entry ( !=0 by OpenMPQ)
	 && (hdr->pqHashCount & (hdr->pqHashCount - 1)) == 0; // hashCount must be a power of two (required by mpqapi_get_hash_index / mpqapi_add_entry)
}

} // namespace

static uint32_t mpqapi_new_block()
{
	FileMpqBlockEntry* pBlock;
	uint32_t i, blockCount;

	pBlock = cur_archive.sgpBlockTbl;
	blockCount = cur_archive.mpqHeader.pqBlockCount;
	for (i = 0; i < blockCount; i++, pBlock++) {
		if (pBlock->bqOffset == 0) {
			// assert((pBlock->bqSizeAlloc | pBlock->bqFlags | pBlock->bqSizeFile) == 0);
			return i;
		}
	}

	app_fatal("Out of free block entries");
	return 0;
}

static void mpqapi_alloc_block(uint32_t block_offset, uint32_t block_size)
{
	FileMpqBlockEntry* pBlock;
	uint32_t i;
//restart:
	pBlock = cur_archive.sgpBlockTbl;
	for (i = cur_archive.mpqHeader.pqBlockCount; i != 0; i--, pBlock++) {
		if (pBlock->bqOffset != 0 && pBlock->bqFlags == 0 && pBlock->bqSizeFile == 0) {
			if (pBlock->bqOffset + pBlock->bqSizeAlloc == block_offset) {
				// preceeding empty block -> mark the block-entry as unallocated and restart(?) with the merged region
				block_offset = pBlock->bqOffset;
				block_size += pBlock->bqSizeAlloc;
				pBlock->bqOffset = 0;
				pBlock->bqSizeAlloc = 0;
				//goto restart;
			}
			if (block_offset + block_size == pBlock->bqOffset) {
				// succeeding empty block -> mark the block-entry as unallocated and restart(?) with the merged region
				block_size += pBlock->bqSizeAlloc;
				pBlock->bqOffset = 0;
				pBlock->bqSizeAlloc = 0;
				//goto restart;
			}
		}
	}
	if (block_offset + block_size > cur_archive.mpqHeader.pqFileSize) {
		app_fatal("MPQ free list error");
	}
	if (block_offset + block_size == cur_archive.mpqHeader.pqFileSize) {
		cur_archive.mpqHeader.pqFileSize = block_offset;
	} else {
		i = mpqapi_new_block();
		pBlock = &cur_archive.sgpBlockTbl[i];
		pBlock->bqOffset = block_offset;
		pBlock->bqSizeAlloc = block_size;
		pBlock->bqSizeFile = 0;
		pBlock->bqFlags = 0;
	}
}

static uint32_t mpqapi_find_free_block(uint32_t size, uint32_t* block_size)
{
	FileMpqBlockEntry* pBlock;
	uint32_t i, result;

	pBlock = cur_archive.sgpBlockTbl;
	for (i = cur_archive.mpqHeader.pqBlockCount; i != 0; i--, pBlock++) {
		result = pBlock->bqOffset;
		if (result != 0 && pBlock->bqFlags == 0
		 && pBlock->bqSizeFile == 0 && pBlock->bqSizeAlloc >= size) {
			*block_size = size;
			pBlock->bqOffset += size;
			pBlock->bqSizeAlloc -= size;

			if (pBlock->bqSizeAlloc == 0) {
				pBlock->bqOffset = 0;
				// memset(pBlock, 0, sizeof(*pBlock));
			}

			return result;
		}
	}

	*block_size = size;
	result = cur_archive.mpqHeader.pqFileSize;
	cur_archive.mpqHeader.pqFileSize += size;
	return result;
}

static int mpqapi_get_hash_index(DWORD index, DWORD hash_a, DWORD hash_b)
{
	uint32_t idx, i, hashCount;
	FileMpqHashEntry* pHash;

	idx = index;
	hashCount = cur_archive.mpqHeader.pqHashCount;
	for (i = hashCount; i != 0; i--, idx++) {
		idx &= hashCount - 1;
		pHash = &cur_archive.sgpHashTbl[idx];
		if (pHash->hqBlock == HASH_ENTRY_FREE)
			break;
		if (pHash->hqHashA == hash_a && pHash->hqHashB == hash_b
		    /*&& pHash->lcid == locale*/ && pHash->hqBlock != HASH_ENTRY_DELETED)
			return idx;
	}

	return -1;
}

static int FetchHandle(const char* pszName)
{
	return mpqapi_get_hash_index(HashStringSlash(pszName, MPQ_HASH_TABLE_INDEX), HashStringSlash(pszName, MPQ_HASH_NAME_A), HashStringSlash(pszName, MPQ_HASH_NAME_B));
}

void mpqapi_remove_entry(const char* pszName)
{
	FileMpqHashEntry* pHash;
	FileMpqBlockEntry* pBlock;
	int hIdx, block_offset, block_size;

	hIdx = FetchHandle(pszName);
	if (hIdx >= 0) {
		pHash = &cur_archive.sgpHashTbl[hIdx];
		pBlock = &cur_archive.sgpBlockTbl[pHash->hqBlock];
		pHash->hqBlock = HASH_ENTRY_DELETED;
		block_offset = pBlock->bqOffset;
		block_size = pBlock->bqSizeAlloc;
		memset(pBlock, 0, sizeof(*pBlock));
		mpqapi_alloc_block(block_offset, block_size);
	}
}

static uint32_t mpqapi_add_entry(const char* pszName, uint32_t block_index)
{
	FileMpqHashEntry* pHash;
	DWORD i, h1, h2, h3, hashCount;

	h1 = HashStringSlash(pszName, MPQ_HASH_TABLE_INDEX);
	h2 = HashStringSlash(pszName, MPQ_HASH_NAME_A);
	h3 = HashStringSlash(pszName, MPQ_HASH_NAME_B);
	if (mpqapi_get_hash_index(h1, h2, h3) >= 0)
		app_fatal("Hash collision between \"%s\" and existing file\n", pszName);

	hashCount = cur_archive.mpqHeader.pqHashCount;
	for (i = hashCount; i != 0; i--, h1++) {
		h1 &= hashCount - 1;
		pHash = &cur_archive.sgpHashTbl[h1];
		if (pHash->hqBlock == HASH_ENTRY_FREE || pHash->hqBlock == HASH_ENTRY_DELETED) {
			if (block_index == HASH_ENTRY_FREE)
				block_index = mpqapi_new_block();

			pHash->hqHashA = h2;
			pHash->hqHashB = h3;
			pHash->hqLocale = 0;
			pHash->hqPlatform = 0;
			pHash->hqBlock = block_index;
			return block_index;
		}
	}

	app_fatal("Out of hash space");
	return 0;
}

static bool mpqapi_write_file_contents(BYTE* pbData, DWORD dwLen, uint32_t block)
{
	FileMpqBlockEntry* pBlk = &cur_archive.sgpBlockTbl[block];
	// assert(dwLen != 0);
	const uint32_t num_sectors = ((dwLen - 1) / MPQ_SECTOR_SIZE) + 1;
	const uint32_t offset_table_bytesize = sizeof(uint32_t) * (num_sectors + 1);
	pBlk->bqOffset = mpqapi_find_free_block(dwLen + offset_table_bytesize, &pBlk->bqSizeAlloc);
	pBlk->bqSizeFile = dwLen;
	pBlk->bqFlags = MPQ_FILE_EXISTS | MPQ_FILE_IMPLODE; //0x80000100

	// We populate the table of sector offset while we write the data.
	// We can't pre-populate it because we don't know the compressed sector sizes yet.
	// First offset is the start of the first sector, last offset is the end of the last sector.
	uint32_t* sectoroffsettable = (uint32_t*)DiabloAllocPtr((num_sectors + 1) * sizeof(uint32_t));
	{

	uint32_t destsize = offset_table_bytesize;
	unsigned cur_sector = 0;
	sectoroffsettable[0] = SwapLE32(destsize);
	BYTE* src = pbData;
	BYTE* dst = pbData;
	while (true) {
		uint32_t len = std::min(dwLen, MPQ_SECTOR_SIZE);
		uint32_t cmplen = PkwareCompress(src, len);
		if (src != dst) {
			memmove(dst, src, cmplen);
		}
		src += len;
		dst += cmplen;
		destsize += cmplen; // compressed length
		sectoroffsettable[++cur_sector] = SwapLE32(destsize);
		if (dwLen > MPQ_SECTOR_SIZE)
			dwLen -= MPQ_SECTOR_SIZE;
		else
			break;
	}

	if (!cur_archive.stream.writeTo(pBlk->bqOffset, reinterpret_cast<const char*>(sectoroffsettable), offset_table_bytesize))
		goto on_error;

	if (!cur_archive.stream.writeTo(pBlk->bqOffset + offset_table_bytesize, reinterpret_cast<const char*>(pbData), destsize - offset_table_bytesize))
		goto on_error;

	if (destsize < pBlk->bqSizeAlloc) {
		const uint32_t emptyBlockSize = pBlk->bqSizeAlloc - destsize;
		//if (emptyBlockSize >= (MPQ_SECTOR_SIZE / 4)) {
			pBlk->bqSizeAlloc = destsize;
			mpqapi_alloc_block(pBlk->bqSizeAlloc + pBlk->bqOffset, emptyBlockSize);
		//}
	}
	mem_free_dbg(sectoroffsettable);
	return true;
	}
on_error:
	mem_free_dbg(sectoroffsettable);
	return false;
}

bool mpqapi_write_entry(const char* pszName, BYTE* pbData, DWORD dwLen)
{
	uint32_t block;

	mpqapi_remove_entry(pszName);
	block = mpqapi_add_entry(pszName, HASH_ENTRY_FREE);
	if (!mpqapi_write_file_contents(pbData, dwLen, block)) {
		mpqapi_remove_entry(pszName);
		return false;
	}
	return true;
}

void mpqapi_rename_entry(char* pszOld, char* pszNew)
{
	int index;
	FileMpqHashEntry* pHash;
	uint32_t block;

	index = FetchHandle(pszOld);
	if (index >= 0) {
		pHash = &cur_archive.sgpHashTbl[index];
		block = pHash->hqBlock;
		pHash->hqBlock = HASH_ENTRY_DELETED;
		mpqapi_add_entry(pszNew, block);
	}
}

bool mpqapi_has_entry(const char* pszName)
{
	return FetchHandle(pszName) >= 0;
}

bool OpenMPQ(const char* pszArchive)
{
	DWORD blockSize, hashSize, key;

	if (!cur_archive.OpenArchive(pszArchive)) {
		return false;
	}
	if (cur_archive.sgpBlockTbl == NULL/* || cur_archive.sgpHashTbl == NULL*/) {
		if (!cur_archive.stream.readFrom(0, &cur_archive.mpqHeader, sizeof(cur_archive.mpqHeader)))
			goto on_error;
		ByteSwapHdr(&cur_archive.mpqHeader);
		if (!IsValidMPQHeader(&cur_archive.mpqHeader))
			goto on_error;

		blockSize = cur_archive.mpqHeader.pqBlockCount * sizeof(FileMpqBlockEntry);
		cur_archive.sgpBlockTbl = (FileMpqBlockEntry*)DiabloAllocPtr(blockSize);
		hashSize = cur_archive.mpqHeader.pqHashCount * sizeof(FileMpqHashEntry);
		cur_archive.sgpHashTbl = (FileMpqHashEntry*)DiabloAllocPtr(hashSize);
		if (cur_archive.sgpBlockTbl == NULL || cur_archive.sgpHashTbl == NULL)
			goto on_error;

		if (!cur_archive.stream.readFrom(cur_archive.mpqHeader.pqBlockOffset, cur_archive.sgpBlockTbl, blockSize))
			goto on_error;
		key = MPQ_KEY_BLOCK_TABLE; //HashStringSlash("(block table)", MPQ_HASH_FILE_KEY);
		DecryptMpqBlock(cur_archive.sgpBlockTbl, blockSize, key);
		ByteSwapBlockTbl(cur_archive.sgpBlockTbl, cur_archive.mpqHeader.pqBlockCount);

		if (!cur_archive.stream.readFrom(cur_archive.mpqHeader.pqHashOffset, cur_archive.sgpHashTbl, hashSize))
			goto on_error;
		key = MPQ_KEY_HASH_TABLE; //HashStringSlash("(hash table)", MPQ_HASH_FILE_KEY);
		DecryptMpqBlock(cur_archive.sgpHashTbl, hashSize, key);
		ByteSwapHashTbl(cur_archive.sgpHashTbl, cur_archive.mpqHeader.pqHashCount);
	}
	return true;
on_error:
	cur_archive.CloseArchive(true);
	return false;
}

bool CreateMPQ(const char* pszArchive, int hashCount, int blockCount)
{
	DWORD blockSize, hashSize;

	if (!cur_archive.CreateArchive(pszArchive)) {
		return false;
	}
	// assert(hashCount != 0);
	// assert(blockCount != 0);
	// hashCount must be a power of two
	// assert((hashCount & (hashCount - 1)) == 0); // required by mpqapi_get_hash_index / mpqapi_add_entry
	// assert(hashCount <= INT_MAX); // required by mpqapi_has_entry / mpqapi_rename_entry / mpqapi_remove_entry
	// InitDefaultMpqHeader
	std::memset(&cur_archive.mpqHeader, 0, sizeof(cur_archive.mpqHeader));
	cur_archive.mpqHeader.pqSignature = ID_MPQ;
	cur_archive.mpqHeader.pqHeaderSize = MPQ_HEADER_SIZE_V1;
	cur_archive.mpqHeader.pqSectorSizeId = MPQ_SECTOR_SIZE_SHIFT_V1;
	cur_archive.mpqHeader.pqVersion = MPQ_FORMAT_VERSION_1;
	cur_archive.mpqHeader.pqHashCount = hashCount;
	cur_archive.mpqHeader.pqBlockCount = blockCount;
	cur_archive.mpqHeader.pqHashOffset = CalcHashOffset(blockCount);
	cur_archive.mpqHeader.pqBlockOffset = MPQ_BLOCK_OFFSET;
	cur_archive.mpqHeader.pqFileSize = cur_archive.mpqHeader.pqHashOffset + cur_archive.mpqHeader.pqHashCount * sizeof(FileMpqHashEntry);

	blockSize = blockCount * sizeof(FileMpqBlockEntry);
	cur_archive.sgpBlockTbl = (FileMpqBlockEntry*)DiabloAllocPtr(blockSize);
	hashSize = hashCount * sizeof(FileMpqHashEntry);
	cur_archive.sgpHashTbl = (FileMpqHashEntry*)DiabloAllocPtr(hashSize);
	if (cur_archive.sgpBlockTbl == NULL || cur_archive.sgpHashTbl == NULL)
		goto on_error;

	std::memset(cur_archive.sgpBlockTbl, 0, blockSize);
	static_assert(HASH_ENTRY_FREE == 0xFFFFFFFF, "CreateMPQ initializes the hashtable with 0xFF to mark the entries as free.");
	std::memset(cur_archive.sgpHashTbl, 0xFF, hashSize);

	return true;
on_error:
	cur_archive.CloseArchive(true);
	return false;
}

void mpqapi_flush_and_close(bool bFree)
{
	cur_archive.FlushArchive();
	cur_archive.CloseArchive(bFree);
}

void mpqapi_close()
{
	cur_archive.CloseArchive(true);
}

DEVILUTION_END_NAMESPACE
