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
#include "../SourceS/file_util.h"

DEVILUTION_BEGIN_NAMESPACE

// Amiga cannot seekp beyond EOF.
// See https://github.com/bebbo/libnix/issues/30
#ifndef __AMIGA__
#define CAN_SEEKP_BEYOND_EOF
#endif

namespace {

// Validates that a Type is of a particular size and that its alignment is <= the size of the type.
// Done with templates so that error messages include actual size.
template <std::size_t A, std::size_t B>
struct assert_eq : std::true_type {
	static_assert(A == B, "");
};
template <std::size_t A, std::size_t B>
struct assert_lte : std::true_type {
	static_assert(A <= B, "");
};
template <typename T, std::size_t S>
struct check_size : assert_eq<sizeof(T), S>, assert_lte<alignof(T), sizeof(T)> {
};

// Check sizes and alignments of the structs that we decrypt and encrypt.
// The decryption algorithm treats them as a stream of 32-bit uints, so the
// sizes must be exact as there cannot be any padding.
static_assert(check_size<_HASHENTRY, 4 * 4>::value, "");
static_assert(check_size<_BLOCKENTRY, 4 * 4>::value, "");

const char *DirToString(std::ios::seekdir dir)
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

struct FStreamWrapper {
public:
	bool Open(const char *path, std::ios::openmode mode)
	{
		s_.reset(new std::fstream(path, mode));
		if (!s_->fail()) {
#ifdef _DEBUG
			SDL_Log("new std::fstream(\"%s\", %s)", path, OpenModeToString(mode).c_str());
#endif
			return true;
		}
		PrintError("new std::fstream(\"%s\", %s)", path, OpenModeToString(mode).c_str());
		return false;
	}

	void Close()
	{
		s_ = NULL;
	}

	bool IsOpen() const
	{
		return s_ != NULL;
	}

	bool seekg(std::streampos pos)
	{
		s_->seekg(pos);
		if (!s_->fail()) {
#ifdef _DEBUG
			SDL_Log("seekg(%" PRIuMAX ")", static_cast<std::uintmax_t>(pos));
#endif
			return true;
		}
		PrintError("seekg(%" PRIuMAX ")", static_cast<std::uintmax_t>(pos));
		return false;
	}

	bool seekg(std::streamoff pos, std::ios::seekdir dir)
	{
		s_->seekg(pos, dir);
		if (!s_->fail()) {
#ifdef _DEBUG
			SDL_Log("seekg(%" PRIdMAX ", %s)", static_cast<std::intmax_t>(pos), DirToString(dir));
#endif
			return true;
		}
		PrintError("seekg(%" PRIdMAX ", %s)", static_cast<std::intmax_t>(pos), DirToString(dir));
		return false;
	}

	bool seekp(std::streampos pos)
	{
		s_->seekp(pos);
		if (!s_->fail()) {
#ifdef _DEBUG
			SDL_Log("seekp(%" PRIuMAX ")", static_cast<std::uintmax_t>(pos));
#endif
			return true;
		}
		PrintError("seekp(%" PRIuMAX ")", static_cast<std::uintmax_t>(pos));
		return false;
	}

	bool seekp(std::streamoff pos, std::ios::seekdir dir)
	{
		s_->seekp(pos, dir);
		if (!s_->fail()) {
#ifdef _DEBUG
			SDL_Log("seekp(%" PRIdMAX ", %s)", static_cast<std::intmax_t>(pos), DirToString(dir));
#endif
			return true;
		}
		PrintError("seekp(%" PRIdMAX ", %s)", static_cast<std::intmax_t>(pos), DirToString(dir));
		return false;
	}

	bool tellg(std::streampos *result)
	{
		*result = s_->tellg();
		if (!s_->fail()) {
#ifdef _DEBUG
			SDL_Log("tellg() = %" PRIuMAX, static_cast<std::uintmax_t>(*result));
#endif
			return true;
		}
		PrintError("tellg() = %" PRIuMAX, static_cast<std::uintmax_t>(*result));
		return false;
	}

	bool tellp(std::streampos *result)
	{
		*result = s_->tellp();
		if (!s_->fail()) {
#ifdef _DEBUG
			SDL_Log("tellp() = %" PRIuMAX, static_cast<std::uintmax_t>(*result));
#endif
			return true;
		}
		PrintError("tellp() = %" PRIuMAX, static_cast<std::uintmax_t>(*result));
		return false;
	}

