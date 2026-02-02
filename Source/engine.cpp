/**
 * @file engine.cpp
 *
 * Implementation of basic engine helper functions:
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
 * @brief Returns the direction a vector from p1(x1, y1) to p2(x2, y2) is pointing to.
 *
 *      W    SW     S
 *            ^
 *            |	
 *     NW ----+---> SE
 *            |
 *            |
 *      N    NE     E
 *
 * @param x1 the x coordinate of p1
 * @param y1 the y coordinate of p1
 * @param x2 the x coordinate of p2
 * @param y2 the y coordinate of p2
 * @return the direction of the p1->p2 vector
*/
int GetDirection(int x1, int y1, int x2, int y2)
{
#if SIMPLE_DIRECTION
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
	//const int DeltaDir[2][4] = { { 0, 1, 2 }, { 2, 1, 0 } };
	const int DeltaDirs[2][4] = { { 1, 0, 2 }, { 1, 2, 0 } };
	const int(&DeltaDir)[4] = DeltaDirs[(dx < 0) ^ (dy < 0)];
	//dir += DeltaDir[2 * adx < ady ? 2 : (2 * ady < adx ? 0 : 1)];
	dir += DeltaDir[2 * adx < ady ? 2 : (2 * ady < adx ? 1 : 0)];
	return dir & 7;
#endif
#else
#if UNOPTIMIZED_DIRECTION
	int mx, my, md;

	mx = x2 - x1;
	my = y2 - y1;
	if (mx >= 0) {
		if (my >= 0) {
			if (5 * mx <= (my << 1)) // mx/my <= 0.4, approximation of tan(22.5)
				return 1;            // DIR_SW
			md = 0;                  // DIR_S
		} else {
			my = -my;
			if (5 * mx <= (my << 1))
				return 5; // DIR_NE
			md = 6;       // DIR_E
		}
		if (5 * my <= (mx << 1)) // my/mx <= 0.4
			md = 7;              // DIR_SE
	} else {
		mx = -mx;
		if (my >= 0) {
			if (5 * mx <= (my << 1))
				return 1; // DIR_SW
			md = 2;       // DIR_W
		} else {
			my = -my;
			if (5 * mx <= (my << 1))
				return 5; // DIR_NE
			md = 4;       // DIR_N
		}
		if (5 * my <= (mx << 1))
			md = 3; // DIR_NW
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
	//const int DeltaDirs[2][4] = { {0, 1, 2}, {2, 1, 0} };
	const int DeltaDirs[2][4] = { { 1, 0, 2 }, { 1, 2, 0 } };
	const int(&DeltaDir)[4] = DeltaDirs[(dx < 0) ^ (dy < 0)];
	//dir += DeltaDir[5 * adx <= (ady << 1) ? 2 : (5 * ady <= (adx << 1) ? 0 : 1)];
	dir += DeltaDir[5 * adx <= (ady << 1) ? 2 : (5 * ady <= (adx << 1) ? 1 : 0)];
	return dir & 7; // % NUM_DIRS
#endif
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
	return sglGameSeed;
}

static unsigned NextRndValue()
{
	return abs(NextRndSeed());
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
		return (NextRndValue() >> 16) % v;
	return NextRndValue() % v;
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
	return (NextRndValue() >> 16) % v;
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
 * @brief Load an asset in to a buffer
 * @param name path/name of the asset
 * @param pdwFileLen Will be set to the size of the asset if non-NULL
 * @return Buffer with content of the asset
 */
BYTE* LoadFileInMem(const char* name, size_t* pdwFileLen)
{
	BYTE* buf = NULL;
	DWORD fileLen = SFileReadFileEx(name, &buf);
	if (pdwFileLen != NULL)
		*pdwFileLen = fileLen;
	return buf;
}

/**
 * @brief Load an asset in to the given buffer
 * @param name path/name of the asset
 * @param p Target buffer
 */
void LoadFileWithMem(const char* name, BYTE* p)
{
	// assert(name != NULL);
	// assert(p != NULL);
	SFileReadFileEx(name, &p);
}

/*
 * @brief Load a text-asset with line-breaks
 * @param name path/name of the asset
 * @param lines number of lines in the text-asset
 * @return address of the content in memory
 */
char** LoadTxtFile(const char* name, int lines)
{
	BYTE* textFile = NULL;
	SFileReadFileEx(name, &textFile);
	char** textLines = (char**)DiabloAllocPtr(sizeof(char*) * lines);

	for (int i = 0; i < lines; i++) {
		textLines[i] = (char*)textFile;

		while (*textFile != '\n') {
			// assert(*textFile != '\0');
			textFile++;
		}
		*textFile = '\0';
		textFile++;
	}
	// assert(*textFile == '\0');
	return textLines;
}

#define LOAD_LE32(b) (SwapLE32(*((DWORD*)(b))))
/*
 * @brief Merge two .CEL assets into a new one
 * @param celA the first asset to merge
 * @param nDataSizeA the size of the first asset
 * @param celA the second asset to merge
 * @param nDataSizeA the size of the second asset
 * @return the merged asset
 */
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
		*pHead = SwapLE32((DWORD)((size_t)pBuf - (size_t)cel));
		memcpy(pBuf, &celA[cData], nData - cData);
		pBuf += nData - cData;
		++*cel;
		pHead++;
	}

	nData = LOAD_LE32(celB + 4);
	for (i = 1; i <= nCelB; i++) {
		cData = nData;
		nData = LOAD_LE32(celB + 4 * (i + 1));
		*pHead = SwapLE32((DWORD)((size_t)pBuf - (size_t)cel));
		memcpy(pBuf, &celB[cData], nData - cData);
		pBuf += nData - cData;
		++*cel;
		pHead++;
	}

	*pHead = SwapLE32((DWORD)((size_t)pBuf - (size_t)cel));
	// assert(*pHead == nDataSize);
	return cel;
}

/**
 * @brief Fade to black and play a video
 * @param pszMovie file path of movie
 */
void PlayInGameMovie(const char* pszMovie)
{
	// Uint32 currTc = SDL_GetTicks();

	PaletteFadeOut();
	play_movie(pszMovie, 0);
	scrollrt_render_game();
	PaletteFadeIn(false);
	// gbRedrawFlags |= REDRAW_DRAW_ALL;
	// skip time due to movie and fadein/out
	extern Uint32 guNextTick;
	guNextTick = SDL_GetTicks() + gnTickDelay; // += SDL_GetTicks() - currTc;
}

DEVILUTION_END_NAMESPACE
