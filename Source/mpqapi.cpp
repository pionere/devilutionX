/**
 * @file mpqapi.cpp
 *
 * Implementation of functions for creating and editing MPQ files.
 */
#include <cerrno>
#include <cinttypes>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <memory>
#include <type_traits>

#include "all.h"
#include "utils/file_util.h"

DEVILUTION_BEGIN_NAMESPACE

// Amiga cannot seekp beyond EOF.
// See https://github.com/bebbo/libnix/issues/30
#ifndef __AMIGA__
#define CAN_SEEKP_BEYOND_EOF
#endif

namespace {

#if DEBUG_MODE
const char* DirToString(std::ios::seekdir dir)
{
	switch (dir) {
	case std::ios::beg:
		return "std::ios::beg";
	case std::ios::end:
		return "std::ios::end";
	case std::ios::cur:
		return "std::ios::cur";
	default:
		return "invalid";
	}
}

std::string OpenModeToString(std::ios::openmode mode)
{
	std::string result;
	if ((mode & std::ios::app) != 0)
		result.append("std::ios::app | ");
	if ((mode & std::ios::ate) != 0)
		result.append("std::ios::ate | ");
	if ((mode & std::ios::binary) != 0)
		result.append("std::ios::binary | ");
	if ((mode & std::ios::in) != 0)
		result.append("std::ios::in | ");
	if ((mode & std::ios::out) != 0)
		result.append("std::ios::out | ");
	if ((mode & std::ios::trunc) != 0)
		result.append("std::ios::trunc | ");
	if (!result.empty())
		result.resize(result.size() - 3);
	return result;
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
// TODO: use TFileStream ?
struct FStreamWrapper {
public:
	bool Open(const char* path, std::ios::openmode mode)
	{
		s_.open(path, mode);
		if (!s_.fail()) {
#if DEBUG_MODE
			DoLog("Open(\"%s\", %s)", path, OpenModeToString(mode).c_str());
#endif
			return true;
		}
		PrintError("Open(\"%s\", %d)", path, mode);
		return false;
	}

	void Close()
	{
		s_.close();
	}

	bool IsOpen() const
	{
		return s_.is_open();
	}
#ifdef FULL
	bool seekg(std::streampos pos)
	{
		s_.seekg(pos);
		if (!s_.fail()) {
#if DEBUG_MODE
			DoLog("seekg(%" PRIuMAX ")", static_cast<std::uintmax_t>(pos));
#endif
			return true;
		}
		PrintError("seekg(%" PRIuMAX ")", static_cast<std::uintmax_t>(pos));
		return false;
	}

	bool seekg(std::streamoff pos, std::ios::seekdir dir)
	{
		s_.seekg(pos, dir);
		if (!s_.fail()) {
#if DEBUG_MODE
			DoLog("seekg(%" PRIdMAX ", %s)", static_cast<std::intmax_t>(pos), DirToString(dir));
#endif
			return true;
		}
		PrintError("seekg(%" PRIdMAX ", %d)", static_cast<std::intmax_t>(pos), dir);
		return false;
	}

	bool seekp(std::streampos pos)
	{
		s_.seekp(pos);
		if (!s_.fail()) {
#if DEBUG_MODE
			DoLog("seekp(%" PRIuMAX ")", static_cast<std::uintmax_t>(pos));
#endif
			return true;
		}
		PrintError("seekp(%" PRIuMAX ")", static_cast<std::uintmax_t>(pos));
		return false;
	}
#endif
	bool seekp(std::streamoff pos, std::ios::seekdir dir)
	{
		s_.seekp(pos, dir);
		if (!s_.fail()) {
#if DEBUG_MODE
			DoLog("seekp(%" PRIdMAX ", %s)", static_cast<std::intmax_t>(pos), DirToString(dir));
#endif
			return true;
		}
		PrintError("seekp(%" PRIdMAX ", %d)", static_cast<std::intmax_t>(pos), dir);
		return false;
	}
#ifdef FULL
	bool tellg(std::streampos* result)
	{
		*result = s_.tellg();
		if (!s_.fail()) {
#if DEBUG_MODE
			DoLog("tellg() = %" PRIuMAX, static_cast<std::uintmax_t>(*result));
#endif
			return true;
		}
		PrintError("tellg() = %" PRIuMAX, static_cast<std::uintmax_t>(*result));
		return false;
	}
#endif // FULL
#ifndef CAN_SEEKP_BEYOND_EOF
	bool tellp(std::streampos* result)
	{
		*result = s_.tellp();
		if (!s_.fail()) {
#if DEBUG_MODE
			DoLog("tellp() = %" PRIuMAX, static_cast<std::uintmax_t>(*result));
#endif
			return true;
		}
		PrintError("tellp() = %" PRIuMAX, static_cast<std::uintmax_t>(*result));
		return false;
	}
#endif // !CAN_SEEKP_BEYOND_EOF
	bool write(const char* data, std::streamsize size)
	{
		s_.write(data, size);
		if (!s_.fail()) {
#if DEBUG_MODE
			DoLog("write(data, %" PRIuMAX ")", static_cast<std::uintmax_t>(size));
#endif
			return true;
		}
		PrintError("write(data, %" PRIuMAX ")", static_cast<std::uintmax_t>(size));
		return false;
	}

	bool read(char* out, std::streamsize size)
	{
		s_.read(out, size);
		if (!s_.fail()) {
#if DEBUG_MODE
			DoLog("read(out, %" PRIuMAX ")", static_cast<std::uintmax_t>(size));
#endif
			return true;
		}
		PrintError("read(out, %" PRIuMAX ")", static_cast<std::uintmax_t>(size));
		return false;
	}

private:

	std::fstream s_;
};

//#define MPQ_BLOCK_SIZE			0x8000
//#define MPQ_HASH_SIZE			0x8000

//constexpr DWORD MPQ_BLOCK_COUNT = MPQ_BLOCK_SIZE / sizeof(FileMpqBlockEntry);
//constexpr DWORD MPQ_HASH_COUNT = MPQ_HASH_SIZE / sizeof(FileMpqHashEntry);
constexpr uint32_t MPQ_BLOCK_OFFSET = sizeof(FileMpqHeader);
constexpr uint32_t MPQ_SECTOR_SIZE = 512 << MPQ_SECTOR_SIZE_SHIFT_V1; // 4096
//constexpr std::ios::off_type MPQ_HASH_OFFSET = MPQ_BLOCK_OFFSET + MPQ_BLOCK_COUNT * sizeof(FileMpqBlockEntry);

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
	std::string name;
	uint32_t archiveSize;
	bool modified;
	bool exists;
	uint32_t blockCount;
	uint32_t hashCount;

#ifndef CAN_SEEKP_BEYOND_EOF
	std::streampos stream_begin;
#endif

	FileMpqHashEntry* sgpHashTbl;
	FileMpqBlockEntry* sgpBlockTbl;

	bool OpenArchive(const char* name)
	{
		CloseArchive(this->name != name);
#if DEBUG_MODE
		DoLog("Opening %s", name);
#endif
		exists = FileExists(name);
		std::ios::openmode mode = std::ios::out | std::ios::binary;
		std::uintmax_t size;
		if (exists) {
			mode |= std::ios::in;
#if DEBUG_MODE
			if (!GetFileSize(name, &size)) {
				DoLog("GetFileSize(\"%s\") failed with \"%s\"", name, std::strerror(errno));
				return false;
			} else {
				DoLog("GetFileSize(\"%s\") = %" PRIuMAX, name, size);
			}
#else
			if (!GetFileSize(name, &size)) {
				DoLog("GetFileSize(\"%s\") failed. (%d)", name, errno);
				return false;
			}
			if (size > UINT32_MAX) {
				DoLog("OpenArchive(\"%s\") failed. File too large: %" PRIuMAX, name, size);
				return false;
			}
#endif
		} else {
			mode |= std::ios::trunc;
			size = 0;
		}
		if (!stream.Open(name, mode)) {
			stream.Close();
			return false;
		}
		this->archiveSize = static_cast<uint32_t>(size);
		this->modified = !exists;
		this->name = name;
		return true;
	}

	void CloseArchive(bool clear_tables)
	{
		if (stream.IsOpen()) {
#if DEBUG_MODE
			DoLog("Closing %s", name.c_str());
#endif

			bool resize = modified && stream.seekp(0, std::ios::beg) && WriteHeaderAndTables();
			stream.Close();
			if (resize && archiveSize != 0) {
#if DEBUG_MODE
				DoLog("ResizeFile(\"%s\", %" PRIuMAX ")", name.c_str(), archiveSize);
#endif
				ResizeFile(name.c_str(), archiveSize);
			}
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

	uint32_t HashOffset() const
	{
		return MPQ_BLOCK_OFFSET + blockCount * sizeof(FileMpqBlockEntry);
	}

	~Archive()
	{
		CloseArchive(true);
	}

private:
	bool WriteHeader()
	{
		FileMpqHeader fhdr;

		fhdr.pqSignature = SwapLE32(ID_MPQ);
		fhdr.pqHeaderSize = SwapLE32(MPQ_HEADER_SIZE_V1);
		fhdr.pqFileSize = SwapLE32(archiveSize);
		fhdr.pqVersion = SwapLE16(MPQ_FORMAT_VERSION_1);
		fhdr.pqSectorSizeId = SwapLE16(MPQ_SECTOR_SIZE_SHIFT_V1);
		fhdr.pqHashOffset = SwapLE32(HashOffset());
		fhdr.pqBlockOffset = SwapLE32(MPQ_BLOCK_OFFSET);
		fhdr.pqHashCount = SwapLE32(hashCount);
		fhdr.pqBlockCount = SwapLE32(blockCount);
		memset(&fhdr.pqPad[0], 0, sizeof(fhdr.pqPad));

		return stream.write(reinterpret_cast<const char*>(&fhdr), sizeof(fhdr));
	}

	bool WriteBlockTable()
	{
		DWORD blockSize, key = MPQ_KEY_BLOCK_TABLE; //HashStringSlash("(block table)", MPQ_HASH_FILE_KEY);
		FileMpqBlockEntry* blockTbl = sgpBlockTbl;

		ByteSwapBlockTbl(blockTbl, blockCount);

		blockSize = blockCount * sizeof(FileMpqBlockEntry);

		EncryptMpqBlock(blockTbl, blockSize, key);
		const bool success = stream.write(reinterpret_cast<const char*>(blockTbl), blockSize);
		DecryptMpqBlock(blockTbl, blockSize, key);
		ByteSwapBlockTbl(blockTbl, blockCount);
		return success;
	}

	bool WriteHashTable()
	{
		DWORD hashSize, key = MPQ_KEY_HASH_TABLE; //HashStringSlash("(hash table)", MPQ_HASH_FILE_KEY);
		FileMpqHashEntry* hashTbl = sgpHashTbl;

		ByteSwapHashTbl(hashTbl, hashCount);

		hashSize = hashCount * sizeof(FileMpqHashEntry);

		EncryptMpqBlock(hashTbl, hashSize, key);
		const bool success = stream.write(reinterpret_cast<const char*>(hashTbl), hashSize);
		DecryptMpqBlock(hashTbl, hashSize, key);
		ByteSwapHashTbl(hashTbl, hashCount);
		return success;
	}
};

static Archive cur_archive;

static bool IsValidMPQHeader(const Archive &archive, FileMpqHeader* hdr)
{
	return hdr->pqSignature == ID_MPQ
		&& hdr->pqHeaderSize == MPQ_HEADER_SIZE_V1
		&& hdr->pqVersion == MPQ_FORMAT_VERSION_1
		&& hdr->pqSectorSizeId == MPQ_SECTOR_SIZE_SHIFT_V1
		&& hdr->pqFileSize == archive.archiveSize
		&& hdr->pqHashOffset == archive.HashOffset()
		&& hdr->pqBlockOffset == MPQ_BLOCK_OFFSET
		&& hdr->pqHashCount == archive.hashCount
		&& hdr->pqBlockCount == archive.blockCount;
}

// Read the header info from the archive, or setup a skeleton
static bool ReadMPQHeader(Archive* archive, FileMpqHeader* hdr)
{
	const bool has_hdr = archive->archiveSize >= sizeof(*hdr);
	if (has_hdr) {
		if (!archive->stream.read(reinterpret_cast<char*>(hdr), sizeof(*hdr)))
			return false;
		ByteSwapHdr(hdr);
	}
	if (!has_hdr || !IsValidMPQHeader(*archive, hdr)) {
		// InitDefaultMpqHeader
		//std::memset(hdr, 0, sizeof(*hdr));
		//hdr->pqSignature = ID_MPQ;
		//hdr->pqHeaderSize = MPQ_HEADER_SIZE_V1;
		//hdr->pqSectorSizeId = MPQ_SECTOR_SIZE_SHIFT_V1;
		//hdr->pqVersion = MPQ_FORMAT_VERSION_1;
		hdr->pqBlockCount = 0;
		hdr->pqHashCount = 0;
		archive->archiveSize = archive->HashOffset() + archive->hashCount * sizeof(FileMpqHashEntry);
		archive->modified = true;
	}
	return true;
}

} // namespace

static uint32_t mpqapi_new_block()
{
	FileMpqBlockEntry* pBlock;
	uint32_t i, blockCount;

	pBlock = cur_archive.sgpBlockTbl;
	blockCount = cur_archive.blockCount;
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
	for (i = cur_archive.blockCount; i != 0; i--, pBlock++) {
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
	if (block_offset + block_size > cur_archive.archiveSize) {
		app_fatal("MPQ free list error");
	}
	if (block_offset + block_size == cur_archive.archiveSize) {
		cur_archive.archiveSize = block_offset;
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
	for (i = cur_archive.blockCount; i != 0; i--, pBlock++) {
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
	result = cur_archive.archiveSize;
	cur_archive.archiveSize += size;
	return result;
}

static int mpqapi_get_hash_index(DWORD index, DWORD hash_a, DWORD hash_b)
{
	uint32_t idx, i, hashCount;
	FileMpqHashEntry* pHash;

	idx = index;
	hashCount = cur_archive.hashCount;
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
	if (hIdx != -1) {
		pHash = &cur_archive.sgpHashTbl[hIdx];
		pBlock = &cur_archive.sgpBlockTbl[pHash->hqBlock];
		pHash->hqBlock = HASH_ENTRY_DELETED;
		block_offset = pBlock->bqOffset;
		block_size = pBlock->bqSizeAlloc;
		memset(pBlock, 0, sizeof(*pBlock));
		mpqapi_alloc_block(block_offset, block_size);
		cur_archive.modified = true;
	}
}

void mpqapi_remove_entries(bool (*fnGetName)(unsigned, char (&)[DATA_ARCHIVE_MAX_PATH]))
{
	unsigned i;
	char pszFileName[DATA_ARCHIVE_MAX_PATH];

	for (i = 0; fnGetName(i, pszFileName); i++)
		mpqapi_remove_entry(pszFileName);
}

static uint32_t mpqapi_add_entry(const char* pszName, uint32_t block_index)
{
	FileMpqHashEntry* pHash;
	DWORD i, h1, h2, h3, hashCount;

	h1 = HashStringSlash(pszName, MPQ_HASH_TABLE_INDEX);
	h2 = HashStringSlash(pszName, MPQ_HASH_NAME_A);
	h3 = HashStringSlash(pszName, MPQ_HASH_NAME_B);
	if (mpqapi_get_hash_index(h1, h2, h3) != -1)
		app_fatal("Hash collision between \"%s\" and existing file\n", pszName);

	hashCount = cur_archive.hashCount;
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

static bool mpqapi_write_file_contents(const char* pszName, const BYTE* pbData, DWORD dwLen, uint32_t block)
{
	FileMpqBlockEntry* pBlk = &cur_archive.sgpBlockTbl[block];
	const char* tmp;
	while ((tmp = strchr(pszName, ':')))
		pszName = tmp + 1;
	while ((tmp = strchr(pszName, '\\')))
		pszName = tmp + 1;

	const uint32_t num_sectors = (dwLen + (MPQ_SECTOR_SIZE - 1)) / MPQ_SECTOR_SIZE;
	const uint32_t offset_table_bytesize = sizeof(uint32_t) * (num_sectors + 1);
	pBlk->bqOffset = mpqapi_find_free_block(dwLen + offset_table_bytesize, &pBlk->bqSizeAlloc);
	pBlk->bqSizeFile = dwLen;
	pBlk->bqFlags = MPQ_FILE_EXISTS | MPQ_FILE_IMPLODE; //0x80000100

	// We populate the table of sector offset while we write the data.
	// We can't pre-populate it because we don't know the compressed sector sizes yet.
	// First offset is the start of the first sector, last offset is the end of the last sector.
	uint32_t* sectoroffsettable = (uint32_t*)DiabloAllocPtr((num_sectors + 1) * sizeof(uint32_t));
	{
#ifdef CAN_SEEKP_BEYOND_EOF
	if (!cur_archive.stream.seekp(pBlk->bqOffset + offset_table_bytesize, std::ios::beg))
		goto on_error;
#else
	// Ensure we do not seekp beyond EOF by filling the missing space.
	std::streampos stream_end;
	if (!cur_archive.stream.seekp(0, std::ios::end) || !cur_archive.stream.tellp(&stream_end))
		goto on_error;
	std::size_t curSize = stream_end - cur_archive.stream_begin;
	if (curSize < pBlk->bqOffset + offset_table_bytesize) {
		if (curSize < pBlk->bqOffset) {
			curSize = pBlk->bqOffset - curSize;
			char* filler = (char*)DiabloAllocPtr(curSize);
			bool res = cur_archive.stream.write(filler, curSize);
			mem_free_dbg(filler);
			if (!res)
				goto on_error;
		}
		if (!cur_archive.stream.write(reinterpret_cast<const char*>(sectoroffsettable), offset_table_bytesize))
			goto on_error;
	} else {
		if (!cur_archive.stream.seekp(pBlk->bqOffset + offset_table_bytesize, std::ios::beg))
			goto on_error;
	}
#endif

	uint32_t destsize = offset_table_bytesize;
	BYTE mpq_buf[MPQ_SECTOR_SIZE];
	std::size_t cur_sector = 0;
	while (true) {
		uint32_t len = std::min(dwLen, MPQ_SECTOR_SIZE);
		memcpy(mpq_buf, pbData, len);
		pbData += len;
		len = PkwareCompress(mpq_buf, len);
		if (!cur_archive.stream.write(reinterpret_cast<const char*>(mpq_buf), len))
			goto on_error;
		sectoroffsettable[cur_sector++] = SwapLE32(destsize);
		destsize += len; // compressed length
		if (dwLen > MPQ_SECTOR_SIZE)
			dwLen -= MPQ_SECTOR_SIZE;
		else
			break;
	}

	sectoroffsettable[num_sectors] = SwapLE32(destsize);
	if (!cur_archive.stream.seekp(pBlk->bqOffset, std::ios::beg))
		goto on_error;
	if (!cur_archive.stream.write(reinterpret_cast<const char*>(sectoroffsettable), offset_table_bytesize))
		goto on_error;
	if (!cur_archive.stream.seekp(destsize - offset_table_bytesize, std::ios::cur))
		goto on_error;

	if (destsize < pBlk->bqSizeAlloc) {
		const uint32_t emptyBlockSize = pBlk->bqSizeAlloc - destsize;
		if (emptyBlockSize >= (MPQ_SECTOR_SIZE / 4)) {
			pBlk->bqSizeAlloc = destsize;
			mpqapi_alloc_block(pBlk->bqSizeAlloc + pBlk->bqOffset, emptyBlockSize);
		}
	}
	return true;
	}
on_error:
	mem_free_dbg(sectoroffsettable);
	return false;
}

bool mpqapi_write_entry(const char* pszName, const BYTE* pbData, DWORD dwLen)
{
	uint32_t block;

	cur_archive.modified = true;
	mpqapi_remove_entry(pszName);
	block = mpqapi_add_entry(pszName, HASH_ENTRY_FREE);
	if (!mpqapi_write_file_contents(pszName, pbData, dwLen, block)) {
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
	if (index != -1) {
		pHash = &cur_archive.sgpHashTbl[index];
		block = pHash->hqBlock;
		pHash->hqBlock = HASH_ENTRY_DELETED;
		mpqapi_add_entry(pszNew, block);
		cur_archive.modified = true;
	}
}

bool mpqapi_has_entry(const char* pszName)
{
	return FetchHandle(pszName) != -1;
}

bool OpenMPQ(const char* pszArchive, int hashCount, int blockCount)
{
	DWORD blockSize, hashSize, key;
	FileMpqHeader fhdr;

	if (!cur_archive.OpenArchive(pszArchive)) {
		return false;
	}
	if (cur_archive.sgpBlockTbl == NULL || cur_archive.sgpHashTbl == NULL) {
		// hashCount and blockCount must be a power of two
		assert((hashCount & (hashCount - 1)) == 0);
		assert((blockCount & (blockCount - 1)) == 0);
		cur_archive.hashCount = hashCount;
		cur_archive.blockCount = blockCount;
		if (!ReadMPQHeader(&cur_archive, &fhdr)) {
			goto on_error;
		}
		blockSize = blockCount * sizeof(FileMpqBlockEntry);
		cur_archive.sgpBlockTbl = (FileMpqBlockEntry*)DiabloAllocPtr(blockSize);
		if (fhdr.pqBlockCount != 0) {
			if (!cur_archive.stream.read(reinterpret_cast<char*>(cur_archive.sgpBlockTbl), blockSize))
				goto on_error;
			key = MPQ_KEY_BLOCK_TABLE; //HashStringSlash("(block table)", MPQ_HASH_FILE_KEY);
			DecryptMpqBlock(cur_archive.sgpBlockTbl, blockSize, key);
			ByteSwapBlockTbl(cur_archive.sgpBlockTbl, fhdr.pqBlockCount);
		} else {
			std::memset(cur_archive.sgpBlockTbl, 0, blockSize);
		}
		hashSize = hashCount * sizeof(FileMpqHashEntry);
		cur_archive.sgpHashTbl = (FileMpqHashEntry*)DiabloAllocPtr(hashSize);
		if (fhdr.pqHashCount != 0) {
			if (!cur_archive.stream.read(reinterpret_cast<char*>(cur_archive.sgpHashTbl), hashSize))
				goto on_error;
			key = MPQ_KEY_HASH_TABLE; //HashStringSlash("(hash table)", MPQ_HASH_FILE_KEY);
			DecryptMpqBlock(cur_archive.sgpHashTbl, hashSize, key);
			ByteSwapHashTbl(cur_archive.sgpHashTbl, fhdr.pqHashCount);
		} else {
			static_assert(HASH_ENTRY_FREE == 0xFFFFFFFF, "OpenMPQ initializes the hashtable with 0xFF to mark the entries as free.");
			std::memset(cur_archive.sgpHashTbl, 0xFF, hashSize);
		}

#ifndef CAN_SEEKP_BEYOND_EOF
		if (!cur_archive.stream.seekp(0, std::ios::beg))
			goto on_error;

		// Memorize stream begin, we'll need it for calculations later.
		if (!cur_archive.stream.tellp(&cur_archive.stream_begin))
			goto on_error;

		// Write garbage header and tables because some platforms cannot `seekp` beyond EOF.
		// The data is incorrect at this point, it will be overwritten on Close.
		if (!cur_archive.exists)
			cur_archive.WriteHeaderAndTables();
#endif
	}
	return true;
on_error:
	cur_archive.CloseArchive(true);
	return false;
}

void mpqapi_flush_and_close(bool bFree)
{
	cur_archive.CloseArchive(bFree);
}

DEVILUTION_END_NAMESPACE
