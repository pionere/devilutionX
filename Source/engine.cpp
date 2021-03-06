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

#ifdef _DEBUG
/** Number of times the current seed has been fetched */
int SeedCount;
#endif
/** Current game seed */
Sint32 sglGameSeed;
static CCritSect sgMemCrit;

/**
 * Specifies the increment used in the Borland C/C++ pseudo-random.
 */
const Uint32 RndInc = 1;

/**
 * Specifies the multiplier used in the Borland C/C++ pseudo-random number generator algorithm.
 */
const Uint32 RndMult = 0x015A4E35;

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
	/*int mx, my;
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

	return md;*/
	// The implementation of above with fewer branches
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
}

/**
 * @brief Set the RNG seed
 * @param s RNG seed
 */
void SetRndSeed(int32_t s)
{
#ifdef _DEBUG
	SeedCount = 0;
#endif
	sglGameSeed = s;
}

/**
 * @brief Get the current RNG seed
 * @return RNG seed
 */
int32_t GetRndSeed()
{
#ifdef _DEBUG
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
	if (v < 0xFFFF)
		return (GetRndSeed() >> 16) % v;
	return GetRndSeed() % v;
}

/**
 * @brief Multithreaded safe malloc
 * @param dwBytes Byte size to allocate
 */
BYTE *DiabloAllocPtr(size_t dwBytes)
{
	BYTE *buf;

	sgMemCrit.Enter();
	buf = (BYTE *)SMemAlloc(dwBytes);
	sgMemCrit.Leave();

	if (buf == NULL) {
		app_fatal("System memory exhausted.\n"
		             "Make sure you have at least 64MB of free system memory before running the game");
	}

	return buf;
}

/**
 * @brief Multithreaded safe memfree
 * @param p Memory pointer to free
 */
void mem_free_dbg(void *p)
{
	if (p != NULL) {
		sgMemCrit.Enter();
		SMemFree(p);
		sgMemCrit.Leave();
	}
}

/**
 * @brief Load a file in to a buffer
 * @param pszName Path of file
 * @param pdwFileLen Will be set to file size if non-NULL
 * @return Buffer with content of file
 */
BYTE *LoadFileInMem(const char *pszName, size_t *pdwFileLen)
{
	HANDLE file;
	BYTE *buf;
	size_t fileLen;

	SFileOpenFile(pszName, &file);
	fileLen = SFileGetFileSize(file);

	if (pdwFileLen != NULL)
		*pdwFileLen = fileLen;

	if (fileLen == 0)
		app_fatal("Zero length SFILE:\n%s", pszName);

	buf = (BYTE *)DiabloAllocPtr(fileLen);

	SFileReadFile(file, buf, fileLen, NULL);
	SFileCloseFile(file);

	return buf;
}

/**
 * @brief Load a file in to the given buffer
 * @param pszName Path of file
 * @param p Target buffer
 */
void LoadFileWithMem(const char *pszName, BYTE *p)
{
	DWORD dwFileLen;
	HANDLE hsFile;

	assert(pszName != NULL);
	if (p == NULL) {
		app_fatal("LoadFileWithMem(NULL):\n%s", pszName);
	}

	SFileOpenFile(pszName, &hsFile);

	dwFileLen = SFileGetFileSize(hsFile);
	if (dwFileLen == 0) {
		app_fatal("Zero length SFILE:\n%s", pszName);
	}

	SFileReadFile(hsFile, p, dwFileLen, NULL);
	SFileCloseFile(hsFile);
}

BYTE* CelMerge(BYTE* celA, size_t nDataSizeA, BYTE* celB, size_t nDataSizeB)
{
	size_t nDataSize;
	DWORD i, nCelA, nCelB, cData, nData;
	BYTE *cel, *pBuf;
	DWORD *pHead;

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
 * Draws a half-transparent rectangle by blacking out odd pixels on odd lines,
 * even pixels on even lines.
 * @brief Render a transparent black rectangle
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param width Rectangle width
 * @param height Rectangle height
 */
void trans_rect(int sx, int sy, int width, int height)
{
	int row, col;
	BYTE *pix = &gpBuffer[sx + BUFFER_WIDTH * sy];
	for (row = 0; row < height; row++) {
		for (col = 0; col < width; col++) {
			if (((row ^ col) & 1) == 0)
				*pix = 0;
			pix++;
		}
		pix += BUFFER_WIDTH - width;
	}
}

/**
 * @brief Fade to black and play a video
 * @param pszMovie file path of movie
 */
void PlayInGameMovie(const char *pszMovie)
{
	PaletteFadeOut();
	play_movie(pszMovie, 0);
	ClearScreenBuffer();
	gbRedrawFlags = REDRAW_ALL;
	scrollrt_draw_game_screen(true);
	PaletteFadeIn();
	gbRedrawFlags = REDRAW_ALL;
}

DEVILUTION_END_NAMESPACE
