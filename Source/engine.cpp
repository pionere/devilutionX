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
/** automap pixel color 8-bit (palette entry) */
char gbPixelCol;
/** flip - if y < x */
BOOL gbRotateMap;
/** valid - if x/y are in bounds */
BOOL gbNotInView;
/** Number of times the current seed has been fetched */
int SeedCount;
#endif
/** Seed value before the most recent call to SetRndSeed() */
int orgseed;
/** Current game seed */
int sglGameSeed;
static CCritSect sgMemCrit;

/**
 * Specifies the increment used in the Borland C/C++ pseudo-random.
 */
const int RndInc = 1;

/**
 * Specifies the multiplier used in the Borland C/C++ pseudo-random number generator algorithm.
 */
const int RndMult = 0x015A4E35;

/**
 * @brief Blit CEL sprite to the given buffer, checks for drawing outside the buffer
 * @param pDecodeTo The output buffer
 * @param pRLEBytes CEL pixel stream (run-length encoded)
 * @param nDataSize Size of CEL in bytes
 * @param nWidth Width of sprite
 */
static void CelBlitSafe(BYTE *pDecodeTo, BYTE *pRLEBytes, int nDataSize, int nWidth)
{
	int i, w;
	char width;
	BYTE *src, *dst;

	assert(pDecodeTo != NULL);
	assert(pRLEBytes != NULL);
	assert(gpBuffer != NULL);

	src = pRLEBytes;
	dst = pDecodeTo;
	w = nWidth;

	for (; src != &pRLEBytes[nDataSize]; dst -= BUFFER_WIDTH + w) {
		for (i = w; i != 0;) {
			width = *src++;
			if (width >= 0) {
				i -= width;
				if (dst < gpBufEnd && dst > gpBufStart) {
					memcpy(dst, src, width);
				}
				src += width;
				dst += width;
			} else {
				dst -= width;
				i += width;
			}
		}
	}
}

/**
 * @brief Blit CEL sprite, and apply lighting, to the given buffer, checks for drawing outside the buffer
 * @param pDecodeTo The output buffer
 * @param pRLEBytes CEL pixel stream (run-length encoded)
 * @param nDataSize Size of CEL in bytes
 * @param nWidth Width of sprite
 * @param tbl Palette translation table
 */
static void CelBlitLightSafe(BYTE *pDecodeTo, BYTE *pRLEBytes, int nDataSize, int nWidth, BYTE *tbl)
{
	int i, w;
	BYTE width;
	BYTE *src, *dst;

	assert(pDecodeTo != NULL);
	assert(pRLEBytes != NULL);
	assert(gpBuffer != NULL);

	src = pRLEBytes;
	dst = pDecodeTo;
	if (tbl == NULL)
		tbl = &pLightTbl[light_table_index * 256];
	w = nWidth;

	for (; src != &pRLEBytes[nDataSize]; dst -= BUFFER_WIDTH + w) {
		for (i = w; i;) {
			width = *src++;
			if (!(width & 0x80)) {
				i -= width;
				if (dst < gpBufEnd && dst > gpBufStart) {
					if (width & 1) {
						dst[0] = tbl[src[0]];
						src++;
						dst++;
					}
					width >>= 1;
					if (width & 1) {
						dst[0] = tbl[src[0]];
						dst[1] = tbl[src[1]];
						src += 2;
						dst += 2;
					}
					width >>= 1;
					for (; width; width--) {
						dst[0] = tbl[src[0]];
						dst[1] = tbl[src[1]];
						dst[2] = tbl[src[2]];
						dst[3] = tbl[src[3]];
						src += 4;
						dst += 4;
					}
				} else {
					src += width;
					dst += width;
				}
			} else {
				width = -(char)width;
				dst += width;
				i -= width;
			}
		}
	}
}

/**
 * @brief Blit a given CEL frame to the given buffer
 * @param pBuff Target buffer
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 * @param nWidth Width of sprite
 */
static void CelBlitFrame(BYTE *pBuff, BYTE *pCelBuff, int nCel, int nWidth)
{
	int nDataSize;
	BYTE *pRLEBytes;

	assert(pCelBuff != NULL);
	assert(pBuff != NULL);

	pRLEBytes = CelGetFrame(pCelBuff, nCel, &nDataSize);
	CelBlitSafe(pBuff, pRLEBytes, nDataSize, nWidth);
}