	bool write(const char *data, std::streamsize size)
	{
		s_->write(data, size);
		if (!s_->fail()) {
#ifdef _DEBUG
			SDL_Log("write(data, %" PRIuMAX ")", static_cast<std::uintmax_t>(size));
#endif
			return true;
		}
		PrintError("write(data, %" PRIuMAX ")", static_cast<std::uintmax_t>(size));
		return false;
	}

	bool read(char *out, std::streamsize size)
	{
		s_->read(out, size);
		if (!s_->fail()) {
#ifdef _DEBUG
			SDL_Log("read(out, %" PRIuMAX ")", static_cast<std::uintmax_t>(size));
#endif
			return true;
		}
		PrintError("read(out, %" PRIuMAX ")", static_cast<std::uintmax_t>(size));
		return false;
	}

private:
	template <typename... PrintFArgs>
	void PrintError(const char *fmt, PrintFArgs... args)
	{
		std::string fmt_with_error = fmt;
#ifdef _DEBUG
		fmt_with_error.append(": failed with \"%s\"");
		const char *error_message = std::strerror(errno);
		if (error_message == NULL)
			error_message = "";
		SDL_Log(fmt_with_error.c_str(), args..., error_message);
#else
		fmt_with_error.append(" failed (%d)");
		SDL_Log(fmt_with_error.c_str(), args..., errno);
#endif
	}

	std::unique_ptr<std::fstream> s_;
};

#define MPQ_BLOCK_SIZE			0x8000
#define MPQ_HASH_SIZE			0x8000

constexpr DWORD MPQ_BLOCK_COUNT = MPQ_BLOCK_SIZE / sizeof(_BLOCKENTRY);
constexpr DWORD MPQ_HASH_COUNT = MPQ_HASH_SIZE / sizeof(_HASHENTRY);
constexpr std::ios::off_type MPQ_BLOCK_OFFSET = sizeof(_FILEHEADER);
constexpr std::ios::off_type MPQ_HASH_OFFSET = MPQ_BLOCK_OFFSET + MPQ_BLOCK_SIZE;

struct Archive {
	FStreamWrapper stream;
	std::string name;
	std::uintmax_t size;
	bool modified;
	bool exists;

#ifndef CAN_SEEKP_BEYOND_EOF
	std::streampos stream_begin;
#endif

	_HASHENTRY *sgpHashTbl;
	_BLOCKENTRY *sgpBlockTbl;

	bool Open(const char *name)
	{
		Close();
#ifdef _DEBUG
		SDL_Log("Opening %s", name);
#endif
		exists = FileExists(name);
		std::ios::openmode mode = std::ios::in | std::ios::out | std::ios::binary;
		if (exists) {
#ifdef _DEBUG
			if (!GetFileSize(name, &size)) {
				SDL_Log("GetFileSize(\"%s\") failed with \"%s\"", name, std::strerror(errno));
				return false;
			} else {
				SDL_Log("GetFileSize(\"%s\") = %" PRIuMAX, name, size);
			}
#else
			if (!GetFileSize(name, &size)) {
				SDL_Log("GetFileSize(\"%s\") failed. (%d)", name, errno);
				return false;
			}
#endif
		} else {
			mode |= std::ios::trunc;
		}
		if (!stream.Open(name, mode)) {
			stream.Close();
			return false;
		}
		modified = !exists;

		this->name = name;
		return true;
	}

	bool Close(bool clear_tables = true)
	{
		if (!stream.IsOpen())
			return true;
#ifdef _DEBUG
		SDL_Log("Closing %s", name.c_str());
#endif

		bool result = true;
		if (modified && !(stream.seekp(0, std::ios::beg) && WriteHeaderAndTables()))
			result = false;
		stream.Close();
		if (modified && result && size != 0) {
#ifdef _DEBUG
			SDL_Log("ResizeFile(\"%s\", %" PRIuMAX ")", name.c_str(), size);
#endif
			result = ResizeFile(name.c_str(), size);
		}
		name.clear();
		if (clear_tables) {
			delete[] sgpHashTbl;
			sgpHashTbl = NULL;
			delete[] sgpBlockTbl;
			sgpBlockTbl = NULL;
		}
		return result;
	}

