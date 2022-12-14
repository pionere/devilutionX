/**
 * @file cel_render.cpp
 *
 * CEL rendering.
 */
#include "cel_render.hpp"

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/**
 * @brief Blit CEL sprite to the given buffer, checks for drawing outside the buffer
 * @param pDecodeTo The output buffer
 * @param pRLEBytes CEL pixel stream (run-length encoded)
 * @param nDataSize Size of CEL in bytes
 * @param nWidth Width of sprite
 */
static void CelBlit(BYTE* pDecodeTo, const BYTE* pRLEBytes, int nDataSize, int nWidth)
{
	int i;
	int8_t width;
	const BYTE *src, *end;
	BYTE* dst;

	assert(pDecodeTo != NULL);
	assert(pRLEBytes != NULL);
	assert(gpBuffer != NULL);

	src = pRLEBytes;
	end = &pRLEBytes[nDataSize];
	dst = pDecodeTo;

	for ( ; src != end; dst -= BUFFER_WIDTH + nWidth) {
		for (i = nWidth; i != 0; ) {
			width = *src++;
			if (width >= 0) {
				i -= width;
				if (dst < gpBufEnd && dst >= gpBufStart) {
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
 * @brief apply lighting to the CEL sprite and blit to the given buffer
 * @param pDecodeTo The output buffer
 * @param pRLEBytes CEL pixel stream (run-length encoded)
 * @param nDataSize Size of CEL in bytes
 * @param nWidth Width of sprite
 * @param tbl Palette translation table
 */
static void CelBlitLight(BYTE* pDecodeTo, const BYTE* pRLEBytes, int nDataSize, int nWidth, const BYTE* tbl)
{
	int i;
	int8_t width;
	const BYTE *src, *end;
	BYTE* dst;

	assert(pDecodeTo != NULL);
	assert(pRLEBytes != NULL);
	assert(gpBuffer != NULL);

	src = pRLEBytes;
	end = &pRLEBytes[nDataSize];
	dst = pDecodeTo;
	//assert(tbl != NULL);
	//if (tbl == NULL)
	//	tbl = ColorTrns[light_trn_index];

	for ( ; src != end; dst -= BUFFER_WIDTH + nWidth) {
		for (i = nWidth; i != 0; ) {
			width = *src++;
			if (width >= 0) {
				i -= width;
				if (dst < gpBufEnd && dst >= gpBufStart) {
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
					for ( ; width != 0; width--) {
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
				dst -= width;
				i += width;
			}
		}
	}
}

/**
 * @brief Blit CEL sprite to the back buffer at the given coordinates
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 */
void CelDraw(int sx, int sy, const CelImageBuf* pCelBuff, int nCel)
{
	int nDataSize;
	const BYTE* pRLEBytes;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);

	pRLEBytes = CelGetFrame((const BYTE*)pCelBuff, nCel, &nDataSize);

	CelBlit(&gpBuffer[sx + BUFFER_WIDTH * sy], pRLEBytes, nDataSize, pCelBuff->ciWidth);
}

/**
 * @brief Apply lighting to the CEL sprite and blit to the back buffer at the given coordinates
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 * @param tbl Palette translation table
 */
void CelDrawLight(int sx, int sy, const CelImageBuf* pCelBuff, int nCel, const BYTE* tbl)
{
	int nDataSize;
	BYTE* pDecodeTo;
	const BYTE* pRLEBytes;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);

	pRLEBytes = CelGetFrame((const BYTE*)pCelBuff, nCel, &nDataSize);
	pDecodeTo = &gpBuffer[sx + BUFFER_WIDTH * sy];

	/*if (tbl == NULL) {
		if (light_trn_index == 0) {
			CelBlit(pDecodeTo, pRLEBytes, nDataSize, nWidth);
			return;
		}
		tbl = ColorTrns[light_trn_index];
	}*/
	CelBlitLight(pDecodeTo, pRLEBytes, nDataSize, pCelBuff->ciWidth, tbl);
}

/**
 * @brief Same as CelBlitLight with stippled transparency applied
 * @param pDecodeTo The output buffer
 * @param pRLEBytes CEL pixel stream (run-length encoded)
 * @param nDataSize Size of CEL in bytes
 * @param nWidth Width of sprite
 */
static void CelBlitLightTrans(BYTE* pDecodeTo, const BYTE* pRLEBytes, int nDataSize, int nWidth)
{
	int i;
	BOOLEAN shift;
	int8_t width;
	const BYTE *tbl, *src, *end;
	BYTE* dst;

	assert(pDecodeTo != NULL);
	assert(pRLEBytes != NULL);
	assert(gpBuffer != NULL);


	src = pRLEBytes;
	end = &pRLEBytes[nDataSize];
	dst = pDecodeTo;
	tbl = ColorTrns[light_trn_index];
	shift = (BYTE)(size_t)dst & 1;

	for ( ; src != end; dst -= BUFFER_WIDTH + nWidth, shift = 1 - shift) {
		for (i = nWidth; i != 0; ) {
			width = *src++;
			if (width >= 0) {
				i -= width;
				if (dst < gpBufEnd && dst >= gpBufStart) {
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
							for ( ; width != 0; width--) {
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
							for ( ; width != 0; width--) {
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
				dst -= width;
				i += width;
			}
		}
	}
}

/**
 * @brief Same as CelClippedDrawLight optionally drawing in stippled-transparent mode
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 * @param nWidth Width of sprite
 */
void CelClippedDrawLightTrans(int sx, int sy, const BYTE* pCelBuff, int nCel, int nWidth)
{
	int nDataSize;
	const BYTE* pRLEBytes;
	BYTE* pDecodeTo;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);
	pDecodeTo = &gpBuffer[sx + BUFFER_WIDTH * sy];

	if (gbCelTransparencyActive)
		CelBlitLightTrans(pDecodeTo, pRLEBytes, nDataSize, nWidth);
	else if (light_trn_index != 0)
		CelBlitLight(pDecodeTo, pRLEBytes, nDataSize, nWidth, ColorTrns[light_trn_index]);
	else
		CelBlit(pDecodeTo, pRLEBytes, nDataSize, nWidth);
}

/**
 * @brief Blit CL2 sprite, and apply a given lighting/trn, to the given buffer at the given coordinates
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 * @param nWidth Width of sprite
 * @param light index of the light shade/trn to use
 */
void CelClippedDrawLightTbl(int sx, int sy, const BYTE* pCelBuff, int nCel, int nWidth, BYTE light)
{
	int nDataSize;
	const BYTE* pRLEBytes;
	BYTE* pDecodeTo;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);
	pDecodeTo = &gpBuffer[sx + BUFFER_WIDTH * sy];

	if (light != 0)
		CelBlitLight(pDecodeTo, pRLEBytes, nDataSize, nWidth, ColorTrns[light]);
	else
		CelBlit(pDecodeTo, pRLEBytes, nDataSize, nWidth);
}

/**
 * @brief Blit to a buffer at given coordinates
 * @param pBuff Target buffer
 * @param x Cordinate in pBuff buffer
 * @param y Cordinate in pBuff buffer
 * @param wdt Width of pBuff
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 * @param nWidth Width of sprite
 */
/*static void CelBlitWidth(BYTE* pBuff, int x, int y, int wdt, BYTE* pCelBuff, int nCel, int nWidth)
{
	BYTE *pRLEBytes, *dst, *end;

	assert(pCelBuff != NULL);
	assert(pBuff != NULL);

	int i, nDataSize;
	int8_t width;

	pRLEBytes = CelGetFrame(pCelBuff, nCel, &nDataSize);
	end = &pRLEBytes[nDataSize];
	dst = &pBuff[y * wdt + x];

	for ( ; pRLEBytes != end; dst -= wdt + nWidth) {
		for (i = nWidth; i != 0; ) {
			width = *pRLEBytes++;
			if (width >= 0) {
				i -= width;
				memcpy(dst, pRLEBytes, width);
				dst += width;
				pRLEBytes += width;
			} else {
				dst -= width;
				i += width;
			}
		}
	}
}*/

/**
 * @brief Blit an outline one pixel larger then the given sprite shape to the target buffer at the given coordinates
 * @param col Color index from current palette
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff CEL buffer
 * @param nCel CEL frame number
 * @param nWidth Width of sprite
 */
void CelClippedDrawOutline(BYTE col, int sx, int sy, const BYTE* pCelBuff, int nCel, int nWidth)
{
	int nDataSize, i;
	const BYTE *src, *end;
	BYTE* dst;
	int8_t width;

	assert(pCelBuff != NULL);
	assert(gpBuffer != NULL);

	src = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);
	end = &src[nDataSize];
	dst = &gpBuffer[sx + BUFFER_WIDTH * sy];

	for ( ; src != end; dst -= BUFFER_WIDTH + nWidth) {
		for (i = nWidth; i != 0; ) {
			width = *src++;
			if (width >= 0) {
				i -= width;
				if (dst < gpBufEnd && dst >= gpBufStart) {
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
				dst -= width;
				i += width;
			}
		}
	}
}

DEVILUTION_END_NAMESPACE
