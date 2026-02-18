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
 * @brief Calculate the best fit direction between two points
 * @param x1 Tile coordinate
 * @param y1 Tile coordinate
 * @param x2 Tile coordinate
 * @param y2 Tile coordinate
 * @return A value from the direction enum
 */
int GetDirection(int x1, int y1, int x2, int y2)
{
#if 0
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
	return dir & 7;
#endif
#endif
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
			assert(*textFile != '\0');
			textFile++;
		}
		*textFile = '\0';
		textFile++;
	}

	return textLines;
}

CCritSect::CCritSect()
{
	m_critsect = SDL_CreateMutex();
	if (m_critsect == NULL) {
		sdl_error(ERR_SDL_MUTEX_CREATE);
	}
}
CCritSect::~CCritSect()
{
	SDL_DestroyMutex(m_critsect);
}
void CCritSect::Enter()
{
	if (SDL_LockMutex(m_critsect) < 0) {
		sdl_error(ERR_SDL_MUTEX_LOCK);
	}
}
void CCritSect::Leave()
{
	if (SDL_UnlockMutex(m_critsect) < 0) {
		sdl_error(ERR_SDL_MUTEX_UNLOCK);
	}
}

DEVILUTION_END_NAMESPACE