	bool WriteHeaderAndTables()
	{
		return WriteHeader() && WriteBlockTable() && WriteHashTable();
	}

	~Archive()
	{
		Close();
	}

private:
	bool WriteHeader()
	{
		_FILEHEADER fhdr;

		memset(&fhdr, 0, sizeof(fhdr));
		fhdr.signature = SDL_SwapLE32('\x1AQPM');
		fhdr.headersize = SDL_SwapLE32(32);
		fhdr.filesize = SDL_SwapLE32(static_cast<uint32_t>(size));
		fhdr.version = SDL_SwapLE16(0);
		fhdr.sectorsizeid = SDL_SwapLE16(3);
		fhdr.hashoffset = SDL_SwapLE32(static_cast<uint32_t>(MPQ_HASH_OFFSET));
		fhdr.blockoffset = SDL_SwapLE32(static_cast<uint32_t>(MPQ_BLOCK_OFFSET));
		fhdr.hashcount = SDL_SwapLE32(MPQ_HASH_COUNT);
		fhdr.blockcount = SDL_SwapLE32(MPQ_BLOCK_COUNT);

		return stream.write(reinterpret_cast<const char *>(&fhdr), sizeof(fhdr));
	}

	bool WriteBlockTable()
	{
		Encrypt((DWORD *)sgpBlockTbl, MPQ_BLOCK_SIZE, Hash("(block table)", 3));
		const bool success = stream.write(reinterpret_cast<const char *>(sgpBlockTbl), MPQ_BLOCK_SIZE);
		Decrypt((DWORD *)sgpBlockTbl, MPQ_BLOCK_SIZE, Hash("(block table)", 3));
		return success;
	}

	bool WriteHashTable()
	{
		Encrypt((DWORD *)sgpHashTbl, MPQ_HASH_SIZE, Hash("(hash table)", 3));
		const bool success = stream.write(reinterpret_cast<const char *>(sgpHashTbl), MPQ_HASH_SIZE);
		Decrypt((DWORD *)sgpHashTbl, MPQ_HASH_SIZE, Hash("(hash table)", 3));
		return success;
	}
};

Archive cur_archive;

static void ByteSwapHdr(_FILEHEADER *hdr)
{
	hdr->signature = SDL_SwapLE32(hdr->signature);
	hdr->headersize = SDL_SwapLE32(hdr->headersize);
	hdr->filesize = SDL_SwapLE32(hdr->filesize);
	hdr->version = SDL_SwapLE16(hdr->version);
	hdr->sectorsizeid = SDL_SwapLE16(hdr->sectorsizeid);
	hdr->hashoffset = SDL_SwapLE32(hdr->hashoffset);
	hdr->blockoffset = SDL_SwapLE32(hdr->blockoffset);
	hdr->hashcount = SDL_SwapLE32(hdr->hashcount);
	hdr->blockcount = SDL_SwapLE32(hdr->blockcount);
}

static void InitDefaultMpqHeader(Archive *archive, _FILEHEADER *hdr)
{
	std::memset(hdr, 0, sizeof(*hdr));
	hdr->signature = '\x1AQPM';
	hdr->headersize = 32;
	hdr->sectorsizeid = 3;
	hdr->version = 0;
	archive->size = MPQ_HASH_OFFSET + MPQ_HASH_SIZE;
	archive->modified = true;
}

static bool IsValidMPQHeader(const Archive &archive, _FILEHEADER *hdr)
{
	return hdr->signature == '\x1AQPM'
	    && hdr->headersize == 32
	    && hdr->version <= 0
	    && hdr->sectorsizeid == 3
	    && hdr->filesize == archive.size
	    && hdr->hashoffset == MPQ_HASH_OFFSET
	    && hdr->blockoffset == sizeof(_FILEHEADER)
	    && hdr->hashcount == MPQ_HASH_COUNT
	    && hdr->blockcount == MPQ_BLOCK_COUNT;
}

static bool ReadMPQHeader(Archive *archive, _FILEHEADER *hdr)
{
	const bool has_hdr = archive->size >= sizeof(*hdr);
	if (has_hdr) {
		if (!archive->stream.read(reinterpret_cast<char *>(hdr), sizeof(*hdr)))
			return false;
		ByteSwapHdr(hdr);
	}
	if (!has_hdr || !IsValidMPQHeader(*archive, hdr)) {
		InitDefaultMpqHeader(archive, hdr);
	}
	return true;
}

} // namespace

