/**
 * @file engine.h
 *
 *  of basic engine helper functions:
 * - Sprite blitting
 * - Drawing
 * - Angle calculation
 * - RNG
 * - Memory allocation
 * - File loading
 * - Video playback
 */
#ifndef __ENGINE_H__
#define __ENGINE_H__

//#include "appfat.h"

DEVILUTION_BEGIN_NAMESPACE

#define CEL_FRAME_HEADER_SIZE 0x0A
#define CEL_BLOCK_HEIGHT      32

inline const BYTE* CelGetFrameStart(const BYTE* pCelBuff, int nCel)
{
	const uint32_t* pFrameTable;

	pFrameTable = (const uint32_t*)pCelBuff;

	return &pCelBuff[SwapLE32(pFrameTable[nCel])];
}

inline const BYTE* CelGetFrame(const BYTE* pCelBuff, int nCel, int* nDataSize)
{
	const uint32_t* pFrameTable;
	uint32_t nCellStart;

	pFrameTable = (const uint32_t*)&pCelBuff[nCel * 4];
	nCellStart = SwapLE32(pFrameTable[0]);
	*nDataSize = SwapLE32(pFrameTable[1]) - nCellStart;
	return &pCelBuff[nCellStart];
}

inline const BYTE* CelGetFrameClipped(const BYTE* pCelBuff, int nCel, int* nDataSize)
{
	const uint16_t* pFrameTable;
	uint16_t nDataStart;
	const BYTE* pRLEBytes = CelGetFrame(pCelBuff, nCel, nDataSize);

	pFrameTable = (const uint16_t*)&pRLEBytes[0];
	nDataStart = SwapLE16(pFrameTable[0]);
	*nDataSize -= nDataStart;

	return &pRLEBytes[nDataStart];
}

inline const BYTE* CelGetFrameClippedAt(const BYTE* pCelBuff, int nCel, int block, int* nDataSize)
{
	const uint16_t* pFrameTable;
	uint16_t nDataStart;
	const BYTE* pRLEBytes = CelGetFrame(pCelBuff, nCel, nDataSize);

	pFrameTable = (const uint16_t*)&pRLEBytes[0];
	nDataStart = SwapLE16(pFrameTable[block]);
	// assert(nDataStart != 0);
	*nDataSize -= nDataStart;

	return &pRLEBytes[nDataStart];
}

BYTE* DiabloAllocPtr(size_t dwBytes);
void mem_free_dbg(void* p);
#define MemFreeDbg(p)       \
	{                       \
		void* p__p;         \
		p__p = p;           \
		p = NULL;           \
		mem_free_dbg(p__p); \
	}
#define MemFreeTxtFile(p)      \
	{                          \
		char** p__p;           \
		p__p = p;              \
		p = NULL;              \
		mem_free_dbg(p__p[0]); \
		mem_free_dbg(p__p);    \
	}
BYTE* LoadFileInMem(const char* pszName, size_t* pdwFileLen = NULL);
void LoadFileWithMem(const char* pszName, BYTE* p);
char** LoadTxtFile(const char* name, int lines);

/* Load .CEL file and overwrite the first (unused) uint32_t with nWidth */
inline CelImageBuf* CelLoadImage(const char* name, uint32_t nWidth)
{
	CelImageBuf* res;

	res = (CelImageBuf*)LoadFileInMem(name);
#if DEBUG_MODE
	res->ciFrameCnt = SwapLE32(*((uint32_t*)res));
#endif
	res->ciWidth = nWidth;
	return res;
}

BYTE* CelMerge(BYTE* celA, size_t nDataSizeA, BYTE* celB, size_t nDataSizeB);

/*
 * Copy string from src to dest.
 * The NULL terminated content of src is copied to dest.
 */
template <size_t N1, size_t N2>
inline void copy_str(char (&dest)[N1], char (&src)[N2])
{
	static_assert(N1 >= N2, "String does not fit the destination.");
	DISABLE_WARNING(deprecated-declarations, deprecated-declarations, 4996)
	strcpy(dest, src);
	ENABLE_WARNING(deprecated-declarations, deprecated-declarations, 4996)
}

/*
 * Copy constant string from src to dest.
 * The whole (padded) length of the src array is copied.
 */
template <size_t N1, size_t N2>
inline void copy_cstr(char (&dest)[N1], const char (&src)[N2])
{
	static_assert(N1 >= N2, "String does not fit the destination.");
	constexpr size_t src_len = ((N2 + sizeof(int) - 1) / sizeof(int)) * sizeof(int);
	constexpr size_t len = N1 >= src_len ? src_len : N1;
	memcpy(dest, src, len);
}

/*
 * Copy POD (Plain-Old-Data) from source to dest.
 */
template <class T>
inline void copy_pod(T& dest, T& src)
{
	memcpy(&dest, &src, sizeof(T));
}

/*
 * Copy POD (Plain-Old-Data) from constant source to dest.
 */
template <class T>
inline void copy_pod(T& dest, const T& src)
{
	memcpy(&dest, &src, sizeof(T));
}

/*
 * Concatenate strings using the provided format with the help of pos as a cursor.
 */
template <int N>
inline void cat_str(char (&dest)[N], int& pos, const char* fmt, ...)
{
	int n, res;
	va_list va;

	va_start(va, fmt);

	n = N - pos;
	res = vsnprintf(&dest[pos], n, fmt, va);
	pos += (res <= n - 1) ? res : n - 1;

	va_end(va);
}

/*
 * Concatenate constant string to the dest with the help of pos as a cursor.
 */
template <int N1, int N2>
inline void cat_cstr(char (&dest)[N1], int& pos, const char (&src)[N2])
{
	int n;
	n = N1 - pos;
	if (n >= N2) {
		memcpy(&dest[pos], src, N2);
		pos += N2 - 1;
	} else {
		n--;
		memcpy(&dest[pos], src, n);
		pos += n;
		dest[pos] = '\0';
	}
}

/*
 * Helper function to simplify 'sizeof(list) / sizeof(list[0])' expressions.
 */
template <class T, int N>
inline constexpr int lengthof(T (&array)[N])
{
	return N;
}

typedef struct CCritSect {
	SDL_mutex* m_critsect;

	CCritSect()
	{
		m_critsect = SDL_CreateMutex();
		if (m_critsect == NULL) {
			sdl_error(ERR_SDL_MUTEX_CREATE);
		}
	}
	~CCritSect()
	{
		SDL_DestroyMutex(m_critsect);
	}
	void Enter()
	{
		if (SDL_LockMutex(m_critsect) < 0) {
			sdl_error(ERR_SDL_MUTEX_LOCK);
		}
	}
	void Leave()
	{
		if (SDL_UnlockMutex(m_critsect) < 0) {
			sdl_error(ERR_SDL_MUTEX_UNLOCK);
		}
	}
} CCritSect;

DEVILUTION_END_NAMESPACE

#endif /* __ENGINE_H__ */
