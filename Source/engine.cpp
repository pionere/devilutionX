/**
 * @file engine.cpp
 *
 * Implementation of basic engine helper functions:
 * - Sprite blitting
 * - Drawing
 * - Angle calculation
 * - RNG
 * - Memory allocation
 * - File loading
 * - Video playback
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#if DEBUG_MODE
/** Number of times the current seed has been fetched */
int SeedCount;
#endif
/** Current game seed */
int32_t sglGameSeed;
#if __cplusplus <= 199711L
static CCritSect sgMemCrit;
#endif

/**
 * Specifies the increment used in the Borland C/C++ pseudo-random.
 */
static const uint32_t RndInc = 1;

/**
 * Specifies the multiplier used in the Borland C/C++ pseudo-random number generator algorithm.
 */
static const uint32_t RndMult = 0x015A4E35;

/**
 * @brief Calculate the best fit direction between two points
 * @param x1 Tile coordinate
 * @param y1 Tile coordinate
 * @param x2 Tile coordinate
 * @param y2 Tile coordinate
 * @return A value from the direction enum
 */
int GetDirection(int x1, int y1, int x2, int y2)
{
#if UNOPTIMIZED_DIRECTION
	int mx, my;
	int md;

	mx = x2 - x1;
	my = y2 - y1;

	if (mx >= 0) {
		if (my >= 0) {
			md = DIR_S;
			if (2 * mx < my)
				return DIR_SW;
		} else {
			my = -my;
			md = DIR_E;
			if (2 * mx < my)
				return DIR_NE;
		}
		if (2 * my < mx)
			md = DIR_SE;
	} else {
		mx = -mx;
		if (my >= 0) {
			md = DIR_W;
			if (2 * mx < my)
				return DIR_SW;
		} else {
			my = -my;
			md = DIR_N;
			if (2 * mx < my)
				return DIR_NE;
		}
		if (2 * my < mx)
			md = DIR_NW;
	}

	return md;
#else
	int dx = x2 - x1;
	int dy = y2 - y1;
	unsigned adx = abs(dx);
	unsigned ady = abs(dy);
	//                        SE  NE  SW  NW
	const int BaseDirs[4] = {  7,  5,  1,  3 };
	int dir = BaseDirs[2 * (dx < 0) + (dy < 0)];
	//const int DeltaDir[2][4] = {{0, 1, 2}, {2, 1, 0}};
	const int DeltaDirs[2][4] = {{1, 0, 2}, {1, 2, 0}};
	const int (&DeltaDir)[4] = DeltaDirs[(dx < 0) ^ (dy < 0)];
	//dir += DeltaDir[2 * adx < ady ? 2 : (2 * ady < adx ? 0 : 1)];
	dir += DeltaDir[2 * adx < ady ? 2 : (2 * ady < adx ? 1 : 0)];
	return dir & 7;
#endif
}

/**
 * @brief Set the RNG seed
 * @param s RNG seed
 */
void SetRndSeed(int32_t s)
{
#if DEBUG_MODE
	SeedCount = 0;
#endif
	sglGameSeed = s;
}

/**
 * @bried Return the current RNG seed
 * @return RNG seed
 */
int32_t GetRndSeed()
{
	return sglGameSeed;
}

/**
 * @brief Get the next RNG seed
 * @return RNG seed
 */
int32_t NextRndSeed()
{
#if DEBUG_MODE
	SeedCount++;
#endif
	sglGameSeed = RndMult * static_cast<uint32_t>(sglGameSeed) + RndInc;
	return abs(sglGameSeed);
}

/**
 * @brief Main RNG function
 * @param idx Unused
 * @param v The upper limit for the return value
 * @return A random number from 0 to (v-1)
 */
int random_(BYTE idx, int v)
{
	if (v <= 0)
		return 0;
	if (v < 0x7FFF)
		return (((unsigned)NextRndSeed()) >> 16) % v;
	return ((unsigned)NextRndSeed()) % v;
}

/**
 * @brief Same as random_ but assumes 0 < v < 0x7FFF
 * @param idx Unused
 * @param v The upper limit for the return value
 * @return A random number from 0 to (v-1)
 */
int random_low(BYTE idx, int v)
{
	// assert(v > 0);
	// assert(v < 0x7FFF);
	return (((unsigned)NextRndSeed()) >> 16) % v;
}