static _BLOCKENTRY *mpqapi_new_block(int *block_index)
{
	_BLOCKENTRY *pBlock;
	DWORD i;

	pBlock = cur_archive.sgpBlockTbl;
	for (i = 0; i < MPQ_BLOCK_COUNT; i++, pBlock++) {
		if ((pBlock->offset | pBlock->sizealloc | pBlock->flags | pBlock->sizefile) == 0) {
			if (block_index != NULL)
				*block_index = i;
			return pBlock;
		}
	}

	app_fatal("Out of free block entries");
	return NULL;
}

static void mpqapi_alloc_block(uint32_t block_offset, uint32_t block_size)
{
	_BLOCKENTRY *pBlock;
	DWORD i;

	pBlock = cur_archive.sgpBlockTbl;
	for (i = MPQ_BLOCK_COUNT; i != 0; i--, pBlock++) {
		if (pBlock->offset != 0 && pBlock->flags == 0 && pBlock->sizefile == 0) {
			if (pBlock->offset + pBlock->sizealloc == block_offset) {
				block_offset = pBlock->offset;
				block_size += pBlock->sizealloc;
				memset(pBlock, 0, sizeof(*pBlock));
				mpqapi_alloc_block(block_offset, block_size);
				return;
			}
			if (block_offset + block_size == pBlock->offset) {
				block_size += pBlock->sizealloc;
				memset(pBlock, 0, sizeof(*pBlock));
				mpqapi_alloc_block(block_offset, block_size);
				return;
			}
		}
	}
	if (block_offset + block_size > cur_archive.size) {
		app_fatal("MPQ free list error");
	}
	if (block_offset + block_size == cur_archive.size) {
		cur_archive.size = block_offset;
	} else {
		pBlock = mpqapi_new_block(NULL);
		pBlock->offset = block_offset;
		pBlock->sizealloc = block_size;
		pBlock->sizefile = 0;
		pBlock->flags = 0;
	}
}

static DWORD mpqapi_find_free_block(uint32_t size, uint32_t *block_size)
{
	_BLOCKENTRY *pBlock;
	DWORD i, result;

	pBlock = cur_archive.sgpBlockTbl;
	for (i = MPQ_BLOCK_COUNT; i != 0; i--, pBlock++) {
		result = pBlock->offset;
		if (result != 0 && pBlock->flags == 0
		 && pBlock->sizefile == 0 && pBlock->sizealloc >= size) {
			*block_size = size;
			pBlock->offset += size;
			pBlock->sizealloc -= size;

			if (pBlock->sizealloc == 0)
				memset(pBlock, 0, sizeof(*pBlock));

			return result;
		}
	}

	*block_size = size;
	result = cur_archive.size;
	cur_archive.size += size;
	return result;
}

static int mpqapi_get_hash_index(short index, int hash_a, int hash_b, int locale)
{
	DWORD idx, i;
	_HASHENTRY *pHash;

	idx = index;
	for (i = MPQ_HASH_COUNT; i != 0; i--, idx++) {
		idx &= MPQ_HASH_COUNT - 1;
		pHash = &cur_archive.sgpHashTbl[idx];
		if (pHash->block == -1)
			break;
		if (pHash->hashcheck[0] == hash_a && pHash->hashcheck[1] == hash_b
		    && pHash->lcid == locale && pHash->block != -2)
			return idx;
	}

	return -1;
}

static int FetchHandle(const char *pszName)
{
	return mpqapi_get_hash_index(Hash(pszName, 0), Hash(pszName, 1), Hash(pszName, 2), 0);
}

void mpqapi_remove_hash_entry(const char *pszName)
{
	_HASHENTRY *pHash;
	_BLOCKENTRY *pBlock;
	int hIdx, block_offset, block_size;

	hIdx = FetchHandle(pszName);
	if (hIdx != -1) {
		pHash = &cur_archive.sgpHashTbl[hIdx];
		pBlock = &cur_archive.sgpBlockTbl[pHash->block];
		pHash->block = -2;
		block_offset = pBlock->offset;
		block_size = pBlock->sizealloc;
		memset(pBlock, 0, sizeof(*pBlock));
		mpqapi_alloc_block(block_offset, block_size);
		cur_archive.modified = true;
	}
}