/**
 * @brief Blit CEL sprite to the back buffer at the given coordinates
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 * @param nWidth Width of sprite
 */
void CelDraw(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth)
{
	CelBlitFrame(&gpBuffer[sx + BUFFER_WIDTH * sy], pCelBuff, nCel, nWidth);
}

/**
 * @brief Same as CelDraw but with the option to skip parts of the top and bottom of the sprite
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 * @param nWidth Width of sprite
 */
void CelClippedDraw(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth)
{
	BYTE *pRLEBytes;
	int nDataSize;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);

	CelBlitSafe(
	    &gpBuffer[sx + BUFFER_WIDTH * sy],
	    pRLEBytes,
	    nDataSize,
	    nWidth);
}

/**
 * @brief Blit CEL sprite, and apply lighting, to the back buffer at the given coordinates
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 * @param nWidth Width of sprite
 * @param tbl Palette translation table
 */
void CelDrawLight(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth, BYTE *tbl)
{
	int nDataSize;
	BYTE *pDecodeTo, *pRLEBytes;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);

	pRLEBytes = CelGetFrame(pCelBuff, nCel, &nDataSize);
	pDecodeTo = &gpBuffer[sx + BUFFER_WIDTH * sy];

	if (light_table_index != 0 || tbl != NULL)
		CelBlitLightSafe(pDecodeTo, pRLEBytes, nDataSize, nWidth, tbl);
	else
		CelBlitSafe(pDecodeTo, pRLEBytes, nDataSize, nWidth);
}

/**
 * @brief Same as CelDrawLight but with the option to skip parts of the top and bottom of the sprite
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 * @param nWidth Width of sprite
 */
void CelClippedDrawLight(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth)
{
	int nDataSize;
	BYTE *pRLEBytes, *pDecodeTo;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);
	pDecodeTo = &gpBuffer[sx + BUFFER_WIDTH * sy];

	if (light_table_index != 0)
		CelBlitLightSafe(pDecodeTo, pRLEBytes, nDataSize, nWidth, NULL);
	else
		CelBlitSafe(pDecodeTo, pRLEBytes, nDataSize, nWidth);
}

static inline int lightidx(char light)
{
	int idx;

	idx = 4096;
#ifdef _DEBUG
	if (light4flag)
		idx = 1024;
#endif
	if (light == 2)
		idx += 256; // gray colors
	if (light >= 4)
		idx += (light - 1) << 8;
	return idx;
}

/**
 * @brief Blit CEL sprite, and apply lighting, to the back buffer at the given coordinates, translated to a red hue
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 * @param nWidth Width of sprite
 * @param light Light shade to use -- disabled for the moment
 */
void CelDrawLightRed(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth)
{
	int nDataSize, w;
	BYTE *pRLEBytes, *dst, *tbl, *end;
	char width;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);
	dst = &gpBuffer[sx + BUFFER_WIDTH * sy];

	//tbl = &pLightTbl[lightidx(light)];
	tbl = &pLightTbl[lightidx(0)];
	end = &pRLEBytes[nDataSize];

	for (; pRLEBytes != end; dst -= BUFFER_WIDTH + nWidth) {
		for (w = nWidth; w;) {
			width = *pRLEBytes++;
			if (width >= 0) {
				w -= width;
				while (width != 0) {
					*dst = tbl[*pRLEBytes];
					pRLEBytes++;
					dst++;
					width--;
				}
			} else {
				dst -= width;
				w += width;
			}
		}
	}
}

/**
 * @brief Same as CelClippedDraw but checks for drawing outside the buffer
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 * @param nWidth Width of sprite
 */
void CelClippedDrawSafe(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth)
{
	BYTE *pRLEBytes;
	int nDataSize;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);

	CelBlitSafe(
	    &gpBuffer[sx + BUFFER_WIDTH * sy],
	    pRLEBytes,
	    nDataSize,
	    nWidth);
}

/**
 * @brief Same as CelBlitLightSafe, with stippled transparancy applied
 * @param pDecodeTo The output buffer
 * @param pRLEBytes CEL pixel stream (run-length encoded)
 * @param nDataSize Size of CEL in bytes
 * @param nWidth Width of sprite
 */