/**
 * @brief Multithreaded safe malloc
 * @param dwBytes Byte size to allocate
 */
BYTE* DiabloAllocPtr(size_t dwBytes)
{
	BYTE* buf;
#if __cplusplus <= 199711L
	sgMemCrit.Enter();
	buf = (BYTE*)malloc(dwBytes);
	sgMemCrit.Leave();
#else
	buf = (BYTE*)malloc(dwBytes);
#endif

	if (buf == NULL)
		app_fatal("Out of memory");

	return buf;
}

/**
 * @brief Multithreaded safe memfree
 * @param p Memory pointer to free
 */
void mem_free_dbg(void* p)
{
	if (p != NULL) {
#if __cplusplus <= 199711L
		sgMemCrit.Enter();
		free(p);
		sgMemCrit.Leave();
#else
		free(p);
#endif
	}
}

/**
 * @brief Load a file in to a buffer
 * @param pszName Path of file
 * @param pdwFileLen Will be set to file size if non-NULL
 * @return Buffer with content of file
 */
BYTE* LoadFileInMem(const char* pszName, size_t* pdwFileLen)
{
	HANDLE file;
	BYTE* buf;
	size_t fileLen;

	file = SFileOpenFile(pszName);
	fileLen = SFileGetFileSize(file);

	if (pdwFileLen != NULL)
		*pdwFileLen = fileLen;

	if (fileLen == 0)
		app_fatal("Zero length SFILE:\n%s", pszName);

	buf = (BYTE*)DiabloAllocPtr(fileLen);

	SFileReadFile(file, buf, fileLen);
	SFileCloseFile(file);

	return buf;
}

/**
 * @brief Load a file in to the given buffer
 * @param pszName Path of file
 * @param p Target buffer
 */
void LoadFileWithMem(const char* pszName, BYTE* p)
{
	DWORD dwFileLen;
	HANDLE hsFile;

	assert(pszName != NULL);
	if (p == NULL) {
		app_fatal("LoadFileWithMem(NULL):\n%s", pszName);
	}

	hsFile = SFileOpenFile(pszName);

	dwFileLen = SFileGetFileSize(hsFile);
	if (dwFileLen == 0) {
		app_fatal("Zero length SFILE:\n%s", pszName);
	}

	SFileReadFile(hsFile, p, dwFileLen);
	SFileCloseFile(hsFile);
}

#define LOAD_LE32(b) (SwapLE32(*((DWORD*)(b))))
BYTE* CelMerge(BYTE* celA, size_t nDataSizeA, BYTE* celB, size_t nDataSizeB)
{
	size_t nDataSize;
	DWORD i, nCelA, nCelB, cData, nData;
	BYTE *cel, *pBuf;
	DWORD* pHead;

	nDataSize = nDataSizeA + nDataSizeB - 4 * 2;
	cel = DiabloAllocPtr(nDataSize);
	*(DWORD*)cel = 0;
	pBuf = cel;
	nCelA = LOAD_LE32(celA);
	nCelB = LOAD_LE32(celB);
	pHead = (DWORD*)pBuf;
	pHead++;
	pBuf += 4 * (nCelA + nCelB + 2);

	nData = LOAD_LE32(celA + 4);
	for (i = 1; i <= nCelA; i++) {
		cData = nData;
		nData = LOAD_LE32(celA + 4 * (i + 1));
		*pHead = SwapLE32(pBuf - cel);
		memcpy(pBuf, &celA[cData], nData - cData);
		pBuf += nData - cData;
		++*cel;
		pHead++;
	}

	nData = LOAD_LE32(celB + 4);
	for (i = 1; i <= nCelB; i++) {
		cData = nData;
		nData = LOAD_LE32(celB + 4 * (i + 1));
		*pHead = SwapLE32(pBuf - cel);
		memcpy(pBuf, &celB[cData], nData - cData);
		pBuf += nData - cData;
		++*cel;
		pHead++;
	}

	*pHead = SwapLE32(pBuf - cel);
	// assert(*pHead == nDataSize);
	return cel;
}

/**
 * @brief Fade to black and play a video
 * @param pszMovie file path of movie
 */
void PlayInGameMovie(const char* pszMovie)
{
	PaletteFadeOut();
	play_movie(pszMovie, 0);
	scrollrt_draw_game();
	PaletteFadeIn(false);
	gbRedrawFlags = REDRAW_ALL;
}

DEVILUTION_END_NAMESPACE
