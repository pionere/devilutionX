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

/* Calculate direction (DIR_) from (x1;y1) to (x2;y2) */
int GetDirection(int x1, int y1, int x2, int y2);
/* Set the current RNG seed */
void SetRndSeed(int32_t s);
/* Retrieve the current RNG seed */
int32_t GetRndSeed();
/* Get the next RNG seed */
int32_t NextRndSeed();
/* Retrieve the next pseudo-random number in the range of 0 <= x < v, where v is a non-negative (32bit) integer. The result is zero if v is negative. */
int random_(BYTE idx, int v);
/* Retrieve the next pseudo-random number in the range of 0 <= x < v, where v is a positive integer and less than or equal to 0x7FFF. */
int random_low(BYTE idx, int v);
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

void PlayInGameMovie(const char* pszMovie);
/* Retrieve the next pseudo-random number in the range of minVal <= x <= maxVal, where (maxVal - minVal) is a non-negative (32bit) integer. The result is minVal if (maxVal - minVal) is negative. */
inline int RandRange(int minVal, int maxVal)
{
	return minVal + random_(0, maxVal - minVal + 1);
}
/* Retrieve the next pseudo-random number in the range of minVal <= x <= maxVal, where (maxVal - minVal) is a non-negative integer and less than 0x7FFF. */
inline int RandRangeLow(int minVal, int maxVal)
{
	return minVal + random_low(0, maxVal - minVal + 1);
}

#if defined(_MSC_VER)
#define DIAG_PRAGMA(x)                                            __pragma(warning(x))
#define DISABLE_WARNING(gcc_unused, clang_unused, msvc_errorcode) DIAG_PRAGMA(push) DIAG_PRAGMA(disable:##msvc_errorcode)
#define ENABLE_WARNING(gcc_unused, clang_unused, msvc_errorcode)  DIAG_PRAGMA(pop)
//#define DISABLE_WARNING(gcc_unused,clang_unused,msvc_errorcode) __pragma(warning(suppress: msvc_errorcode))
//#define ENABLE_WARNING(gcc_unused,clang_unused,msvc_unused) ((void)0)
#else
#define DIAG_STR(s)              #s
#define DIAG_JOINSTR(x, y)       DIAG_STR(x ## y)
#define DO_DIAG_PRAGMA(x)        _Pragma(#x)
#define DIAG_PRAGMA(compiler, x) DO_DIAG_PRAGMA(compiler diagnostic x)
#if defined(__clang__)
# define DISABLE_WARNING(gcc_unused, clang_option, msvc_unused) DIAG_PRAGMA(clang, push) DIAG_PRAGMA(clang, ignored DIAG_JOINSTR(-W, clang_option))
# define ENABLE_WARNING(gcc_unused, clang_option, msvc_unused)  DIAG_PRAGMA(clang, pop)
#elif defined(__GNUC__)
#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 406
# define DISABLE_WARNING(gcc_option, clang_unused, msvc_unused) DIAG_PRAGMA(GCC, push) DIAG_PRAGMA(GCC, ignored DIAG_JOINSTR(-W, gcc_option))
# define ENABLE_WARNING(gcc_option, clang_unused, msvc_unused)  DIAG_PRAGMA(GCC, pop)
#else
# define DISABLE_WARNING(gcc_option, clang_unused, msvc_unused) DIAG_PRAGMA(GCC, ignored DIAG_JOINSTR(-W, gcc_option))
# define ENABLE_WARNING(gcc_option, clang_option, msvc_unused)  DIAG_PRAGMA(GCC, warning DIAG_JOINSTR(-W, gcc_option))
#endif
#else
#define DISABLE_WARNING(gcc_unused, clang_unused, msvc_unused) ;
#define ENABLE_WARNING(gcc_unused, clang_unused, msvc_unused)  ;
#endif
#endif
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