static void CelBlitLightTransSafe(BYTE *pDecodeTo, BYTE *pRLEBytes, int nDataSize, int nWidth)
{
	int w;
	BOOL shift;
	BYTE *tbl;

	assert(pDecodeTo != NULL);
	assert(pRLEBytes != NULL);
	assert(gpBuffer != NULL);

	int i;
	BYTE width;
	BYTE *src, *dst;

	src = pRLEBytes;
	dst = pDecodeTo;
	tbl = &pLightTbl[light_table_index * 256];
	w = nWidth;
	shift = (BYTE)(size_t)dst & 1;

	for (; src != &pRLEBytes[nDataSize]; dst -= BUFFER_WIDTH + w, shift = (shift + 1) & 1) {
		for (i = w; i;) {
			width = *src++;
			if (!(width & 0x80)) {
				i -= width;
				if (dst < gpBufEnd && dst > gpBufStart) {
					if (((BYTE)(size_t)dst & 1) == shift) {
						if (!(width & 1)) {
							goto L_ODD;
						} else {
							src++;
							dst++;
						L_EVEN:
							width >>= 1;
							if (width & 1) {
								dst[0] = tbl[src[0]];
								src += 2;
								dst += 2;
							}
							width >>= 1;
							for (; width; width--) {
								dst[0] = tbl[src[0]];
								dst[2] = tbl[src[2]];
								src += 4;
								dst += 4;
							}
						}
					} else {
						if (!(width & 1)) {
							goto L_EVEN;
						} else {
							dst[0] = tbl[src[0]];
							src++;
							dst++;
						L_ODD:
							width >>= 1;
							if (width & 1) {
								dst[1] = tbl[src[1]];
								src += 2;
								dst += 2;
							}
							width >>= 1;
							for (; width; width--) {
								dst[1] = tbl[src[1]];
								dst[3] = tbl[src[3]];
								src += 4;
								dst += 4;
							}
						}
					}
				} else {
					src += width;
					dst += width;
				}
			} else {
				width = -(char)width;
				dst += width;
				i -= width;
			}
		}
	}
}

/**
 * @brief Same as CelBlitLightTransSafe
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 * @param nWidth Width of sprite
 */
void CelClippedDrawLightTrans(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth)
{
	int nDataSize;
	BYTE *pRLEBytes, *dst;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);
	dst = &gpBuffer[sx + BUFFER_WIDTH * sy];

	if (cel_transparency_active)
		CelBlitLightTransSafe(dst, pRLEBytes, nDataSize, nWidth);
	else if (light_table_index)
		CelBlitLightSafe(dst, pRLEBytes, nDataSize, nWidth, NULL);
	else
		CelBlitSafe(dst, pRLEBytes, nDataSize, nWidth);
}

/**
 * @brief Same as CelDrawLightRed but checks for drawing outside the buffer
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 * @param nWidth Width of cel
 * @param light Light shade to use -- disabled for the moment
 */
void CelDrawLightRedSafe(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth)
{
	int nDataSize, w;
	BYTE *pRLEBytes, *dst, *tbl, *end;
	BYTE width;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);
	dst = &gpBuffer[sx + BUFFER_WIDTH * sy];

	//tbl = &pLightTbl[lightidx(light)];
	tbl = &pLightTbl[lightidx(0)];

	end = &pRLEBytes[nDataSize];

	for (; pRLEBytes != end; dst -= BUFFER_WIDTH + nWidth) {
		for (w = nWidth; w;) {
			width = *pRLEBytes++;
			if (!(width & 0x80)) {
				w -= width;
				if (dst < gpBufEnd && dst > gpBufStart) {
					while (width) {
						*dst = tbl[*pRLEBytes];
						pRLEBytes++;
						dst++;
						width--;
					}
				} else {
					pRLEBytes += width;
					dst += width;
				}
			} else {
				width = -(char)width;
				dst += width;
				w -= width;
			}
		}
	}
}

/**
 * @brief Blit to a buffer at given coordinates
 * @param pBuff Target buffer
 * @param x Cordinate in pBuff buffer
 * @param y Cordinate in pBuff buffer
 * @param wdt Width of pBuff
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 * @param nWidth Width of cel
 */