void mpqapi_remove_hash_entries(BOOL (*fnGetName)(DWORD, char (&)[MAX_PATH]))
{
	DWORD i;
	char pszFileName[MAX_PATH];

	for (i = 0; fnGetName(i, pszFileName); i++)
		mpqapi_remove_hash_entry(pszFileName);
}

static _BLOCKENTRY *mpqapi_add_file(const char *pszName, _BLOCKENTRY *pBlk, int block_index)
{
	_HASHENTRY *pHash;
	DWORD i, h1, h2, h3;

	h1 = Hash(pszName, 0);
	h2 = Hash(pszName, 1);
	h3 = Hash(pszName, 2);
	if (mpqapi_get_hash_index(h1, h2, h3, 0) != -1)
		app_fatal("Hash collision between \"%s\" and existing file\n", pszName);

	for (i = MPQ_HASH_COUNT; i != 0; i--, h1++) {
		h1 &= MPQ_BLOCK_COUNT - 1;
		pHash = &cur_archive.sgpHashTbl[h1];
		if (pHash->block == -1 || pHash->block == -2) {
			if (pBlk == NULL)
				pBlk = mpqapi_new_block(&block_index);

			pHash->hashcheck[0] = h2;
			pHash->hashcheck[1] = h3;
			pHash->lcid = 0;
			pHash->block = block_index;
			return pBlk;
		}
	}

	app_fatal("Out of hash space");
	return NULL;
}

static BOOL mpqapi_write_file_contents(const char *pszName, const BYTE *pbData, DWORD dwLen, _BLOCKENTRY *pBlk)
{
	const char *str_ptr = pszName;
	const char *tmp;
	while ((tmp = strchr(str_ptr, ':')))
		str_ptr = tmp + 1;
	while ((tmp = strchr(str_ptr, '\\')))
		str_ptr = tmp + 1;
	Hash(str_ptr, 3);

	constexpr uint32_t kSectorSize = 4096;
	const uint32_t num_sectors = (dwLen + (kSectorSize - 1)) / kSectorSize;
	const uint32_t offset_table_bytesize = sizeof(uint32_t) * (num_sectors + 1);
	pBlk->offset = mpqapi_find_free_block(dwLen + offset_table_bytesize, &pBlk->sizealloc);
	pBlk->sizefile = dwLen;
	pBlk->flags = 0x80000100;

	// We populate the table of sector offset while we write the data.
	// We can't pre-populate it because we don't know the compressed sector sizes yet.
	// First offset is the start of the first sector, last offset is the end of the last sector.
	std::unique_ptr<uint32_t[]> sectoroffsettable(new uint32_t[num_sectors + 1]);

#ifdef CAN_SEEKP_BEYOND_EOF
	if (!cur_archive.stream.seekp(pBlk->offset + offset_table_bytesize, std::ios::beg))
		return FALSE;
#else
	// Ensure we do not seekp beyond EOF by filling the missing space.
	std::streampos stream_end;
	if (!cur_archive.stream.seekp(0, std::ios::end) || !cur_archive.stream.tellp(&stream_end))
		return FALSE;
	const std::uintmax_t cur_size = stream_end - cur_archive.stream_begin;
	if (cur_size < pBlk->offset + offset_table_bytesize) {
		if (cur_size < pBlk->offset) {
			std::unique_ptr<char[]> filler(new char[pBlk->offset - cur_size]);
			if (!cur_archive.stream.write(filler.get(), pBlk->offset - cur_size))
				return FALSE;
		}
		if (!cur_archive.stream.write(reinterpret_cast<const char *>(sectoroffsettable.get()), offset_table_bytesize))
			return FALSE;
	} else {
		if (!cur_archive.stream.seekp(pBlk->offset + offset_table_bytesize, std::ios::beg))
			return FALSE;
	}
#endif

	const BYTE *src = pbData;
	uint32_t destsize = offset_table_bytesize;
	BYTE mpq_buf[kSectorSize];
	std::size_t cur_sector = 0;
	while (true) {
		uint32_t len = std::min(dwLen, kSectorSize);
		memcpy(mpq_buf, src, len);
		src += len;
		len = PkwareCompress(mpq_buf, len);
		if (!cur_archive.stream.write((char *)mpq_buf, len))
			return FALSE;
		sectoroffsettable[cur_sector++] = SwapLE32(destsize);
		destsize += len; // compressed length
		if (dwLen > kSectorSize)
			dwLen -= kSectorSize;
		else
			break;
	}

	sectoroffsettable[num_sectors] = SwapLE32(destsize);
	if (!cur_archive.stream.seekp(pBlk->offset, std::ios::beg))
		return FALSE;
	if (!cur_archive.stream.write(reinterpret_cast<const char *>(sectoroffsettable.get()), offset_table_bytesize))
		return FALSE;
	if (!cur_archive.stream.seekp(destsize - offset_table_bytesize, std::ios::cur))
		return FALSE;

	if (destsize < pBlk->sizealloc) {
		const uint32_t block_size = pBlk->sizealloc - destsize;
		if (block_size >= 1024) {
			pBlk->sizealloc = destsize;
			mpqapi_alloc_block(pBlk->sizealloc + pBlk->offset, block_size);
		}
	}
	return TRUE;
}

BOOL mpqapi_write_file(const char *pszName, const BYTE *pbData, DWORD dwLen)
{
	_BLOCKENTRY *pBlock;

	cur_archive.modified = true;
	mpqapi_remove_hash_entry(pszName);
	pBlock = mpqapi_add_file(pszName, 0, 0);
	if (!mpqapi_write_file_contents(pszName, pbData, dwLen, pBlock)) {
		mpqapi_remove_hash_entry(pszName);
		return FALSE;
	}
	return TRUE;
}

void mpqapi_rename(char *pszOld, char *pszNew)
{
	int index, block;
	_HASHENTRY *pHash;
	_BLOCKENTRY *pBlock;

	index = FetchHandle(pszOld);
	if (index != -1) {
		pHash = &cur_archive.sgpHashTbl[index];
		block = pHash->block;
		pBlock = &cur_archive.sgpBlockTbl[block];
		pHash->block = -2;
		mpqapi_add_file(pszNew, pBlock, block);
		cur_archive.modified = true;
	}
}

BOOL mpqapi_has_file(const char *pszName)
{
	return FetchHandle(pszName) != -1;
}

BOOL OpenMPQ(const char *pszArchive, DWORD dwChar)
{
	DWORD key;
	_FILEHEADER fhdr;

	InitHash();

	if (!cur_archive.Open(pszArchive)) {
		return FALSE;
	}
	if (cur_archive.sgpBlockTbl == NULL || cur_archive.sgpHashTbl == NULL) {
		if (!cur_archive.exists) {
			InitDefaultMpqHeader(&cur_archive, &fhdr);
		} else if (!ReadMPQHeader(&cur_archive, &fhdr)) {
			goto on_error;
		}
		cur_archive.sgpBlockTbl = new _BLOCKENTRY[MPQ_BLOCK_SIZE / sizeof(_BLOCKENTRY)];
		std::memset(cur_archive.sgpBlockTbl, 0, MPQ_BLOCK_SIZE);
		if (fhdr.blockcount) {
			if (!cur_archive.stream.read(reinterpret_cast<char *>(cur_archive.sgpBlockTbl), MPQ_BLOCK_SIZE))
				goto on_error;
			key = Hash("(block table)", 3);
			Decrypt((DWORD *)cur_archive.sgpBlockTbl, MPQ_BLOCK_SIZE, key);
		}
		cur_archive.sgpHashTbl = new _HASHENTRY[MPQ_HASH_SIZE / sizeof(_HASHENTRY)];
		std::memset(cur_archive.sgpHashTbl, 255, MPQ_HASH_SIZE);
		if (fhdr.hashcount) {
			if (!cur_archive.stream.read(reinterpret_cast<char *>(cur_archive.sgpHashTbl), MPQ_HASH_SIZE))
				goto on_error;
			key = Hash("(hash table)", 3);
			Decrypt((DWORD *)cur_archive.sgpHashTbl, MPQ_HASH_SIZE, key);
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
	return TRUE;
on_error:
	cur_archive.Close(/*clear_tables=*/true);
	return FALSE;
}

BOOL mpqapi_flush_and_close(const char *pszArchive, BOOL bFree, DWORD dwChar)
{
	return cur_archive.Close(/*clear_tables=*/bFree);
}

DEVILUTION_END_NAMESPACE