static void CelBlitWidth(BYTE *pBuff, int x, int y, int wdt, BYTE *pCelBuff, int nCel, int nWidth)
{
	BYTE *pRLEBytes, *dst, *end;

	assert(pCelBuff != NULL);
	assert(pBuff != NULL);

	int i, nDataSize;
	BYTE width;

	pRLEBytes = CelGetFrame(pCelBuff, nCel, &nDataSize);
	end = &pRLEBytes[nDataSize];
	dst = &pBuff[y * wdt + x];

	for (; pRLEBytes != end; dst -= wdt + nWidth) {
		for (i = nWidth; i;) {
			width = *pRLEBytes++;
			if (!(width & 0x80)) {
				i -= width;
				memcpy(dst, pRLEBytes, width);
				dst += width;
				pRLEBytes += width;
			} else {
				width = -(char)width;
				dst += width;
				i -= width;
			}
		}
	}
}

/**
 * @brief Blit a solid colder shape one pixel larger then the given sprite shape, to the back buffer at the given coordianates
 * @param col Color index from current palette
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff CEL buffer
 * @param nCel CEL frame number
 * @param nWidth Width of sprite
 */
void CelDrawOutline(BYTE col, int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth)
{
	int nDataSize, w;
	BYTE *src, *dst, *end;
	BYTE width;

	assert(pCelBuff != NULL);
	assert(gpBuffer != NULL);

	src = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);
	end = &src[nDataSize];
	dst = &gpBuffer[sx + BUFFER_WIDTH * sy];

	for (; src != end; dst -= BUFFER_WIDTH + nWidth) {
		for (w = nWidth; w;) {
			width = *src++;
			if (!(width & 0x80)) {
				w -= width;
				if (dst < gpBufEnd && dst > gpBufStart) {
					if (dst >= gpBufEnd - BUFFER_WIDTH) {
						while (width != 0) {
							if (*src++) {
								dst[-BUFFER_WIDTH] = col;
								dst[-1] = col;
								dst[1] = col;
							}
							dst++;
							width--;
						}
					} else {
						while (width != 0) {
							if (*src++) {
								dst[-BUFFER_WIDTH] = col;
								dst[-1] = col;
								dst[1] = col;
								dst[BUFFER_WIDTH] = col;
							}
							dst++;
							width--;
						}
					}
				} else {
					src += width;
					dst += width;
				}
			} else {
				width = -(char)width;
				dst += width;
				w -= width;
			}
		}
	}
}

/**
 * @brief Set the value of a single pixel in the back buffer, checks bounds
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param col Color index from current palette
 */
void ENG_set_pixel(int sx, int sy, BYTE col)
{
	BYTE *dst;

	assert(gpBuffer != NULL);

	if (sy < 0 || sy >= SCREEN_HEIGHT + SCREEN_Y || sx < SCREEN_X || sx >= SCREEN_WIDTH + SCREEN_X)
		return;

	dst = &gpBuffer[sx + BUFFER_WIDTH * sy];

	if (dst < gpBufEnd && dst > gpBufStart)
		*dst = col;
}

#ifdef _DEBUG
/**
 * @brief Set the value of a single pixel in the back buffer to that of gbPixelCol, checks bounds
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 */
void engine_draw_pixel(int sx, int sy)
{
	BYTE *dst;

	assert(gpBuffer != NULL);

	if (gbRotateMap) {
		if (gbNotInView && (sx < 0 || sx >= SCREEN_HEIGHT + SCREEN_Y || sy < SCREEN_X || sy >= SCREEN_WIDTH + SCREEN_X))
			return;
		dst = &gpBuffer[sy + BUFFER_WIDTH * sx];
	} else {
		if (gbNotInView && (sy < 0 || sy >= SCREEN_HEIGHT + SCREEN_Y || sx < SCREEN_X || sx >= SCREEN_WIDTH + SCREEN_X))
			return;
		dst = &gpBuffer[sx + BUFFER_WIDTH * sy];
	}

	if (dst < gpBufEnd && dst > gpBufStart)
		*dst = gbPixelCol;
}
#endif

/**
 * @brief Draw a line on the back buffer
 * @param x0 Back buffer coordinate
 * @param y0 Back buffer coordinate
 * @param x1 Back buffer coordinate
 * @param y1 Back buffer coordinate
 * @param col Color index from current palette
 */
void DrawLine(int x0, int y0, int x1, int y1, BYTE col)
{
	int di, ip, dx, dy, ax, ay, steps;
	float df, fp;

	dx = x1 - x0;
	dy = y1 - y0;
	ax = abs(dx);
	ay = abs(dy);
	if (ax > ay) {
		steps = ax;
		di = dx / ax;
		df = dy / (float)steps;
		ip = x0;
		fp = (float)y0;
		for ( ; steps >= 0; steps--, ip += di, fp += df) {
			ENG_set_pixel(ip, (int)fp, col);
		}
	} else {
		steps = ay;
		di = dy / ay;
		df = dx / float(steps);
		fp = (float)x0;
		ip = y0;
		for ( ; steps >= 0; steps--, fp += df, ip += di) {
			ENG_set_pixel((int)fp, ip, col);
		}
	}
}

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
void SetRndSeed(int s)
{
#ifdef _DEBUG
	SeedCount = 0;
#endif
	sglGameSeed = s;
	orgseed = s;
}

/**
 * @brief Get the current RNG seed
 * @return RNG seed
 */
int GetRndSeed()
{
#ifdef _DEBUG
	SeedCount++;
#endif
	sglGameSeed = static_cast<unsigned int>(RndMult) * sglGameSeed + RndInc;
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
BYTE *DiabloAllocPtr(DWORD dwBytes)
{
	BYTE *buf;

	sgMemCrit.Enter();
	buf = (BYTE *)SMemAlloc(dwBytes);
	sgMemCrit.Leave();

	if (buf == NULL) {
		const char *text = "System memory exhausted.\n"
		             "Make sure you have at least 64MB of free system memory before running the game";
		ERR_DLG("Out of Memory Error", text);
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
BYTE *LoadFileInMem(const char *pszName, DWORD *pdwFileLen)
{
	HANDLE file;
	BYTE *buf;
	int fileLen;

	SFileOpenFile(pszName, &file);
	fileLen = SFileGetFileSize(file, NULL);

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
 * @return Size of file
 */
DWORD LoadFileWithMem(const char *pszName, BYTE *p)
{
	DWORD dwFileLen;
	HANDLE hsFile;

	assert(pszName != NULL);
	if (p == NULL) {
		app_fatal("LoadFileWithMem(NULL):\n%s", pszName);
	}

	SFileOpenFile(pszName, &hsFile);

	dwFileLen = SFileGetFileSize(hsFile, NULL);
	if (dwFileLen == 0) {
		app_fatal("Zero length SFILE:\n%s", pszName);
	}

	SFileReadFile(hsFile, p, dwFileLen, NULL);
	SFileCloseFile(hsFile);

	return dwFileLen;
}

/**
 * @brief Apply the color swaps to a CL2 sprite
 * @param p CL2 buffer
 * @param ttbl Palette translation table
 * @param nCel Frame number in CL2 file
 */
void Cl2ApplyTrans(BYTE *p, BYTE *ttbl, int nCel)
{
	int i, nDataSize;
	char width;
	BYTE *dst;

	assert(p != NULL);
	assert(ttbl != NULL);

	for (i = 1; i <= nCel; i++) {
		dst = CelGetFrame(p, i, &nDataSize) + 10;
		nDataSize -= 10;
		while (nDataSize != 0) {
			width = *dst++;
			nDataSize--;
			assert(nDataSize >= 0);
			if (width < 0) {
				width = -width;
				if (width > 65) {
					nDataSize--;
					assert(nDataSize >= 0);
					*dst = ttbl[*dst];
					dst++;
				} else {
					nDataSize -= width;
					assert(nDataSize >= 0);
					while (width--) {
						*dst = ttbl[*dst];
						dst++;
					}
				}
			}
		}
	}
}

/**
 * @brief Blit CL2 sprite to the given buffer
 * @param pDecodeTo The output buffer
 * @param pRLEBytes CL2 pixel stream (run-length encoded)
 * @param nDataSize Size of CL2 in bytes
 * @param nWidth Width of sprite
 */
static void Cl2BlitSafe(BYTE *pDecodeTo, BYTE *pRLEBytes, int nDataSize, int nWidth)
{
	int w;
	char width;
	BYTE fill;
	BYTE *src, *dst;

	src = pRLEBytes;
	dst = pDecodeTo;
	w = nWidth;

	while (nDataSize != 0) {
		width = *src++;
		nDataSize--;
		if (width < 0) {
			width = -width;
			if (width > 65) {
				width -= 65;
				nDataSize--;
				fill = *src++;
				if (dst < gpBufEnd && dst > gpBufStart) {
					w -= width;
					while (width != 0) {
						*dst = fill;
						dst++;
						width--;
					}
					if (w == 0) {
						w = nWidth;
						dst -= BUFFER_WIDTH + w;
					}
					continue;
				}
			} else {
				nDataSize -= width;
				if (dst < gpBufEnd && dst > gpBufStart) {
					w -= width;
					while (width != 0) {
						*dst = *src;
						src++;
						dst++;
						width--;
					}
					if (w == 0) {
						w = nWidth;
						dst -= BUFFER_WIDTH + w;
					}
					continue;
				} else {
					src += width;
				}
			}
		}
		while (width != 0) {
			if (width > w) {
				dst += w;
				width -= w;
				w = 0;
			} else {
				dst += width;
				w -= width;
				width = 0;
			}
			if (w == 0) {
				w = nWidth;
				dst -= BUFFER_WIDTH + w;
			}
		}
	}
}

/**
 * @brief Blit a solid colder shape one pixel larger then the given sprite shape, to the given buffer
 * @param pDecodeTo The output buffer
 * @param pRLEBytes CL2 pixel stream (run-length encoded)
 * @param nDataSize Size of CL2 in bytes
 * @param nWidth Width of sprite
 * @param col Color index from current palette
 */
static void Cl2BlitOutlineSafe(BYTE *pDecodeTo, BYTE *pRLEBytes, int nDataSize, int nWidth, BYTE col)
{
	int w;
	char width;
	BYTE *src, *dst;

	src = pRLEBytes;
	dst = pDecodeTo;
	w = nWidth;

	while (nDataSize != 0) {
		width = *src++;
		nDataSize--;
		if (width < 0) {
			width = -width;
			if (width > 65) {
				width -= 65;
				nDataSize--;
				if (*src++ && dst < gpBufEnd && dst > gpBufStart) {
					w -= width;
					dst[-1] = col;
					dst[width] = col;
					while (width != 0) {
						dst[-BUFFER_WIDTH] = col;
						dst[BUFFER_WIDTH] = col;
						dst++;
						width--;
					}
					if (w == 0) {
						w = nWidth;
						dst -= BUFFER_WIDTH + w;
					}
					continue;
				}
			} else {
				nDataSize -= width;
				if (dst < gpBufEnd && dst > gpBufStart) {
					w -= width;
					while (width != 0) {
						if (*src++) {
							dst[-1] = col;
							dst[1] = col;
							dst[-BUFFER_WIDTH] = col;
							// BUGFIX: only set `if (dst+BUFFER_WIDTH < gpBufEnd)`
							dst[BUFFER_WIDTH] = col;
						}
						dst++;
						width--;
					}
					if (w == 0) {
						w = nWidth;
						dst -= BUFFER_WIDTH + w;
					}
					continue;
				} else {
					src += width;
				}
			}
		}
		while (width != 0) {
			if (width > w) {
				dst += w;
				width -= w;
				w = 0;
			} else {
				dst += width;
				w -= width;
				width = 0;
			}
			if (w == 0) {
				w = nWidth;
				dst -= BUFFER_WIDTH + w;
			}
		}
	}
}

/**
 * @brief Blit CL2 sprite, and apply lighting, to the given buffer
 * @param pDecodeTo The output buffer
 * @param pRLEBytes CL2 pixel stream (run-length encoded)
 * @param nDataSize Size of CL2 in bytes
 * @param nWidth With of CL2 sprite
 * @param pTable Light color table
 */
static void Cl2BlitLightSafe(BYTE *pDecodeTo, BYTE *pRLEBytes, int nDataSize, int nWidth, BYTE *pTable)
{
	int w, spriteWidth;
	char width;
	BYTE fill;
	BYTE *src, *dst;

	src = pRLEBytes;
	dst = pDecodeTo;
	w = nWidth;
	spriteWidth = nWidth;

	while (nDataSize != 0) {
		width = *src++;
		nDataSize--;
		if (width < 0) {
			width = -width;
			if (width > 65) {
				width -= 65;
				nDataSize--;
				fill = pTable[*src++];
				if (dst < gpBufEnd && dst > gpBufStart) {
					w -= width;
					while (width != 0) {
						*dst = fill;
						dst++;
						width--;
					}
					if (w == 0) {
						w = spriteWidth;
						dst -= BUFFER_WIDTH + w;
					}
					continue;
				}
			} else {
				nDataSize -= width;
				if (dst < gpBufEnd && dst > gpBufStart) {
					w -= width;
					while (width != 0) {
						*dst = pTable[*src];
						src++;
						dst++;
						width--;
					}
					if (w == 0) {
						w = spriteWidth;
						dst -= BUFFER_WIDTH + w;
					}
					continue;
				} else {
					src += width;
				}
			}
		}
		while (width != 0) {
			if (width > w) {
				dst += w;
				width -= w;
				w = 0;
			} else {
				dst += width;
				w -= width;
				width = 0;
			}
			if (w == 0) {
				w = spriteWidth;
				dst -= BUFFER_WIDTH + w;
			}
		}
	}
}

/**
 * @brief Blit CL2 sprite, to the back buffer at the given coordianates
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff CL2 buffer
 * @param nCel CL2 frame number
 * @param nWidth Width of sprite
 */
void Cl2Draw(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth)
{
	BYTE *pRLEBytes;
	int nDataSize;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);
	assert(nCel > 0);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);

	Cl2BlitSafe(
	    &gpBuffer[sx + BUFFER_WIDTH * sy],
	    pRLEBytes,
	    nDataSize,
	    nWidth);
}

/**
 * @brief Blit a solid colder shape one pixel larger then the given sprite shape, to the back buffer at the given coordianates
 * @param col Color index from current palette
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff CL2 buffer
 * @param nCel CL2 frame number
 * @param nWidth Width of sprite
 */
void Cl2DrawOutline(BYTE col, int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth)
{
	int nDataSize;
	BYTE *pRLEBytes;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);
	assert(nCel > 0);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);

	gpBufEnd -= BUFFER_WIDTH;
	Cl2BlitOutlineSafe(
	    &gpBuffer[sx + BUFFER_WIDTH * sy],
	    pRLEBytes,
	    nDataSize,
	    nWidth,
	    col);
	gpBufEnd += BUFFER_WIDTH;
}

/**
 * @brief Blit CL2 sprite, and apply a given lighting, to the back buffer at the given coordianates
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff CL2 buffer
 * @param nCel CL2 frame number
 * @param nWidth Width of sprite
 * @param light Light shade to use
 */
void Cl2DrawLightTbl(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth, char light)
{
	int nDataSize;
	BYTE *pRLEBytes, *pDecodeTo, *tbl;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);
	assert(nCel > 0);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);
	pDecodeTo = &gpBuffer[sx + BUFFER_WIDTH * sy];
	tbl = &pLightTbl[lightidx(light)];

	Cl2BlitLightSafe(
	    pDecodeTo,
	    pRLEBytes,
	    nDataSize,
	    nWidth,
	    tbl);
}

/**
 * @brief Blit CL2 sprite, and apply lighting, to the back buffer at the given coordinates
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff CL2 buffer
 * @param nCel CL2 frame number
 * @param nWidth Width of sprite
 */
void Cl2DrawLight(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth)
{
	int nDataSize;
	BYTE *pRLEBytes, *pDecodeTo;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);
	assert(nCel > 0);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);
	pDecodeTo = &gpBuffer[sx + BUFFER_WIDTH * sy];

	if (light_table_index)
		Cl2BlitLightSafe(pDecodeTo, pRLEBytes, nDataSize, nWidth, &pLightTbl[light_table_index * 256]);
	else
		Cl2BlitSafe(pDecodeTo, pRLEBytes, nDataSize, nWidth);
}

BYTE* CelMerge(BYTE* celA, int nDataSizeA, BYTE* celB, int nDataSizeB)
{
	DWORD nDataSize, i, nCelA, nCelB, cData, nData;
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
		*pHead = SDL_SwapLE32(pBuf - cel);
		memcpy(pBuf, &celA[cData], nData - cData);
		pBuf += nData - cData;
		++*cel;
		pHead++;
	}

	nData = LOAD_LE32(celB + 4);
	for (i = 1; i <= nCelB; i++) {
		cData = nData;
		nData = LOAD_LE32(celB + 4 * (i + 1));
		*pHead = SDL_SwapLE32(pBuf - cel);
		memcpy(pBuf, &celB[cData], nData - cData);
		pBuf += nData - cData;
		++*cel;
		pHead++;
	}

	*pHead = SDL_SwapLE32(pBuf - cel);
	// assert(*pHead == nDataSize);
	return cel;
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
	scrollrt_draw_game_screen(TRUE);
	PaletteFadeIn();
	gbRedrawFlags = REDRAW_ALL;
}

DEVILUTION_END_NAMESPACE
